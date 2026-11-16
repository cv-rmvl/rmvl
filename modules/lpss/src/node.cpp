/**
 * @file node.cpp
 * @author zhaoxi (535394140@qq.com)
 * @brief 轻量级发布订阅服务：节点实现
 * @version 1.0
 * @date 2025-11-04
 *
 * @copyright Copyright 2025 (c), zhaoxi
 *
 */

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#else
#include <arpa/inet.h>
#endif

#include <algorithm>

#include "rmvl/lpss/node.hpp"
#include "rmvlpara/lpss.hpp"

using namespace std::chrono_literals;

namespace rm {

namespace lpss {

std::pair<Guid, std::vector<ip::Networkv4>> generateNodeGuid() {
    Guid res{};
    auto interfaces = NetworkInterface::list();
    std::vector<NetworkInterface> candidate_interfaces;

    for (const auto &iface : interfaces)
        if (iface.up() && iface.running() && !iface.loopback() && iface.multicast())
            candidate_interfaces.push_back(iface);

    // 按类型优先级和名称排序
    std::sort(candidate_interfaces.begin(), candidate_interfaces.end(), [](const NetworkInterface &a, const NetworkInterface &b) {
        return a.type() != b.type() ? a.type() < b.type() : a.name() < b.name();
    });

    std::vector<ip::Networkv4> networks{};
    networks.reserve(candidate_interfaces.size());
    if (!candidate_interfaces.empty()) {
        // 选取最优接口为 GUID MAC 和 basic_mac
        const auto &primary_iface = candidate_interfaces.front();
        auto basic_mac = primary_iface.address();
        res.set_host((static_cast<uint32_t>(basic_mac[2]) << 24) | (static_cast<uint32_t>(basic_mac[3]) << 16) |
                     (static_cast<uint32_t>(basic_mac[4]) << 8) | static_cast<uint32_t>(basic_mac[5]));
        res.set_pid(static_cast<uint16_t>(processId()));
        // 收集 IP
        for (const auto &iface : candidate_interfaces) {
            auto nets = iface.ipv4();
            networks.insert(networks.end(), std::make_move_iterator(nets.begin()), std::make_move_iterator(nets.end()));
        }
    } else {
        WARNING_("[LPSS Node] No valid network interface found, using 00:00:00:00:00:00 as MAC address");
        res.set_pid(static_cast<uint16_t>(processId()));
    }
    return {res, networks};
}

Node::Node(std::string_view name, uint8_t domain) : _rndp_port(7500 + domain), _rndp_writer(Sender(ip::udp::v4()).create()) {
    static_assert(sizeof(Locator) == 6, "Locator size must be 6 bytes");

    std::vector<ip::Networkv4> networks{};
    std::tie(_uid, networks) = generateNodeGuid();

    // 创建监听 Socket
    auto rndp_reader = Listener(Endpoint(ip::udp::v4(), _rndp_port)).create();
    auto redp_socket = std::make_shared<DgramSocket>(Listener(Endpoint(ip::udp::v4(), Endpoint::ANY_PORT)).create());
    _redp_port = redp_socket->endpoint().port();
    _redp_socket = redp_socket;
    // 关闭 REDP 控制 Socket 的地址复用
#ifdef _WIN32
    const char no_reuse = 0;
#else
    const int no_reuse = 0;
#endif
    if (::setsockopt(redp_socket->native_handle(), SOL_SOCKET, SO_REUSEADDR,
                     reinterpret_cast<const char *>(&no_reuse), sizeof(no_reuse)) != 0)
        WARNING_("[LPSS Node] Failed to disable SO_REUSEADDR on REDP control socket");

    // 设置 Socket 选项
    _rndp_writer.setOption(ip::multicast::Loopback(true));
    rndp_reader.setOption(ip::multicast::JoinGroup(BROADCAST_IP));

    // 启动广播线程
    _bcast_thrd = std::thread(&Node::rndp_multicast, this, std::move(networks), std::string(name));

    // 启动监听线程
    _rndp_listen_thrd = std::thread(&Node::rndp_listener, this, std::move(rndp_reader));

    // 启动 REDP 监听线程
    _redp_listen_thrd = std::thread(&Node::redp_listener, this, redp_socket);

    // 启动心跳检测线程
    _hbt_detect_thrd = std::thread(&Node::heartbeat_detect, this);
}

void Node::shutdown() noexcept {
    // 准备本地端点下线的 REDP 快照
    std::vector<std::pair<Locator, REDPMessage>> final_snapshots{};
    {
        std::lock_guard lock(_redp_mtx);
        for (auto &[peer, session] : _redp_peers) {
            if (session.sender.waiting() || session.sender.dirty())
                continue;
            session.sender.markDirty();
            if (auto packet = session.sender.request({}))
                final_snapshots.emplace_back(session.locator, std::move(*packet));
        }
    }
    // 向已发现节点单播 REDP 快照
    for (const auto &[loc, packet] : final_snapshots)
        sendREDPMessage(*_redp_socket, loc, packet);
    // 移除本地注册的 Writers/Readers
    {
        std::lock_guard lk(_local_mtx);
        _local_writers.clear();
        _local_readers.clear();
    }

    constexpr const char wake_msg[] = "Done";

    // 停止所有线程
    _running.store(false, std::memory_order_release);
    if (_bcast_thrd.joinable())
        _bcast_thrd.join();
    if (_rndp_listen_thrd.joinable()) {
        _rndp_writer.write("127.0.0.1", Endpoint(ip::udp::v4(), _rndp_port), wake_msg);
        _rndp_listen_thrd.join();
    }
    if (_redp_listen_thrd.joinable()) {
        _rndp_writer.write("127.0.0.1", Endpoint(ip::udp::v4(), _redp_port), wake_msg);
        _redp_listen_thrd.join();
    }
    if (_hbt_detect_thrd.joinable()) {
        _hbt_cv.notify_one();
        _hbt_detect_thrd.join();
    }
}

void Node::rndp_multicast(std::vector<ip::Networkv4> networks, std::string node_name) {
    // 准备 RNDP 消息
    const auto rndp_msg = [this, &networks, name = std::move(node_name)]() {
        uint8_t locator_num = static_cast<uint8_t>(networks.size() + 1);
        RNDPMessage res;
        res.guid = _uid;
        res.heartbeat_timeout = para::lpss_param.MAX_NODE_HEARTBEAT_PERIOD;
        res.name = name;
        res.locators.reserve(locator_num);
        for (auto network : networks)
            res.locators.push_back({_redp_port, network.address()});
        return res;
    }();

    // 序列化 RNDP 消息
    auto rndp_msg_data = rndp_msg.serialize();

    // 广播 RNDP 消息
    while (_running.load(std::memory_order_acquire)) {
        std::this_thread::sleep_for(25ms);

        // 为 Outbound 设置不同的接口地址，并分别发送消息
        for (const auto &network : networks) {
            _rndp_writer.setOption(ip::multicast::Interface(network.address()));
            _rndp_writer.write(BROADCAST_IP, Endpoint(ip::udp::v4(), _rndp_port), rndp_msg_data);
        }
    }
}

void Node::rndp_listener(DgramSocket rndp_reader) {
    while (_running.load(std::memory_order_acquire)) {
        // 接收原始 UDP 报文
        auto [data, addr_str, port] = rndp_reader.read();
        // 解析 RNDP 消息
        auto ndp_msg = RNDPMessage::deserialize(data);
        if (!ndp_msg || ndp_msg->guid == _uid)
            continue;
        std::array<uint8_t, 4> addr{};
        if (inet_pton(AF_INET, addr_str.c_str(), addr.data()) != 1)
            continue;
        // 选择最佳控制平面定位器
        const auto ctrl_loc = selectBestLocator(addr, *ndp_msg);
        if (ctrl_loc.invalid())
            continue;
        const auto selected = negotiate(CURRENT_EDP_VERSION, CURRENT_MTP_VERSION,
                                        ndp_msg->edp_version, ndp_msg->mtp_version);

        std::optional<REDPMessage> first{};
        {
            // 与心跳超时检测/清理串行化，避免旧超时列表删除刚刷新心跳的节点。
            // 同时保证节点记录刷新与 REDP 会话初始化作为一次状态迁移完成。
            std::lock_guard registration_lock(_endpoint_registration_mtx);
            {
                std::lock_guard lock(_nodes_mtx);
                const auto now = std::chrono::steady_clock::now();
                auto &node = _discovered_nodes[ndp_msg->guid];
                node.rndp_msg = *ndp_msg;
                node.last_alive = now;
                node.ctrl_loc = ctrl_loc;
                node.selected = selected;
            }
            // 建立 REDP 会话并同步本地 Writers/Readers 快照
            first = activateREDPPeer(ndp_msg->guid, ctrl_loc);
        }
        if (first)
            sendREDPMessage(*_redp_socket, ctrl_loc, *first);
    }
}

bool Node::canRegisterREDPEndpoints(
    std::initializer_list<std::pair<std::string_view, std::string_view>> additions) {
    size_t wire_size = 17;
    {
        std::shared_lock lock(_local_mtx);
        for (const auto &[topic, writer] : _local_writers)
            if (!countREDPEndpointSize(wire_size, topic, writer->msgtype()))
                return false;
        for (const auto &[topic, reader] : _local_readers)
            if (!countREDPEndpointSize(wire_size, topic, reader->msgtype()))
                return false;
    }
    for (const auto &[topic, type] : additions)
        if (!countREDPEndpointSize(wire_size, topic, type))
            return false;
    return true;
}

std::vector<REDPEndpoint> Node::snapshot() {
    std::shared_lock lock(_local_mtx);
    std::vector<REDPEndpoint> endpoints{};
    endpoints.reserve(_local_writers.size() + _local_readers.size());
    for (const auto &[topic, writer] : _local_writers)
        endpoints.push_back({writer->guid(), 0, topic, std::string(writer->msgtype())});
    for (const auto &[topic, reader] : _local_readers)
        endpoints.push_back({reader->guid(), reader->port(), topic, std::string(reader->msgtype())});
    return endpoints;
}

void Node::endpointChanged() {
    const auto current = snapshot();
    std::vector<std::pair<Locator, REDPMessage>> outbound{};
    {
        std::lock_guard lock(_redp_mtx);
        for (auto &[peer, session] : _redp_peers) {
            session.sender.markDirty();
            if (session.online && !session.sender.waiting())
                if (auto message = session.sender.request(current))
                    outbound.emplace_back(session.locator, std::move(*message));
        }
    }
    for (const auto &[loc, message] : outbound)
        sendREDPMessage(*_redp_socket, loc, message);
}

std::optional<REDPMessage> Node::activateREDPPeer(Guid peer, Locator locator) {
    {
        std::lock_guard lock(_redp_mtx);
        const auto it = _redp_peers.find(peer);
        if (it != _redp_peers.end() && it->second.online &&
            it->second.locator.port == locator.port &&
            it->second.locator.addr == locator.addr)
            return {};
    }

    std::optional<REDPMessage> first{};
    std::vector<REDPEndpoint> remote_snapshot{};
    {
        std::lock_guard peer_lock(_redp_mtx);
        auto it = _redp_peers.find(peer);
        if (it != _redp_peers.end()) {
            auto &session = it->second;
            const bool resumed = !session.online;
            const bool locator_changed = session.locator.port != locator.port ||
                                         session.locator.addr != locator.addr;
            session.locator = locator;
            session.online = true;
            if (resumed)
                remote_snapshot = session.receiver.snapshot();
            if (resumed || locator_changed) {
                if (!session.sender.waiting() && !session.sender.dirty())
                    session.sender.markDirty();
                first = session.sender.request(snapshot());
            }
        } else {
            auto &session = _redp_peers.try_emplace(peer, _uid, peer, locator).first->second;
            session.locator = locator;
            session.online = true;
            session.sender.markDirty();
            first = session.sender.request(snapshot());
        }
    }
    if (!remote_snapshot.empty())
        applyREDPSnapshot(peer, locator.addr, {}, remote_snapshot);
    return first;
}

void Node::applyREDPSnapshot(Guid peer, std::array<uint8_t, 4> source_ip,
                             const std::vector<REDPEndpoint> &previous,
                             const std::vector<REDPEndpoint> &current) {
    // 应用快照期间防止节点被心跳线程清理
    std::shared_lock node_lock(_nodes_mtx);
    const auto node = _discovered_nodes.find(peer);
    if (node == _discovered_nodes.end())
        return;
    const bool mt02_compatible = node->second.selected.mtp >= 2;
    const auto belongs = [peer](const REDPEndpoint &ep) {
        return ep.guid.full != 0 && ep.guid.host() == peer.host() && ep.guid.pid() == peer.pid();
    };
    // 比较新旧快照中的端点变化
    std::vector<REDPEndpoint> removed{}, added{};
    for (const auto &old_ep : previous)
        if (belongs(old_ep) && std::none_of(current.begin(), current.end(),
                                            [&](const auto &ep) { return sameREDPEndpoint(old_ep, ep); }))
            removed.push_back(old_ep);
    for (const auto &new_ep : current)
        if (belongs(new_ep) && std::none_of(previous.begin(), previous.end(),
                                            [&](const auto &ep) { return sameREDPEndpoint(new_ep, ep); }))
            added.push_back(new_ep);

    // 移除已下线的 Writers/Readers 并更新本地缓存
    for (const auto &ep : removed) {
        if (ep.port == 0) {
            {
                std::lock_guard lock(_discovered_mtx);
                auto it = _discovered_writers.find(ep.topic);
                if (it != _discovered_writers.end()) {
                    it->second.writers.erase(ep.guid);
                    if (it->second.writers.empty())
                        _discovered_writers.erase(it);
                }
            }
            std::lock_guard lock(_local_mtx);
            auto it = _local_readers.find(ep.topic);
            if (it != _local_readers.end())
                it->second->remove(ep.guid);
        } else {
            {
                std::lock_guard lock(_discovered_mtx);
                auto it = _discovered_readers.find(ep.topic);
                if (it != _discovered_readers.end()) {
                    it->second.readers.erase(ep.guid);
                    if (it->second.readers.empty())
                        _discovered_readers.erase(it);
                }
            }
            std::lock_guard lock(_local_mtx);
            auto it = _local_writers.find(ep.topic);
            if (it != _local_writers.end())
                it->second->remove(ep.guid);
        }
    }

    // 注册新增的 Writers/Readers 并更新本地缓存
    if (!mt02_compatible)
        return;
    for (const auto &ep : added) {
        if (ep.port == 0) {
            {
                std::lock_guard lock(_discovered_mtx);
                auto &storage = _discovered_writers[ep.topic];
                if (storage.msgtype.empty())
                    storage.msgtype = ep.msgtype;
                if (storage.msgtype != ep.msgtype)
                    RMVL_Error_(RMVL_StsError, "[LPSS Node] Inconsistent msgtype for topic '%s'",
                                ep.topic.c_str());
                storage.writers.insert(ep.guid);
            }
            std::lock_guard lock(_local_mtx);
            auto it = _local_readers.find(ep.topic);
            if (mt02_compatible && it != _local_readers.end())
                it->second->add(ep.guid);
        } else {
            {
                std::lock_guard lock(_discovered_mtx);
                auto &storage = _discovered_readers[ep.topic];
                if (storage.msgtype.empty())
                    storage.msgtype = ep.msgtype;
                if (storage.msgtype != ep.msgtype)
                    RMVL_Error_(RMVL_StsError, "[LPSS Node] Inconsistent msgtype for topic '%s'",
                                ep.topic.c_str());
                storage.readers[ep.guid] = {ep.port, source_ip};
            }
            std::lock_guard lock(_local_mtx);
            auto it = _local_writers.find(ep.topic);
            if (mt02_compatible && it != _local_writers.end())
                it->second->add(ep.guid, {ep.port, source_ip});
        }
    }
}

void Node::redp_listener(std::shared_ptr<DgramSocket> redp_socket) {
    while (_running.load(std::memory_order_acquire)) {
        auto [redp_msg, addr_str, port] = redp_socket->read();
        // 解析 REDP 消息
        auto msg = REDPMessage::deserialize(redp_msg);
        if (!msg)
            continue;
        std::array<uint8_t, 4> addr{};
        if (inet_pton(AF_INET, addr_str.c_str(), addr.data()) != 1)
            continue;
        std::unique_lock registration_lock(_endpoint_registration_mtx);
        {
            // 校验报文是否来自 RNDP 公告的控制端口
            std::shared_lock lock(_nodes_mtx);
            auto it = _discovered_nodes.find(msg->source);
            if (it == _discovered_nodes.end() ||
                it->second.ctrl_loc.addr != addr ||
                it->second.ctrl_loc.port != port)
                continue;
        }
        if (!msg->ack && !snapshotBelongsToPeer(msg->source, msg->snapshot))
            continue;

        std::optional<REDPMessage> response{}, next{};
        std::vector<REDPEndpoint> previous{}, current{};
        bool applied{};
        Locator locator{};
        {
            std::lock_guard lock(_redp_mtx);
            auto it = _redp_peers.find(msg->source);
            if (it == _redp_peers.end())
                continue;
            auto &session = it->second;
            locator = session.locator;
            if (msg->ack) {
                const auto local = snapshot();
                if (session.sender.confirm(*msg, local))
                    next = session.sender.request(local);
            } else {
                previous = session.receiver.snapshot();
                auto received = session.receiver.accept(*msg, _uid);
                if (received) {
                    response = received->ack;
                    applied = received->applied;
                    current = session.receiver.snapshot();
                }
            }
        }
        if (applied)
            applyREDPSnapshot(msg->source, addr, previous, current);
        registration_lock.unlock();
        if (response)
            sendREDPMessage(*_redp_socket, locator, *response);
        if (next)
            sendREDPMessage(*_redp_socket, locator, *next);
    }
}

static bool is_same_node(const Guid &lhs, const Guid &rhs) { return lhs.host() == rhs.host() && lhs.pid() == rhs.pid(); }

void Node::heartbeat_detect() {
    while (_running.load(std::memory_order_acquire)) {
        std::unique_lock registration_lock(_endpoint_registration_mtx);
        // 检测心跳超时的节点
        std::vector<Guid> timeout_guids{};
        auto now = std::chrono::steady_clock::now();
        {
            std::shared_lock lk(_nodes_mtx);
            for (const auto &[guid, data] : _discovered_nodes)
                if (now - data.last_alive > std::chrono::milliseconds(data.rndp_msg.heartbeat_timeout * 1000))
                    timeout_guids.push_back(guid);
        }
        if (!timeout_guids.empty()) {
            std::lock_guard lk(_nodes_mtx);
            for (const auto &guid : timeout_guids)
                _discovered_nodes.erase(guid);
        }
        // 移除超时节点关联的端点缓存，遍历期间安全擦除元素
        if (!timeout_guids.empty()) {
            std::lock_guard lk(_discovered_mtx);
            for (const auto &dead_guid : timeout_guids) {
                // 清理 Writers
                for (auto it = _discovered_writers.begin(); it != _discovered_writers.end();) {
                    auto &writer_storage = it->second;
                    for (auto set_it = writer_storage.writers.begin(); set_it != writer_storage.writers.end();)
                        is_same_node(*set_it, dead_guid) ? set_it = writer_storage.writers.erase(set_it) : ++set_it;
                    writer_storage.writers.empty() ? it = _discovered_writers.erase(it) : ++it;
                }
                // 清理 Readers
                for (auto it = _discovered_readers.begin(); it != _discovered_readers.end();) {
                    auto &reader_storage = it->second;
                    for (auto map_it = reader_storage.readers.begin(); map_it != reader_storage.readers.end();)
                        is_same_node(map_it->first, dead_guid) ? map_it = reader_storage.readers.erase(map_it) : ++map_it;
                    reader_storage.readers.empty() ? it = _discovered_readers.erase(it) : ++it;
                }
            }
        }
        if (!timeout_guids.empty()) {
            std::lock_guard lk(_local_mtx);
            for (const auto &dead_guid : timeout_guids) {
                for (const auto &[topic, writer] : _local_writers)
                    writer->remove(dead_guid);
                for (const auto &[topic, reader] : _local_readers)
                    reader->remove(dead_guid);
            }
        }

        const auto local = snapshot();
        std::vector<std::pair<Locator, REDPMessage>> retries{};
        {
            std::lock_guard lock(_redp_mtx);
            for (const auto &dead : timeout_guids) {
                auto it = _redp_peers.find(dead);
                if (it != _redp_peers.end())
                    it->second.online = false;
            }
            for (auto &[peer, session] : _redp_peers)
                if (session.online)
                    if (auto packet = session.sender.request(local))
                        retries.emplace_back(session.locator, std::move(*packet));
        }
        registration_lock.unlock();
        for (const auto &[loc, packet] : retries)
            sendREDPMessage(*_redp_socket, loc, packet);

        std::unique_lock lk(_hbt_mtx);
        _hbt_cv.wait_for(lk, 500ms);
    }
}

msg::Time now() noexcept {
    auto now = std::chrono::system_clock::now().time_since_epoch();
    return msg::Time{static_cast<int32_t>(std::chrono::duration_cast<std::chrono::seconds>(now).count()),
                     static_cast<uint32_t>(std::chrono::duration_cast<std::chrono::nanoseconds>(now).count() % 1'000'000'000ll)};
}

} // namespace lpss

namespace msg {

Time operator+(Time t1, Time t2) noexcept {
    int32_t sec_sum = t1.sec + t2.sec;
    uint32_t nsec_sum = t1.nsec + t2.nsec;
    if (nsec_sum >= 1'000'000'000) {
        sec_sum += 1;
        nsec_sum -= 1'000'000'000;
    }
    return {sec_sum, nsec_sum};
}

Time operator-(Time t1, Time t2) noexcept {
    int32_t sec_diff = t1.sec - t2.sec;
    int64_t nsec_diff = static_cast<int64_t>(t1.nsec) - static_cast<int64_t>(t2.nsec);
    if (nsec_diff < 0) {
        sec_diff -= 1;
        nsec_diff += 1'000'000'000;
    }
    return {sec_diff, static_cast<uint32_t>(nsec_diff)};
}

} // namespace msg

} // namespace rm

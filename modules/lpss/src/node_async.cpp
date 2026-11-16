/**
 * @file node_async.cpp
 * @author zhaoxi (535394140@qq.com)
 * @brief 轻量级发布订阅服务：节点实现（基于异步协程）
 * @version 1.0
 * @date 2026-01-25
 *
 * @copyright Copyright 2026 (c), zhaoxi
 *
 */

#if __cplusplus >= 202002L

#include <algorithm>
#include <csignal>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#else
#include <arpa/inet.h>
#endif

#include "rmvl/lpss/node.hpp"

#include "rmvlpara/lpss.hpp"

using namespace std::chrono_literals;

namespace rm::lpss {

namespace async {

rm::async::Task<> Node::rndp_multicast(std::vector<ip::Networkv4> networks, std::string node_name) {
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

    while (_running) {
        co_await _broadcast_timer.sleep_for(25ms);

        // 为 Outbound 设置不同的接口地址，并分别发送消息
        for (const auto &network : networks) {
            _rndp_writer.setOption(ip::multicast::Interface(network.address()));
            co_await _rndp_writer.write(BROADCAST_IP, Endpoint(ip::udp::v4(), _rndp_port), rndp_msg_data);
        }
    }
}

rm::async::Task<> Node::rndp_listener(rm::async::DgramSocket rndp_reader) {
    while (_running) {
        // 接收原始 UDP 报文
        auto [data, addr_str, port] = co_await rndp_reader.read();
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

        // 处理发现的节点信息
        const auto now = std::chrono::steady_clock::now();
        auto &node = _discovered_nodes[ndp_msg->guid];
        node.rndp_msg = *ndp_msg;
        node.last_alive = now;
        node.ctrl_loc = ctrl_loc;
        node.selected = selected;
        // 建立 REDP 会话并同步本地 Writers/Readers 快照
        activateREDPPeer(ndp_msg->guid, ctrl_loc);
    }
}

bool Node::canRegisterREDPEndpoints(
    std::initializer_list<std::pair<std::string_view, std::string_view>> additions) {
    size_t wire_size = 17;
    for (const auto &[topic, writer] : _local_writers)
        if (!countREDPEndpointSize(wire_size, topic, writer->msgtype()))
            return false;
    for (const auto &[topic, reader] : _local_readers)
        if (!countREDPEndpointSize(wire_size, topic, reader->msgtype()))
            return false;
    for (const auto &[topic, type] : additions)
        if (!countREDPEndpointSize(wire_size, topic, type))
            return false;
    return true;
}

std::vector<REDPEndpoint> Node::snapshot() {
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
    for (auto &[peer, session] : _redp_peers) {
        session.sender.markDirty();
        if (session.online && !session.sender.waiting())
            if (auto packet = session.sender.request(current))
                outbound.emplace_back(session.locator, std::move(*packet));
    }
    for (const auto &[loc, packet] : outbound)
        sendREDPMessage(*_redp_socket, loc, packet);
}

void Node::activateREDPPeer(Guid peer, Locator locator) {
    auto it = _redp_peers.find(peer);
    if (it != _redp_peers.end() && it->second.online &&
        it->second.locator.port == locator.port &&
        it->second.locator.addr == locator.addr)
        return;

    std::optional<REDPMessage> first{};
    std::vector<REDPEndpoint> remote_snapshot{};
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
            // 保留未确认报文，仅在空闲时重新同步当前快照。
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
    if (!remote_snapshot.empty())
        applyREDPSnapshot(peer, locator.addr, {}, remote_snapshot);
    if (first)
        sendREDPMessage(*_redp_socket, locator, *first);
}

void Node::applyREDPSnapshot(Guid peer, std::array<uint8_t, 4> source_ip,
                             const std::vector<REDPEndpoint> &previous,
                             const std::vector<REDPEndpoint> &current) {
    const auto peer_info = _discovered_nodes.find(peer);
    const bool mt02_compatible = peer_info != _discovered_nodes.end() &&
                                 peer_info->second.selected.mtp >= 2;
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
            auto it = _discovered_writers.find(ep.topic);
            if (it != _discovered_writers.end()) {
                it->second.writers.erase(ep.guid);
                if (it->second.writers.empty())
                    _discovered_writers.erase(it);
            }
            auto reader = _local_readers.find(ep.topic);
            if (reader != _local_readers.end())
                reader->second->remove(ep.guid);
        } else {
            auto it = _discovered_readers.find(ep.topic);
            if (it != _discovered_readers.end()) {
                it->second.readers.erase(ep.guid);
                if (it->second.readers.empty())
                    _discovered_readers.erase(it);
            }
            auto writer = _local_writers.find(ep.topic);
            if (writer != _local_writers.end())
                writer->second->remove(ep.guid);
        }
    }

    // 注册新增的 Writers/Readers 并更新本地缓存
    if (!mt02_compatible)
        return;
    for (const auto &ep : added) {
        if (ep.port == 0) {
            auto &storage = _discovered_writers[ep.topic];
            if (storage.msgtype.empty())
                storage.msgtype = ep.msgtype;
            if (storage.msgtype != ep.msgtype)
                RMVL_Error_(RMVL_StsError, "[LPSS Node] Inconsistent msgtype for topic '%s'",
                            ep.topic.c_str());
            storage.writers.insert(ep.guid);
            auto reader = _local_readers.find(ep.topic);
            if (mt02_compatible && reader != _local_readers.end())
                reader->second->add(ep.guid);
        } else {
            auto &storage = _discovered_readers[ep.topic];
            if (storage.msgtype.empty())
                storage.msgtype = ep.msgtype;
            if (storage.msgtype != ep.msgtype)
                RMVL_Error_(RMVL_StsError, "[LPSS Node] Inconsistent msgtype for topic '%s'",
                            ep.topic.c_str());
            storage.readers[ep.guid] = {ep.port, source_ip};
            auto writer = _local_writers.find(ep.topic);
            if (mt02_compatible && writer != _local_writers.end())
                writer->second->add(ep.guid, {ep.port, source_ip});
        }
    }
}

rm::async::Task<> Node::redp_listener(std::shared_ptr<rm::async::DgramSocket> redp_socket) {
    while (_running) {
        auto [redp_msg, addr_str, port] = co_await redp_socket->read();
        // 解析 REDP 消息
        auto msg = REDPMessage::deserialize(redp_msg);
        if (!msg)
            continue;
        std::array<uint8_t, 4> addr{};
        if (inet_pton(AF_INET, addr_str.c_str(), addr.data()) != 1)
            continue;
        auto it = _discovered_nodes.find(msg->source);
        // 校验报文是否来自 RNDP 公告的控制端口
        if (it == _discovered_nodes.end() || it->second.ctrl_loc.addr != addr ||
            it->second.ctrl_loc.port != port)
            continue;
        if (!msg->ack && !snapshotBelongsToPeer(msg->source, msg->snapshot))
            continue;

        std::optional<REDPMessage> response{}, next{};
        std::vector<REDPEndpoint> previous{}, current{};
        bool applied{};
        Locator locator{};
        auto peer = _redp_peers.find(msg->source);
        if (peer == _redp_peers.end())
            continue;
        auto &session = peer->second;
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
        if (applied)
            applyREDPSnapshot(msg->source, addr, previous, current);
        if (response)
            sendREDPMessage(*_redp_socket, locator, *response);
        if (next)
            sendREDPMessage(*_redp_socket, locator, *next);
    }
}

static bool is_same_node(const Guid &lhs, const Guid &rhs) { return lhs.host() == rhs.host() && lhs.pid() == rhs.pid(); }

rm::async::Task<> Node::heartbeat_detect() {
    while (_running) {
        // 检测心跳超时的节点
        auto now = std::chrono::steady_clock::now();
        std::vector<Guid> guid_ready_to_erase{};
        for (const auto &[guid, data] : _discovered_nodes)
            if (now - data.last_alive > std::chrono::milliseconds(data.rndp_msg.heartbeat_timeout * 1000))
                guid_ready_to_erase.push_back(guid);
        for (const auto &g : guid_ready_to_erase)
            _discovered_nodes.erase(g);
        // 移除超时节点关联的端点缓存，遍历期间安全擦除元素
        if (!guid_ready_to_erase.empty()) {
            for (const auto &dead_guid : guid_ready_to_erase) {
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
        if (!guid_ready_to_erase.empty()) {
            for (const auto &dead_guid : guid_ready_to_erase) {
                for (const auto &[topic, writer] : _local_writers)
                    writer->remove(dead_guid);
                for (const auto &[topic, reader] : _local_readers)
                    reader->remove(dead_guid);
            }
        }
        const auto local = snapshot();
        std::vector<std::pair<Locator, REDPMessage>> retries{};
        for (const auto &dead : guid_ready_to_erase) {
            auto it = _redp_peers.find(dead);
            if (it != _redp_peers.end())
                it->second.online = false;
        }
        for (auto &[peer, session] : _redp_peers)
            if (session.online)
                if (auto packet = session.sender.request(local))
                    retries.emplace_back(session.locator, std::move(*packet));
        for (const auto &[loc, packet] : retries)
            sendREDPMessage(*_redp_socket, loc, packet);

        co_await _hbt_timer.sleep_for(500ms);
    }
}

rm::async::Task<> Node::on_shutdown_signal(int signum) {
    rm::async::Signal sig(_ctx, signum);
    co_await sig.wait();
    printf("\nReceived %s, stopping node...\n", signum == SIGINT ? "SIGINT" : "SIGTERM");
    shutdown();
}

Node::Node(std::string_view name, uint8_t domain_id) : _rndp_port(7500 + domain_id), _rndp_writer(rm::async::Sender(_ctx, ip::udp::v4()).create()) {
    static_assert(sizeof(Locator) == 6, "Locator size must be 6 bytes");

    std::vector<ip::Networkv4> networks{};
    std::tie(_uid, networks) = generateNodeGuid();

    // 创建监听 Socket
    auto rndp_reader = rm::async::Listener(_ctx, Endpoint(ip::udp::v4(), _rndp_port)).create();
    auto redp_socket = std::make_shared<rm::async::DgramSocket>(rm::async::Listener(_ctx, Endpoint(ip::udp::v4(), Endpoint::ANY_PORT)).create());
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

    // 启动广播协程任务
    co_spawn(_ctx, &Node::rndp_multicast, this, std::move(networks), std::string(name));

    // 启动监听协程任务
    co_spawn(_ctx, &Node::rndp_listener, this, std::move(rndp_reader));

    // 启动 REDP 监听协程任务
    co_spawn(_ctx, &Node::redp_listener, this, redp_socket);

    // 启动心跳检测协程任务
    co_spawn(_ctx, &Node::heartbeat_detect, this);

    // 启动正常退出信号处理协程任务
    co_spawn(_ctx, &Node::on_shutdown_signal, this, SIGINT);
    co_spawn(_ctx, &Node::on_shutdown_signal, this, SIGTERM);
}

void Node::shutdown() noexcept {
    if (!_running.exchange(false, std::memory_order_acq_rel))
        return;
    // 准备本地端点下线的 REDP 快照
    std::vector<std::pair<Locator, REDPMessage>> final_snapshots{};
    for (auto &[peer, session] : _redp_peers) {
        if (session.sender.waiting() || session.sender.dirty())
            continue;
        session.sender.markDirty();
        if (auto packet = session.sender.request({}))
            final_snapshots.emplace_back(session.locator, std::move(*packet));
    }
    // 移除本地注册的 Writers/Readers
    _local_readers.clear();
    _local_writers.clear();
    // 向已发现节点单播 REDP 快照
    for (const auto &[loc, packet] : final_snapshots)
        sendREDPMessage(*_redp_socket, loc, packet);
    _ctx.stop();
}

} // namespace async

} // namespace rm::lpss

#endif

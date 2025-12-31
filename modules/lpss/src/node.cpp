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

#include "rmvl/core/timer.hpp"
#include "rmvl/core/util.hpp"
#include "rmvl/lpss.hpp"

#include "rmvlpara/lpss.hpp"

using namespace std::chrono_literals;

namespace rm::lpss {

void sendREDPMessage(Locator ctrl_loc, const REDPMessage &msg);

Node::Node(uint8_t domain) : _rndp_port(7500 + domain), _rndp_writer(Sender(ip::udp::v4()).create()) {
    static_assert(sizeof(Locator) == 6, "Locator size must be 6 bytes");

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
        _uid.fields.host = (static_cast<uint32_t>(basic_mac[2]) << 24) |
                           (static_cast<uint32_t>(basic_mac[3]) << 16) |
                           (static_cast<uint32_t>(basic_mac[4]) << 8) |
                           static_cast<uint32_t>(basic_mac[5]);
        _uid.fields.pid = static_cast<uint16_t>(processId());
        _uid.fields.entity = 0;
        // 收集 IP
        for (const auto &iface : candidate_interfaces) {
            auto nets = iface.ipv4();
            networks.insert(networks.end(), std::make_move_iterator(nets.begin()), std::make_move_iterator(nets.end()));
        }
    } else {
        WARNING_("[LPSS Node] No valid network interface found, using 00:00:00:00:00:00 as MAC address");
        _uid.fields.pid = static_cast<uint16_t>(processId());
    }

    // 创建监听 Socket
    auto rndp_reader = Listener(Endpoint(ip::udp::v4(), _rndp_port)).create();
    auto redp_socket = Listener(Endpoint(ip::udp::v4(), Endpoint::ANY_PORT)).create();
    _redp_port = redp_socket.endpoint().port();

    // 设置 Socket 选项
    _rndp_writer.setOption(ip::multicast::Loopback(true));
    rndp_reader.setOption(ip::multicast::JoinGroup(BROADCAST_IP));

    // 启动广播线程
    _bcast_thrd = std::thread(&Node::rndp_multicast, this, std::move(networks));

    // 启动监听线程
    _rndp_listen_thrd = std::thread(&Node::rndp_listener, this, std::move(rndp_reader));

    // 启动 REDP 监听线程
    _redp_listen_thrd = std::thread(&Node::redp_listener, this, std::move(redp_socket));

    // 启动心跳检测线程
    _hbt_detect_thrd = std::thread(&Node::heartbeat_detect, this);
}

Node::~Node() {
    // 向已发现节点发送 Remove 的 REDP 消息
    std::vector<REDPMessage> redp_msgs{};
    redp_msgs.reserve(_local_writers.size() + _local_readers.size());
    {
        std::shared_lock lk(_local_mtx);
        for (const auto &[topic, writer] : _local_writers)
            redp_msgs.push_back(REDPMessage::removeWriter(writer->guid(), topic));
        for (const auto &[topic, reader] : _local_readers)
            redp_msgs.push_back(REDPMessage::removeReader(reader->guid(), topic));
    }
    {
        std::shared_lock lk(_nodes_mtx);
        for (const auto &[guid, node_info] : _discovered_nodes)
            for (const auto &msg : redp_msgs)
                sendREDPMessage(node_info.ctrl_loc, msg);
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

void Node::rndp_multicast(std::vector<ip::Networkv4> networks) {
    // 准备 RNDP 消息
    uint8_t locator_num = static_cast<uint8_t>(networks.size() + 1);
    RNDPMessage rndp_msg{_uid, para::lpss_param.MAX_NODE_HEARTBEAT_PERIOD};
    rndp_msg.locators.reserve(locator_num);
    for (auto network : networks)
        rndp_msg.locators.push_back({_redp_port, network.address()});

    // 序列化 RNDP 消息
    auto rndp_msg_data = rndp_msg.serialize();

    // 广播 RNDP 消息
    while (_running.load(std::memory_order_acquire)) {
        std::this_thread::sleep_for(20ms);

        // 为 Outbound 设置不同的接口地址，并分别发送消息
        for (const auto &network : networks) {
            _rndp_writer.setOption(ip::multicast::Interface(network.address()));
            _rndp_writer.write(BROADCAST_IP, Endpoint(ip::udp::v4(), _rndp_port), rndp_msg_data);
        }
    }
}

static Locator selectBestLocator(std::array<uint8_t, 4> target_ip, const RNDPMessage &rndp_msg) {
    for (const auto &loc : rndp_msg.locators)
        if (loc.addr == target_ip)
            return loc;
    return {};
}

void Node::rndp_listener(DgramSocket rndp_reader) {
    while (_running.load(std::memory_order_acquire)) {
        // 接收原始 UDP 报文
        auto [data, addr_str, port] = rndp_reader.read();

        if (data.size() < RNDP_HEADER_SIZE)
            continue;
        // 解析的 RNDP 消息
        auto rndp_msg = RNDPMessage::deserialize(const_cast<const char *>(data.data()));
        if (rndp_msg.guid == _uid)
            continue;
        // 处理发现的节点信息
        bool is_new_node{};
        {
            std::lock_guard lk(_nodes_mtx);
            auto [it, inserted] = _discovered_nodes.try_emplace(rndp_msg.guid, rndp_msg);
            if (inserted)
                is_new_node = true;
            else
                it->second.last_alive = Timer::now();
        }

        if (is_new_node) {
            std::array<uint8_t, 4> addr{};
            inet_pton(AF_INET, addr_str.c_str(), addr.data());
            // 选择最佳控制平面定位器
            auto ctrl_loc = selectBestLocator(addr, rndp_msg);
            std::vector<REDPMessage> redp_msgs{};
            redp_msgs.reserve(_local_writers.size() + _local_readers.size());
            if (!ctrl_loc.invalid()) {
                std::lock_guard lk(_nodes_mtx);
                _discovered_nodes[rndp_msg.guid].ctrl_loc = ctrl_loc;
                for (const auto &[topic, writer] : _local_writers)
                    redp_msgs.push_back(REDPMessage::addWriter(writer->guid(), topic));
                for (const auto &[topic, reader] : _local_readers)
                    redp_msgs.push_back(REDPMessage::addReader(reader->guid(), topic, reader->port()));
            }
            // 向新发现的节点单播包含所有本地 Writers/Readers 的 REDP 消息
            for (const auto &msg : redp_msgs)
                sendREDPMessage(ctrl_loc, msg);
        }
    }
}

void Node::redp_listener(DgramSocket redp_socket) {
    while (_running.load(std::memory_order_acquire)) {
        auto [redp_msg, addr_str, port] = redp_socket.read();
        std::array<uint8_t, 4> addr{};
        inet_pton(AF_INET, addr_str.c_str(), addr.data());
        // 解析 REDP 消息
        auto msg = REDPMessage::deserialize(reinterpret_cast<const char *>(redp_msg.data()));
        // 设置已发现的 Writers/Readers 列表
        if (msg.action == REDPMessage::Action::Add) {
            std::lock_guard lk(_discovered_mtx);
            if (msg.type == REDPMessage::Type::Writer)
                _discovered_writers[msg.topic].insert(msg.endpoint_guid);
            else // Reader
                _discovered_readers[msg.topic][msg.endpoint_guid] = {msg.port, addr};
        } else { // Remove
            std::lock_guard lk(_discovered_mtx);
            if (msg.type == REDPMessage::Type::Writer) {
                _discovered_writers[msg.topic].erase(msg.endpoint_guid);
                if (_discovered_writers[msg.topic].empty())
                    _discovered_writers.erase(msg.topic);
            } else { // Reader
                _discovered_readers[msg.topic].erase(msg.endpoint_guid);
                if (_discovered_readers[msg.topic].empty())
                    _discovered_readers.erase(msg.topic);
            }
        }
        // 为本地 DataWriters 更新缓存
        if (msg.type == REDPMessage::Type::Reader) {
            std::lock_guard lk(_local_mtx);
            auto it = _local_writers.find(msg.topic);
            if (it != _local_writers.end()) {
                if (msg.action == REDPMessage::Action::Add)
                    it->second->add(msg.endpoint_guid, {msg.port, addr});
                else // Remove
                    it->second->remove(msg.endpoint_guid);
            }
        }
    }
}

void Node::heartbeat_detect() {
    while (_running.load(std::memory_order_acquire)) {
        std::vector<Guid> timeout_guids;
        auto now = Timer::now();
        {
            std::shared_lock lk(_nodes_mtx);
            for (const auto &[guid, data] : _discovered_nodes)
                if (now - data.last_alive > data.rndp_msg.heartbeat_timeout * 1000.)
                    timeout_guids.push_back(guid);
        }
        if (!timeout_guids.empty()) {
            std::lock_guard lk(_nodes_mtx);
            for (const auto &guid : timeout_guids)
                _discovered_nodes.erase(guid);
        }
        std::unique_lock lk(_hbt_mtx);
        _hbt_cv.wait_for(lk, 500ms);
    }
}

} // namespace rm::lpss

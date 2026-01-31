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

#include "rmvl/lpss/node.hpp"

#include "rmvlpara/lpss.hpp"

using namespace std::chrono_literals;

namespace rm::lpss {

std::pair<Guid, std::vector<ip::Networkv4>> generateNodeGuid();
Locator selectBestLocator(std::array<uint8_t, 4> target_ip, const RNDPMessage &rndp_msg);

Node::Node(std::string_view name, uint8_t domain) : helper::NodeRunningInfo(7500 + domain), _rndp_writer(Sender(ip::udp::v4()).create()) {
    static_assert(sizeof(Locator) == 6, "Locator size must be 6 bytes");

    std::vector<ip::Networkv4> networks{};
    std::tie(_uid, networks) = generateNodeGuid();

    // 创建监听 Socket
    auto rndp_reader = Listener(Endpoint(ip::udp::v4(), _rndp_port)).create();
    auto redp_socket = Listener(Endpoint(ip::udp::v4(), Endpoint::ANY_PORT)).create();
    _redp_port = redp_socket.endpoint().port();

    // 设置 Socket 选项
    _rndp_writer.setOption(ip::multicast::Loopback(true));
    rndp_reader.setOption(ip::multicast::JoinGroup(BROADCAST_IP));

    // 启动广播线程
    _bcast_thrd = std::thread(&Node::rndp_multicast, this, std::move(networks), std::string(name));

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
            auto [it, inserted] = _discovered_nodes.try_emplace(rndp_msg.guid, NodeStorageInfo{rndp_msg});
            if (inserted)
                is_new_node = true;
            else
                it->second.last_alive = std::chrono::steady_clock::now();
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

static bool is_same_node(const Guid &lhs, const Guid &rhs) { return lhs.fields.host == rhs.fields.host && lhs.fields.pid == rhs.fields.pid; }

void Node::heartbeat_detect() {
    while (_running.load(std::memory_order_acquire)) {
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
        // 删除节点关联的发布者、订阅者缓存，使用 iterator 遍历以避免 crash
        if (!timeout_guids.empty()) {
            std::lock_guard lk(_discovered_mtx);
            for (const auto &dead_guid : timeout_guids) {
                // 清理 Writers
                for (auto it = _discovered_writers.begin(); it != _discovered_writers.end();) {
                    auto &writers = it->second;
                    for (auto set_it = writers.begin(); set_it != writers.end();)
                        is_same_node(*set_it, dead_guid) ? set_it = writers.erase(set_it) : ++set_it;
                    writers.empty() ? it = _discovered_writers.erase(it) : ++it;
                }
                // 清理 Readers
                for (auto it = _discovered_readers.begin(); it != _discovered_readers.end();) {
                    auto &readers = it->second;
                    for (auto map_it = readers.begin(); map_it != readers.end();)
                        is_same_node(map_it->first, dead_guid) ? map_it = readers.erase(map_it) : ++map_it;
                    readers.empty() ? it = _discovered_readers.erase(it) : ++it;
                }
            }
        }

        std::unique_lock lk(_hbt_mtx);
        _hbt_cv.wait_for(lk, 500ms);
    }
}

} // namespace rm::lpss

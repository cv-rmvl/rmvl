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

#include <vector>
#if __cplusplus >= 202002L

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

std::pair<Guid, std::vector<ip::Networkv4>> generateNodeGuid();
Locator selectBestLocator(std::array<uint8_t, 4> target_ip, const RNDPMessage &rndp_msg);

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

    while (_running.load(std::memory_order_acquire)) {
        co_await _broadcast_timer.sleep_for(25ms);

        // 为 Outbound 设置不同的接口地址，并分别发送消息
        for (const auto &network : networks) {
            _rndp_writer.setOption(ip::multicast::Interface(network.address()));
            co_await _rndp_writer.write(BROADCAST_IP, Endpoint(ip::udp::v4(), _rndp_port), rndp_msg_data);
        }
    }
}

rm::async::Task<> Node::rndp_listener(rm::async::DgramSocket rndp_reader) {
    while (_running.load(std::memory_order_acquire)) {
        // 接收原始 UDP 报文
        auto [data, addr_str, port] = co_await rndp_reader.read();

        if (data.size() < RNDP_HEADER_SIZE)
            continue;
        // 解析的 RNDP 消息
        auto rndp_msg = RNDPMessage::deserialize(const_cast<const char *>(data.data()));
        if (rndp_msg.guid == _uid)
            continue;
        // 处理发现的节点信息
        bool is_new_node{};
        auto [it, inserted] = _discovered_nodes.try_emplace(rndp_msg.guid, NodeStorageInfo{rndp_msg});
        if (inserted)
            is_new_node = true;
        else
            it->second.last_alive = std::chrono::steady_clock::now();

        if (is_new_node) {
            std::array<uint8_t, 4> addr{};
            inet_pton(AF_INET, addr_str.c_str(), addr.data());
            // 选择最佳控制平面定位器
            auto ctrl_loc = selectBestLocator(addr, rndp_msg);
            std::vector<REDPMessage> redp_msgs{};
            redp_msgs.reserve(_local_writers.size() + _local_readers.size());
            if (!ctrl_loc.invalid()) {
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

rm::async::Task<> Node::redp_listener(rm::async::DgramSocket redp_socket) {
    while (_running.load(std::memory_order_acquire)) {
        auto [redp_msg, addr_str, port] = co_await redp_socket.read();
        std::array<uint8_t, 4> addr{};
        inet_pton(AF_INET, addr_str.c_str(), addr.data());
        // 解析 REDP 消息
        auto msg = REDPMessage::deserialize(reinterpret_cast<const char *>(redp_msg.data()));
        // 设置已发现的 Writers/Readers 列表
        if (msg.action == REDPMessage::Action::Add) {
            if (msg.type == REDPMessage::Type::Writer)
                _discovered_writers[msg.topic].insert(msg.endpoint_guid);
            else // Reader
                _discovered_readers[msg.topic][msg.endpoint_guid] = {msg.port, addr};
        } else { // Remove
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

rm::async::Task<> Node::heartbeat_detect() {
    while (_running.load(std::memory_order_acquire)) {
        auto now = std::chrono::steady_clock::now();
        std::vector<Guid> guid_ready_to_erase{};
        for (const auto &[guid, data] : _discovered_nodes)
            if (now - data.last_alive > std::chrono::milliseconds(data.rndp_msg.heartbeat_timeout * 1000))
                guid_ready_to_erase.push_back(guid);
        for (const auto &g : guid_ready_to_erase)
            _discovered_nodes.erase(g);

        co_await _hbt_timer.sleep_for(500ms);
    }
}

static void sendStopMessage(const std::unordered_map<Guid, NodeStorageInfo, GuidHash> &discovered_nodes,
                            const std::unordered_map<std::string, DataWriterBase::ptr> &local_writers,
                            const std::unordered_map<std::string, DataReaderBase::ptr> &local_readers) {
    // 向已发现节点发送 Remove 的 REDP 消息
    std::vector<REDPMessage> redp_msgs{};
    redp_msgs.reserve(local_writers.size() + local_readers.size());
    for (const auto &[topic, writer] : local_writers)
        redp_msgs.push_back(REDPMessage::removeWriter(writer->guid(), topic));
    for (const auto &[topic, reader] : local_readers)
        redp_msgs.push_back(REDPMessage::removeReader(reader->guid(), topic));

    for (const auto &[guid, node_info] : discovered_nodes)
        for (const auto &msg : redp_msgs)
            sendREDPMessage(node_info.ctrl_loc, msg);
}

rm::async::Task<> Node::on_sigint() {
    rm::async::Signal sig(_ctx, SIGINT);
    co_await sig.wait();
    printf("\nReceived interrupt signal, stopping node...\n");
    sendStopMessage(_discovered_nodes, _local_writers, _local_readers);
    _ctx.stop();
}

Node::Node(std::string_view name, uint8_t domain_id) : helper::NodeRunningInfo(7500 + domain_id), _rndp_writer(rm::async::Sender(_ctx, ip::udp::v4()).create()) {
    static_assert(sizeof(Locator) == 6, "Locator size must be 6 bytes");

    std::vector<ip::Networkv4> networks{};
    std::tie(_uid, networks) = generateNodeGuid();

    // 创建监听 Socket
    auto rndp_reader = rm::async::Listener(_ctx, Endpoint(ip::udp::v4(), _rndp_port)).create();
    auto redp_socket = rm::async::Listener(_ctx, Endpoint(ip::udp::v4(), Endpoint::ANY_PORT)).create();
    _redp_port = redp_socket.endpoint().port();

    // 设置 Socket 选项
    _rndp_writer.setOption(ip::multicast::Loopback(true));
    rndp_reader.setOption(ip::multicast::JoinGroup(BROADCAST_IP));

    // 启动广播协程任务
    co_spawn(_ctx, &Node::rndp_multicast, this, std::move(networks), std::string(name));

    // 启动监听协程任务
    co_spawn(_ctx, &Node::rndp_listener, this, std::move(rndp_reader));

    // 启动 REDP 监听协程任务
    co_spawn(_ctx, &Node::redp_listener, this, std::move(redp_socket));

    // 启动心跳检测协程任务
    co_spawn(_ctx, &Node::heartbeat_detect, this);

    // 启动 SIGINT 信号处理协程任务
    co_spawn(_ctx, &Node::on_sigint, this);
}

Node::~Node() {
    sendStopMessage(_discovered_nodes, _local_writers, _local_readers);
    _ctx.stop();
}

} // namespace async

} // namespace rm::lpss

#endif

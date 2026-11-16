/**
 * @file node_rsd.hpp
 * @author zhaoxi (535394140@qq.com)
 * @brief RMVL 发现协议定义实现
 * @version 1.0
 * @date 2025-12-04
 *
 * @copyright Copyright 2025 (c), zhaoxi
 *
 */

#pragma once

#include <chrono>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>

#include "node_util.hpp"

namespace rm::lpss {

//! @cond

//! 当前支持的 RNDP、REDP、RMTP 协议版本
constexpr uint8_t CURRENT_NDP_VERSION = 2;
constexpr uint8_t CURRENT_EDP_VERSION = 2;
constexpr uint8_t CURRENT_MTP_VERSION = 2;

//! RNDP 节点发现消息
struct RNDPMessage {
    //! 序列化 RNDP 消息
    std::string serialize() const noexcept;

    //! 反序列化 RNDP 消息
    static std::optional<RNDPMessage> deserialize(std::string_view data) noexcept;

    Guid guid{};                     //!< 节点唯一标识符
    uint8_t ndp_version{CURRENT_NDP_VERSION};          //!< NDP 协议版本
    uint8_t edp_version{CURRENT_EDP_VERSION};          //!< EDP 协议版本
    uint8_t mtp_version{CURRENT_MTP_VERSION};          //!< MTP 协议版本
    uint8_t heartbeat_timeout{};     //!< 心跳周期（单位：s）
    std::vector<Locator> locators{}; //!< REDP 控制消息的单播定位器列表
    std::string name{};              //!< 节点名称
};

//! REDP 快照中的一个端点；端口为 0 表示 Writer
struct REDPEndpoint {
    Guid guid{};
    uint16_t port{};
    std::string topic{};
    std::string msgtype{};
};

//! REDP: magic(4) + source(8) + SYN(2) + ACK(1) + count(2) + Snapshot[]
struct REDPMessage {
    Guid source{};
    uint16_t syn{};
    bool ack{};
    std::vector<REDPEndpoint> snapshot{};

    std::string serialize() const;
    static std::optional<REDPMessage> deserialize(std::string_view data) noexcept;
};

//! 双方能共同使用的协议版本，低于 v2 时回退到 v2
struct CompatibleProtocol {
    uint8_t edp{2}, mtp{2};
};

CompatibleProtocol negotiate(uint8_t cur_edp, uint8_t cur_mtp,
                             uint8_t peer_edp, uint8_t peer_mtp) noexcept;
bool sameREDPEndpoint(const REDPEndpoint &a, const REDPEndpoint &b) noexcept;
bool snapshotBelongsToPeer(Guid peer, const std::vector<REDPEndpoint> &snapshot) noexcept;

uint16_t initialREDPSyn();
bool countREDPEndpointSize(size_t &total, std::string_view topic, std::string_view type) noexcept;

//! 每个 Peer 独立停等，端点变化合并为最新状态，丢 ACK 时重发同一报文
class REDPSender {
public:
    REDPSender(Guid local, Guid peer) : _local(local), _peer(peer), _next(initialREDPSyn()) {}
    void markDirty() noexcept { _dirty = true; }
    std::optional<REDPMessage> request(const std::vector<REDPEndpoint> &snapshot);
    bool confirm(const REDPMessage &ack, const std::vector<REDPEndpoint> &snapshot);

    bool waiting() const noexcept { return _pending.has_value(); }
    bool dirty() const noexcept { return _dirty; }

private:
    Guid _local{}, _peer{};
    uint16_t _next{};
    std::optional<REDPMessage> _pending{};
    bool _dirty{};
};

//! 接收已解码校验的 REDP 报文；乱序、冲突重复报文均拒绝
class REDPReceiver {
public:
    struct Result {
        bool applied{};
        REDPMessage ack{};
    };
    explicit REDPReceiver(Guid peer) : _peer(peer) {}
    std::optional<Result> accept(const REDPMessage &request, Guid local);
    const std::vector<REDPEndpoint> &snapshot() const noexcept { return _snapshot; }

private:
    Guid _peer{};
    bool _initialized{};
    uint16_t _last_syn{};
    std::vector<REDPEndpoint> _snapshot{};
};

//! REDP Peer 会话状态
struct REDPPeer {
    Locator locator{};
    REDPSender sender;
    REDPReceiver receiver;
    bool online{true};

    REDPPeer(Guid local, Guid peer, Locator loc)
        : locator(loc), sender(local, peer), receiver(peer) {}
};

bool sendREDPMessage(DgramSocket &socket, Locator locator, const REDPMessage &message);

//! 节点存储信息
struct NodeStorageInfo {
    RNDPMessage rndp_msg{};                                          //!< RNDP 消息
    std::chrono::time_point<std::chrono::steady_clock> last_alive{}; //!< 上次收到该节点消息的时间戳
    Locator ctrl_loc{};                                              //!< 与该节点通信的最佳控制平面定位器
    CompatibleProtocol selected{};                                  //!< 与此节点实际选择的 EDP/MTP 版本
};

//! 发现的订阅者端点存储信息
struct DiscoveredReaderStorageInfo {
    std::unordered_map<Guid, Locator, GuidHash> readers; //!< 相关的端点 GUID 与定位器映射表 [Guid: Locator]
    std::string msgtype;                                 //!< 消息类型
};

Locator selectBestLocator(std::array<uint8_t, 4> target_ip, const RNDPMessage &rndp_msg);

//! @endcond

} // namespace rm::lpss

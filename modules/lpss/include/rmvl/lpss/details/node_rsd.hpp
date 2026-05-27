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
#include <string>
#include <string_view>
#include <unordered_map>

#include "node_util.hpp"

namespace rm::lpss {

//! @cond

//! RNDP 节点发现消息
struct RNDPMessage {
    //! 序列化 RNDP 消息
    std::string serialize() const noexcept;

    //! 反序列化 RNDP 消息
    static RNDPMessage deserialize(const char *data) noexcept;

    Guid guid{};                     //!< 节点唯一标识符
    uint8_t heartbeat_timeout{};     //!< 心跳周期（单位：s）
    std::vector<Locator> locators{}; //!< 用于 REDP 的单播定位器列表
    std::string name{};              //!< 节点名称
};

//! REDP 通信端点发现消息
struct REDPMessage {
    //! 序列化 REDP 消息
    std::string serialize() const noexcept;

    //! 反序列化 REDP 消息
    static REDPMessage deserialize(const char *data) noexcept;

    //! 创建添加读取端点消息
    static inline REDPMessage addReader(const Guid &g, std::string_view topic, uint16_t port, std::string_view msgtype) noexcept {
        return {Action::Add, Type::Reader, g, port, std::string(topic), std::string(msgtype)};
    }

    //! 创建添加写入端点消息
    static inline REDPMessage addWriter(const Guid &g, std::string_view topic, std::string_view msgtype) noexcept {
        return {Action::Add, Type::Writer, g, {}, std::string(topic), std::string(msgtype)};
    }

    //! 创建移除读取端点消息
    static inline REDPMessage removeReader(const Guid &g, std::string_view topic) noexcept {
        return {Action::Remove, Type::Reader, g, {}, std::string(topic)};
    }

    //! 创建移除写入端点消息
    static inline REDPMessage removeWriter(const Guid &g, std::string_view topic) noexcept {
        return {Action::Remove, Type::Writer, g, {}, std::string(topic)};
    }

    enum class Action : uint8_t {
        Remove = 0b00, //!< 端点移除
        Add = 0b01     //!< 端点更新
    } action{};        //!< 端点动作

    enum class Type : uint8_t {
        Reader = 0b00, //!< 数据读取端点
        Writer = 0b10  //!< 数据写入端点
    } type{};          //!< 端点类型

    Guid endpoint_guid{};  //!< 端点所属实体 GUID
    uint16_t port{};       //!< 监听端口
    std::string topic{};   //!< 话题名称
    std::string msgtype{}; //!< 消息类型
};

//! 节点存储信息
struct NodeStorageInfo {
    RNDPMessage rndp_msg{};                                          //!< RNDP 消息
    std::chrono::time_point<std::chrono::steady_clock> last_alive{}; //!< 上次收到该节点消息的时间戳
    Locator ctrl_loc{};                                              //!< 与该节点通信的最佳控制平面定位器
};

//! 发现的订阅者端点存储信息
struct DiscoveredReaderStorageInfo {
    std::unordered_map<Guid, Locator, GuidHash> readers; //!< 相关的端点 GUID 与定位器映射表 [Guid: Locator]
    std::string msgtype;                                 //!< 消息类型
};

void sendREDPMessage(Locator loc, const REDPMessage &msg);

Locator selectBestLocator(std::array<uint8_t, 4> target_ip, const RNDPMessage &rndp_msg);

//! @endcond

} // namespace rm::lpss

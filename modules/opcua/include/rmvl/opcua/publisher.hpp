/**
 * @file publisher.hpp
 * @author zhaoxi (535394140@qq.com)
 * @brief OPC UA 发布者
 * @version 1.0
 * @date 2023-11-30
 *
 * @copyright Copyright 2023 (c), zhaoxi
 *
 */

#pragma once

#include <open62541/config.h>

#ifdef UA_ENABLE_PUBSUB

#include "server.hpp"

namespace rm
{

//! @addtogroup opcua
//! @{

//! 传输协议
enum class TransportID : uint8_t
{
    UDP_UADP = 0U,  //!< 使用 `UDP` 传输协议映射和 `UADP` 消息映射的组合，此协议用于 **无代理** 的消息传递
    MQTT_UADP = 1U, //!< 使用 `MQTT` 传输协议映射和 `UADP` 消息映射的组合，此协议用于 **基于代理** 的消息传递
    MQTT_JSON = 2U, //!< 使用 `MQTT` 传输协议映射和 `JSON` 消息映射的组合，此协议用于 **基于代理** 的消息传递
};

//! OPC UA 发布者
class Publisher : public Server
{
    UA_NodeId _connection_id{UA_NODEID_NULL}; //!< 连接 ID
    UA_NodeId _pds_id{UA_NODEID_NULL};        //!< PublishedDataSet 已发布数据集 ID
    UA_NodeId _wg_id{UA_NODEID_NULL};         //!< WriterGroup 写入组 ID
    UA_NodeId _dsw_id{UA_NODEID_NULL};        //!< DataSetWriter 数据集写入器 ID

public:
    /**
     * @brief 创建 OPC UA 发布者
     *
     * @param[in] port 端口号
     * @param[in] name 发布者名称
     * @param[in] address 网络地址，形如 `opc.udp://224.0.1.20:4840`
     * @param[in] duration 发布周期，单位为 `ms`
     * @param[in] tp 传输协议 ID，默认为 `UDP_UADP` @see TransportID
     * @param[in] users 用户列表，默认为空 @see UserConfig
     */
    Publisher(uint16_t port, const std::string &name, const std::string &address, double duration,
              TransportID tp = TransportID::UDP_UADP, const std::vector<UserConfig> &users = {});

    /**
     * @brief 发布变量节点
     *
     * @param[in] name 变量节点发布的名称
     * @param[in] node_id 变量节点 ID
     * @return 是否发布成功
     */
    bool publish(std::string_view name, const UA_NodeId &node_id);
};

//! @} opcua

} // namespace rm

#endif // UA_ENABLE_PUBSUB

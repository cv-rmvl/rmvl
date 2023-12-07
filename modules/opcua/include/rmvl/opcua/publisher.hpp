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

#include "server.hpp"

#ifdef UA_ENABLE_PUBSUB

namespace rm
{

//! @addtogroup opcua
//! @{

/**
 * @brief 待发布的数据集 (PDS)
 * @brief
 * - 包含变量节点发布的名称及其对应的 ID
 */
struct PublishedDataSet
{
    std::string name;  //!< 变量节点发布的名称
    UA_NodeId node_id; //!< 变量节点 ID
};

/**
 * @brief OPC UA 发布者
 * 
 * @tparam Tpid 传输协议 ID，可参考 `rm::TransportID`
 * 
 * @details **特化**
 * - @ref Publisher<TransportID::UDP_UADP>
 */
template <TransportID Tpid>
class Publisher final
{
    static_assert(Tpid != TransportID::MQTT_UADP, "OPC UA publisher specialization using \033[33mMQTT\033[0m protocol"
                                                  " and \033[33mUADP\033[0m serialization is not currently supported!");
    static_assert(Tpid != TransportID::MQTT_JSON, "OPC UA publisher specialization using \033[33mMQTT\033[0m protocol"
                                                  " and \033[33mJSON\033[0m serialization is not currently supported!");
};

//! 使用 `UDP` 协议以及 `UADP` 序列化方式的 Publisher 特化
template <>
class Publisher<TransportID::UDP_UADP> : public Server
{
    UA_NodeId _connection_id{UA_NODEID_NULL}; //!< 连接 ID
    UA_NodeId _pds_id{UA_NODEID_NULL};        //!< PublishedDataSet 已发布数据集 ID
    UA_NodeId _wg_id{UA_NODEID_NULL};         //!< WriterGroup 写入组 ID
    UA_NodeId _dsw_id{UA_NODEID_NULL};        //!< DataSetWriter 数据集写入器 ID

    std::string _name;               //!< 发布者名称
    std::hash<std::string> _strhash; //!< 字符串哈希函数

public:
    /**
     * @brief 创建 OPC UA 发布者
     *
     * @param[in] pub_name 发布者名称
     * @param[in] address 网络多播 IP 地址，形如 `opc.udp://224.0.0.22:4840`，端口号与参数 `port` 一致
     * @param[in] port 端口号，与 @ref Server::Server 的端口号概念一致，默认为 `4840U`
     * @param[in] users 用户列表，默认为空，可参考 @ref UserConfig
     */
    Publisher(const std::string &pub_name, const std::string &address, uint16_t port = 4840U,
              const std::vector<UserConfig> &users = {});

    /**
     * @brief 发布数据集
     *
     * @param[in] datas 待发布的数据集列表
     * @param[in] duration 发布周期，单位为 `ms`
     * @return 是否发布成功
     */
    bool publish(const std::vector<PublishedDataSet> &datas, double duration);
};

//! @} opcua

} // namespace rm

#endif // UA_ENABLE_PUBSUB

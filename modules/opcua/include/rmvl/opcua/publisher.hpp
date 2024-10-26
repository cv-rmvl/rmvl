/**
 * @file publisher.hpp
 * @author zhaoxi (535394140@qq.com)
 * @brief OPC UA 发布者
 * @version 2.1
 * @date 2024-03-07
 *
 * @copyright Copyright 2024 (c), zhaoxi
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
 * - 包含变量节点发布的字段名称及其对应的 ID
 */
struct RMVL_EXPORTS_W_AG PublishedDataSet
{
    RMVL_W_RW std::string name; //!< 变量节点发布的字段名称
    RMVL_W_RW NodeId node_id;   //!< 变量节点 ID
};

//! OPC UA 发布者
class Publisher : public Server
{
public:
    /**
     * @brief 创建 OPC UA 发布者
     *
     * @param[in] pub_name 发布者名称
     * @param[in] addr 不加端口的网络多播 IP 地址，形如 `opc.udp://224.0.1.22`
     * @param[in] port 端口号，并且作为多播 IP 地址的端口号，与 @ref Server::Server 的端口号概念一致，默认为 `4840U`
     * @param[in] users 用户列表，默认为空，可参考 @ref UserConfig
     */
    RMVL_W_RW Publisher(std::string_view pub_name, const std::string &addr, uint16_t port = 4840U, const std::vector<UserConfig> &users = {});

    /**
     * @brief 发布数据集
     *
     * @param[in] datas 待发布的数据集列表
     * @param[in] duration 发布周期，单位为 `ms`
     * @return 是否发布成功
     */
    RMVL_W_RW bool publish(const std::vector<PublishedDataSet> &datas, double duration);

private:
    UA_NodeId _connection_id{}; //!< 连接 ID
    UA_NodeId _pds_id{};        //!< PublishedDataSet 已发布数据集 ID
    UA_NodeId _wg_id{};         //!< WriterGroup 写入组 ID
    UA_NodeId _dsw_id{};        //!< DataSetWriter 数据集写入器 ID

    std::string _name;               //!< 发布者名称
    std::hash<std::string> _strhash; //!< 字符串哈希函数
};

//! @} opcua

} // namespace rm

#endif // UA_ENABLE_PUBSUB

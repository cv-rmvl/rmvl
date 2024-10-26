/**
 * @file subscriber.hpp
 * @author zhaoxi (535394140@qq.com)
 * @brief OPC UA 订阅者
 * @version 2.2
 * @date 2024-03-29
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

//! 数据集字段元数据
struct RMVL_EXPORTS_W_AG FieldMetaData final
{
    /**
     * @brief 从变量创建字段元数据
     * @brief
     * - `Variable::browse_name` 用于设置字段名称
     * @brief
     * - `Variable::getDataType()` 用于设置字段类型
     * @brief
     * - `Variable::size()` 用于辅助设置字段 ValueRank
     * @brief
     * - `Variable::ns` 用于设置命名空间索引
     *
     * @param[in] val 变量，可参考 @ref rm::Variable
     */
    static FieldMetaData makeFrom(const Variable &val) { return {val.browse_name, val.getDataType(), val.size() == 1 ? -1 : 1, val.ns}; }

    //! 字段名称
    RMVL_W_RW std::string name;

    /**
     * @brief 字段类型
     * @see DataType
     */
    RMVL_W_RW DataType type;

    //! 字段 ValueRank
    RMVL_W_RW int value_rank{};

    //! 命名空间索引，默认为 `1`
    RMVL_W_RW uint16_t ns{1U};
};

//! OPC UA 订阅者
class RMVL_EXPORTS_W Subscriber : public Server
{
public:
    /**
     * @brief 创建 OPC UA 订阅者
     *
     * @param[in] sub_name 订阅者名称
     * @param[in] addr 订阅地址，形如 `opc.udp://224.0.0.22:4840`
     * @param[in] port 端口号，与 @ref Server::Server 的端口号概念一致，默认为 4850
     * @param[in] users 用户列表，默认为空，可参考 @ref UserConfig
     */
    RMVL_W Subscriber(std::string_view sub_name, const std::string &addr, uint16_t port = 4850U, const std::vector<UserConfig> &users = {});

    /**
     * @brief 订阅数据集
     *
     * @param[in] pub_name 发布者名称
     * @param[in] fields 数据集字段元数据列表
     * @return 订阅的变量对应的 `NodeId` 列表，每个 `NodeId` 均存在于订阅者自身的服务器中
     */
    RMVL_W std::vector<NodeId> subscribe(const std::string &pub_name, const std::vector<FieldMetaData> &fields);

private:
    UA_NodeId _connection_id{}; //!< 连接 ID
    UA_NodeId _rg_id{};         //!< ReaderGroup 读取组 ID
    UA_NodeId _dsr_id{};        //!< DataSetReader 数据集读取器 ID

    std::string _name;               //!< 订阅者名称
    std::hash<std::string> _strhash; //!< 字符串哈希函数
};

//! @} opcua

} // namespace rm

#endif // UA_ENABLE_PUBSUB

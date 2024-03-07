/**
 * @file subscriber.hpp
 * @author zhaoxi (535394140@qq.com)
 * @brief OPC UA 订阅者
 * @version 2.1
 * @date 2024-03-07
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

//! 数据集字段元数据
struct FieldMetaData final
{
    //! 字段名称
    std::string name;

    /**
     * @brief 字段类型
     * @see UA_TypeFlag
     */
    UA_TypeFlag type;

    //! 字段 ValueRank
    int value_rank;

    FieldMetaData() = default;

    /**
     * @brief 创建字段元数据
     *
     * @param[in] name_ 字段名称
     * @param[in] type_ 字段类型，可参考 @ref UA_TypeFlag
     * @param[in] value_rank_ 字段 ValueRank
     */
    FieldMetaData(const std::string &name_, UA_TypeFlag type_, int value_rank_) : name(name_), type(type_), value_rank(value_rank_) {}

    /**
     * @brief 从变量创建字段元数据
     * @brief
     * - 变量的 `Variable::browse_name` 用于设置字段名称
     * @brief
     * - 变量的 `Variable::getDataType()` 用于设置字段类型
     * @brief
     * - 变量的 `Variable::size()` 用于辅助设置字段 ValueRank
     *
     * @param[in] val 变量，可参考 @ref rm::Variable
     * @return FieldMetaData 字段元数据
     */
    FieldMetaData(const Variable &val) : name(val.browse_name), type(val.getDataType()), value_rank(val.size() == 1 ? UA_VALUERANK_SCALAR : 1) {}
};

/**
 * @brief OPC UA 订阅者
 * 
 * @tparam Tpid 传输协议 ID，可参考 `rm::TransportID`
 * 
 * @details **特化**
 * - @ref Subscriber<TransportID::UDP_UADP>
 */
template <TransportID Tpid>
class Subscriber final
{
    static_assert(Tpid != TransportID::MQTT_UADP, "OPC UA subscriber specialization using \033[33mMQTT\033[0m protocol"
                                                  " and \033[33mUADP\033[0m serialization is not currently supported!");
    static_assert(Tpid != TransportID::MQTT_JSON, "OPC UA subscriber specialization using \033[33mMQTT\033[0m protocol"
                                                  " and \033[33mJSON\033[0m serialization is not currently supported!");
};

//! 使用 `UDP` 协议以及 `UADP` 序列化方式的订阅者特化
template <>
class Subscriber<TransportID::UDP_UADP> : public Server
{
    UA_NodeId _connection_id{UA_NODEID_NULL}; //!< 连接 ID
    UA_NodeId _rg_id{UA_NODEID_NULL};         //!< ReaderGroup 读取组 ID
    UA_NodeId _dsr_id{UA_NODEID_NULL};        //!< DataSetReader 数据集读取器 ID

    std::string _name;               //!< 订阅者名称
    std::hash<std::string> _strhash; //!< 字符串哈希函数

public:
    /**
     * @brief 创建 OPC UA 订阅者
     *
     * @param[in] sub_name 订阅者名称
     * @param[in] address 订阅地址，形如 `opc.udp://224.0.0.22:4840`
     * @param[in] port 端口号，与 @ref Server::Server 的端口号概念一致，默认为 4850
     * @param[in] users 用户列表，默认为空，可参考 @ref UserConfig
     */
    Subscriber(const std::string &sub_name, const std::string &address, uint16_t port = 4850U,
               const std::vector<UserConfig> &users = {});

    /**
     * @brief 订阅数据集
     *
     * @param[in] pub_name 发布者名称
     * @param[in] fields 数据集字段元数据列表
     * @return 订阅的变量对应的 `UA_NodeId` 列表，每个 `UA_NodeId` 均存在于订阅者自身的服务器中
     */
    std::vector<UA_NodeId> subscribe(const std::string &pub_name, const std::vector<FieldMetaData> &fields);
};

//! @} opcua

} // namespace rm

#endif // UA_ENABLE_PUBSUB

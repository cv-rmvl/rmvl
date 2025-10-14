/**
 * @file subscriber.cpp
 * @author zhaoxi (535394140@qq.com)
 * @brief OPC UA 订阅者
 * @version 1.1
 * @date 2024-03-29
 *
 * @copyright Copyright 2023 (c), zhaoxi
 *
 */

#include "rmvl/opcua/subscriber.hpp"

#ifdef UA_ENABLE_PUBSUB

#include <open62541/plugin/log_stdout.h>
#include <open62541/server_config_default.h>

#ifdef UA_ENABLE_PUBSUB_MQTT
#include <open62541/plugin/pubsub_mqtt.h>
#endif // UA_ENABLE_PUBSUB_MQTT

#include "rmvl/core/util.hpp"
#include "rmvl/opcua/utilities.hpp"

namespace rm {

OpcuaSubscriber::OpcuaSubscriber(std::string_view sub_name, const std::string &addr, uint16_t port,
                                 const std::vector<UserConfig> &users) : OpcuaServer(port, sub_name, users), _name(sub_name) {
    //////////////////// 添加连接配置 ////////////////////
    UA_PubSubConnectionConfig connect_config{};
    std::string cn_name_str = _name + "Connection";
    connect_config.name = UA_STRING(helper::to_char(cn_name_str));
    connect_config.transportProfileUri = UA_STRING(const_cast<char *>("http://opcfoundation.org/UA-Profile/Transport/pubsub-udp-uadp"));
    connect_config.enabled = UA_TRUE;
    UA_NetworkAddressUrlDataType address_url{UA_STRING_NULL, UA_STRING(helper::to_char(addr))};
    UA_Variant_setScalarCopy(&connect_config.address, &address_url, &UA_TYPES[UA_TYPES_NETWORKADDRESSURLDATATYPE]);
    connect_config.publisherId.uint32 = UA_UInt32_random();
    auto status = UA_Server_addPubSubConnection(_server, &connect_config, &_connection_id);
    if (status != UA_STATUSCODE_GOOD) {
        ERROR_("Failed to add connection, \"%s\"", UA_StatusCode_name(status));
        return;
    }
}

std::vector<NodeId> OpcuaSubscriber::subscribe(const std::string &pub_name, const std::vector<FieldMetaData> &fields) {
    //////////////// 添加 ReaderGroup (RG) ///////////////
    UA_ReaderGroupConfig rg_config{};
    std::string pub_name_str = pub_name + "ReaderGroup";
    rg_config.name = UA_STRING(helper::to_char(pub_name_str));
    auto status = UA_Server_addReaderGroup(_server, _connection_id, &rg_config, &_rg_id);
    if (status != UA_STATUSCODE_GOOD) {
        ERROR_("Failed to add reader group, \"%s\"", UA_StatusCode_name(status));
        return {};
    }
    status = UA_Server_setReaderGroupOperational(_server, _rg_id);
    if (status != UA_STATUSCODE_GOOD) {
        ERROR_("Failed to set reader group operational, \"%s\"", UA_StatusCode_name(status));
        return {};
    }

    ////////////// 添加 DataSetReader (DSR) //////////////
    UA_DataSetReaderConfig dsr_config{};
    std::string dsr_name = pub_name + "DataSetReader";
    dsr_config.name = UA_STRING(helper::to_char(dsr_name));
    UA_UInt16 publisher_id = _strhash(pub_name + "Connection") % 0x4000u;
    UA_Variant_setScalar(&dsr_config.publisherId, &publisher_id, &UA_TYPES[UA_TYPES_UINT16]);
    dsr_config.writerGroupId = 0x4000u + _strhash(pub_name + "WriterGroup") % 0x4000u;
    dsr_config.dataSetWriterId = 0x8000u + _strhash(pub_name + "DataSetWriter") % 0x4000u;

    // `DataType` 到对应 `NS0` 下的类型名的映射
    constexpr UA_Byte typeflag_ns0[] = {UA_NS0ID_BOOLEAN, UA_NS0ID_SBYTE, UA_NS0ID_BYTE,
                                        UA_NS0ID_INT16, UA_NS0ID_UINT16, UA_NS0ID_INT32,
                                        UA_NS0ID_UINT32, UA_NS0ID_INT64, UA_NS0ID_UINT64,
                                        UA_NS0ID_FLOAT, UA_NS0ID_DOUBLE, UA_NS0ID_STRING};

    // 设置数 DSR 中的元数据配置
    std::string dataset_name = _name + "DataSetMetaData";
    dsr_config.dataSetMetaData.name = UA_STRING(helper::to_char(dataset_name));
    std::vector<UA_FieldMetaData> raw_fields(fields.size());
    for (size_t i = 0; i < fields.size(); i++) {
        UA_NodeId_copy(&UA_TYPES[fields[i].type].typeId, &raw_fields[i].dataType);
        raw_fields[i].builtInType = typeflag_ns0[fields[i].type];
        raw_fields[i].name = UA_STRING(helper::to_char(fields[i].name));
        raw_fields[i].description = UA_LOCALIZEDTEXT(helper::zh_CN(), helper::to_char(fields[i].name));
        raw_fields[i].valueRank = fields[i].value_rank;
    }
    dsr_config.dataSetMetaData.fieldsSize = raw_fields.size();
    dsr_config.dataSetMetaData.fields = raw_fields.data();

    status = UA_Server_addDataSetReader(_server, _rg_id, &dsr_config, &_dsr_id);
    if (status != UA_STATUSCODE_GOOD) {
        ERROR_("Failed to add data set reader, \"%s\"", UA_StatusCode_name(status));
        return {};
    }

    /////////////////// 订阅数据集变量 ///////////////////
    Object sub_obj;
    sub_obj.browse_name = sub_obj.description = sub_obj.display_name = dataset_name;
    auto obj_id = addObjectNode(sub_obj);
    if (obj_id.empty()) {
        ERROR_("Failed to add object node, \"%s\"", UA_StatusCode_name(status));
        return {};
    }
    // 根据数据集元数据 DataSetMetaData 的字段创建 FieldTargetVariable
    std::vector<UA_FieldTargetVariable> target_vars(fields.size());
    std::vector<NodeId> retval;
    retval.reserve(fields.size());
    for (size_t i = 0; i < fields.size(); i++) {
        UA_VariableAttributes attr = UA_VariableAttributes_default;
        attr.displayName = UA_LOCALIZEDTEXT(helper::en_US(), helper::to_char(fields[i].name));
        attr.description = UA_LOCALIZEDTEXT(helper::zh_CN(), helper::to_char(fields[i].name));
        attr.dataType = raw_fields[i].dataType;
        attr.valueRank = raw_fields[i].valueRank;
        attr.accessLevel = UA_ACCESSLEVELMASK_READ | UA_ACCESSLEVELMASK_WRITE;
        UA_NodeId node_id;
        status = UA_Server_addVariableNode(
            _server, UA_NODEID_NULL, obj_id, nodeHasComponent,
            UA_QUALIFIEDNAME(fields[i].ns, helper::to_char(fields[i].name)),
            nodeBaseDataVariableType, attr, nullptr, &node_id);
        if (status != UA_STATUSCODE_GOOD) {
            ERROR_("Failed to add variable node, \"%s\"", UA_StatusCode_name(status));
            continue;
        }
        UA_FieldTargetDataType_init(&target_vars[i].targetVariable);
        target_vars[i].targetVariable.attributeId = UA_ATTRIBUTEID_VALUE;
        target_vars[i].targetVariable.targetNodeId = node_id;
        retval.push_back(node_id);
    }
    status = UA_Server_DataSetReader_createTargetVariables(_server, _dsr_id, target_vars.size(), target_vars.data());
    if (status != UA_STATUSCODE_GOOD) {
        ERROR_("Failed to create target variables, \"%s\"", UA_StatusCode_name(status));
        return {};
    }
    return retval;
}

} // namespace rm

#endif // UA_ENABLE_PUBSUB

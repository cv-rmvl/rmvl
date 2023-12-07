/**
 * @file subscriber.cpp
 * @author zhaoxi (535394140@qq.com)
 * @brief OPC UA 订阅者
 * @version 1.0
 * @date 2023-12-01
 *
 * @copyright Copyright 2023 (c), zhaoxi
 *
 */

#include "rmvl/opcua/subscriber.hpp"

#ifdef UA_ENABLE_PUBSUB

#include <open62541/plugin/log_stdout.h>
#include <open62541/plugin/pubsub_udp.h>
#include <open62541/server_config_default.h>

#ifdef UA_ENABLE_PUBSUB_MQTT
#include <open62541/plugin/pubsub_mqtt.h>
#endif // UA_ENABLE_PUBSUB_MQTT

#include "rmvl/core/util.hpp"
#include "rmvl/opcua/utilities.hpp"

#include "rmvlpara/opcua.hpp"

namespace rm
{

Subscriber<TransportID::UDP_UADP>::Subscriber(const std::string &sub_name, const std::string &address, uint16_t port,
                                              const std::vector<UserConfig> &users) : Server(port, users), _name(sub_name)
{
    //////////////////// 添加连接配置 ////////////////////
    UA_PubSubConnectionConfig connect_config{};
    connect_config.name = UA_String_fromChars((_name + "Connection").c_str());
    connect_config.transportProfileUri = UA_String_fromChars("http://opcfoundation.org/UA-Profile/Transport/pubsub-udp-uadp");
    connect_config.enabled = UA_TRUE;
    UA_NetworkAddressUrlDataType address_url{UA_STRING_NULL, UA_String_fromChars(address.c_str())};
    UA_Variant_setScalarCopy(&connect_config.address, &address_url, &UA_TYPES[UA_TYPES_NETWORKADDRESSURLDATATYPE]);
    connect_config.publisherId.numeric = UA_UInt32_random();
    auto status = UA_Server_addPubSubConnection(_server, &connect_config, &_connection_id);
    if (status != UA_STATUSCODE_GOOD)
    {
        UA_LOG_ERROR(UA_Log_Stdout, UA_LOGCATEGORY_SERVER, "Failed to add connection, \"%s\"", UA_StatusCode_name(status));
        return;
    }
}

bool Subscriber<TransportID::UDP_UADP>::subscribe(const std::string &pub_name, const std::vector<FieldMetaData> &fields)
{
    //////////////// 添加 ReaderGroup (RG) ///////////////
    UA_ReaderGroupConfig rg_config{};
    rg_config.name = UA_String_fromChars((pub_name + "ReaderGroup").c_str());
    auto status = UA_Server_addReaderGroup(_server, _connection_id, &rg_config, &_rg_id);
    if (status != UA_STATUSCODE_GOOD)
    {
        UA_LOG_ERROR(UA_Log_Stdout, UA_LOGCATEGORY_SERVER, "Failed to add reader group, \"%s\"", UA_StatusCode_name(status));
        return false;
    }
    status = UA_Server_setReaderGroupOperational(_server, _rg_id);
    if (status != UA_STATUSCODE_GOOD)
    {
        UA_LOG_ERROR(UA_Log_Stdout, UA_LOGCATEGORY_SERVER, "Failed to set reader group operational, \"%s\"",
                     UA_StatusCode_name(status));
        return false;
    }

    ////////////// 添加 DataSetReader (DSR) //////////////
    UA_DataSetReaderConfig dsr_config{};
    dsr_config.name = UA_String_fromChars((pub_name + "DataSetWriter").c_str());
    UA_UInt16 publisher_id = _strhash(pub_name + "Connection") % 0x8000000u;
    UA_Variant_setScalar(&dsr_config.publisherId, &publisher_id, &UA_TYPES[UA_TYPES_UINT16]);
    dsr_config.writerGroupId = _strhash(pub_name + "WriterGroup") % 0x8000u;
    dsr_config.dataSetWriterId = _strhash(pub_name + "DataSetWriter") % 0x8000u;

    // 设置数 DSR 中的元数据配置
    dsr_config.dataSetMetaData.name = UA_String_fromChars((_name + "DataSetMetaData").c_str());
    std::vector<UA_FieldMetaData> raw_fields(fields.size());
    for (size_t i = 0; i < fields.size(); i++)
    {
        UA_NodeId_copy(&UA_TYPES[fields[i].type].typeId, &raw_fields[i].dataType);
        raw_fields[i].builtInType = typeflag_ns0[fields[i].type];
        raw_fields[i].name = UA_String_fromChars(fields[i].name.c_str());
        raw_fields[i].valueRank = fields[i].value_rank;
    }
    dsr_config.dataSetMetaData.fieldsSize = raw_fields.size();
    dsr_config.dataSetMetaData.fields = raw_fields.data();    

    status = UA_Server_addDataSetReader(_server, _rg_id, &dsr_config, &_dsr_id);
    return true;
}

} // namespace rm

#endif // UA_ENABLE_PUBSUB

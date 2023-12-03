/**
 * @file publisher.cpp
 * @author zhaoxi (535394140@qq.com)
 * @brief OPC UA 发布者
 * @version 1.0
 * @date 2023-12-01
 *
 * @copyright Copyright 2023 (c), zhaoxi
 *
 */

#include "rmvl/opcua/publisher.hpp"

#ifdef UA_ENABLE_PUBSUB

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

#ifndef UA_ENABLE_PUBSUB_MQTT
static UA_PubSubTransportLayer UA_PubSubTransportLayerMQTT_nullptr()
{
    RMVL_Error(RMVL_StsInvFmt, "MQTT transport layer is not enabled, please use another transport layer like UDP.");
    return UA_PubSubTransportLayer{};
}
#endif // UA_ENABLE_PUBSUB_MQTT

// 传输协议 URI 映射
static constexpr const char *tp_profile[] = {
    "http://opcfoundation.org/UA-Profile/Transport/pubsub-udp-uadp",
    "http://opcfoundation.org/UA-Profile/Transport/pubsub-mqtt-uadp",
    "http://opcfoundation.org/UA-Profile/Transport/pubsub-mqtt-json",
};

// 传输层 URI 映射
static constexpr UA_PubSubTransportLayer (*tp_layer[])() = {
    UA_PubSubTransportLayerUDPMP,
#ifdef UA_ENABLE_PUBSUB_MQTT
    UA_PubSubTransportLayerMQTT, UA_PubSubTransportLayerMQTT
#else
    UA_PubSubTransportLayerMQTT_nullptr, UA_PubSubTransportLayerMQTT_nullptr
#endif // UA_ENABLE_PUBSUB_MQTT
};

Publisher::Publisher(uint16_t port, const std::string &name, const std::string &address, double duration, TransportID tp_id,
                     const std::vector<UserConfig> &users) : Server(port, users)
{
    //////////////////// 添加连接配置 ////////////////////
    UA_ServerConfig_addPubSubTransportLayer(UA_Server_getConfig(_server), tp_layer[static_cast<size_t>(tp_id)]());
    UA_PubSubConnectionConfig connect_config{};
    connect_config.name = UA_String_fromChars((name + " Connection").c_str());
    connect_config.transportProfileUri = UA_String_fromChars(tp_profile[static_cast<size_t>(tp_id)]);
    connect_config.enabled = UA_TRUE;
    UA_NetworkAddressUrlDataType address_url{UA_STRING_NULL, UA_String_fromChars(address.c_str())};
    UA_Variant_setScalarCopy(&connect_config.address, &address_url, &UA_TYPES[UA_TYPES_NETWORKADDRESSURLDATATYPE]);
    // 用哈希值作为发布者 ID
    connect_config.publisherId.numeric = std::hash<std::string>{}(name + " Connection") % 0x8000000u;
    UA_Server_addPubSubConnection(_server, &connect_config, &_connection_id);
    //////////// 添加 PublishedDataSet (PDS) /////////////
    UA_PublishedDataSetConfig pds_config{};
    pds_config.publishedDataSetType = UA_PUBSUB_DATASET_PUBLISHEDITEMS;
    pds_config.name = UA_String_fromChars((name + " PublishedDataSet").c_str());
    UA_Server_addPublishedDataSet(_server, &pds_config, &_pds_id);
    //////////////// 添加 WriterGroup (WG) ///////////////
    UA_WriterGroupConfig wg_config{};
    wg_config.name = UA_String_fromChars((name + " WriterGroup").c_str());
    wg_config.publishingInterval = duration;
    wg_config.enabled = UA_FALSE;
    wg_config.writerGroupId = std::hash<std::string>{}(name + " WriterGroup") % 0x8000u;
    wg_config.encodingMimeType = UA_PUBSUB_ENCODING_UADP;
    wg_config.messageSettings.encoding = UA_EXTENSIONOBJECT_DECODED;
    wg_config.messageSettings.content.decoded.type = &UA_TYPES[UA_TYPES_UADPWRITERGROUPMESSAGEDATATYPE];
    // 将写入组的消息设置更改为在网络消息的发布者 ID、写入组 ID 和数据集写入器 ID 中发送
    UA_UadpWriterGroupMessageDataType wg_msg{};
    wg_msg.networkMessageContentMask = UA_UADPNETWORKMESSAGECONTENTMASK_PUBLISHERID |
                                       UA_UADPNETWORKMESSAGECONTENTMASK_GROUPHEADER |
                                       UA_UADPNETWORKMESSAGECONTENTMASK_WRITERGROUPID |
                                       UA_UADPNETWORKMESSAGECONTENTMASK_PAYLOADHEADER;
    wg_config.messageSettings.content.decoded.data = &wg_msg;
    UA_Server_addWriterGroup(_server, _connection_id, &wg_config, &_wg_id);
    UA_Server_setWriterGroupOperational(_server, _wg_id);
    ////////////// 添加 DataSetWriter (DSW) //////////////
    UA_DataSetWriterConfig dsw_config{};
    dsw_config.name = UA_String_fromChars((name + " DataSetWriter").c_str());
    dsw_config.dataSetWriterId = std::hash<std::string>{}(name + " WriterGroup") % 0x8000u;
    dsw_config.keyFrameCount = para::opcua_param.KEY_FRAME_COUNT;
    UA_Server_addDataSetWriter(_server, _wg_id, _pds_id, &dsw_config, &_dsw_id);
}

bool Publisher::publish(std::string_view name, const UA_NodeId &node_id)
{
    ////////////// 添加 DataSetField (DSF) ///////////////
    UA_DataSetFieldConfig dsf_config{};
    dsf_config.dataSetFieldType = UA_PUBSUB_DATASETFIELD_VARIABLE;
    dsf_config.field.variable.fieldNameAlias = UA_String_fromChars(name.data());
    dsf_config.field.variable.promotedField = false;
    dsf_config.field.variable.publishParameters.publishedVariable = node_id;
    dsf_config.field.variable.publishParameters.attributeId = UA_ATTRIBUTEID_VALUE;

    UA_NodeId dsf_node_id;
    auto result = UA_Server_addDataSetField(_server, _pds_id, &dsf_config, &dsf_node_id);
    return result.result == UA_STATUSCODE_GOOD;
}

} // namespace rm

#endif // UA_ENABLE_PUBSUB

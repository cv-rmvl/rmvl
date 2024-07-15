/**
 * @file client.cpp
 * @author zhaoxi (535394140@qq.com)
 * @brief OPC UA 客户端
 * @version 1.1
 * @date 2024-03-29
 *
 * @copyright Copyright 2023 (c), zhaoxi
 *
 */

#include <open62541/client.h>
#include <open62541/client_config_default.h>
#include <open62541/client_highlevel.h>
#include <open62541/plugin/log_stdout.h>

#include "rmvl/opcua/client.hpp"
#include "rmvlpara/opcua.hpp"

#include "cvt.hpp"

namespace rm
{

////////////////////////// 通用配置 //////////////////////////

Client::Client()
{
    UA_ClientConfig init_config{};
    // 修改日志
#if OPCUA_VERSION < 10400
    init_config.logger = UA_Log_Stdout_withLevel(UA_LOGLEVEL_ERROR);
#else
    init_config.logging = UA_Log_Stdout_new(UA_LOGLEVEL_ERROR);
#endif
    // 设置默认配置
    auto status = UA_ClientConfig_setDefault(&init_config);
    if (status != UA_STATUSCODE_GOOD)
    {
        ERROR_("Failed to create client: %s", UA_StatusCode_name(status));
        UA_Client_delete(_client);
        _client = nullptr;
        return;
    }
    _client = UA_Client_newWithConfig(&init_config);
}

Client::Client(std::string_view address, const UserConfig &usr) : Client()
{
    if (!connect(address, usr))
    {
        ERROR_("Failed to connect to the server: %s", address.data());
        UA_Client_delete(_client);
        _client = nullptr;
    }
}

Client::~Client()
{
    if (_client != nullptr)
    {
        disconnect();
        UA_Client_delete(_client);
    }
}

bool Client::connect(std::string_view address, const UserConfig &usr)
{
    if (usr.id.empty() || usr.passwd.empty())
        return UA_Client_connect(_client, address.data()) == UA_STATUSCODE_GOOD;
    else
        return UA_Client_connectUsername(_client, address.data(), usr.id.c_str(), usr.passwd.c_str()) == UA_STATUSCODE_GOOD;
}

void Client::disconnect()
{
    if (_client == nullptr)
        return;
    auto status = UA_Client_disconnect(_client);
    if (status != UA_STATUSCODE_GOOD)
        WARNING_("Failed to disconnect the client");
}

void Client::spin() const
{
    bool warning{};
    while (true)
    {
        auto status = UA_Client_run_iterate(_client, para::opcua_param.SPIN_TIMEOUT);
        if (!warning && status != UA_STATUSCODE_GOOD)
        {
            WARNING_("No events and message received, spinning indefinitely, error status: %s", UA_StatusCode_name(status));
            warning = true;
        }
        warning = (status == UA_STATUSCODE_GOOD) ? false : warning;
    }
}

void Client::spinOnce() const
{
    UA_Client_run_iterate(_client, para::opcua_param.SPIN_TIMEOUT);
}

////////////////////////// 功能配置 //////////////////////////

Variable Client::read(const NodeId &node) const
{
    RMVL_DbgAssert(_client != nullptr);

    UA_Variant p_val;
    UA_Variant_init(&p_val);
    UA_StatusCode status = UA_Client_readValueAttribute(_client, node, &p_val);
    if (status != UA_STATUSCODE_GOOD)
        return {};
    Variable retval = helper::cvtVariable(p_val);
    UA_Variant_clear(&p_val);
    return retval;
}

bool Client::write(const NodeId &node, const Variable &val) const
{
    RMVL_DbgAssert(_client != nullptr);

    UA_Variant new_variant = helper::cvtVariable(val);
    auto status = UA_Client_writeValueAttribute(_client, node, &new_variant);
    UA_Variant_clear(&new_variant);
    if (status != UA_STATUSCODE_GOOD)
    {
        ERROR_("Failed to write value to the specific node, error: %s", UA_StatusCode_name(status));
        return false;
    }
    return true;
}

bool Client::call(const NodeId &obj_node, const std::string &name, const std::vector<Variable> &inputs, std::vector<Variable> &outputs) const
{
    RMVL_DbgAssert(_client != nullptr);

    // 初始化输入、输出参数
    std::vector<UA_Variant> input_variants;
    input_variants.reserve(inputs.size());
    for (const auto &input : inputs)
        input_variants.emplace_back(helper::cvtVariable(input));
    size_t output_size;
    UA_Variant *output_variants;
    // 获取方法节点
    NodeId method_node = obj_node | find(name);
    if (method_node.empty())
    {
        ERROR_("Failed to find the method node: %s", name.c_str());
        return false;
    }
    // 调用方法
    UA_StatusCode status = UA_Client_call(_client, obj_node, method_node, input_variants.size(),
                                          input_variants.data(), &output_size, &output_variants);
    for (auto &input_variant : input_variants)
        UA_Variant_clear(&input_variant);
    if (status != UA_STATUSCODE_GOOD)
    {
        ERROR_("Failed to call the method, node id: %d, error code: %s",
               method_node.nid.identifier.numeric, UA_StatusCode_name(status));
        return false;
    }
    outputs.reserve(output_size);
    for (size_t i = 0; i < output_size; ++i)
        outputs.push_back(helper::cvtVariable(output_variants[i]));
    for (size_t i = 0; i < output_size; ++i)
        UA_Variant_clear(&output_variants[i]);
    return true;
}

NodeId Client::addViewNode(const View &view) const
{
    RMVL_DbgAssert(_client != nullptr);

    // 准备数据
    NodeId retval;
    UA_ViewAttributes attr = UA_ViewAttributes_default;
    attr.displayName = UA_LOCALIZEDTEXT(helper::en_US(), helper::to_char(view.display_name));
    attr.description = UA_LOCALIZEDTEXT(helper::en_US(), helper::to_char(view.description));
    // 创建并添加 View 节点
    auto status = UA_Client_addViewNode(
        _client, UA_NODEID_NULL, nodeViewsFolder, nodeOrganizes,
        UA_QUALIFIEDNAME(view.ns, helper::to_char(view.browse_name)), attr, &retval);
    if (status != UA_STATUSCODE_GOOD)
    {
        ERROR_("Failed to add view node, error: %s", UA_StatusCode_name(status));
        return {};
    }
    // 添加引用
    for (const auto &node : view.data())
    {
        UA_ExpandedNodeId exp = UA_EXPANDEDNODEID_NULL;
        exp.nodeId = node;
        status = UA_Client_addReference(_client, retval, nodeOrganizes, true, UA_STRING_NULL, exp, UA_NODECLASS_VARIABLE);
        if (status != UA_STATUSCODE_GOOD)
        {
            ERROR_("Failed to add reference, error: %s", UA_StatusCode_name(status));
            return {};
        }
    }

    return retval;
}

bool Client::monitor(NodeId node, UA_Client_DataChangeNotificationCallback on_change, uint32_t queue_size) const
{
    RMVL_DbgAssert(_client != nullptr);

    // 创建订阅
    UA_CreateSubscriptionResponse resp;
    auto status = createSubscription(resp);
    if (!status)
        return false;
    // 创建监视项请求
    UA_MonitoredItemCreateRequest request = UA_MonitoredItemCreateRequest_default(node);
    request.requestedParameters.samplingInterval = para::opcua_param.SAMPLING_INTERVAL;
    request.requestedParameters.discardOldest = true;
    request.requestedParameters.queueSize = queue_size;
    // 创建监视器
    UA_MonitoredItemCreateResult result = UA_Client_MonitoredItems_createDataChange(
        _client, resp.subscriptionId, UA_TIMESTAMPSTORETURN_BOTH, request, &node, on_change, nullptr);
    if (result.statusCode != UA_STATUSCODE_GOOD)
    {
        ERROR_("Failed to create variable monitor, error: %s", UA_StatusCode_name(result.statusCode));
        return false;
    }
    return true;
}

bool Client::monitor(NodeId node, const std::vector<std::string> &names, UA_Client_EventNotificationCallback on_event) const
{
    RMVL_DbgAssert(_client != nullptr);
    // 创建订阅
    UA_CreateSubscriptionResponse sub_resp;
    if (!createSubscription(sub_resp))
        return false;

    UA_MonitoredItemCreateRequest request_item;
    UA_MonitoredItemCreateRequest_init(&request_item);
    request_item.itemToMonitor.nodeId = node;
    request_item.itemToMonitor.attributeId = UA_ATTRIBUTEID_EVENTNOTIFIER;
    request_item.monitoringMode = UA_MONITORINGMODE_REPORTING;
    // 准备 BrowseName 列表
    std::vector<UA_QualifiedName> browse_names(names.size());
    for (size_t i = 0; i < browse_names.size(); ++i)
    {
        uint16_t ns = (names[i] == "SourceName" || names[i] == "Message" || names[i] == "Severity") ? 0 : 1;
        browse_names[i] = UA_QUALIFIEDNAME(ns, helper::to_char(names[i]));
    }
    // 准备 select_clauses 列表
    std::vector<UA_SimpleAttributeOperand> select_clauses(browse_names.size());
    for (size_t i = 0; i < browse_names.size(); ++i)
    {
        select_clauses[i].typeDefinitionId = nodeBaseEventType;
        select_clauses[i].attributeId = UA_ATTRIBUTEID_VALUE;
        select_clauses[i].browsePathSize = 1;
        select_clauses[i].browsePath = &browse_names[i];
    }
    // 创建事件过滤器
    UA_EventFilter filter;
    UA_EventFilter_init(&filter);
    filter.selectClauses = select_clauses.data();
    filter.selectClausesSize = select_clauses.size();
    request_item.requestedParameters.filter.encoding = UA_EXTENSIONOBJECT_DECODED;
    request_item.requestedParameters.filter.content.decoded.data = &filter;
    request_item.requestedParameters.filter.content.decoded.type = &UA_TYPES[UA_TYPES_EVENTFILTER];
    // 创建事件监视器
    UA_UInt32 monitor_id{};
    UA_MonitoredItemCreateResult result = UA_Client_MonitoredItems_createEvent(
        _client, sub_resp.subscriptionId, UA_TIMESTAMPSTORETURN_BOTH, request_item, &monitor_id, on_event, nullptr);
    if (result.statusCode != UA_STATUSCODE_GOOD)
    {
        ERROR_("Failed to create event monitor, error: %s", UA_StatusCode_name(result.statusCode));
        return false;
    }
    else
        return true;
}

bool Client::createSubscription(UA_CreateSubscriptionResponse &response) const
{
    UA_CreateSubscriptionRequest request = UA_CreateSubscriptionRequest_default();
    request.requestedPublishingInterval = para::opcua_param.PUBLISHING_INTERVAL;
    request.requestedLifetimeCount = para::opcua_param.LIFETIME_COUNT;
    request.requestedMaxKeepAliveCount = para::opcua_param.MAX_KEEPALIVE_COUNT;
    request.maxNotificationsPerPublish = para::opcua_param.MAX_NOTIFICATIONS;
    request.publishingEnabled = true;
    request.priority = para::opcua_param.PRIORITY;

    response = UA_Client_Subscriptions_create(_client, request, nullptr, nullptr, nullptr);
    if (response.responseHeader.serviceResult != UA_STATUSCODE_GOOD)
    {
        ERROR_("Failed to create subscription, error: %s", UA_StatusCode_name(response.responseHeader.serviceResult));
        return false;
    }
    return true;
}

} // namespace rm

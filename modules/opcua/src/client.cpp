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

#include "rmvl/core/str.hpp"
#include "rmvl/core/util.hpp"
#include "rmvl/opcua/client.hpp"
#include "rmvlpara/opcua.hpp"

#include "cvt.hpp"

namespace rm {

static const std::unordered_map<para::LogLevel, UA_LogLevel> loglvl_cli{
    {para::LogLevel::LOG_TRACE, UA_LOGLEVEL_TRACE},
    {para::LogLevel::LOG_DEBUG, UA_LOGLEVEL_DEBUG},
    {para::LogLevel::LOG_INFO, UA_LOGLEVEL_INFO},
    {para::LogLevel::LOG_WARNING, UA_LOGLEVEL_WARNING},
    {para::LogLevel::LOG_ERROR, UA_LOGLEVEL_ERROR},
    {para::LogLevel::LOG_FATAL, UA_LOGLEVEL_FATAL},
};

////////////////////////// 通用配置 //////////////////////////

OpcuaClient::OpcuaClient(std::string_view address, const UserConfig &usr) {
    UA_ClientConfig init_config{};
    // 修改日志
    init_config.logging = UA_Log_Stdout_new(loglvl_cli.at(para::opcua_param.CLIENT_LOGLEVEL));
    // 设置默认配置
    auto status = UA_ClientConfig_setDefault(&init_config);
    if (status != UA_STATUSCODE_GOOD) {
        ERROR_("Failed to create client: %s", UA_StatusCode_name(status));
        UA_Client_delete(_client);
        _client = nullptr;
        return;
    }

    // 其余配置
    init_config.timeout = para::opcua_param.CONNECT_TIMEOUT;

    _client = UA_Client_newWithConfig(&init_config);

    if (usr.id.empty() || usr.passwd.empty())
        status = UA_Client_connect(_client, address.data());
    else
        status = UA_Client_connectUsername(_client, address.data(), usr.id.c_str(), usr.passwd.c_str());
    if (status != UA_STATUSCODE_GOOD) {
        ERROR_("Failed to connect to the server: %s", address.data());
        UA_Client_delete(_client);
        _client = nullptr;
    }
}

bool OpcuaClient::shutdown() {
    if (_client == nullptr)
        return false;
    auto status = UA_Client_disconnect(_client);
    if (status != UA_STATUSCODE_GOOD) {
        ERROR_("Failed to disconnect the client: %s", UA_StatusCode_name(status));
        return false;
    }
    UA_Client_delete(_client);
    _client = nullptr;
    return true;
}

OpcuaClient::~OpcuaClient() {
    if (_client != nullptr)
        shutdown();
}

void OpcuaClient::spin() const {
    bool warning{};
    while (true) {
        auto status = UA_Client_run_iterate(_client, para::opcua_param.CLIENT_WAIT_TIMEOUT);
        if (!warning && status != UA_STATUSCODE_GOOD) {
            WARNING_("No events and message received, spinning indefinitely, error status: %s", UA_StatusCode_name(status));
            warning = true;
        }
        warning = (status == UA_STATUSCODE_GOOD) ? false : warning;
    }
}

void OpcuaClient::spinOnce() const {
    UA_Client_run_iterate(_client, para::opcua_param.CLIENT_WAIT_TIMEOUT);
}

////////////////////////// 功能配置 //////////////////////////

static NodeId clientFindNode(UA_Client *p_cli, std::string_view browse_path, const NodeId &src_nd) {
    RMVL_DbgAssert(p_cli != nullptr);

    auto paths = str::split(browse_path, "/");
    if (paths.empty())
        return src_nd;
    OpcuaClientView cv{p_cli};
    NodeId retval = src_nd;
    for (const auto &path : paths) {
        retval = retval | cv.node(path);
        if (retval.empty())
            break;
    }
    return retval;
}

static Variable clientRead(UA_Client *p_client, const NodeId &node) {
    RMVL_DbgAssert(p_client != nullptr);

    UA_Variant p_val;
    UA_Variant_init(&p_val);
    UA_StatusCode status = UA_Client_readValueAttribute(p_client, node, &p_val);
    if (status != UA_STATUSCODE_GOOD)
        return {};
    Variable retval = helper::cvtVariable(p_val);
    UA_Variant_clear(&p_val);
    return retval;
}

static bool clientWrite(UA_Client *p_client, const NodeId &node, const Variable &val) {
    RMVL_DbgAssert(p_client != nullptr);

    UA_Variant new_variant = helper::cvtVariable(val);
    auto status = UA_Client_writeValueAttribute(p_client, node, &new_variant);
    UA_Variant_clear(&new_variant);
    if (status != UA_STATUSCODE_GOOD) {
        ERROR_("Failed to write value to the specific node, error: %s", UA_StatusCode_name(status));
        return false;
    }
    return true;
}

NodeId OpcuaClient::find(std::string_view browse_path, const NodeId &src_nd) const noexcept { return clientFindNode(_client, browse_path, src_nd); }

Variable OpcuaClient::read(const NodeId &nd) const { return clientRead(_client, nd); }

bool OpcuaClient::write(const NodeId &nd, const Variable &val) const { return clientWrite(_client, nd, val); }

std::pair<bool, Variables> OpcuaClient::call(const NodeId &obj_nd, std::string_view name, const Variables &inputs) const {
    RMVL_DbgAssert(_client != nullptr);

    // 初始化输入、输出参数
    std::vector<UA_Variant> input_variants{};
    input_variants.reserve(inputs.size());
    for (const auto &input : inputs)
        input_variants.emplace_back(helper::cvtVariable(input));
    size_t output_size{};
    UA_Variant *output_variants{};
    // 获取方法节点
    NodeId method_node = obj_nd | node(name);
    if (method_node.empty()) {
        ERROR_("Failed to find the method node: %s", name.data());
        return {false, {}};
    }
    // 调用方法
    UA_StatusCode status = UA_Client_call(_client, obj_nd, method_node, input_variants.size(),
                                          input_variants.data(), &output_size, &output_variants);
    for (auto &input_variant : input_variants)
        UA_Variant_clear(&input_variant);
    if (status != UA_STATUSCODE_GOOD) {
        ERROR_("Failed to call the method, node id: %d, error code: %s",
               method_node.data().identifier.numeric, UA_StatusCode_name(status));
        return {false, {}};
    }
    rm::Variables outputs{};
    outputs.reserve(output_size);
    for (size_t i = 0; i < output_size; ++i)
        outputs.push_back(helper::cvtVariable(output_variants[i]));
    for (size_t i = 0; i < output_size; ++i)
        UA_Variant_clear(&output_variants[i]);
    return {true, outputs};
}

std::pair<bool, Variables> OpcuaClient::findcall(std::string_view name, const Variables &inputs) const {
    RMVL_DbgAssert(_client != nullptr);

    auto pos = name.find_last_of('/');
    if (pos == std::string_view::npos) 
        return call(nodeObjectsFolder, name, inputs);
    std::string_view obj_path = name.substr(0, pos);
    std::string_view method_name =  name.substr(pos + 1);
    NodeId obj_nd = find(obj_path);
    if (obj_nd.empty()) {
        std::string s_obj_path(obj_path);
        ERROR_("Failed to find the object node: %s", s_obj_path.c_str());
        return {false, {}};
    }
    return call(obj_nd, method_name, inputs);
}

NodeId OpcuaClient::addViewNode(const View &view) const {
    RMVL_DbgAssert(_client != nullptr);

    // 准备数据
    NodeId retval;
    UA_ViewAttributes attr = UA_ViewAttributes_default;
    attr.displayName = UA_LOCALIZEDTEXT(helper::en_US(), helper::to_char(view.display_name));
    attr.description = UA_LOCALIZEDTEXT(helper::en_US(), helper::to_char(view.description));
    // 创建并添加 View 节点
    UA_NodeId out_new_nd{};
    auto status = UA_Client_addViewNode(
        _client, UA_NODEID_NULL, nodeViewsFolder, nodeOrganizes,
        UA_QUALIFIEDNAME(view.ns, helper::to_char(view.browse_name)), attr, &out_new_nd);
    if (status != UA_STATUSCODE_GOOD) {
        ERROR_("Failed to add view node, error: %s", UA_StatusCode_name(status));
        return {};
    }
    retval = out_new_nd;
    // 添加引用
    for (const auto &node : view.data()) {
        UA_ExpandedNodeId exp = UA_EXPANDEDNODEID_NULL;
        exp.nodeId = node;
        status = UA_Client_addReference(_client, retval, nodeOrganizes, true, UA_STRING_NULL, exp, UA_NODECLASS_VARIABLE);
        if (status != UA_STATUSCODE_GOOD) {
            ERROR_("Failed to add reference, error: %s", UA_StatusCode_name(status));
            return {};
        }
    }

    return retval;
}

static bool createSubscription(UA_Client *p_client, UA_CreateSubscriptionResponse &response) {
    UA_CreateSubscriptionRequest request = UA_CreateSubscriptionRequest_default();
    request.requestedPublishingInterval = para::opcua_param.PUBLISHING_INTERVAL;
    request.requestedLifetimeCount = para::opcua_param.LIFETIME_COUNT;
    request.requestedMaxKeepAliveCount = para::opcua_param.MAX_KEEPALIVE_COUNT;
    request.maxNotificationsPerPublish = para::opcua_param.MAX_NOTIFICATIONS;
    request.publishingEnabled = true;
    request.priority = para::opcua_param.PRIORITY;

    response = UA_Client_Subscriptions_create(p_client, request, nullptr, nullptr, nullptr);
    if (response.responseHeader.serviceResult != UA_STATUSCODE_GOOD) {
        ERROR_("Failed to create subscription, error: %s", UA_StatusCode_name(response.responseHeader.serviceResult));
        return false;
    }
    return true;
}

static void data_change_notify_cb(UA_Client *client, UA_UInt32, void *, UA_UInt32, void *context, UA_DataValue *value) {
    auto &on_change = *reinterpret_cast<DataChangeNotificationCallback *>(context);
    on_change(client, helper::cvtVariable(value->value));
}

bool OpcuaClient::monitor(NodeId nd, DataChangeNotificationCallback on_change, uint32_t q_size) {
    RMVL_DbgAssert(_client != nullptr);

    // 创建订阅
    UA_CreateSubscriptionResponse resp;
    if (!createSubscription(_client, resp))
        return false;
    // 创建监视项请求
    UA_MonitoredItemCreateRequest request = UA_MonitoredItemCreateRequest_default(nd);
    request.requestedParameters.samplingInterval = para::opcua_param.SAMPLING_INTERVAL;
    request.requestedParameters.discardOldest = true;
    request.requestedParameters.queueSize = q_size;
    // 创建监视器
    auto context = std::make_unique<DataChangeNotificationCallback>(on_change);
    UA_MonitoredItemCreateResult result = UA_Client_MonitoredItems_createDataChange(
        _client, resp.subscriptionId, UA_TIMESTAMPSTORETURN_BOTH, request, context.get(), data_change_notify_cb, nullptr);
    if (result.statusCode != UA_STATUSCODE_GOOD) {
        ERROR_("Failed to create variable monitor, error: %s", UA_StatusCode_name(result.statusCode));
        return false;
    }
    _dccb_gc.push_back(std::move(context));
    _monitor_map[nd.data().identifier.numeric] = {resp.subscriptionId, result.monitoredItemId};
    return true;
}

static void event_notify_cb(UA_Client *client, UA_UInt32, void *, UA_UInt32, void *context, size_t events_num, UA_Variant *event_fields) {
    auto &on_event = *reinterpret_cast<EventNotificationCallback *>(context);
    std::vector<Variable> datas(events_num);
    for (size_t i = 0; i < events_num; ++i)
        datas[i] = helper::cvtVariable(event_fields[i]);
    on_event(client, datas);
}

bool OpcuaClient::monitor(const std::vector<std::string> &names, EventNotificationCallback on_event) {
    RMVL_DbgAssert(_client != nullptr);
    // 创建订阅
    UA_CreateSubscriptionResponse sub_resp;
    if (!createSubscription(_client, sub_resp))
        return false;

    UA_MonitoredItemCreateRequest request_item;
    UA_MonitoredItemCreateRequest_init(&request_item);

    auto nd = UA_NODEID_NUMERIC(0, UA_NS0ID_SERVER);
    request_item.itemToMonitor.nodeId = nd;
    request_item.itemToMonitor.attributeId = UA_ATTRIBUTEID_EVENTNOTIFIER;
    request_item.monitoringMode = UA_MONITORINGMODE_REPORTING;
    // 准备 BrowseName 列表
    std::vector<UA_QualifiedName> browse_names(names.size());
    for (size_t i = 0; i < browse_names.size(); ++i) {
        uint16_t ns = (names[i] == "SourceName" || names[i] == "Message" || names[i] == "Severity") ? 0 : 1;
        browse_names[i] = UA_QUALIFIEDNAME(ns, helper::to_char(names[i]));
    }
    // 准备 select_clauses 列表
    std::vector<UA_SimpleAttributeOperand> select_clauses(browse_names.size());
    for (size_t i = 0; i < browse_names.size(); ++i) {
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
    auto context = std::make_unique<EventNotificationCallback>(on_event);
    UA_MonitoredItemCreateResult result = UA_Client_MonitoredItems_createEvent(
        _client, sub_resp.subscriptionId, UA_TIMESTAMPSTORETURN_BOTH, request_item, context.get(), event_notify_cb, nullptr);
    if (result.statusCode != UA_STATUSCODE_GOOD) {
        ERROR_("Failed to create event monitor, error: %s", UA_StatusCode_name(result.statusCode));
        return false;
    }
    _encb_gc.push_back(std::move(context));
    return true;
}

bool OpcuaClient::remove(NodeId nd) {
    if (_monitor_map.find(nd.data().identifier.numeric) == _monitor_map.end()) {
        ERROR_("Failed to find the monitor, node id: %d", nd.data().identifier.numeric);
        return false;
    }
    auto [sub_id, mon_id] = _monitor_map.at(nd.data().identifier.numeric);
    auto status = UA_Client_MonitoredItems_deleteSingle(_client, sub_id, mon_id);
    if (status != UA_STATUSCODE_GOOD) {
        ERROR_("Failed to remove monitor, error: %s", UA_StatusCode_name(status));
        return false;
    }
    _monitor_map.erase(nd.data().identifier.numeric);
    return true;
}

//////////////////////// 客户端视图 ////////////////////////

NodeId OpcuaClientView::find(std::string_view browse_path, const NodeId &src_nd) const noexcept { return clientFindNode(_client, browse_path, src_nd); }
Variable OpcuaClientView::read(const NodeId &nd) const { return clientRead(_client, nd); }
bool OpcuaClientView::write(const NodeId &nd, const Variable &val) const { return clientWrite(_client, nd, val); }

/////////////////////// 客户端定时器 ///////////////////////

static void timer_cb(UA_Client *, void *data) {
    auto &func = *reinterpret_cast<std::function<void()> *>(data);
    func();
}

OpcuaClientTimer::OpcuaClientTimer(OpcuaClientView cv, double period, std::function<void()> callback) : _cv(cv), _cb(callback) {
    auto status = UA_Client_addRepeatedCallback(_cv.get(), timer_cb, &_cb, period, &_id);
    if (status != UA_STATUSCODE_GOOD) {
        ERROR_("Failed to add repeated callback: %s", UA_StatusCode_name(status));
        _id = 0;
    }
}

void OpcuaClientTimer::cancel() {
    if (_id != 0) {
        UA_Client_removeCallback(_cv.get(), _id);
        _id = 0;
    }
}

} // namespace rm

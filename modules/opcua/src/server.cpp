/**
 * @file server.cpp
 * @author zhaoxi (535394140@qq.com)
 * @brief OPC UA 服务器
 * @version 3.0
 * @date 2024-09-03
 *
 * @copyright Copyright 2023 (c), zhaoxi
 *
 */

#include <stack>

#include <open62541/plugin/accesscontrol_default.h>
#include <open62541/plugin/log_stdout.h>
#include <open62541/server_config_default.h>

#include "rmvl/opcua/server.hpp"
#include "rmvlpara/opcua.hpp"

#include "cvt.hpp"

namespace rm
{

///////////////////////// 基本配置 /////////////////////////

Server::Server(uint16_t port, std::string_view name, const std::vector<UserConfig> &users)
{
    UA_ServerConfig init_config{};
    // 修改日志
    static const std::unordered_map<para::LogLevel, UA_LogLevel> loglvl_srv{
        {para::LogLevel::LOG_TRACE, UA_LOGLEVEL_TRACE},
        {para::LogLevel::LOG_DEBUG, UA_LOGLEVEL_DEBUG},
        {para::LogLevel::LOG_INFO, UA_LOGLEVEL_INFO},
        {para::LogLevel::LOG_WARNING, UA_LOGLEVEL_WARNING},
        {para::LogLevel::LOG_ERROR, UA_LOGLEVEL_ERROR},
        {para::LogLevel::LOG_FATAL, UA_LOGLEVEL_FATAL},
    };

    init_config.logging = UA_Log_Stdout_new(loglvl_srv.at(para::opcua_param.SERVER_LOGLEVEL));
    // 设置服务器配置
    UA_ServerConfig_setMinimal(&init_config, port, nullptr);
    // 初始化服务器
    _server = UA_Server_newWithConfig(&init_config);
    UA_ServerConfig *config = UA_Server_getConfig(_server);
    // 修改名字
    if (!name.empty())
    {
        UA_LocalizedText_clear(&config->applicationDescription.applicationName);
        config->applicationDescription.applicationName = UA_LOCALIZEDTEXT_ALLOC("en-US", name.data());
        for (size_t i = 0; i < config->endpointsSize; ++i)
        {
            UA_LocalizedText *ptr = &config->endpoints[i].server.applicationName;
            UA_LocalizedText_clear(ptr);
            *ptr = UA_LOCALIZEDTEXT_ALLOC("en-US", name.data());
        }
    }
    // 修改采样间隔和发布间隔
    config->samplingIntervalLimits.min = 2.0;
    config->publishingIntervalLimits.min = 2.0;

    if (!users.empty())
    {
        std::vector<UA_UsernamePasswordLogin> usr_passwd;
        usr_passwd.reserve(users.size());
        for (const auto &[id, passwd] : users)
        {
            UA_UsernamePasswordLogin each;
            each.username = UA_STRING(helper::to_char(id));
            each.password = UA_STRING(helper::to_char(passwd));
            usr_passwd.emplace_back(each);
        }
        // 修改访问控制
        config->accessControl.clear(&config->accessControl);
        UA_AccessControl_default(config, false, nullptr, usr_passwd.size(), usr_passwd.data());
    }

    // 设置垃圾回收容量
    _vcb_gc.reserve(16);
    _dscb_gc.reserve(16);
    _mcb_gc.reserve(16);

    // 启动网络层
    UA_StatusCode retval = UA_Server_run_startup(_server);
    if (retval != UA_STATUSCODE_GOOD)
        ERROR_("Failed to initialize server: %s", UA_StatusCode_name(retval));
}

Server::Server(UA_StatusCode (*on_config)(UA_Server *), uint16_t port, std::string_view name, const std::vector<UserConfig> &users) : Server(port, name, users)
{
    if (on_config != nullptr)
        on_config(_server);
}

void Server::spinOnce() { UA_Server_run_iterate(_server, para::opcua_param.SERVER_WAIT); }

void Server::spin()
{
    _running = true;
    while (_running)
        UA_Server_run_iterate(_server, para::opcua_param.SERVER_WAIT);
}

Server::~Server()
{
    shutdown();
    UA_Server_run_shutdown(_server);
    UA_Server_delete(_server);
}

static NodeId serverFindNode(UA_Server *p_server, std::string_view browse_path, const NodeId &src_nd)
{
    RMVL_DbgAssert(p_server != nullptr);

    auto paths = helper::split(browse_path, '/');
    if (paths.empty())
        return src_nd;
    ServerView sv{p_server};
    NodeId retval = src_nd;
    for (const auto &path : paths)
    {
        retval = retval | sv.node(path);
        if (retval.empty())
            break;
    }
    return retval;
}

static Variable serverRead(UA_Server *p_server, const NodeId &nd)
{
    RMVL_DbgAssert(p_server != nullptr);

    UA_Variant p_val;
    UA_Variant_init(&p_val);
    auto status = UA_Server_readValue(p_server, nd, &p_val);
    if (status != UA_STATUSCODE_GOOD)
        return {};
    Variable retval = helper::cvtVariable(p_val);
    UA_Variant_clear(&p_val);
    return retval;
}

static bool serverWrite(UA_Server *p_server, const NodeId &nd, const Variable &val)
{
    RMVL_DbgAssert(p_server != nullptr);

    auto variant = helper::cvtVariable(val);
    auto status = UA_Server_writeValue(p_server, nd, variant);
    UA_Variant_clear(&variant);
    if (status != UA_STATUSCODE_GOOD)
        ERROR_("Failed to write variable, error code: %s", UA_StatusCode_name(status));
    return status == UA_STATUSCODE_GOOD;
}

static bool serverTriggerEvent(UA_Server *server, const Event &event)
{
    RMVL_DbgAssert(server != nullptr);

    ServerView sv{server};
    NodeId type_id = nodeBaseEventType | sv.node(event.type().browse_name);
    if (type_id.empty())
    {
        ERROR_("Failed to find the event type ID during triggering event");
        return false;
    }
    // 创建事件
    UA_NodeId event_id;
    auto status = UA_Server_createEvent(server, type_id, &event_id);
    if (status != UA_STATUSCODE_GOOD)
    {
        ERROR_("Failed to create event: %s", UA_StatusCode_name(status));
        return false;
    }

    // 设置事件默认属性
    UA_DateTime time = UA_DateTime_now();
    UA_String source_name = UA_STRING(helper::to_char(event.source_name));
    UA_LocalizedText evt_msg = UA_LOCALIZEDTEXT(helper::en_US(), helper::to_char(event.message));
    UA_Server_writeObjectProperty_scalar(server, event_id, UA_QUALIFIEDNAME(0, const_cast<char *>("Time")), &time, &UA_TYPES[UA_TYPES_DATETIME]);
    UA_Server_writeObjectProperty_scalar(server, event_id, UA_QUALIFIEDNAME(0, const_cast<char *>("SourceName")), &source_name, &UA_TYPES[UA_TYPES_STRING]);
    UA_Server_writeObjectProperty_scalar(server, event_id, UA_QUALIFIEDNAME(0, const_cast<char *>("Severity")), &event.severity, &UA_TYPES[UA_TYPES_UINT16]);
    UA_Server_writeObjectProperty_scalar(server, event_id, UA_QUALIFIEDNAME(0, const_cast<char *>("Message")), &evt_msg, &UA_TYPES[UA_TYPES_LOCALIZEDTEXT]);
    // 设置事件自定义属性
    for (const auto &[browse_name, prop] : event.data())
    {
        NodeId sub_nd = event_id | sv.node(browse_name);
        if (!sub_nd.empty())
            UA_Server_writeObjectProperty_scalar(server, event_id, UA_QUALIFIEDNAME(event.ns, helper::to_char(browse_name)),
                                                 &prop, &UA_TYPES[UA_TYPES_INT32]);
    }

    // 触发事件
    status = UA_Server_triggerEvent(server, event_id, UA_NODEID_NUMERIC(0, UA_NS0ID_SERVER), nullptr, true);
    if (status != UA_STATUSCODE_GOOD)
    {
        ERROR_("Failed to trigger event: %s", UA_StatusCode_name(status));
        return false;
    }
    return true;
}

///////////////////////// 节点配置 /////////////////////////

NodeId Server::find(std::string_view browse_path, const NodeId &src_nd) const noexcept { return serverFindNode(_server, browse_path, src_nd); }

NodeId Server::addVariableTypeNode(const VariableType &vtype)
{
    RMVL_DbgAssert(_server != nullptr);

    UA_VariableTypeAttributes attr = UA_VariableTypeAttributes_default;
    UA_Variant variant = helper::cvtVariable(vtype);
    // 设置属性
    attr.value = variant;
    attr.dataType = variant.type->typeId;
    attr.valueRank = vtype.size() == 1 ? UA_VALUERANK_SCALAR : 1;
    if (attr.valueRank != UA_VALUERANK_SCALAR)
    {
        attr.arrayDimensionsSize = variant.arrayDimensionsSize;
        attr.arrayDimensions = variant.arrayDimensions;
    }
    attr.description = UA_LOCALIZEDTEXT(helper::zh_CN(), helper::to_char(vtype.description));
    attr.displayName = UA_LOCALIZEDTEXT(helper::en_US(), helper::to_char(vtype.display_name));
    UA_NodeId retval{};
    auto status = UA_Server_addVariableTypeNode(
        _server, UA_NODEID_NULL, nodeBaseDataVariableType, nodeHasSubtype,
        UA_QUALIFIEDNAME(vtype.ns, helper::to_char(vtype.browse_name)),
        UA_NODEID_NULL, attr, nullptr, &retval);
    UA_Variant_clear(&variant);
    if (status != UA_STATUSCODE_GOOD)
    {
        ERROR_("Failed to add variable type node: %s", UA_StatusCode_name(status));
        return UA_NODEID_NULL;
    }
    return retval;
}

NodeId Server::addVariableNode(const Variable &val, const NodeId &parent_nd) noexcept
{
    RMVL_DbgAssert(_server != nullptr);

    // 变量节点属性 `UA_VariableAttributes`
    UA_VariableAttributes attr = UA_VariableAttributes_default;
    UA_Variant variant = helper::cvtVariable(val);
    // 设置属性
    attr.value = variant;
    attr.dataType = variant.type->typeId;
    attr.accessLevel = val.access_level;
    attr.valueRank = val.size() == 1 ? UA_VALUERANK_SCALAR : 1;
    if (attr.valueRank != UA_VALUERANK_SCALAR)
    {
        attr.arrayDimensionsSize = variant.arrayDimensionsSize;
        attr.arrayDimensions = variant.arrayDimensions;
    }
    attr.description = UA_LOCALIZEDTEXT(helper::zh_CN(), helper::to_char(val.description));
    attr.displayName = UA_LOCALIZEDTEXT(helper::en_US(), helper::to_char(val.display_name));
    // 获取变量节点的变量类型节点
    NodeId type_id{nodeBaseDataVariableType};
    const auto variable_type = val.type();
    if (!variable_type.empty())
    {
        type_id = type_id | node(variable_type.browse_name);
        if (type_id.empty())
        {
            ERROR_("Failed to find the variable type ID during adding variable node");
            type_id = nodeBaseDataVariableType;
        }
    }
    UA_NodeId retval;
    // 添加节点至服务器
    NodeId object_folder_id{nodeObjectsFolder};
    NodeId ref_id = (parent_nd == object_folder_id) ? nodeOrganizes : nodeHasComponent;
    auto status = UA_Server_addVariableNode(
        _server, UA_NODEID_NULL, parent_nd, ref_id, UA_QUALIFIEDNAME(val.ns, helper::to_char(val.browse_name)),
        type_id, attr, nullptr, &retval);
    if (status != UA_STATUSCODE_GOOD)
    {
        ERROR_("Failed to add variable node: %s", UA_StatusCode_name(status));
        return UA_NODEID_NULL;
    }
    UA_Variant_clear(&variant);
    return retval;
}

Variable Server::read(const NodeId &node) const { return serverRead(_server, node); }
bool Server::write(const NodeId &node, const Variable &val) { return serverWrite(_server, node, val); }

static void value_cb_before_read(UA_Server *server, const UA_NodeId *, void *, const UA_NodeId *nodeid,
                                 void *context, const UA_NumericRange *, const UA_DataValue *value)
{
    auto &on_read = reinterpret_cast<Server::ValueCallbackWrapper *>(context)->first;
    on_read(server, *nodeid, value->hasValue ? helper::cvtVariable(value->value) : Variable{});
}

static void value_cb_after_write(UA_Server *server, const UA_NodeId *, void *, const UA_NodeId *nodeId,
                                 void *context, const UA_NumericRange *, const UA_DataValue *data)
{
    auto &on_write = reinterpret_cast<Server::ValueCallbackWrapper *>(context)->second;
    on_write(server, *nodeId, data->hasValue ? helper::cvtVariable(data->value) : Variable{});
}

bool Server::addVariableNodeValueCallback(NodeId nd, ValueCallbackBeforeRead before_read, ValueCallbackAfterWrite after_write) noexcept
{
    RMVL_DbgAssert(_server != nullptr);
    // 设置节点上下文
    auto context = std::make_unique<ValueCallbackWrapper>(std::forward<ValueCallbackBeforeRead>(before_read),
                                                          std::forward<ValueCallbackAfterWrite>(after_write));
    auto status = UA_Server_setNodeContext(_server, nd, context.get());
    if (status != UA_STATUSCODE_GOOD)
    {
        ERROR_("Failed to set node context: %s", UA_StatusCode_name(status));
        return false;
    }
    _vcb_gc.push_back(std::move(context));
    // 设置回调函数
    UA_ValueCallback callback{value_cb_before_read, value_cb_after_write};
    status = UA_Server_setVariableNode_valueCallback(_server, nd, callback);
    if (status != UA_STATUSCODE_GOOD)
        ERROR_("Function addVariableNodeValueCallBack: %s", UA_StatusCode_name(status));
    return status == UA_STATUSCODE_GOOD;
}

static UA_StatusCode datasource_cb_on_read(UA_Server *, const UA_NodeId *, void *, const UA_NodeId *nodeid, void *context,
                                           UA_Boolean, const UA_NumericRange *, UA_DataValue *value)
{
    auto on_read = reinterpret_cast<Server::DataSourceCallbackWrapper *>(context)->first;
    if (on_read == nullptr)
        return UA_STATUSCODE_BADNOTSUPPORTED;
    auto retval = on_read(*nodeid);
    if (retval.empty())
        return UA_STATUSCODE_BADNOTFOUND;
    value->hasValue = true;
    value->value = helper::cvtVariable(retval);
    return UA_STATUSCODE_GOOD;
}

static UA_StatusCode datasource_cb_on_write(UA_Server *, const UA_NodeId *, void *, const UA_NodeId *nodeid, void *context,
                                            const UA_NumericRange *, const UA_DataValue *value)
{
    auto on_write = reinterpret_cast<Server::DataSourceCallbackWrapper *>(context)->second;
    if (on_write == nullptr)
        return UA_STATUSCODE_BADNOTSUPPORTED;
    on_write(*nodeid, value->hasValue ? helper::cvtVariable(value->value) : Variable{});
    return value->hasValue ? UA_STATUSCODE_GOOD : UA_STATUSCODE_BADNOTFOUND;
}

NodeId Server::addDataSourceVariableNode(const DataSourceVariable &val, NodeId parent_nd) noexcept
{
    RMVL_DbgAssert(_server != nullptr);

    // 设置变量节点属性
    UA_VariableAttributes attr = UA_VariableAttributes_default;
    attr.accessLevel = val.access_level;
    attr.displayName = UA_LOCALIZEDTEXT(helper::en_US(), helper::to_char(val.display_name));
    attr.description = UA_LOCALIZEDTEXT(helper::zh_CN(), helper::to_char(val.description));
    UA_NodeId retval;
    // 获取数据源重定向信息
    UA_DataSource data_source = {datasource_cb_on_read, datasource_cb_on_write};
    // 添加节点至服务器
    auto context = std::make_unique<DataSourceCallbackWrapper>(val.on_read, val.on_write);
    auto status = UA_Server_addDataSourceVariableNode(
        _server, UA_NODEID_NULL, parent_nd, nodeOrganizes, UA_QUALIFIEDNAME(val.ns, helper::to_char(val.browse_name)),
        UA_NODEID_NULL, attr, data_source, context.get(), &retval);
    if (status != UA_STATUSCODE_GOOD)
    {
        ERROR_("Failed to add data source variable node: %s", UA_StatusCode_name(status));
        return UA_NODEID_NULL;
    }
    // 设置节点上下文
    _dscb_gc.push_back(std::move(context));
    return retval;
}

static UA_StatusCode method_cb(UA_Server *server, const UA_NodeId *, void *, const UA_NodeId *, void *context, const UA_NodeId *object_id,
                               void *, size_t input_size, const UA_Variant *input, size_t output_size, UA_Variant *output)
{
    auto &on_method = *reinterpret_cast<MethodCallback *>(context);
    std::vector<Variable> iargs(input_size);
    for (size_t i = 0; i < input_size; ++i)
        iargs[i] = helper::cvtVariable(input[i]);
    auto [res, oargs] = on_method(server, *object_id, iargs);
    if (!res)
        return UA_STATUSCODE_BADINTERNALERROR;
    else
    {
        if (oargs.size() != output_size)
        {
            ERROR_("The number of output arguments does not match the number of input arguments");
            return UA_STATUSCODE_BADINVALIDARGUMENT;
        }
        for (size_t i = 0; i < output_size; ++i)
            output[i] = helper::cvtVariable(oargs[i]);
        return UA_STATUSCODE_GOOD;
    }
}

NodeId Server::addMethodNode(const Method &method, const NodeId &parent_nd)
{
    RMVL_DbgAssert(_server != nullptr);

    UA_MethodAttributes attr = UA_MethodAttributes_default;
    attr.displayName = UA_LOCALIZEDTEXT(helper::en_US(), helper::to_char(method.display_name));
    attr.description = UA_LOCALIZEDTEXT(helper::zh_CN(), helper::to_char(method.description));
    attr.executable = true;
    attr.userExecutable = true;
    // 提取数据至 `UA_Argument`
    std::vector<UA_Argument> inputs;
    inputs.reserve(method.iargs.size());
    for (const auto &arg : method.iargs)
        inputs.push_back(helper::cvtArgument(arg));
    std::vector<UA_Argument> outputs;
    outputs.reserve(method.oargs.size());
    for (const auto &arg : method.oargs)
        outputs.push_back(helper::cvtArgument(arg));
    // 添加节点
    auto context = std::make_unique<MethodCallback>(method.func);
    UA_NodeId retval;
    auto status = UA_Server_addMethodNode(
        _server, UA_NODEID_NULL, parent_nd, nodeHasComponent, UA_QUALIFIEDNAME(method.ns, helper::to_char(method.browse_name)),
        attr, method_cb, inputs.size(), inputs.data(), outputs.size(), outputs.data(), context.get(), &retval);
    if (status != UA_STATUSCODE_GOOD)
    {
        ERROR_("Failed to add method node: %s", UA_StatusCode_name(status));
        return UA_NODEID_NULL;
    }
    _mcb_gc.push_back(std::move(context));
    // 添加 Mandatory 属性
    status = UA_Server_addReference(_server, retval, nodeHasModellingRule,
                                    UA_EXPANDEDNODEID_NUMERIC(0, UA_NS0ID_MODELLINGRULE_MANDATORY), true);
    if (status != UA_STATUSCODE_GOOD)
    {
        ERROR_("Failed to add the \"Mandatory\" reference node: %s", UA_StatusCode_name(status));
        return UA_NODEID_NULL;
    }
    return retval;
}

bool Server::setMethodNodeCallBack(const NodeId &nd, MethodCallback on_method)
{
    RMVL_DbgAssert(_server != nullptr);
    auto context = std::make_unique<MethodCallback>(on_method);
    if (UA_Server_setNodeContext(_server, nd, context.get()))
    {
        ERROR_("Failed to set node context");
        return false;
    }
    auto ret = UA_Server_setMethodNodeCallback(_server, nd, method_cb);
    if (ret != UA_STATUSCODE_GOOD)
    {
        ERROR_("Failed to set method node callback: %s", UA_StatusCode_name(ret));
        return false;
    }
    _mcb_gc.push_back(std::move(context));
    return true;
}

NodeId Server::addObjectTypeNode(const ObjectType &otype)
{
    RMVL_DbgAssert(_server != nullptr);

    // 定义对象类型节点
    UA_ObjectTypeAttributes attr = UA_ObjectTypeAttributes_default;
    attr.displayName = UA_LOCALIZEDTEXT(helper::en_US(), helper::to_char(otype.display_name));
    attr.description = UA_LOCALIZEDTEXT(helper::zh_CN(), helper::to_char(otype.description));
    // 获取父节点的 NodeId
    NodeId parent_nd{nodeBaseObjectType};
    const ObjectType *current = otype.base();
    std::stack<std::string> base_stack;
    while (current != nullptr)
    {
        base_stack.push(current->browse_name);
        current = current->base();
    }
    while (!base_stack.empty())
    {
        parent_nd = parent_nd | node(base_stack.top());
        base_stack.pop();
    }
    if (parent_nd.empty())
    {
        ERROR_("Failed to find the base object type ID during adding object type node");
        parent_nd = nodeBaseObjectType;
    }
    UA_NodeId retval;
    auto status = UA_Server_addObjectTypeNode(
        _server, UA_NODEID_NULL, parent_nd,
        nodeHasSubtype,
        UA_QUALIFIEDNAME(otype.ns, helper::to_char(otype.browse_name)),
        attr, nullptr, &retval);
    if (status != UA_STATUSCODE_GOOD)
    {
        ERROR_("Failed to add object type: %s", UA_StatusCode_name(status));
        return UA_NODEID_NULL;
    }
    // 添加变量节点作为对象类型节点的子节点
    for (const auto &[browse_name, val] : otype.getVariables())
    {
        // 添加至服务器
        NodeId sub_retval = addVariableNode(val, retval);
        // 设置子变量节点为强制生成
        status = UA_Server_addReference(
            _server, sub_retval, nodeHasModellingRule,
            UA_EXPANDEDNODEID_NUMERIC(0, UA_NS0ID_MODELLINGRULE_MANDATORY), true);
        if (status != UA_STATUSCODE_GOOD)
        {
            ERROR_("Failed to add reference during adding object type node, browse name: %s, error code: %s", browse_name.c_str(), UA_StatusCode_name(status));
            return UA_NODEID_NULL;
        }
    }
    // 添加方法节点作为对象类型节点的子节点
    for (const auto &val : otype.getMethods())
        addMethodNode(val.second, retval);
    return retval;
}

NodeId Server::addObjectNode(const Object &obj, NodeId parent_nd)
{
    RMVL_DbgAssert(_server != nullptr);

    UA_ObjectAttributes attr{UA_ObjectAttributes_default};
    attr.displayName = UA_LOCALIZEDTEXT(helper::en_US(), helper::to_char(obj.display_name));
    attr.description = UA_LOCALIZEDTEXT(helper::zh_CN(), helper::to_char(obj.description));
    // 获取对象类型节点
    ObjectType obj_type = obj.type();
    const rm::ObjectType *current = &obj_type;
    NodeId type_id{nodeBaseObjectType};
    std::stack<std::string> base_stack;
    while (current != nullptr && !current->empty())
    {
        base_stack.push(current->browse_name);
        current = current->base();
    }
    while (!base_stack.empty())
    {
        type_id = type_id | node(base_stack.top());
        base_stack.pop();
    }
    if (type_id.empty())
    {
        WARNING_("The object node \"%s\" does not belong to any object type node", obj.browse_name.c_str());
        type_id = nodeBaseObjectType;
    }
    // 添加至服务器
    UA_NodeId retval;
    auto status = UA_Server_addObjectNode(
        _server, UA_NODEID_NULL, parent_nd, nodeOrganizes, UA_QUALIFIEDNAME(obj.ns, helper::to_char(obj.browse_name)),
        type_id, attr, nullptr, &retval);
    if (status != UA_STATUSCODE_GOOD)
    {
        ERROR_("Failed to add object node to server, error code: %s", UA_StatusCode_name(status));
        return UA_NODEID_NULL;
    }
    // 添加额外变量节点
    for (const auto &[browse_name, variable] : obj.getVariables())
    {
        auto sub_nd = retval | node(browse_name);
        if (!sub_nd.empty())
            write(sub_nd, variable);
        else
            addVariableNode(variable, retval);
    }
    // 添加额外方法节点
    for (const auto &[browse_name, method] : obj.getMethods())
    {
        auto sub_nd = retval | node(browse_name);
        if (!sub_nd.empty())
            setMethodNodeCallBack(sub_nd, method.func);
        else
            addMethodNode(method, retval);
    }
    return retval;
}

NodeId Server::addViewNode(const View &view)
{
    RMVL_DbgAssert(_server != nullptr);

    // 准备数据
    UA_NodeId retval;
    UA_ViewAttributes attr = UA_ViewAttributes_default;
    attr.displayName = UA_LOCALIZEDTEXT(helper::en_US(), helper::to_char(view.display_name));
    attr.description = UA_LOCALIZEDTEXT(helper::en_US(), helper::to_char(view.description));
    // 创建并添加 View 节点
    auto status = UA_Server_addViewNode(
        _server, UA_NODEID_NULL, nodeViewsFolder, nodeOrganizes,
        UA_QUALIFIEDNAME(view.ns, helper::to_char(view.browse_name)), attr, nullptr, &retval);
    if (status != UA_STATUSCODE_GOOD)
    {
        ERROR_("Failed to add view node, error: %s", UA_StatusCode_name(status));
        return UA_NODEID_NULL;
    }
    // 添加引用
    for (const auto &node : view.data())
    {
        UA_ExpandedNodeId exp = UA_EXPANDEDNODEID_NULL;
        exp.nodeId = node;
        status = UA_Server_addReference(_server, retval, nodeOrganizes, exp, true);
        if (status != UA_STATUSCODE_GOOD)
        {
            ERROR_("Failed to add reference, error: %s", UA_StatusCode_name(status));
            return UA_NODEID_NULL;
        }
    }

    return retval;
}

NodeId Server::addEventTypeNode(const EventType &etype)
{
    RMVL_DbgAssert(_server != nullptr);

    UA_NodeId retval;
    UA_ObjectTypeAttributes attr = UA_ObjectTypeAttributes_default;
    attr.displayName = UA_LOCALIZEDTEXT(helper::en_US(), helper::to_char(etype.display_name));
    attr.description = UA_LOCALIZEDTEXT(helper::zh_CN(), helper::to_char(etype.description));

    auto status = UA_Server_addObjectTypeNode(
        _server, UA_NODEID_NULL, nodeBaseEventType,
        nodeHasSubtype,
        UA_QUALIFIEDNAME(etype.ns, helper::to_char(etype.browse_name)),
        attr, nullptr, &retval);
    if (status != UA_STATUSCODE_GOOD)
    {
        ERROR_("Failed to add event type: %s", UA_StatusCode_name(status));
        return UA_NODEID_NULL;
    }
    // 添加自定义数据（非默认属性）
    for (const auto &[browse_name, val] : etype.data())
    {
        UA_VariableAttributes val_attr = UA_VariableAttributes_default;
        val_attr.displayName = UA_LOCALIZEDTEXT(helper::en_US(), helper::to_char(browse_name));
        val_attr.accessLevel = UA_ACCESSLEVELMASK_READ | UA_ACCESSLEVELMASK_WRITE;
        UA_Variant_setScalarCopy(&val_attr.value, &val, &UA_TYPES[UA_TYPES_INT32]);
        UA_NodeId sub_id;
        status = UA_Server_addVariableNode(
            _server, UA_NODEID_NULL, retval, nodeHasProperty,
            UA_QUALIFIEDNAME(etype.ns, helper::to_char(browse_name)), nodePropertyType,
            val_attr, nullptr, &sub_id);
        if (status != UA_STATUSCODE_GOOD)
        {
            ERROR_("Failed to add event type property: %s", UA_StatusCode_name(status));
            return UA_NODEID_NULL;
        }
        // 设置子变量节点为强制生成
        status = UA_Server_addReference(
            _server, sub_id, nodeHasModellingRule,
            UA_EXPANDEDNODEID_NUMERIC(0, UA_NS0ID_MODELLINGRULE_MANDATORY), true);
        if (status != UA_STATUSCODE_GOOD)
        {
            ERROR_("Failed to add reference during adding event type node, browse name: %s, error code: %s",
                   browse_name.c_str(), UA_StatusCode_name(status));
            return UA_NODEID_NULL;
        }
    }
    return retval;
}

bool Server::triggerEvent(const Event &event) const { return serverTriggerEvent(_server, event); }

//////////////////////// 服务端视图 ////////////////////////

NodeId ServerView::find(std::string_view browse_path, const NodeId &src_nd) const noexcept { return serverFindNode(_server, browse_path, src_nd); }
Variable ServerView::read(const NodeId &nd) const { return serverRead(_server, nd); }
bool ServerView::write(const NodeId &nd, const Variable &val) const { return serverWrite(_server, nd, val); }
bool ServerView::triggerEvent(const Event &event) const { return serverTriggerEvent(_server, event); }

/////////////////////// 服务器定时器 ///////////////////////

static void timer_cb(UA_Server *p_server, void *data)
{
    auto &func = *reinterpret_cast<std::function<void(ServerView)> *>(data);
    func(p_server);
}

ServerTimer::ServerTimer(ServerView sv, double period, std::function<void(ServerView)> callback) : _sv(sv), _cb(callback)
{
    auto status = UA_Server_addRepeatedCallback(_sv.get(), timer_cb, &_cb, period, &_id);
    if (status != UA_STATUSCODE_GOOD)
    {
        ERROR_("Failed to add repeated callback: %s", UA_StatusCode_name(status));
        _id = 0;
    }
}

void ServerTimer::cancel()
{
    if (_id != 0)
    {
        UA_Server_removeCallback(_sv.get(), _id);
        _id = 0;
    }
}

} // namespace rm

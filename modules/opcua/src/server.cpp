/**
 * @file server.cpp
 * @author zhaoxi (535394140@qq.com)
 * @brief OPC UA 服务器
 * @version 1.0
 * @date 2023-10-21
 *
 * @copyright Copyright 2023 (c), zhaoxi
 *
 */

#include <stack>

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#include <open62541.h>

#ifdef __cplusplus
}
#endif // __cplusplus

#include "rmvl/opcua/server.hpp"

// ============================= 基本配置 =============================

rm::Server::Server(uint16_t port, const std::vector<rm::UserConfig> &users)
{
    _server = UA_Server_new();

    UA_ServerConfig *config = UA_Server_getConfig(_server);
    UA_ServerConfig_setMinimal(config, port, nullptr);

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
        // 配置
        config->accessControl.clear(&config->accessControl);
        UA_AccessControl_default(config, false, nullptr,
                                 &config->securityPolicies[config->securityPoliciesSize - 1].policyUri,
                                 usr_passwd.size(), usr_passwd.data());
    }
}

void rm::Server::run()
{
    _running = true;
    _run = std::thread([this]() {
        UA_StatusCode retval = UA_Server_run(_server, &_running);
        if (retval != UA_STATUSCODE_GOOD)
            UA_LOG_ERROR(UA_Log_Stdout, UA_LOGCATEGORY_SERVER, "Failed to initialize server: %s", UA_StatusCode_name(retval));
    });
}

rm::Server::~Server()
{
    for (auto &val : _variant_gc)
        UA_Variant_delete(val);
    for (auto &val : _argument_gc)
        UA_Argument_delete(val);
    UA_Server_delete(_server);
}

// ============================= 节点配置 =============================

UA_NodeId rm::Server::addVariableTypeNode(const rm::VariableType &vtype)
{
    UA_VariableTypeAttributes attr = UA_VariableTypeAttributes_default;
    UA_Variant *variant = helper::cvtVariable(vtype);
    // 添加至 GC
    _variant_gc.insert(variant);
    // 设置属性
    attr.value = *variant;
    attr.dataType = variant->type->typeId;
    attr.valueRank = vtype.getValueRank();
    if (attr.valueRank != UA_VALUERANK_SCALAR)
    {
        attr.arrayDimensionsSize = variant->arrayDimensionsSize;
        attr.arrayDimensions = variant->arrayDimensions;
    }
    attr.description = UA_LOCALIZEDTEXT(helper::zh_CN(), helper::to_char(vtype.description));
    attr.displayName = UA_LOCALIZEDTEXT(helper::en_US(), helper::to_char(vtype.display_name));
    UA_NodeId retval{UA_NODEID_NULL};
    auto status = UA_Server_addVariableTypeNode(
        _server, UA_NODEID_NULL,
        UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE),
        UA_NODEID_NUMERIC(0, UA_NS0ID_HASSUBTYPE),
        UA_QUALIFIEDNAME(1, helper::to_char(vtype.browse_name)),
        UA_NODEID_NULL, attr, nullptr, &retval);
    if (status != UA_STATUSCODE_GOOD)
    {
        UA_LOG_ERROR(UA_Log_Stdout, UA_LOGCATEGORY_SERVER, "Failed to add variable type node: %s", UA_StatusCode_name(status));
        return UA_NODEID_NULL;
    }
    return retval;
}

UA_NodeId rm::Server::addVariableNodeEx(const rm::Variable &val, UA_NodeId parent_id)
{
    // 变量节点属性 `UA_VariableAttributes`
    UA_VariableAttributes attr = UA_VariableAttributes_default;
    UA_Variant *variant = helper::cvtVariable(val);
    // 添加至 GC
    _variant_gc.insert(variant);
    // 设置属性
    attr.value = *variant;
    attr.dataType = variant->type->typeId;
    attr.accessLevel = val.getAccessLevel();
    attr.valueRank = val.getValueRank();
    if (attr.valueRank != UA_VALUERANK_SCALAR)
    {
        attr.arrayDimensionsSize = variant->arrayDimensionsSize;
        attr.arrayDimensions = variant->arrayDimensions;
    }
    attr.description = UA_LOCALIZEDTEXT(helper::zh_CN(), helper::to_char(val.description));
    attr.displayName = UA_LOCALIZEDTEXT(helper::en_US(), helper::to_char(val.display_name));
    // 获取变量节点的变量类型节点
    UA_NodeId type_id = UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE);
    const auto *p_type = val.getType();
    if (p_type != nullptr)
    {
        type_id = type_id | find(p_type->browse_name);
        if (UA_NodeId_equal(&type_id, &UA_NODEID_NULL))
        {
            UA_LOG_ERROR(UA_Log_Stdout, UA_LOGCATEGORY_SERVER, "Failed to find the variable type ID during adding variable node");
            type_id = UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE);
        }
    }
    UA_NodeId retval{UA_NODEID_NULL};
    // 添加节点至服务器
    auto status = UA_Server_addVariableNode(
        // 预先定义的变量节点 NodeId
        _server, UA_NODEID_NULL,
        // 父节点（通过派生得到当前节点），这里是 ObjectsFolder
        parent_id,
        // 与父节点之间的引用类型，这里是 ORGANIZES
        UA_NODEID_NUMERIC(0, UA_NS0ID_ORGANIZES),
        // 浏览名 BrowseName
        UA_QUALIFIEDNAME(1, helper::to_char(val.browse_name)),
        // 变量类型节点（通过实例化得到当前节点）
        type_id, attr, nullptr, &retval);
    if (status != UA_STATUSCODE_GOOD)
    {
        UA_LOG_ERROR(UA_Log_Stdout, UA_LOGCATEGORY_SERVER, "Failed to add variable node: %s", UA_StatusCode_name(status));
        return UA_NODEID_NULL;
    }
    return retval;
}

void rm::Server::writeVariable(UA_NodeId node, const rm::Variable &val)
{
    auto variant = helper::cvtVariable(val);
    _variant_gc.insert(variant);
    auto status = UA_Server_writeValue(_server, node, *variant);
    if (status != UA_STATUSCODE_GOOD)
        UA_LOG_ERROR(UA_Log_Stdout, UA_LOGCATEGORY_SERVER, "Failed to write variable, error code: %s", UA_StatusCode_name(status));
}

UA_NodeId rm::Server::addMethodNodeEx(const rm::Method &method, UA_NodeId parent_id)
{
    UA_MethodAttributes attr = UA_MethodAttributes_default;
    attr.displayName = UA_LOCALIZEDTEXT(helper::en_US(), helper::to_char(method.display_name));
    attr.description = UA_LOCALIZEDTEXT(helper::zh_CN(), helper::to_char(method.description));
    attr.executable = true;
    attr.userExecutable = true;
    // 提取数据至 `UA_Argument`
    std::vector<UA_Argument> inputs;
    inputs.reserve(method.iargs.size());
    for (auto &arg : method.iargs)
    {
        UA_Argument *ua_arg = helper::cvtArgument(arg);
        _argument_gc.insert(ua_arg);
        inputs.push_back(*ua_arg);
    }
    std::vector<UA_Argument> outputs;
    outputs.reserve(method.oargs.size());
    for (auto &arg : method.oargs)
    {
        UA_Argument *ua_arg = helper::cvtArgument(arg);
        _argument_gc.insert(ua_arg);
        outputs.push_back(*ua_arg);
    }
    // 添加节点
    UA_NodeId retval{UA_NODEID_NULL};
    auto status = UA_Server_addMethodNode(
        _server, UA_NODEID_NULL, parent_id,
        UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT),
        UA_QUALIFIEDNAME(1, helper::to_char(method.browse_name.c_str())),
        attr, method.func, inputs.size(), inputs.data(),
        outputs.size(), outputs.data(), nullptr, &retval);
    if (status != UA_STATUSCODE_GOOD)
    {
        UA_LOG_ERROR(UA_Log_Stdout, UA_LOGCATEGORY_SERVER,
                     "Failed to add method node: %s", UA_StatusCode_name(status));
        return UA_NODEID_NULL;
    }
    // 添加 Mandatory 属性
    status = UA_Server_addReference(_server, retval, UA_NODEID_NUMERIC(0, UA_NS0ID_HASMODELLINGRULE),
                                    UA_EXPANDEDNODEID_NUMERIC(0, UA_NS0ID_MODELLINGRULE_MANDATORY), true);
    if (status != UA_STATUSCODE_GOOD)
    {
        UA_LOG_ERROR(UA_Log_Stdout, UA_LOGCATEGORY_SERVER,
                     "Failed to add the \"Mandatory\" reference node: %s", UA_StatusCode_name(status));
        return UA_NODEID_NULL;
    }
    return retval;
}

UA_NodeId rm::Server::addObjectTypeNode(const rm::ObjectType &otype)
{
    // 定义对象类型节点
    UA_ObjectTypeAttributes attr = UA_ObjectTypeAttributes_default;
    attr.displayName = UA_LOCALIZEDTEXT(helper::en_US(), helper::to_char(otype.display_name));
    attr.description = UA_LOCALIZEDTEXT(helper::zh_CN(), helper::to_char(otype.description));
    UA_NodeId retval{UA_NODEID_NULL};
    // 获取父节点的 NodeID
    UA_NodeId parent_id{UA_NODEID_NUMERIC(0, UA_NS0ID_BASEOBJECTTYPE)};
    const rm::ObjectType *current = otype.getBase();
    std::stack<std::string> base_stack;
    while (current != nullptr)
    {
        base_stack.push(current->browse_name);
        current = current->getBase();
    }
    while (!base_stack.empty())
    {
        parent_id = parent_id | find(base_stack.top());
        base_stack.pop();
    }
    if (UA_NodeId_equal(&parent_id, &UA_NODEID_NULL))
    {
        UA_LOG_ERROR(UA_Log_Stdout, UA_LOGCATEGORY_SERVER, "Failed to find the base object type ID during adding object type node");
        parent_id = UA_NODEID_NUMERIC(0, UA_NS0ID_BASEOBJECTTYPE);
    }
    auto status = UA_Server_addObjectTypeNode(
        _server, UA_NODEID_NULL, parent_id,
        UA_NODEID_NUMERIC(0, UA_NS0ID_HASSUBTYPE),
        UA_QUALIFIEDNAME(1, helper::to_char(otype.browse_name)),
        attr, nullptr, &retval);
    if (status != UA_STATUSCODE_GOOD)
    {
        UA_LOG_ERROR(UA_Log_Stdout, UA_LOGCATEGORY_SERVER, "Failed to add object type: %s", UA_StatusCode_name(status));
        return UA_NODEID_NULL;
    }
    // 添加变量节点作为对象类型节点的子节点
    for (const auto &[browse_name, val] : otype.getVariables())
    {
        // 添加至服务器
        UA_NodeId sub_retval = addVariableNodeEx(val, retval);
        // 设置子变量节点为强制生成
        status = UA_Server_addReference(
            _server, sub_retval, UA_NODEID_NUMERIC(0, UA_NS0ID_HASMODELLINGRULE),
            UA_EXPANDEDNODEID_NUMERIC(0, UA_NS0ID_MODELLINGRULE_MANDATORY), true);
        if (status != UA_STATUSCODE_GOOD)
        {
            UA_LOG_ERROR(
                UA_Log_Stdout, UA_LOGCATEGORY_SERVER, "Failed to add reference during adding object type node, browse name: %s, error code: %s",
                browse_name.c_str(), UA_StatusCode_name(status));
            return UA_NODEID_NULL;
        }
    }
    // 添加方法节点作为对象类型节点的子节点
    for (const auto &val : otype.getMethods())
        addMethodNodeEx(val.second, retval);
    return retval;
}

UA_NodeId rm::Server::addObjectNodeEx(const rm::Object &obj, UA_NodeId parent_id)
{
    UA_ObjectAttributes attr{UA_ObjectAttributes_default};
    attr.displayName = UA_LOCALIZEDTEXT(helper::en_US(), helper::to_char(obj.display_name));
    attr.description = UA_LOCALIZEDTEXT(helper::zh_CN(), helper::to_char(obj.description));
    // 获取对象类型节点
    const rm::ObjectType *current = obj.getType();
    UA_NodeId type_id{UA_NODEID_NUMERIC(0, UA_NS0ID_BASEOBJECTTYPE)};
    std::stack<std::string> base_stack;
    while (current != nullptr)
    {
        base_stack.push(current->browse_name);
        current = current->getBase();
    }
    while (!base_stack.empty())
    {
        type_id = type_id | find(base_stack.top());
        base_stack.pop();
    }
    if (UA_NodeId_equal(&type_id, &UA_NODEID_NULL))
    {
        UA_LOG_ERROR(UA_Log_Stdout, UA_LOGCATEGORY_SERVER, "Failed to find the object type of the object: \"%s\"", obj.browse_name.c_str());
        type_id = UA_NODEID_NUMERIC(0, UA_NS0ID_BASEOBJECTTYPE);
    }
    // 添加至服务器
    UA_NodeId retval = UA_NODEID_NULL;
    auto status = UA_Server_addObjectNode(
        _server, UA_NODEID_NULL, parent_id,
        UA_NODEID_NUMERIC(0, UA_NS0ID_ORGANIZES),
        UA_QUALIFIEDNAME(1, helper::to_char(obj.browse_name)),
        type_id, attr, nullptr, &retval);
    if (status != UA_STATUSCODE_GOOD)
    {
        UA_LOG_ERROR(UA_Log_Stdout, UA_LOGCATEGORY_SERVER, "Failed to add object node to server, error code: %s", UA_StatusCode_name(status));
        return UA_NODEID_NULL;
    }
    // 添加变量
    for (const auto &[browse_name, variable] : obj.getVariables())
    {
        auto sub_node_id = retval | find(browse_name);
        if (UA_NodeId_equal(&sub_node_id, &UA_NODEID_NULL))
            writeVariable(sub_node_id, variable);
        else
            addVariableNodeEx(variable, retval);
    }
    return retval;
}

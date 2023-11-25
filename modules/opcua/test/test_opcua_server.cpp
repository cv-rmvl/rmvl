/**
 * @file test_opcua_server.cpp
 * @author zhaoxi (535394140@qq.com)
 * @brief OPC UA 服务器单元测试
 * @version 1.0
 * @date 2023-11-09
 *
 * @copyright Copyright 2023 (c), zhaoxi
 *
 */

#include <gtest/gtest.h>

#include "rmvl/opcua/server.hpp"

namespace rm_test
{

// 变量（类型）配置
TEST(OPC_UA_Server, value_config)
{
    // 变量类型节点、字符串
    rm::VariableType variable_type = "string_test";
    EXPECT_EQ(variable_type.size(), 1);
    EXPECT_EQ(variable_type.getDataType(), UA_TYPES_STRING);
    // 添加变量节点、双精度浮点数
    rm::Variable variable = 3.1415;
    EXPECT_EQ(variable.size(), 1);
    EXPECT_EQ(variable.getDataType(), UA_TYPES_DOUBLE);
    // 添加变量节点、数组
    rm::Variable variable_array = std::vector<int>{1, 2, 3};
    EXPECT_EQ(variable_array.size(), 3);
    EXPECT_EQ(variable_array.getDataType(), UA_TYPES_INT32);
}

// 服务器添加变量节点
TEST(OPC_UA_Server, server_config_add_node)
{
    rm::Server server(4840);
    rm::Variable variable = 3.1415;
    variable.browse_name = "test_double";
    variable.description = "this is test double";
    variable.display_name = "测试双精度浮点数";
    auto node = server.addVariableNode(variable);
    EXPECT_FALSE(UA_NodeId_isNull(&node));
    server.start();
    server.stop();
    server.join();
}

// 服务器添加变量类型节点
TEST(OPC_UA_Server, server_config_add_type_node)
{
    rm::Server server(4842);
    rm::VariableType variable_type = "string_test";
    variable_type.browse_name = "test_string";
    variable_type.description = "this is test string";
    variable_type.display_name = "测试字符串";
    auto node = server.addVariableTypeNode(variable_type);
    EXPECT_FALSE(UA_NodeId_isNull(&node));
    server.start();
    server.stop();
    server.join();
}

// 服务器添加方法节点
TEST(OPC_UA_Server, server_config_call_method)
{
    rm::Server server(4845);
    rm::Method method;
    method.browse_name = "test_method";
    method.description = "this is test method";
    method.display_name = "测试方法";
    method.func = [](UA_Server *, const UA_NodeId *, void *, const UA_NodeId *, void *, const UA_NodeId *,
                     void *, size_t, const UA_Variant *, size_t, UA_Variant *) -> UA_StatusCode {
        return UA_STATUSCODE_GOOD;
    };
    server.addMethodNode(method);
    server.start();
    server.stop();
    server.join();
}

// 服务器添加对象节点
TEST(OPC_UA_Server, server_config_add_object_node)
{
    rm::Server server(4846);
    rm::Object object;
    object.browse_name = "test_object";
    object.description = "this is test object";
    object.display_name = "测试对象";
    rm::Variable val1 = 3.14;
    val1.browse_name = "test_val1";
    val1.description = "this is test val1";
    val1.display_name = "测试变量 1";
    object.add(val1);
    auto id = server.addObjectNode(object);
    EXPECT_FALSE(UA_NodeId_isNull(&id));
    server.start();
    server.stop();
    server.join();
}

// 服务器添加对象类型节点
TEST(OPC_UA_Server, server_config_add_object_type_node)
{
    rm::Server server(4847);
    rm::ObjectType object_type;
    object_type.browse_name = "test_object_type";
    object_type.description = "this is test object type";
    object_type.display_name = "测试对象类型";
    rm::Variable val1 = 3.14;
    val1.browse_name = "test_val1";
    val1.description = "this is test val1";
    val1.display_name = "测试变量 1";
    object_type.add(val1);
    auto id = server.addObjectTypeNode(object_type);
    EXPECT_FALSE(UA_NodeId_isNull(&id));
    server.start();
    server.stop();
    server.join();
}

// 从对象类型节点派生对象节点，并添加到服务器
TEST(OPC_UA_Server, create_object_by_object_type_and_add_to_server)
{
    rm::Server server(4848);
    rm::ObjectType object_type;
    object_type.browse_name = "test_object_type";
    object_type.description = "this is test object type";
    object_type.display_name = "测试对象类型";
    rm::Variable val1 = 3.14;
    val1.browse_name = "test_val1";
    val1.description = "this is test val1";
    val1.display_name = "测试变量 1";
    object_type.add(val1);
    server.addObjectTypeNode(object_type);
    rm::Object object(object_type);
    object.browse_name = "test_object";
    object.description = "this is test object";
    object.display_name = "测试对象";
    auto id = server.addObjectNode(object);
    EXPECT_FALSE(UA_NodeId_isNull(&id));
    server.start();
    server.stop();
    server.join();
}

// 服务器节点服务端路径搜索
TEST(OPC_UA_Server, server_config_find_node)
{
    rm::Server server(4850);
    rm::Object object;
    object.browse_name = "test_object";
    object.description = "this is test object";
    object.display_name = "测试对象";
    rm::Variable val1 = 3.14;
    val1.browse_name = "test_val1";
    val1.description = "this is test val1";
    val1.display_name = "测试变量 1";
    object.add(val1);
    auto id = server.addObjectNode(object);
    auto target = UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER) | server.find("test_object");
    EXPECT_TRUE(UA_NodeId_equal(&id, &target));
    server.start();
    server.stop();
    server.join();
}

// 添加自定义事件类型节点
TEST(OPC_UA_Server, server_config_add_event_type_node)
{
    rm::Server server(4855);
    rm::EventType event_type;
    event_type.browse_name = "test_event_type";
    event_type.description = "this is test event type";
    event_type.display_name = "测试事件类型";
    int val = 3;
    event_type.add("test_val", val);
    auto id = server.addEventTypeNode(event_type);
    auto target = rm::nodeBaseEventType | server.find("test_event_type");
    EXPECT_TRUE(UA_NodeId_equal(&id, &target));
    server.start();
    server.stop();
    server.join();
}

// 手动触发事件
TEST(OPC_UA_Server, server_config_trigger_event)
{
    rm::Server server(4860);
    // 添加事件类型
    rm::EventType event_type;
    event_type.browse_name = "test_event_type";
    event_type.description = "this is test event type";
    event_type.display_name = "测试事件类型";
    int val = 3;
    event_type.add("test_val", val);
    server.addEventTypeNode(event_type);
    // 创建事件
    rm::Event event(event_type);
    event.source_name = "test_event";
    event.message = "this is test event";
    event.severity = 1;
    event["test_val1"] = 99;
    // 触发事件
    EXPECT_TRUE(server.triggerEvent(UA_NODEID_NUMERIC(0, UA_NS0ID_SERVER), event));
    server.start();
    server.stop();
    server.join();
}

} // namespace rm_test

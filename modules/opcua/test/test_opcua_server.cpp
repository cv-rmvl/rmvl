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

#include "testnum.h"

namespace rm_test {

// 变量（类型）配置
TEST(OPC_UA_Server, variable_config)
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
TEST(OPC_UA_Server, add_variable_node)
{
    rm::Server srv(4810, "TestServer");
    rm::Variable variable{3.1415};
    variable.browse_name = "test_double";
    variable.description = "this is test double";
    variable.display_name = "测试双精度浮点数";
    auto node = srv.addVariableNode(variable);
    EXPECT_FALSE(UA_NodeId_isNull(&node));
    srv.start();
}

TEST(OPC_UA_Server, variable_node_io)
{
    rm::Server srv(4820, "TestServer");
    uaCreateVariable(variable, 1);
    auto node = srv.addVariableNode(variable);
    srv.start();
    EXPECT_EQ(srv.read(node), 1);
    EXPECT_TRUE(srv.write(node, 2));
    EXPECT_EQ(srv.read(node), 2);
}

// 服务器添加数据源变量节点
TEST(OPC_UA_Server, add_data_source_variable_node)
{
    int data_source{};
    rm::Server srv(4825, "TestServer");
    uaCreateVariable(variable);

    // Callback
    auto on_read = [&](rm::ServerView, const rm::NodeId &) -> rm::Variable {
        return data_source;
    };
    auto on_write = [&](rm::ServerView, const rm::NodeId &, const rm::Variable &val) {
        data_source = val.cast<int>();
    };
    auto node = srv.addDataSourceVariableNode(variable, on_read, on_write);
    EXPECT_FALSE(UA_NodeId_isNull(&node));
    srv.start();
}

// 服务器添加变量类型节点
TEST(OPC_UA_Server, add_variable_type_node)
{
    rm::Server srv(4830);
    rm::VariableType variable_type = "string_test";
    variable_type.browse_name = "test_string";
    variable_type.description = "this is test string";
    variable_type.display_name = "测试字符串";
    auto node = srv.addVariableTypeNode(variable_type);
    EXPECT_FALSE(UA_NodeId_isNull(&node));
    srv.start();
}

// 服务器添加方法节点
TEST(OPC_UA_Server, add_method_node)
{
    rm::Server srv(4832);
    rm::Method method = [](rm::ServerView, const rm::NodeId &, const std::vector<rm::Variable> &) -> std::vector<rm::Variable> {
        return {};
    };
    method.browse_name = "test_method";
    method.description = "this is test method";
    method.display_name = "测试方法";
    srv.addMethodNode(method);
    srv.start();
}

// 服务器添加对象节点
TEST(OPC_UA_Server, add_object_node)
{
    rm::Server srv(4835);
    rm::Object object;
    object.browse_name = "test_object";
    object.description = "this is test object";
    object.display_name = "测试对象";
    rm::Variable val1 = 3.14;
    val1.browse_name = "test_val1";
    val1.description = "this is test val1";
    val1.display_name = "测试变量 1";
    object.add(val1);
    auto id = srv.addObjectNode(object);
    EXPECT_FALSE(UA_NodeId_isNull(&id));
    srv.start();
}

// 服务器添加包含方法节点的对象节点
TEST(OPC_UA_Server, add_object_node_with_method)
{
    rm::Server srv(4840);
    rm::Object object;
    object.browse_name = "test_object";
    object.description = "this is test object";
    object.display_name = "测试对象";
    rm::Variable val1 = 3.14;
    val1.browse_name = "test_val1";
    val1.description = "this is test val1";
    val1.display_name = "测试变量 1";
    object.add(val1);
    rm::Method method;
    method.browse_name = "test_method";
    method.description = "this is test method";
    method.display_name = "测试方法";
    method.func = [](rm::ServerView, const rm::NodeId &, const std::vector<rm::Variable> &) -> std::vector<rm::Variable> {
        return {};
    };
    object.add(method);
    auto id = srv.addObjectNode(object);
    EXPECT_FALSE(UA_NodeId_isNull(&id));
    srv.start();
}

// 服务器添加对象类型节点
TEST(OPC_UA_Server, add_object_type_node)
{
    rm::Server srv(4845);
    rm::ObjectType object_type;
    object_type.browse_name = "test_object_type";
    object_type.description = "this is test object type";
    object_type.display_name = "测试对象类型";
    rm::Variable val1 = 3.14;
    val1.browse_name = "test_val1";
    val1.description = "this is test val1";
    val1.display_name = "测试变量 1";
    object_type.add(val1);
    auto id = srv.addObjectTypeNode(object_type);
    EXPECT_FALSE(UA_NodeId_isNull(&id));
    srv.start();
}

// 从对象类型节点派生对象节点，并添加到服务器
TEST(OPC_UA_Server, create_object_by_object_type)
{
    rm::Server srv(4846);
    rm::ObjectType object_type;
    object_type.browse_name = "test_object_type";
    object_type.description = "this is test object type";
    object_type.display_name = "测试对象类型";
    rm::Variable val1 = 3.14;
    val1.browse_name = "test_val1";
    val1.description = "this is test val1";
    val1.display_name = "测试变量 1";
    object_type.add(val1);
    srv.addObjectTypeNode(object_type);
    rm::Object object(object_type);
    object.browse_name = "test_object";
    object.description = "this is test object";
    object.display_name = "测试对象";
    auto id = srv.addObjectNode(object);
    EXPECT_FALSE(UA_NodeId_isNull(&id));
    srv.start();
}

// 服务器节点服务端路径搜索
TEST(OPC_UA_Server, find_node)
{
    rm::Server srv(4850);
    rm::Object object;
    object.browse_name = "test_object";
    object.description = "this is test object";
    object.display_name = "测试对象";
    rm::Variable val1 = 3.14;
    val1.browse_name = "test_val1";
    val1.description = "this is test val1";
    val1.display_name = "测试变量 1";
    object.add(val1);
    auto id = srv.addObjectNode(object);
    auto target = rm::nodeObjectsFolder | srv.find("test_object");
    EXPECT_TRUE(UA_NodeId_equal(&id, &target));
    srv.start();
}

// 添加自定义事件类型节点
TEST(OPC_UA_Server, add_event_type_node)
{
    rm::Server srv(4855);
    rm::EventType event_type;
    event_type.browse_name = "test_event_type";
    event_type.description = "this is test event type";
    event_type.display_name = "测试事件类型";
    int val = 3;
    event_type.add("test_val", val);
    auto id = srv.addEventTypeNode(event_type);
    auto target = rm::nodeBaseEventType | srv.find("test_event_type");
    EXPECT_TRUE(UA_NodeId_equal(&id, &target));
    srv.start();
}

// 手动触发事件
TEST(OPC_UA_Server, trigger_event)
{
    rm::Server srv(4860);
    // 添加事件类型
    rm::EventType event_type;
    event_type.browse_name = "test_event_type";
    event_type.description = "this is test event type";
    event_type.display_name = "测试事件类型";
    int val = 3;
    event_type.add("test_val", val);
    srv.addEventTypeNode(event_type);
    // 创建事件
    rm::Event event(event_type);
    event.source_name = "test_event";
    event.message = "this is test event";
    event.severity = 1;
    event["test_val1"] = 99;
    // 触发事件
    EXPECT_TRUE(srv.triggerEvent(UA_NODEID_NUMERIC(0, UA_NS0ID_SERVER), event));
    srv.start();
}

// 从函数指针配置服务器
TEST(OPC_UA_Server, function_ptr)
{
    rm::Server srv(testnum, 4865);
    srv.start();
    auto id = rm::nodeObjectsFolder | srv.find("TestNumber");
    EXPECT_FALSE(UA_NodeId_isNull(&id));
}

// 视图节点
TEST(OPC_UA_Server, view_node)
{
    rm::Server srv(4870);
    uaCreateVariable(demo1, 3.14);
    auto node1 = srv.addVariableNode(demo1);
    uaCreateVariable(demo2, 1);
    srv.addVariableNode(demo2);
    uaCreateVariable(demo3, "abc");
    auto node3 = srv.addVariableNode(demo3);
    srv.start();

    rm::View view;
    view.add(node1, node3);
    view.browse_name = "test_view";
    view.description = "this is test view";
    view.display_name = "测试视图";
    auto view_id = srv.addViewNode(view);
    auto target_view_id = rm::nodeViewsFolder | srv.find("test_view");
    EXPECT_TRUE(UA_NodeId_equal(&view_id, &target_view_id));
}

} // namespace rm_test

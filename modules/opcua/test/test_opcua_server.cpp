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

namespace rm_test
{

// 变量（类型）配置
TEST(OPC_UA_Server, variable_config)
{
    // 变量类型节点、字符串
    rm::VariableType variable_type{"string_test"};
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
    EXPECT_FALSE(node.empty());
    srv.spinOnce();
}

TEST(OPC_UA_Server, variable_node_io)
{
    rm::Server srv(4820, "TestServer");
    uaCreateVariable(variable, 1);
    auto node = srv.addVariableNode(variable);
    srv.spinOnce();
    EXPECT_EQ(srv.read(node), 1);
    EXPECT_TRUE(srv.write(node, 2));
    EXPECT_EQ(srv.read(node), 2);
}

// 服务器添加数据源变量节点
TEST(OPC_UA_Server, add_data_source_variable_node)
{
    int data_source{};
    rm::Server srv(4825, "TestServer");
    rm::DataSourceVariable v;
    v.browse_name = v.display_name = "test_int";
    v.description = "this is test int";
    v.on_read = [&](const rm::NodeId &) -> rm::Variable { return data_source; };
    v.on_write = [&](const rm::NodeId &, const rm::Variable &val) {
        data_source = val.cast<int>();
    };
    auto node = srv.addDataSourceVariableNode(v);
    EXPECT_FALSE(node.empty());
    srv.spinOnce();
}

// 服务器添加变量类型节点
TEST(OPC_UA_Server, add_variable_type_node)
{
    rm::Server srv(4830);
    rm::VariableType variable_type{"string_test"};
    variable_type.browse_name = "test_string";
    variable_type.description = "this is test string";
    variable_type.display_name = "测试字符串";
    auto node = srv.addVariableTypeNode(variable_type);
    EXPECT_FALSE(node.empty());
    srv.spinOnce();
}

// 服务器添加方法节点
TEST(OPC_UA_Server, add_method_node)
{
    rm::Server srv(4832);
    rm::Method method = [](rm::ServerView, const rm::Variables &) -> std::pair<bool, rm::Variables> { return {true, {}}; };
    method.browse_name = "test_method";
    method.description = "this is test method";
    method.display_name = "测试方法";
    srv.addMethodNode(method);
    srv.spinOnce();
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
    EXPECT_FALSE(id.empty());
    srv.spinOnce();
    // 路径搜索变量节点
    auto val1_id = srv.find("test_object/test_val1");
    EXPECT_FALSE(val1_id.empty());
}

// 服务器添加包含数据源变量节点的对象节点
TEST(OPC_UA_Server, add_object_node_with_dsv)
{
    rm::Server srv(4837);
    rm::Object object;
    object.browse_name = "test_object";
    object.description = "this is test object";
    object.display_name = "测试对象";
    rm::DataSourceVariable dsv1;
    dsv1.browse_name = "test_dsv1";
    dsv1.description = "this is test data source variable";
    dsv1.display_name = "测试数据源变量 1";
    dsv1.access_level = rm::VARIABLE_READ | rm::VARIABLE_WRITE;
    int src_val{};
    dsv1.on_read = [&](const rm::NodeId &) -> rm::Variable { return src_val; };
    dsv1.on_write = [&](const rm::NodeId &, const rm::Variable &val) { src_val = val; };
    object.add(dsv1);
    auto id = srv.addObjectNode(object);
    EXPECT_FALSE(id.empty());
    srv.spinOnce();
    // 路径搜索数据源变量节点
    auto dsv_id = srv.find("test_object/test_dsv1");
    EXPECT_FALSE(dsv_id.empty());
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
    method.func = [](rm::ServerView, const rm::Variables &) -> std::pair<bool, rm::Variables> { return {true, {}}; };
    object.add(method);
    auto id = srv.addObjectNode(object);
    EXPECT_FALSE(id.empty());
    srv.spinOnce();
    // 路径搜索方法节点
    auto method_id = srv.find("test_object/test_method");
    EXPECT_FALSE(method_id.empty());
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
    EXPECT_FALSE(id.empty());
    srv.spinOnce();
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
    auto object = rm::Object::makeFrom(object_type);
    object.browse_name = "test_object";
    object.description = "this is test object";
    object.display_name = "测试对象";
    auto id = srv.addObjectNode(object);
    EXPECT_FALSE(id.empty());
    srv.spinOnce();
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
    auto target = srv.find("test_object");
    EXPECT_EQ(id, target);
    srv.spinOnce();
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
    auto target = srv.find("test_event_type", rm::nodeBaseEventType);
    EXPECT_EQ(id, target);
    srv.spinOnce();
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
    auto event = rm::Event::makeFrom(event_type);
    event.source_name = "test_event";
    event.message = "this is test event";
    event.severity = 1;
    event["test_val1"] = 99;
    // 触发事件
    EXPECT_TRUE(srv.triggerEvent(event));
    srv.spinOnce();
}

// 从函数指针配置服务器
TEST(OPC_UA_Server, function_ptr)
{
    rm::Server srv(testnum, 4865);
    srv.spinOnce();
    auto id = srv.find("TestNumber");
    EXPECT_FALSE(id.empty());
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
    srv.spinOnce();

    rm::View view;
    view.add(node1, node3);
    view.browse_name = "test_view";
    view.description = "this is test view";
    view.display_name = "测试视图";
    auto view_id = srv.addViewNode(view);
    auto target_view_id = rm::nodeViewsFolder | srv.node("test_view");
    EXPECT_EQ(view_id, target_view_id);
}

TEST(OPC_UA_Server, timer_test)
{
    rm::Server srv(4875);
    uaCreateVariable(val, 0);
    auto nd = srv.addVariableNode(val);
    srv.spinOnce();

    int times{};

    auto timer = rm::ServerTimer(srv, 10, [&](rm::ServerView sv) {
        int num = sv.read(nd).cast<int>() + 10;
        sv.write(nd, num);
        times++;
    });

    while (times != 5)
        srv.spinOnce();
    EXPECT_EQ(srv.read(nd).cast<int>(), 50);
}

} // namespace rm_test

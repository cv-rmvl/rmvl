/**
 * @file test_opcua_client.cpp
 * @author zhaoxi (535394140@qq.com)
 * @brief OPC UA 客户端单元测试
 * @version 1.0
 * @date 2023-11-09
 *
 * @copyright Copyright 2023 (c), zhaoxi
 *
 */

#include <gtest/gtest.h>

#include "rmvl/opcua/client.hpp"
#include "rmvl/opcua/server.hpp"

#include "rmvlpara/opcua.hpp"

namespace rm_test
{

using namespace std::chrono_literals;

TEST(OPC_UA_ClientTest, connect)
{
    rm::UserConfig config;
    rm::Server srv(4999, "Test Server", {{"admin", "admin"}});
    srv.start();
    std::this_thread::sleep_for(10ms);

    rm::Client cli1("opc.tcp://127.0.0.1:4999", {"admin", "admin"});
    EXPECT_TRUE(cli1.ok());
    rm::Client cli2("opc.tcp://127.0.0.1:4999", {"admin", "123456"});
    EXPECT_FALSE(cli2.ok());
};

void configServer(rm::Server &srv)
{
    // 添加单变量节点
    rm::Variable single_value = 42;
    single_value.browse_name = "single";
    single_value.description = "this is single value";
    single_value.display_name = "单值";
    srv.addVariableNode(single_value);
    // 添加数组变量节点
    rm::Variable variable = std::vector({1, 2, 3, 4, 5});
    variable.browse_name = "array";
    variable.description = "this is array";
    variable.display_name = "数组";
    srv.addVariableNode(variable);
    // 添加加法方法节点
    rm::Method method = [](rm::ServerView, const rm::NodeId &, const std::vector<rm::Variable> &input) -> std::vector<rm::Variable> {
        int a = input[0], b = input[1];
        return {a + b};
    };
    method.browse_name = "add";
    method.description = "this is add method";
    method.display_name = "加法";
    method.iargs = {{"a", UA_TYPES_INT32}, {"b", UA_TYPES_INT32}};
    method.oargs = {{"c", UA_TYPES_INT32}};
    srv.addMethodNode(method);
    // 添加对象节点，包含字符串变量和乘法方法
    srv.start();
    std::this_thread::sleep_for(10ms);
}

// 路径搜索
TEST(OPC_UA_ClientTest, read_variable)
{
    rm::Server srv(5000);
    configServer(srv);
    rm::Client cli("opc.tcp://127.0.0.1:5000");
    // 读取测试服务器上的变量值
    auto id = rm::nodeObjectsFolder | cli.find("array");
    rm::Variable variable = cli.read(id);
    EXPECT_FALSE(variable.empty());
    std::vector<int> vec = variable;
    for (size_t i = 0; i < vec.size(); ++i)
        EXPECT_EQ(vec[i], i + 1);
    srv.stop();
    srv.join();
}

// 变量读写
TEST(OPC_UA_ClientTest, variable_IO)
{
    rm::Server srv(5001);
    configServer(srv);
    rm::Client cli("opc.tcp://127.0.0.1:5001");
    // 读取测试服务器上的变量值
    auto id = rm::nodeObjectsFolder | cli.find("single");
    EXPECT_TRUE(cli.write(id, 99));
    rm::Variable variable = cli.read(id);
    EXPECT_FALSE(variable.empty());
    int single_value = rm::Variable::cast<int>(variable);
    EXPECT_EQ(single_value, 99);
    srv.stop();
    srv.join();
}

// 方法调用
TEST(OPC_UA_ClientTest, call)
{
    rm::Server srv(5002);
    configServer(srv);
    rm::Client cli("opc.tcp://127.0.0.1:5002");
    // 调用测试服务器上的方法
    std::vector<rm::Variable> input = {1, 2};
    std::vector<rm::Variable> output;
    EXPECT_TRUE(cli.call("add", input, output));
    EXPECT_EQ(rm::Variable::cast<int>(output[0]), 3);
    srv.stop();
    srv.join();
}

// 订阅
TEST(OPC_UA_ClientTest, variable_monitor)
{
    rm::Server srv(5003);
    configServer(srv);
    rm::Client cli("opc.tcp://127.0.0.1:5003");
    // 订阅测试服务器上的变量
    int receive_data{};
    auto node_id = rm::nodeObjectsFolder | cli.find("single");
    auto on_change = [&](rm::ClientView, const rm::Variable &value) {
        receive_data = value;
    };
    EXPECT_TRUE(cli.monitor(node_id, on_change, 5));
    // 数据更新
    cli.write(node_id, 66);
    std::this_thread::sleep_for(10ms);
    cli.spinOnce();
    EXPECT_EQ(receive_data, 66);
    // 移除监视后的数据按预期不会更新
    cli.remove(node_id);
    cli.write(node_id, 123);
    std::this_thread::sleep_for(10ms);
    cli.spinOnce();
    EXPECT_EQ(receive_data, 66);
    srv.stop();
    srv.join();
}

TEST(OPC_UA_ClientTest, event_monitor)
{
    rm::Server srv(5004);
    configServer(srv);
    rm::EventType etype;
    etype.browse_name = "TestEventType";
    etype.display_name = "测试事件类型";
    etype.description = "测试事件类型";
    etype.add("aaa", 3);
    srv.addEventTypeNode(etype);
    rm::Client cli("opc.tcp://127.0.0.1:5004");

    std::string source_name;
    int aaa{};
    cli.monitor(rm::nodeServer, {"SourceName", "aaa"}, [&](rm::ClientView, const std::vector<rm::Variable> &fields) {
        source_name = fields[0].cast<const char *>();
        aaa = fields[1];
    });
    // 触发事件
    rm::Event event(etype);
    event.source_name = "GtestServer";
    event.message = "this is test event";
    event["aaa"] = 66;
    srv.triggerEvent(rm::nodeServer, event);
    cli.spinOnce();
    std::this_thread::sleep_for(10ms);
    EXPECT_EQ(source_name, "GtestServer");
    EXPECT_EQ(aaa, 66);
    srv.stop();
    srv.join();
}

} // namespace rm_test

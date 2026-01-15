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

#include <chrono>
#include <thread>

#include <gtest/gtest.h>
#include <utility>

#include "rmvl/opcua/client.hpp"
#include "rmvl/opcua/method.hpp"
#include "rmvl/opcua/object.hpp"
#include "rmvl/opcua/server.hpp"

namespace rm_test {

using namespace std::chrono_literals;

TEST(OPC_UA_ClientTest, connect) {
    rm::UserConfig config;
    rm::OpcuaServer srv(4999, "Test OpcuaServer", {{"admin", "admin"}});
    std::thread t(&rm::OpcuaServer::spin, &srv);
    std::this_thread::sleep_for(50ms);

    rm::OpcuaClient cli1("opc.tcp://127.0.0.1:4999", {"admin", "admin"});
    EXPECT_TRUE(cli1.ok());
    rm::OpcuaClient cli2("opc.tcp://127.0.0.1:4999", {"admin", "123456"});
    EXPECT_FALSE(cli2.ok());

    cli1.shutdown();
    cli2.shutdown();
    srv.shutdown();
    t.join();
}

static void configServer(rm::OpcuaServer &srv) {
    // 添加单变量节点
    rm::Variable single_value = 42;
    single_value.browse_name = "single";
    single_value.description = "this is single value";
    single_value.display_name = "单值";
    srv.addVariableNode(single_value);
    // 添加数组变量节点
    rm::Variable variable = std::vector{1, 2, 3, 4, 5};
    variable.browse_name = "array";
    variable.description = "this is array";
    variable.display_name = "数组";
    srv.addVariableNode(variable);
    // 添加数据源变量节点
    static int dv_src{};
    rm::DataSourceVariable dv;
    dv.browse_name = "data_source";
    dv.display_name = "this is data source";
    dv.description = "单值（数据源）";
    dv.access_level = rm::VARIABLE_READ | rm::VARIABLE_WRITE;
    dv.on_read = [](const rm::NodeId &) -> rm::Variable { return dv_src; };
    dv.on_write = [](const rm::NodeId &, const rm::Variable &val) { dv_src = val; };
    srv.addDataSourceVariableNode(dv);
    // 添加加法方法节点
    rm::Method method = [](const rm::NodeId &, const rm::Variables &input) {
        int a = input[0], b = input[1];
        rm::Variables output = {a + b};
        return std::make_pair(true, output);
    };
    method.browse_name = "add";
    method.description = "this is add method";
    method.display_name = "加法";
    method.iargs = {{"a", rm::tpInt32}, {"b", rm::tpInt32}};
    method.oargs = {{"c", rm::tpInt32}};
    srv.addMethodNode(method);
    // 添加 person/name 对象节点及其方法
    rm::Object person;
    person.display_name = "Person";
    person.browse_name = "person";
    person.description = "人物";
    rm::Variable age = 18;
    age.display_name = "Age";
    age.browse_name = "age";
    age.description = "年龄";
    person.add(age);

    auto person_nd = srv.addObjectNode(person);
    rm::Object name;
    name.display_name = "Name";
    name.browse_name = "name";
    name.description = "名字";
    rm::Method get = [](const rm::NodeId &, const rm::Variables &) {
        rm::Variables output = {"Alice"};
        return std::make_pair(true, output);
    };
    get.browse_name = "get";
    get.display_name = "Get";
    get.description = "获取名字";
    get.oargs = {{"name", rm::tpString, 1, "名字"}};
    name.add(get);
    srv.addObjectNode(name, person_nd);
}

// 路径搜索
TEST(OPC_UA_ClientTest, path_search) {
    rm::OpcuaServer srv(5000);
    configServer(srv);
    std::thread t(&rm::OpcuaServer::spin, &srv);
    std::this_thread::sleep_for(10ms);
    rm::OpcuaClient cli("opc.tcp://127.0.0.1:5000");
    // 读取测试服务器上的变量值
    auto id = cli.find("array");
    rm::Variable variable = cli.read(id);
    EXPECT_FALSE(variable.empty());
    std::vector<int> vec = variable;
    for (size_t i = 0; i < vec.size(); ++i)
        EXPECT_EQ(vec[i], i + 1);

    cli.shutdown();
    srv.shutdown();
    t.join();
}

// 变量读写
TEST(OPC_UA_ClientTest, variable_io) {
    rm::OpcuaServer srv(5001);
    configServer(srv);
    std::thread t(&rm::OpcuaServer::spin, &srv);
    std::this_thread::sleep_for(10ms);
    rm::OpcuaClient cli("opc.tcp://127.0.0.1:5001");
    // 读取测试服务器上的变量值
    auto id = cli.find("single");
    EXPECT_TRUE(cli.write(id, 99));
    rm::Variable variable = cli.read(id);
    EXPECT_FALSE(variable.empty());
    int single_value = rm::Variable::cast<int>(variable);
    EXPECT_EQ(single_value, 99);

    cli.shutdown();
    srv.shutdown();
    t.join();
}

// 方法调用
TEST(OPC_UA_ClientTest, call) {
    rm::OpcuaServer srv(5002);
    configServer(srv);
    std::thread t(&rm::OpcuaServer::spin, &srv);
    std::this_thread::sleep_for(10ms);
    rm::OpcuaClient cli("opc.tcp://127.0.0.1:5002");
    // 调用测试服务器上的方法
    rm::Variables input = {1, 2};
    auto [res, output] = cli.call("add", input);
    EXPECT_TRUE(res);
    EXPECT_EQ(rm::Variable::cast<int>(output[0]), 3);

    cli.shutdown();
    srv.shutdown();
    t.join();
}

// 高级方法调用
TEST(OPC_UA_ClientTest, callx) {
    rm::OpcuaServer srv(5003);
    configServer(srv);
    std::thread t(&rm::OpcuaServer::spin, &srv);
    std::this_thread::sleep_for(10ms);
    rm::OpcuaClient cli("opc.tcp://127.0.0.1:5003");
    // 以 callx 调用测试服务器上的方法
    auto [res, output] = cli.callx("add", 1, 2);
    EXPECT_TRUE(res);
    EXPECT_EQ(rm::Variable::cast<int>(output[0]), 3);

    cli.shutdown();
    srv.shutdown();
    t.join();
}

// 自动路径搜索方法调用
TEST(OPC_UA_ClientTest, findcall) {
    rm::OpcuaServer srv(5004);
    configServer(srv);
    std::thread t(&rm::OpcuaServer::spin, &srv);
    std::this_thread::sleep_for(10ms);
    rm::OpcuaClient cli("opc.tcp://127.0.0.1:5004");
    // 以 findcall 调用测试服务器上的方法
    auto [res, output] = cli.findcall("person/name/get", {});
    EXPECT_TRUE(res);
    EXPECT_EQ(output[0], "Alice");

    std::tie(res, output) = cli.findcallx("person/name/get");
    EXPECT_TRUE(res);
    EXPECT_EQ(output[0], "Alice");

    cli.shutdown();
    srv.shutdown();
    t.join();
}

// 订阅
TEST(OPC_UA_ClientTest, variable_monitor) {
    rm::OpcuaServer srv(5005);
    configServer(srv);
    std::thread t(&rm::OpcuaServer::spin, &srv);
    std::this_thread::sleep_for(10ms);
    rm::OpcuaClient cli("opc.tcp://127.0.0.1:5005");
    // 订阅测试服务器上的变量
    int receive_data{};
    auto node_id = cli.find("single");
    auto on_change = [&](rm::OpcuaClientView, const rm::Variable &value) {
        receive_data = value;
    };
    EXPECT_TRUE(cli.monitor(node_id, on_change, 5));
    // 数据更新
    EXPECT_TRUE(cli.write(node_id, 66));
    std::this_thread::sleep_for(10ms);
    cli.spinOnce();
    EXPECT_EQ(receive_data, 66);
    // 移除监视后的数据按预期不会更新
    cli.remove(node_id);
    EXPECT_TRUE(cli.write(node_id, 123));
    std::this_thread::sleep_for(10ms);
    cli.spinOnce();
    EXPECT_EQ(receive_data, 66);

    cli.shutdown();
    srv.shutdown();
    t.join();
}

TEST(OPC_UA_ClientTest, event_monitor) {
    rm::OpcuaServer srv(5010);
    configServer(srv);
    std::thread t(&rm::OpcuaServer::spin, &srv);
    std::this_thread::sleep_for(10ms);
    rm::EventType etype;
    etype.browse_name = "TestEventType";
    etype.display_name = "测试事件类型";
    etype.description = "测试事件类型";
    etype.add("aaa", 3);
    srv.addEventTypeNode(etype);
    rm::OpcuaClient cli("opc.tcp://127.0.0.1:5010");

    std::string source_name{};
    std::string messgae{};
    int aaa{};
    cli.monitor({"Message", "SourceName", "aaa"}, [&](rm::OpcuaClientView, const rm::Variables &fields) {
        messgae = fields[0].cast<std::string>();
        source_name = fields[1].cast<std::string>();
        aaa = fields[2];
    });
    // 触发事件
    auto event = rm::Event::makeFrom(etype);
    event.source_name = "GtestServer";
    event.message = "this is test event";
    event["aaa"] = 66;
    EXPECT_TRUE(srv.triggerEvent(event));
    std::this_thread::sleep_for(10ms);
    cli.spinOnce();
    EXPECT_EQ(source_name, "GtestServer");
    EXPECT_EQ(aaa, 66);

    cli.shutdown();
    srv.shutdown();
    t.join();
}

TEST(OPC_UA_ClientTest, timer_test) {
    rm::OpcuaServer srv(5015);
    configServer(srv);
    std::thread t(&rm::OpcuaServer::spin, &srv);
    std::this_thread::sleep_for(10ms);
    rm::OpcuaClient cli("opc.tcp://127.0.0.1:5015");
    int times{};
    auto timer = rm::OpcuaClientTimer(cli, 10, [&]() { times++; });
    std::this_thread::sleep_for(60ms);
    cli.spinOnce();
    std::this_thread::sleep_for(60ms);
    cli.spinOnce();
    EXPECT_EQ(times, 2);

    cli.shutdown();
    srv.shutdown();
    t.join();
}

// 数据源变量读写
TEST(OPC_UA_ClientTest, data_source_variable_IO) {
    rm::OpcuaServer srv(5020);
    configServer(srv);
    std::thread t(&rm::OpcuaServer::spin, &srv);
    std::this_thread::sleep_for(10ms);
    rm::OpcuaClient cli("opc.tcp://127.0.0.1:5020");
    // 读取测试服务器上的变量值
    auto id = cli.find("data_source");
    EXPECT_TRUE(cli.write(id, 99));
    rm::Variable variable = cli.read(id);
    EXPECT_FALSE(variable.empty());
    int single_value = variable;
    EXPECT_EQ(single_value, 99);

    cli.shutdown();
    srv.shutdown();
    t.join();
}

} // namespace rm_test

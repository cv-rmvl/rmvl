/**
 * @file test_opcua_serverview.cpp
 * @author zhaoxi (535394140@qq.com)
 * @brief OPC UA 服务器视图单元测试
 * @version 1.0
 * @date 2024-07-06
 *
 * @copyright Copyright 2024 (c), zhaoxi
 *
 */

#include <thread>

#include <gtest/gtest.h>

#include "rmvl/opcua/client.hpp"
#include "rmvl/opcua/server.hpp"

namespace rm::rm_test
{

using namespace std::chrono_literals;

void setup(Server &srv)
{
    Variable num_var = 1;
    num_var.browse_name = "num";
    num_var.display_name = "Number";
    num_var.description = "数";
    srv.addVariableNode(num_var);

    Method method;
    method.browse_name = "plus";
    method.display_name = "Input + Number";
    method.description = "输入值加数";
    method.func = [](ServerView sv, const NodeId &obj_id, InputVariables inputs, OutputVariables) {
        auto num_node = obj_id | sv.find("num");
        int num = sv.read(num_node);
        Variable dst = inputs.front().cast<int>() + num;
        sv.write(num_node, dst);
        return true;
    };
    method.iargs = {{"input", UA_TYPES_INT32, 1, "输入值"}};
    srv.addMethodNode(method);
}

TEST(OPC_UA_ServerView, read_variable_in_method)
{
    Server srv(6000);
    setup(srv);
    std::thread t(&Server::spin, &srv);

    Client cli("opc.tcp://127.0.0.1:6000");
    ASSERT_TRUE(cli.ok());
    EXPECT_EQ(cli.read(nodeObjectsFolder | cli.find("num")), 1);
    std::vector<Variable> inputs = {2};
    std::vector<Variable> outputs;

    EXPECT_TRUE(cli.call("plus", inputs, outputs));
    EXPECT_EQ(cli.read(nodeObjectsFolder | cli.find("num")), 3);

    cli.shutdown();
    srv.shutdown();
    t.join();
}

} // namespace rm::rm_test

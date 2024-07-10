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
    method.func = [](UA_Server *p_server, const UA_NodeId *, void *, const UA_NodeId *, void *, const UA_NodeId *,
                     void *, size_t, const UA_Variant *inputs, size_t, UA_Variant *) -> UA_StatusCode {
        ServerView sv = p_server;
        auto num_node = nodeObjectsFolder | sv.find("num");
        int num = sv.read(num_node).cast<int>();
        Variable dst = *reinterpret_cast<int *>(inputs->data) + num;
        sv.write(num_node, dst);
        return UA_STATUSCODE_GOOD;
    };
    method.iargs = {{"input", UA_TYPES_INT32, 1, "输入值"}};
    srv.addMethodNode(method);

    srv.start();
    std::this_thread::sleep_for(10ms);
}

TEST(OPC_UA_ServerView, read_variable_in_method)
{
    Server srv(6000);
    setup(srv);

    Client clt("opc.tcp://127.0.0.1:6000");
    ASSERT_TRUE(clt.ok());
    EXPECT_EQ(clt.read(nodeObjectsFolder | clt.find("num")), 1);
    std::vector<Variable> inputs = {2};
    std::vector<Variable> outputs;

    EXPECT_TRUE(clt.call("plus", inputs, outputs));
    EXPECT_EQ(clt.read(nodeObjectsFolder | clt.find("num")), 3);
}

} // namespace rm::rm_test

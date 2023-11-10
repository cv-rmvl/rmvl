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

namespace rm_test
{

void setSvr(rm::Server &svr)
{
    // 添加数组变量节点
    rm::Variable variable = std::vector({1, 2, 3, 4, 5});
    variable.browse_name = "array";
    variable.description = "this is array";
    variable.display_name = "数组";
    svr.addVariableNode(variable);
    // 添加加法方法节点
    rm::Method method;
    method.browse_name = "add";
    method.description = "this is add method";
    method.display_name = "加法";
    method.iargs = {{"a", UA_TYPES_INT32}, {"b", UA_TYPES_INT32}};
    method.oargs = {{"c", UA_TYPES_INT32}};
    method.func = [](UA_Server *, const UA_NodeId *, void *, const UA_NodeId *, void *, const UA_NodeId *,
                     void *, size_t, const UA_Variant *input, size_t, UA_Variant *output) -> UA_StatusCode {
        int32_t a = *reinterpret_cast<int *>(input[0].data);
        int32_t b = *reinterpret_cast<int *>(input[1].data);
        int32_t c = a + b;
        return UA_Variant_setScalarCopy(output, &c, &UA_TYPES[UA_TYPES_INT32]);
    };
    svr.addMethodNode(method);
    // 添加对象节点，包含字符串变量和乘法方法
    svr.start();
    std::this_thread::sleep_for(std::chrono::milliseconds(5));
}

TEST(OPC_UA_ClientTest, read_variable)
{
    rm::Server svr(5000);
    setSvr(svr);
    rm::Client client("opc.tcp://localhost:5000");
    // 读取测试服务器上的变量值
    rm::Variable variable;
    auto id = UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER) | client.find("array");
    EXPECT_TRUE(client.read(id, variable));
    auto vec = rm::Variable::cast<std::vector<int>>(variable);
    for (size_t i = 0; i < vec.size(); ++i)
        EXPECT_EQ(vec[i], i + 1);
    svr.stop();
    svr.join();
}

TEST(OPC_UA_ClientTest, call)
{
    rm::Server svr(5002);
    setSvr(svr);
    rm::Client client("opc.tcp://localhost:5002");
    // 调用测试服务器上的方法
    std::vector<rm::Variable> input = {1, 2};
    std::vector<rm::Variable> output;
    EXPECT_TRUE(client.call("add", input, output));
    EXPECT_EQ(rm::Variable::cast<int>(output[0]), 3);
    svr.stop();
    svr.join();
}

} // namespace rm_test

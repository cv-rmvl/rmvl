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

// namespace rm_test
// {

// class OPC_UA_ClientTest : public testing::Test
// {
//     // 测试服务器对象
//     rm::Server server = rm::Server(4840);

// protected:
//     void SetUp() override
//     {
//         // 添加数组变量节点
//         rm::Variable variable = std::vector({1, 2, 3, 4, 5});
//         variable.browse_name = "array";
//         variable.description;
//         // 添加加法方法节点

//         // 添加对象节点，包含字符串变量和乘法方法
//         server.start();
//     }

//     void TearDown() override
//     {
//         server.stop();
//         server.join();
//     }
// };

// TEST_F(OPC_UA_ClientTest, ReadVariable)
// {
//     // 读取测试服务器上的变量值
//     int32_t value = 0;
//     EXPECT_TRUE(client_.ReadVariable<int32_t>("ns=1;s=TestVariable", value));
//     EXPECT_EQ(value, 42);
// }

// TEST_F(OPC_UA_ClientTest, WriteVariable)
// {
//     // 修改测试服务器上的变量值
//     EXPECT_TRUE(client_.WriteVariable<int32_t>("ns=1;s=TestVariable", 100));
//     // 验证变量值是否修改成功
//     int32_t value = 0;
//     EXPECT_TRUE(client_.ReadVariable<int32_t>("ns=1;s=TestVariable", value));
//     EXPECT_EQ(value, 100);
// }

// TEST_F(OPC_UA_ClientTest, CallMethod)
// {
//     // 调用测试服务器上的方法
//     int32_t result = 0;
//     EXPECT_TRUE(client_.CallMethod<int32_t>("ns=1;s=TestMethod", 10, 20, result));
//     EXPECT_EQ(result, 30);
// }

// } // namespace rm_test

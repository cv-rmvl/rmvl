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

// 值配置
TEST(OPC_UA_Server, value_config)
{
    // 添加变量类型节点
    rm::VariableType variable_type = "string_test";
    EXPECT_EQ(variable_type.getArrayDimensions(), 1);
    // 添加变量节点
    rm::Variable variable = 3.1415;
    EXPECT_EQ(variable.getArrayDimensions(), 1);
    EXPECT_EQ(variable.getDataType(), UA_TYPES_DOUBLE);
}

// 服务器添加节点
TEST(OPC_UA_Server, server_config_add_node)
{

}

// 服务器调用方法
TEST(OPC_UA_Server, server_config_call_method)
{

}

// 服务器节点服务端路径搜索
TEST(OPC_UA_Server, server_config_find_node)
{

}

} // namespace rm_test

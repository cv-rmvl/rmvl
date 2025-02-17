/**
 * @file test_opcua_addrspace.cpp
 * @author zhaoxi (535394140@qq.com)
 * @brief OPC UA 地址空间模型单元测试
 * @version 1.0
 * @date 2025-02-17
 *
 * @copyright Copyright 2025 (c), zhaoxi
 *
 */

#include <gtest/gtest.h>

#include "rmvl/opcua/object.hpp"

namespace rm_test
{

TEST(OPC_UA_AddressSpace, Variable)
{
    // 单值构造
    rm::Variable val1 = 42;
    EXPECT_EQ(val1.size(), 1);
    EXPECT_EQ(val1.getDataType(), rm::tpInt32);

    rm::Variable val2 = 3.1415;
    EXPECT_EQ(val2.size(), 1);
    EXPECT_EQ(val2.getDataType(), rm::tpDouble);

    rm::Variable val3 = false;
    EXPECT_EQ(val3.size(), 1);
    EXPECT_EQ(val3.getDataType(), rm::tpBoolean);

    rm::Variable val4 = "test";
    EXPECT_EQ(val4.size(), 1);
    EXPECT_EQ(val4.getDataType(), rm::tpString);

    rm::Variable val5 = std::string("test");
    EXPECT_EQ(val5.size(), 1);
    EXPECT_EQ(val5.getDataType(), rm::tpString);

    // 列表构造
    rm::Variable arr1 = {1, 2, 3};
    EXPECT_EQ(arr1.size(), 3);
    EXPECT_EQ(arr1.getDataType(), rm::tpInt32);

    rm::Variable arr2 = {3.142, 2.718, 1.414};
    EXPECT_EQ(arr2.size(), 3);
    EXPECT_EQ(arr2.getDataType(), rm::tpDouble);

    rm::Variable arr3 = std::vector{1, 2, 3};
    EXPECT_EQ(arr3.size(), 3);
    EXPECT_EQ(arr3.getDataType(), rm::tpInt32);

    // 单值比较
    EXPECT_EQ(val1, 42);
    EXPECT_EQ(val2, 3.1415);
    EXPECT_EQ(val3, false);
    EXPECT_EQ(val4, "test");
    EXPECT_EQ(val5, val4);

    // 列表比较
    EXPECT_EQ(arr1, arr3);
    std::vector<double> arr{3.142, 2.718, 1.414};
    EXPECT_EQ(arr2, arr);
}

TEST(OPC_UA_AddressSpace, VariableType)
{
    // 单值构造
    rm::VariableType vt1 = 42;
    EXPECT_EQ(vt1.size(), 1);
    EXPECT_EQ(vt1.getDataType(), rm::tpInt32);

    rm::VariableType vt2 = 3.1415;
    EXPECT_EQ(vt2.size(), 1);
    EXPECT_EQ(vt2.getDataType(), rm::tpDouble);

    rm::VariableType vt3 = false;
    EXPECT_EQ(vt3.size(), 1);
    EXPECT_EQ(vt3.getDataType(), rm::tpBoolean);

    rm::VariableType vt4 = "test";
    EXPECT_EQ(vt4.size(), 1);
    EXPECT_EQ(vt4.getDataType(), rm::tpString);

    rm::VariableType vt5 = std::string("test");
    EXPECT_EQ(vt5.size(), 1);
    EXPECT_EQ(vt5.getDataType(), rm::tpString);

    // 列表构造
    rm::VariableType at1 = {1, 2, 3};
    EXPECT_EQ(at1.size(), 3);
    EXPECT_EQ(at1.getDataType(), rm::tpInt32);

    rm::VariableType at2 = {3.142, 2.718, 1.414};
    EXPECT_EQ(at2.size(), 3);
    EXPECT_EQ(at2.getDataType(), rm::tpDouble);

    rm::VariableType at3 = std::vector{1, 2, 3};
    EXPECT_EQ(at3.size(), 3);
    EXPECT_EQ(at3.getDataType(), rm::tpInt32);
}

} // namespace rm_test
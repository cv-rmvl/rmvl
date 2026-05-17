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

#include "rmvl/opcua/variable.hpp"

namespace rm_test {

using namespace rm::ua;

TEST(OPC_UA_AddressSpace, Variable) {
    // 单值构造
    Variable val1 = 42;
    EXPECT_EQ(val1.size(), -1);
    EXPECT_EQ(val1.getDataType(), tpInt32);

    Variable val2 = 3.1415;
    EXPECT_EQ(val2.size(), -1);
    EXPECT_EQ(val2.getDataType(), tpDouble);

    Variable val3 = false;
    EXPECT_EQ(val3.size(), -1);
    EXPECT_EQ(val3.getDataType(), tpBoolean);

    Variable val4 = "test";
    EXPECT_EQ(val4.size(), -1);
    EXPECT_EQ(val4.getDataType(), tpString);

    Variable val5 = std::string("test");
    EXPECT_EQ(val5.size(), -1);
    EXPECT_EQ(val5.getDataType(), tpString);

    // 列表构造
    Variable arr1 = {1, 2, 3};
    EXPECT_EQ(arr1.size(), 3);
    EXPECT_EQ(arr1.getDataType(), tpInt32);

    Variable arr2 = {3.142, 2.718, 1.414};
    EXPECT_EQ(arr2.size(), 3);
    EXPECT_EQ(arr2.getDataType(), tpDouble);

    Variable arr3 = std::vector{1, 2, 3};
    EXPECT_EQ(arr3.size(), 3);
    EXPECT_EQ(arr3.getDataType(), tpInt32);

    Variable arr4 = {1};
    EXPECT_EQ(arr4.size(), 1);
    EXPECT_EQ(arr4.getDataType(), tpInt32);

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

TEST(OPC_UA_AddressSpace, VariableType) {
    // 单值构造
    VariableType vt1 = 42;
    EXPECT_EQ(vt1.size(), -1);
    EXPECT_EQ(vt1.getDataType(), tpInt32);

    VariableType vt2 = 3.1415;
    EXPECT_EQ(vt2.size(), -1);
    EXPECT_EQ(vt2.getDataType(), tpDouble);

    VariableType vt3 = false;
    EXPECT_EQ(vt3.size(), -1);
    EXPECT_EQ(vt3.getDataType(), tpBoolean);

    VariableType vt4 = "test";
    EXPECT_EQ(vt4.size(), -1);
    EXPECT_EQ(vt4.getDataType(), tpString);

    VariableType vt5 = std::string("test");
    EXPECT_EQ(vt5.size(), -1);
    EXPECT_EQ(vt5.getDataType(), tpString);

    // 列表构造
    VariableType at1 = {1, 2, 3};
    EXPECT_EQ(at1.size(), 3);
    EXPECT_EQ(at1.getDataType(), tpInt32);

    VariableType at2 = {3.142, 2.718, 1.414};
    EXPECT_EQ(at2.size(), 3);
    EXPECT_EQ(at2.getDataType(), tpDouble);

    VariableType at3 = std::vector{1, 2, 3};
    EXPECT_EQ(at3.size(), 3);
    EXPECT_EQ(at3.getDataType(), tpInt32);
}

} // namespace rm_test
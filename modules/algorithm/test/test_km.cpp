/**
 * @file test_km.cpp
 * @author zhaoxi (535394140@qq.com)
 * @brief KM 求解器测试
 * @version 1.0
 * @date 2024-09-07
 *
 * @copyright Copyright 2024 (c), zhaoxi
 *
 */

#include <gtest/gtest.h>

#include "rmvl/algorithm/math.hpp"

TEST(Munkres, test1)
{
    std::vector<std::vector<double>> cost = {
        {1, 2, 3},
        {3, 2, 1},
        {2, 1, 3}};
    rm::Munkres km(cost);
    auto result = km.solve();
    EXPECT_EQ(result.size(), 3);
    EXPECT_EQ(result[0], 0);
    EXPECT_EQ(result[1], 2);
    EXPECT_EQ(result[2], 1);
}

TEST(Munkres, test2)
{
    std::vector<std::vector<double>> cost = {
        {2, 3, 4},
        {4, 3, 2},
        {3, 2, 4},
        {4, 2, 1}};
    rm::Munkres km(cost);
    auto result = km.solve();
    EXPECT_EQ(result.size(), 4);
    EXPECT_EQ(result[0], 0);
    EXPECT_EQ(result[1], 2);
    EXPECT_EQ(result[2], 1);
    EXPECT_EQ(result[3], 0); // not match
}

TEST(Munkres, test3)
{
    std::vector<std::vector<double>> cost = {
        {82, 83, 69, 92},
        {77, 37, 49, 92},
        {11, 69, 5, 86},
        {8, 9, 98, 23}};
    rm::Munkres km(cost);
    auto result = km.solve();
    EXPECT_EQ(result.size(), 4);
    EXPECT_EQ(result[0], 2);
    EXPECT_EQ(result[1], 1);
    EXPECT_EQ(result[2], 0);
    EXPECT_EQ(result[3], 3);
}

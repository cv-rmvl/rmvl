/**
 * @file test_types.cpp
 * @author RoboMaster Vision Community
 * @brief RMStatus 类型测试
 * @version 1.0
 * @date 2023-09-16
 *
 * @copyright Copyright 2023 (c), RoboMaster Vision Community
 *
 */

#include <gtest/gtest.h>

#include "rmvl/types.hpp"

namespace rm::rm_test
{

TEST(RMStatus_test, add_type)
{
    StateInfo state;
    state.add("robot", "hero");
    state.add("armor_size", "big");
    EXPECT_EQ(state.at_string("robot"), "hero");
    EXPECT_EQ(state.at_string("armor_size"), "big");
    state.add("value", 1.3);
    EXPECT_EQ(state.at_numeric("value"), 1.3);
    state["armor_size"] = 2.5;
    EXPECT_EQ(state.at_numeric("armor_size"), 2.5);
}

} // namespace rm::rm_test

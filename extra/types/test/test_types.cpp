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
    state.add("robot: hero, armor_size: big");
    EXPECT_EQ(state.at("robot"), "hero");
    EXPECT_EQ(state.at("armor_size"), "big");
}

} // namespace rm::rm_test

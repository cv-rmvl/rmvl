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

namespace rm_test
{

TEST(RMStatus_test, tag_type_to_string)
{
    auto str = std::string{};
    str = rm::RMStatus::to_string(rm::TagType::NUM_0);
    EXPECT_EQ(str, "0");
    str = rm::RMStatus::to_string(rm::TagType::CHAR_K);
    EXPECT_EQ(str, "K");
}

TEST(RMStatus_test, robot_type_to_string)
{
    auto str = std::string{};
    str = rm::RMStatus::to_string(rm::RobotType{});
    EXPECT_EQ(str, "UNKNOWN");
    str = rm::RMStatus::to_string(rm::RobotType::INFANTRY_4);
    EXPECT_EQ(str, "INFANTRY_4");
    str = rm::RMStatus::to_string(static_cast<rm::RobotType>(1));
    EXPECT_EQ(str, "HERO");
}

} // namespace rm_test

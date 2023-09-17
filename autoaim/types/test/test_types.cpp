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

TEST(RMStatus_test, default_type_to_string)
{
    auto str = std::string{};
    str = rm::RMStatus::to_string(rm::ArmorSizeType::SMALL);
    EXPECT_EQ(str, "1");
    str = rm::RMStatus::to_string(rm::CompensateType::LEFT);
    EXPECT_EQ(str, "3");
    str = rm::RMStatus::to_string(2);
    EXPECT_EQ(str, "2");
}

TEST(RMStatus_test, robot_type_to_string)
{
    auto str = std::string{};
    str = rm::RMStatus::to_string({});
    EXPECT_EQ(str, "UNKNOWN");
    str = rm::RMStatus::to_string(rm::RobotType::INFANTRY_4);
    EXPECT_EQ(str, "INFANTRY_4");
    str = rm::RMStatus::to_string(static_cast<rm::RobotType>(1));
    EXPECT_EQ(str, "HERO");
}

} // namespace rm_test

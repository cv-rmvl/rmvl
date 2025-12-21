/**
 * @file test_serialization.cpp
 * @author zhaoxi (535394140@qq.com)
 * @brief
 * @version 1.0
 * @date 2025-12-25
 *
 * @copyright Copyright 2025 (c), zhaoxi
 *
 */

#include <gtest/gtest.h>

#include "rmvlmsg/sensor/imu.hpp"
#include "rmvlmsg/sensor/joint_state.hpp"

namespace rm_test {

using namespace rm;

TEST(LPSS_serialization, joint) {
    lpss::msg::JointState msg;
    msg.header.frame_id = "joint_frame";
    msg.header.stamp = 100.5;
    msg.position = {1.0, 2.0, 3.0};
    msg.velocity = {0.1, 0.2, 0.3};
    msg.effort = {0.01, 0.02, 0.03};

    auto str = msg.serialize();

    auto dst = lpss::msg::JointState::deserialize(str.data());

    EXPECT_EQ(dst.header.frame_id, "joint_frame");
    EXPECT_DOUBLE_EQ(dst.header.stamp, 100.5);
    EXPECT_DOUBLE_EQ(dst.position[0], 1.0);
    EXPECT_DOUBLE_EQ(dst.position[1], 2.0);
    EXPECT_DOUBLE_EQ(dst.position[2], 3.0);
    EXPECT_DOUBLE_EQ(dst.velocity[0], 0.1);
    EXPECT_DOUBLE_EQ(dst.velocity[1], 0.2);
    EXPECT_DOUBLE_EQ(dst.velocity[2], 0.3);
    EXPECT_DOUBLE_EQ(dst.effort[0], 0.01);
    EXPECT_DOUBLE_EQ(dst.effort[1], 0.02);
    EXPECT_DOUBLE_EQ(dst.effort[2], 0.03);
}

TEST(LPSS_serialization, imu) {
    lpss::msg::Imu msg;
    msg.header.frame_id = "imu_frame";
    msg.header.stamp = 200.5;
    msg.orientation = {0.0, 0.0, 0.0, 1.0};
    msg.angular_velocity = {0.1, 0.2, 0.3};
    msg.linear_acceleration = {9.8, 0.0, 0.0};

    auto str = msg.serialize();

    auto dst = lpss::msg::Imu::deserialize(str.data());

    EXPECT_EQ(dst.header.frame_id, "imu_frame");
    EXPECT_DOUBLE_EQ(dst.header.stamp, 200.5);
    EXPECT_DOUBLE_EQ(dst.orientation.x, 0.0);
    EXPECT_DOUBLE_EQ(dst.orientation.y, 0.0);
    EXPECT_DOUBLE_EQ(dst.orientation.z, 0.0);
    EXPECT_DOUBLE_EQ(dst.orientation.w, 1.0);
    EXPECT_DOUBLE_EQ(dst.angular_velocity.x, 0.1);
    EXPECT_DOUBLE_EQ(dst.angular_velocity.y, 0.2);
    EXPECT_DOUBLE_EQ(dst.angular_velocity.z, 0.3);
    EXPECT_DOUBLE_EQ(dst.linear_acceleration.x, 9.8);
    EXPECT_DOUBLE_EQ(dst.linear_acceleration.y, 0.0);
    EXPECT_DOUBLE_EQ(dst.linear_acceleration.z, 0.0);
}

} // namespace rm_test

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

#include "rmvlmsg/std/bool.hpp"
#include "rmvlmsg/std/string.hpp"
#include "rmvlmsg/std/int32.hpp"
#include "rmvlmsg/sensor/imu.hpp"
#include "rmvlmsg/sensor/joint_state.hpp"

namespace rm_test {

using namespace rm;

TEST(LPSS_serialization, bool) {
    lpss::msg::Bool msg;
    msg.data = true;

    auto str = msg.serialize();

    auto dst = lpss::msg::Bool::deserialize(str.data());
    EXPECT_EQ(dst.data, true);
}

TEST(LPSS_serialization, string) {
    lpss::msg::String msg;
    msg.data = "Hello, LPSS!";

    auto str = msg.serialize();

    auto dst = lpss::msg::String::deserialize(str.data());
    EXPECT_EQ(dst.data, "Hello, LPSS!");
}

TEST(LPSS_serialization, int32) {
    lpss::msg::Int32 msg;
    msg.data = 42;

    auto str = msg.serialize();

    auto dst = lpss::msg::Int32::deserialize(str.data());
    EXPECT_EQ(dst.data, 42);
}

TEST(LPSS_serialization, header) {
    lpss::msg::Header msg;
    msg.seq = 1;
    msg.frame_id = "base_link";
    msg.stamp = 123.456;

    auto str = msg.serialize();

    auto dst = lpss::msg::Header::deserialize(str.data());
    EXPECT_EQ(dst.seq, 1);
    EXPECT_EQ(dst.frame_id, "base_link");
    EXPECT_DOUBLE_EQ(dst.stamp, 123.456);
}

TEST(LPSS_serialization, quaternion) {
    lpss::msg::Quaternion msg;
    msg.x = 0.7;
    msg.y = 0.0;
    msg.z = 0.7;
    msg.w = 0.0;

    auto str = msg.serialize();

    auto dst = lpss::msg::Quaternion::deserialize(str.data());
    EXPECT_DOUBLE_EQ(dst.x, 0.7);
    EXPECT_DOUBLE_EQ(dst.y, 0.0);
    EXPECT_DOUBLE_EQ(dst.z, 0.7);
    EXPECT_DOUBLE_EQ(dst.w, 0.0);
}

TEST(LPSS_serialization, vector3) {
    lpss::msg::Vector3 msg;
    msg.x = 1.0;
    msg.y = 2.0;
    msg.z = 3.0;

    auto str = msg.serialize();

    auto dst = lpss::msg::Vector3::deserialize(str.data());
    EXPECT_DOUBLE_EQ(dst.x, 1.0);
    EXPECT_DOUBLE_EQ(dst.y, 2.0);
    EXPECT_DOUBLE_EQ(dst.z, 3.0);
}

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

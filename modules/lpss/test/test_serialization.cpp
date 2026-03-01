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
#include "rmvlmsg/geometry/polygon.hpp"

namespace rm_test {

using namespace rm;

TEST(LPSS_serialization, bool) {
    msg::Bool msg;
    msg.data = true;

    auto str = msg.serialize();

    auto dst = msg::Bool::deserialize(str.data());
    EXPECT_EQ(dst.data, true);
}

TEST(LPSS_serialization, string) {
    msg::String msg;
    msg.data = "Hello, LPSS!";

    auto str = msg.serialize();

    auto dst = msg::String::deserialize(str.data());
    EXPECT_EQ(dst.data, "Hello, LPSS!");
}

TEST(LPSS_serialization, int32) {
    msg::Int32 msg;
    msg.data = 42;

    auto str = msg.serialize();

    auto dst = msg::Int32::deserialize(str.data());
    EXPECT_EQ(dst.data, 42);
}

TEST(LPSS_serialization, header) {
    msg::Header msg;
    msg.seq = 1;
    msg.frame_id = "base_link";
    msg.stamp = 123456;

    auto str = msg.serialize();

    auto dst = msg::Header::deserialize(str.data());
    EXPECT_EQ(dst.seq, 1);
    EXPECT_EQ(dst.frame_id, "base_link");
    EXPECT_EQ(dst.stamp, 123456);
}

TEST(LPSS_serialization, quaternion) {
    msg::Quaternion msg;
    msg.x = 0.7;
    msg.y = 0.0;
    msg.z = 0.7;
    msg.w = 0.0;

    auto str = msg.serialize();

    auto dst = msg::Quaternion::deserialize(str.data());
    EXPECT_DOUBLE_EQ(dst.x, 0.7);
    EXPECT_DOUBLE_EQ(dst.y, 0.0);
    EXPECT_DOUBLE_EQ(dst.z, 0.7);
    EXPECT_DOUBLE_EQ(dst.w, 0.0);
}

TEST(LPSS_serialization, vector3) {
    msg::Vector3 msg;
    msg.x = 1.0;
    msg.y = 2.0;
    msg.z = 3.0;

    auto str = msg.serialize();

    auto dst = msg::Vector3::deserialize(str.data());
    EXPECT_DOUBLE_EQ(dst.x, 1.0);
    EXPECT_DOUBLE_EQ(dst.y, 2.0);
    EXPECT_DOUBLE_EQ(dst.z, 3.0);
}

TEST(LPSS_serialization, joint) {
    msg::JointState msg;
    msg.header.frame_id = "joint_frame";
    msg.header.stamp = 100;
    msg.position = {1.0, 2.0, 3.0};
    msg.velocity = {0.1, 0.2, 0.3};
    msg.effort = {0.01, 0.02, 0.03};

    auto str = msg.serialize();

    auto dst = msg::JointState::deserialize(str.data());

    EXPECT_EQ(dst.header.frame_id, "joint_frame");
    EXPECT_EQ(dst.header.stamp, 100);
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

TEST(LPSS_serialization, joint_with_names) {
    msg::JointState msg;
    msg.header.seq = 10;
    msg.header.frame_id = "robot_arm";
    msg.header.stamp = 999;
    msg.name = {"joint1", "joint2", "joint3"};
    msg.position = {0.5, 1.5, 2.5};
    msg.velocity = {-0.1, -0.2, -0.3};
    msg.effort = {10.0, 20.0, 30.0};

    auto str = msg.serialize();
    auto dst = msg::JointState::deserialize(str.data());

    EXPECT_EQ(dst.header.seq, 10);
    EXPECT_EQ(dst.header.frame_id, "robot_arm");
    EXPECT_EQ(dst.header.stamp, 999);
    ASSERT_EQ(dst.name.size(), 3u);
    EXPECT_EQ(dst.name[0], "joint1");
    EXPECT_EQ(dst.name[1], "joint2");
    EXPECT_EQ(dst.name[2], "joint3");
    ASSERT_EQ(dst.position.size(), 3u);
    EXPECT_DOUBLE_EQ(dst.position[0], 0.5);
    EXPECT_DOUBLE_EQ(dst.position[1], 1.5);
    EXPECT_DOUBLE_EQ(dst.position[2], 2.5);
    ASSERT_EQ(dst.velocity.size(), 3u);
    EXPECT_DOUBLE_EQ(dst.velocity[0], -0.1);
    EXPECT_DOUBLE_EQ(dst.velocity[1], -0.2);
    EXPECT_DOUBLE_EQ(dst.velocity[2], -0.3);
    ASSERT_EQ(dst.effort.size(), 3u);
    EXPECT_DOUBLE_EQ(dst.effort[0], 10.0);
    EXPECT_DOUBLE_EQ(dst.effort[1], 20.0);
    EXPECT_DOUBLE_EQ(dst.effort[2], 30.0);
}

TEST(LPSS_serialization, joint_empty_arrays) {
    msg::JointState msg;
    msg.header.frame_id = "empty";
    msg.header.stamp = 0;

    auto str = msg.serialize();
    auto dst = msg::JointState::deserialize(str.data());

    EXPECT_EQ(dst.header.frame_id, "empty");
    EXPECT_EQ(dst.header.stamp, 0);
    EXPECT_TRUE(dst.name.empty());
    EXPECT_TRUE(dst.position.empty());
    EXPECT_TRUE(dst.velocity.empty());
    EXPECT_TRUE(dst.effort.empty());
}

TEST(LPSS_serialization, joint_different_array_sizes) {
    msg::JointState msg;
    msg.header.frame_id = "flex";
    msg.header.stamp = 42;
    msg.name = {"a", "bb", "ccc", "dddd", "eeeee"};
    msg.position = {1.0, 2.0};
    msg.velocity = {3.0};
    msg.effort = {};

    auto str = msg.serialize();
    auto dst = msg::JointState::deserialize(str.data());

    EXPECT_EQ(dst.header.frame_id, "flex");
    EXPECT_EQ(dst.header.stamp, 42);
    ASSERT_EQ(dst.name.size(), 5u);
    EXPECT_EQ(dst.name[0], "a");
    EXPECT_EQ(dst.name[1], "bb");
    EXPECT_EQ(dst.name[2], "ccc");
    EXPECT_EQ(dst.name[3], "dddd");
    EXPECT_EQ(dst.name[4], "eeeee");
    ASSERT_EQ(dst.position.size(), 2u);
    EXPECT_DOUBLE_EQ(dst.position[0], 1.0);
    EXPECT_DOUBLE_EQ(dst.position[1], 2.0);
    ASSERT_EQ(dst.velocity.size(), 1u);
    EXPECT_DOUBLE_EQ(dst.velocity[0], 3.0);
    EXPECT_TRUE(dst.effort.empty());
}

TEST(LPSS_serialization, joint_compact_size) {
    msg::JointState msg;
    msg.header.frame_id = "sz";
    msg.header.stamp = 1;
    msg.name = {"x"};
    msg.position = {1.0, 2.0};
    msg.velocity = {};
    msg.effort = {3.0};

    auto str = msg.serialize();
    EXPECT_EQ(str.size(), msg.compact_size());
}

TEST(LPSS_serialization, imu) {
    msg::Imu msg;
    msg.header.frame_id = "imu_frame";
    msg.header.stamp = 123;
    msg.orientation = {0.0, 0.0, 0.0, 1.0};
    msg.angular_velocity = {0.1, 0.2, 0.3};
    msg.linear_acceleration = {9.8, 0.0, 0.0};

    auto str = msg.serialize();

    auto dst = msg::Imu::deserialize(str.data());

    EXPECT_EQ(dst.header.frame_id, "imu_frame");
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

TEST(LPSS_serialization, polygon_basic) {
    msg::Polygon msg;
    msg.points = {{1.0f, 2.0f, 3.0f}, {4.0f, 5.0f, 6.0f}, {7.0f, 8.0f, 9.0f}};

    auto str = msg.serialize();
    auto dst = msg::Polygon::deserialize(str.data());

    ASSERT_EQ(dst.points.size(), 3u);
    EXPECT_FLOAT_EQ(dst.points[0].x, 1.0f);
    EXPECT_FLOAT_EQ(dst.points[0].y, 2.0f);
    EXPECT_FLOAT_EQ(dst.points[0].z, 3.0f);
    EXPECT_FLOAT_EQ(dst.points[1].x, 4.0f);
    EXPECT_FLOAT_EQ(dst.points[1].y, 5.0f);
    EXPECT_FLOAT_EQ(dst.points[1].z, 6.0f);
    EXPECT_FLOAT_EQ(dst.points[2].x, 7.0f);
    EXPECT_FLOAT_EQ(dst.points[2].y, 8.0f);
    EXPECT_FLOAT_EQ(dst.points[2].z, 9.0f);
}

TEST(LPSS_serialization, polygon_empty) {
    msg::Polygon msg;

    auto str = msg.serialize();
    auto dst = msg::Polygon::deserialize(str.data());

    EXPECT_TRUE(dst.points.empty());
}

TEST(LPSS_serialization, polygon_single_point) {
    msg::Polygon msg;
    msg.points = {{-1.5f, 0.0f, 2.5f}};

    auto str = msg.serialize();
    auto dst = msg::Polygon::deserialize(str.data());

    ASSERT_EQ(dst.points.size(), 1u);
    EXPECT_FLOAT_EQ(dst.points[0].x, -1.5f);
    EXPECT_FLOAT_EQ(dst.points[0].y, 0.0f);
    EXPECT_FLOAT_EQ(dst.points[0].z, 2.5f);
}

TEST(LPSS_serialization, polygon_compact_size) {
    msg::Polygon msg;
    msg.points = {{0.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 1.0f}};

    auto str = msg.serialize();
    EXPECT_EQ(str.size(), msg.compact_size());
}

} // namespace rm_test

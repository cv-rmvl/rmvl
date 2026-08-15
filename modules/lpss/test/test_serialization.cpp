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

#include "nlohmann/json.hpp"

#include "rmvlmsg/std/bool.hpp"
#include "rmvlmsg/std/duration.hpp"
#include "rmvlmsg/std/string.hpp"
#include "rmvlmsg/std/int32.hpp"
#include "rmvlmsg/motion/joint_trajectory_point.hpp"
#include "rmvlmsg/sensor/imu.hpp"
#include "rmvlmsg/sensor/joint_state.hpp"
#include "rmvlmsg/geometry/polygon.hpp"
#include "rmvlsrv/sensor/set_camera_info.hpp"
#include "rmvlsrv/std/empty.hpp"
#include "rmvlsrv/std/set_bool.hpp"
#include "rmvlsrv/std/trigger.hpp"

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

TEST(LPSS_serialization, string_json) {
    msg::String msg;
    msg.data = "Hello, LPSS!";

    auto j = rm::json::parse(msg.json());

    EXPECT_EQ(j["data"].get<std::string>(), "Hello, LPSS!");
}

TEST(LPSS_serialization, int32) {
    msg::Int32 msg;
    msg.data = 42;

    auto str = msg.serialize();

    auto dst = msg::Int32::deserialize(str.data());
    EXPECT_EQ(dst.data, 42);
}

TEST(LPSS_serialization, duration) {
    msg::Duration msg;
    msg.nanoseconds = -1'700'000'000;

    auto str = msg.serialize();
    auto dst = msg::Duration::deserialize(str.data());

    EXPECT_EQ(str.size(), sizeof(int64_t));
    EXPECT_EQ(dst.nanoseconds, -1'700'000'000);
    EXPECT_EQ(rm::json::parse(msg.json())["nanoseconds"].get<int64_t>(), -1'700'000'000);
}

TEST(LPSS_serialization, builtin_duration) {
    msg::JointTrajectoryPoint msg;
    msg.time_from_start.nanoseconds = 123'456'789;

    auto str = msg.serialize();
    auto dst = msg::JointTrajectoryPoint::deserialize(str.data());

    EXPECT_EQ(dst.time_from_start.nanoseconds, 123'456'789);
    EXPECT_EQ(rm::json::parse(msg.json())["time_from_start"]["nanoseconds"].get<int64_t>(), 123'456'789);
}

TEST(LPSS_serialization, header) {
    msg::Header msg;
    msg.frame_id = "base_link";
    msg.stamp = {123456, 789};

    auto str = msg.serialize();

    auto dst = msg::Header::deserialize(str.data());
    EXPECT_EQ(dst.frame_id, "base_link");
    EXPECT_EQ(dst.stamp.sec, 123456);
    EXPECT_EQ(dst.stamp.nsec, 789);
}

TEST(LPSS_serialization, header_json) {
    msg::Header msg;
    msg.frame_id = "base_link";
    msg.stamp = {123456, 789};

    auto j = rm::json::parse(msg.json());

    EXPECT_EQ(j["frame_id"].get<std::string>(), "base_link");
    EXPECT_EQ(j["stamp"]["sec"].get<int32_t>(), 123456);
    EXPECT_EQ(j["stamp"]["nsec"].get<uint32_t>(), 789);
}

TEST(LPSS_serialization, quaternion) {
    msg::Quaternion msg;
    msg.x = 0.7071;
    msg.y = 0.0;
    msg.z = 0.7071;
    msg.w = 0.0;

    auto str = msg.serialize();

    auto dst = msg::Quaternion::deserialize(str.data());
    EXPECT_DOUBLE_EQ(dst.x, 0.7071);
    EXPECT_DOUBLE_EQ(dst.y, 0.0);
    EXPECT_DOUBLE_EQ(dst.z, 0.7071);
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
    msg.position = {1.0, 2.0, 3.0};
    msg.velocity = {0.1, 0.2, 0.3};
    msg.effort = {0.01, 0.02, 0.03};

    auto str = msg.serialize();

    auto dst = msg::JointState::deserialize(str.data());

    EXPECT_EQ(dst.header.frame_id, "joint_frame");
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

TEST(LPSS_serialization, joint_json) {
    msg::JointState msg;
    msg.header.frame_id = "robot_arm";
    msg.name = {"joint1", "joint2"};
    msg.position = {0.5, 1.5};
    msg.velocity = {-0.1, -0.2};
    msg.effort = {10.0, 20.0};

    auto j = rm::json::parse(msg.json());

    EXPECT_EQ(j["header"]["frame_id"].get<std::string>(), "robot_arm");
    ASSERT_EQ(j["name"].size(), 2u);
    EXPECT_EQ(j["name"][0].get<std::string>(), "joint1");
    EXPECT_EQ(j["name"][1].get<std::string>(), "joint2");
    ASSERT_EQ(j["position"].size(), 2u);
    EXPECT_DOUBLE_EQ(j["position"][0].get<double>(), 0.5);
    EXPECT_DOUBLE_EQ(j["position"][1].get<double>(), 1.5);
    ASSERT_EQ(j["velocity"].size(), 2u);
    EXPECT_DOUBLE_EQ(j["velocity"][0].get<double>(), -0.1);
    EXPECT_DOUBLE_EQ(j["velocity"][1].get<double>(), -0.2);
    ASSERT_EQ(j["effort"].size(), 2u);
    EXPECT_DOUBLE_EQ(j["effort"][0].get<double>(), 10.0);
    EXPECT_DOUBLE_EQ(j["effort"][1].get<double>(), 20.0);
}

TEST(LPSS_serialization, joint_with_names) {
    msg::JointState msg;
    msg.header.frame_id = "robot_arm";
    msg.name = {"joint1", "joint2", "joint3"};
    msg.position = {0.5, 1.5, 2.5};
    msg.velocity = {-0.1, -0.2, -0.3};
    msg.effort = {10.0, 20.0, 30.0};

    auto str = msg.serialize();
    auto dst = msg::JointState::deserialize(str.data());

    EXPECT_EQ(dst.header.frame_id, "robot_arm");
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

    auto str = msg.serialize();
    auto dst = msg::JointState::deserialize(str.data());

    EXPECT_EQ(dst.header.frame_id, "empty");
    EXPECT_TRUE(dst.name.empty());
    EXPECT_TRUE(dst.position.empty());
    EXPECT_TRUE(dst.velocity.empty());
    EXPECT_TRUE(dst.effort.empty());
}

TEST(LPSS_serialization, joint_different_array_sizes) {
    msg::JointState msg;
    msg.header.frame_id = "flex";
    msg.name = {"a", "bb", "ccc", "dddd", "eeeee"};
    msg.position = {1.0, 2.0};
    msg.velocity = {3.0};
    msg.effort = {};

    auto str = msg.serialize();
    auto dst = msg::JointState::deserialize(str.data());

    EXPECT_EQ(dst.header.frame_id, "flex");
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

TEST(LPSS_serialization, empty_service) {
    srv::Empty::Request request{};
    srv::Empty::Response response{};
    auto request_data = request.serialize();
    auto response_data = response.serialize();

    EXPECT_TRUE(request_data.empty());
    EXPECT_TRUE(response_data.empty());
    EXPECT_EQ(request.compact_size(), 0u);
    EXPECT_EQ(response.compact_size(), 0u);
    EXPECT_EQ(srv::Empty::Request::deserialize(request_data.data()).compact_size(), 0u);
    EXPECT_EQ(srv::Empty::Response::deserialize(response_data.data()).compact_size(), 0u);
}

TEST(LPSS_serialization, set_bool_service) {
    srv::SetBool::Request request{};
    request.data = true;
    auto request_data = request.serialize();
    auto decoded_request = srv::SetBool::Request::deserialize(request_data.data());
    EXPECT_TRUE(decoded_request.data);
    EXPECT_EQ(request_data.size(), request.compact_size());

    srv::SetBool::Response response{};
    response.success = true;
    response.message = "enabled";
    auto response_data = response.serialize();
    auto decoded_response = srv::SetBool::Response::deserialize(response_data.data());
    EXPECT_TRUE(decoded_response.success);
    EXPECT_EQ(decoded_response.message, "enabled");
    EXPECT_EQ(response_data.size(), response.compact_size());
}

TEST(LPSS_serialization, trigger_service) {
    srv::Trigger::Response response{};
    response.success = false;
    response.message = "not ready";
    auto data = response.serialize();
    auto decoded = srv::Trigger::Response::deserialize(data.data());

    EXPECT_FALSE(decoded.success);
    EXPECT_EQ(decoded.message, "not ready");
}

TEST(LPSS_serialization, nested_message_service) {
    srv::SetCameraInfo::Request request{};
    request.camera_info.header.frame_id = "camera";
    request.camera_info.width = 1920;
    request.camera_info.height = 1080;
    request.camera_info.K[0] = 1000.0;
    auto data = request.serialize();
    auto decoded = srv::SetCameraInfo::Request::deserialize(data.data());

    EXPECT_EQ(decoded.camera_info.header.frame_id, "camera");
    EXPECT_EQ(decoded.camera_info.width, 1920u);
    EXPECT_EQ(decoded.camera_info.height, 1080u);
    EXPECT_DOUBLE_EQ(decoded.camera_info.K[0], 1000.0);
    EXPECT_EQ(data.size(), request.compact_size());
}

} // namespace rm_test

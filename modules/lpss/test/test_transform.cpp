/**
 * @file test_transform.cpp
 * @author zhaoxi (535394140@qq.com)
 * @brief tf::Buffer 单元测试
 * @version 1.0
 * @date 2026-08-16
 *
 * @copyright Copyright 2026 (c), zhaoxi
 *
 */

#include <chrono>
#include <cmath>
#include <limits>

#include <gtest/gtest.h>

#include "rmvl/lpss/node.hpp"
#include "rmvl/lpss/transform.hpp"

namespace rm_test {

using namespace rm;
using namespace rm::lpss::tf;
using namespace std::chrono_literals;

namespace {

constexpr double kEps = 1e-9;

msg::Quaternion yawQuaternion(double yaw) {
    return {0.0, 0.0, std::sin(yaw * 0.5), std::cos(yaw * 0.5)};
}

msg::TransformStamped makeTransform(
    const char *parent,
    const char *child,
    int32_t sec,
    double x = 0.0,
    double y = 0.0,
    double z = 0.0,
    msg::Quaternion rotation = {0.0, 0.0, 0.0, 1.0}) {
    msg::TransformStamped result{};
    result.header.frame_id = parent;
    result.header.stamp.sec = sec;
    result.child_frame_id = child;
    result.transform.translation = {x, y, z};
    result.transform.rotation = rotation;
    return result;
}

void expectIdentity(const msg::Transform &transform) {
    EXPECT_NEAR(transform.translation.x, 0.0, kEps);
    EXPECT_NEAR(transform.translation.y, 0.0, kEps);
    EXPECT_NEAR(transform.translation.z, 0.0, kEps);
    EXPECT_NEAR(transform.rotation.x, 0.0, kEps);
    EXPECT_NEAR(transform.rotation.y, 0.0, kEps);
    EXPECT_NEAR(transform.rotation.z, 0.0, kEps);
    EXPECT_NEAR(std::abs(transform.rotation.w), 1.0, kEps);
}

} // namespace

TEST(LPSS_tf_Buffer, static_transform_direction_and_composition) {
    EXPECT_STREQ(to_string(Status::ConnectivityError), "frames are not connected");
    constexpr double pi = 3.14159265358979323846;
    Buffer buffer;
    EXPECT_EQ(buffer.setStatic(makeTransform("map", "odom", 0, 10.0)), Status::Ok);
    EXPECT_EQ(buffer.setStatic(makeTransform("odom", "base_link", 0, 1.0, 0.0, 0.0, yawQuaternion(pi / 2.0))), Status::Ok);

    const auto lookup = buffer.lookup("map", "base_link");
    ASSERT_TRUE(lookup);
    EXPECT_EQ(lookup.transform.header.frame_id, "map");
    EXPECT_EQ(lookup.transform.child_frame_id, "base_link");
    EXPECT_NEAR(lookup.transform.transform.translation.x, 11.0, kEps);
    EXPECT_NEAR(lookup.transform.transform.translation.y, 0.0, kEps);

    const msg::Point point_in_base{1.0, 0.0, 0.0};
    const auto point_in_map = lookup.transform.transform * point_in_base;
    EXPECT_NEAR(point_in_map.x, 11.0, kEps);
    EXPECT_NEAR(point_in_map.y, 1.0, kEps);

    const auto inverse_lookup = buffer.lookup("base_link", "map");
    ASSERT_TRUE(inverse_lookup);
    expectIdentity(inverse_lookup.transform.transform * lookup.transform.transform);
}

TEST(LPSS_tf_Buffer, query_between_sibling_frames) {
    Buffer buffer;
    ASSERT_EQ(buffer.setStatic(makeTransform("map", "left", 0, 2.0)), Status::Ok);
    ASSERT_EQ(buffer.setStatic(makeTransform("map", "right", 0, -3.0)), Status::Ok);

    const auto lookup = buffer.lookup("left", "right");
    ASSERT_TRUE(lookup);
    EXPECT_NEAR(lookup.transform.transform.translation.x, -5.0, kEps);
}

TEST(LPSS_tf_Buffer, dynamic_transform_interpolates_translation_and_rotation) {
    constexpr double pi = 3.14159265358979323846;
    Buffer buffer(20s);
    ASSERT_EQ(buffer.set(makeTransform("odom", "base_link", 10, 0.0)), Status::Ok);
    ASSERT_EQ(buffer.set(makeTransform("odom", "base_link", 20, 10.0, 0.0, 0.0, yawQuaternion(pi))), Status::Ok);

    const auto lookup = buffer.lookup("odom", "base_link", {15, 0});
    ASSERT_TRUE(lookup);
    EXPECT_NEAR(lookup.transform.transform.translation.x, 5.0, kEps);

    const auto rotated = msg::rotate(lookup.transform.transform.rotation, {1.0, 0.0, 0.0});
    EXPECT_NEAR(rotated.x, 0.0, kEps);
    EXPECT_NEAR(rotated.y, 1.0, kEps);
}

TEST(LPSS_tf_Buffer, latest_query_uses_path_common_time) {
    Buffer buffer(30s);
    ASSERT_EQ(buffer.set(makeTransform("map", "odom", 10, 10.0)), Status::Ok);
    ASSERT_EQ(buffer.set(makeTransform("map", "odom", 30, 30.0)), Status::Ok);
    ASSERT_EQ(buffer.set(makeTransform("odom", "base_link", 20, 200.0)), Status::Ok);
    ASSERT_EQ(buffer.set(makeTransform("odom", "base_link", 40, 400.0)), Status::Ok);

    const auto lookup = buffer.lookup("map", "base_link");
    ASSERT_TRUE(lookup);
    EXPECT_EQ(lookup.transform.header.stamp.sec, 30);
    EXPECT_EQ(lookup.transform.header.stamp.nsec, 0u);
    EXPECT_NEAR(lookup.transform.transform.translation.x, 330.0, kEps);
}

TEST(LPSS_tf_Buffer, latest_query_rejects_non_overlapping_histories) {
    Buffer buffer(30s);
    ASSERT_EQ(buffer.set(makeTransform("map", "odom", 1)), Status::Ok);
    ASSERT_EQ(buffer.set(makeTransform("map", "odom", 2)), Status::Ok);
    ASSERT_EQ(buffer.set(makeTransform("odom", "base_link", 3)), Status::Ok);
    ASSERT_EQ(buffer.set(makeTransform("odom", "base_link", 4)), Status::Ok);

    EXPECT_EQ(buffer.lookup("map", "base_link").status, Status::NoCommonTime);
}

TEST(LPSS_tf_Buffer, explicit_query_rejects_extrapolation) {
    Buffer buffer;
    ASSERT_EQ(buffer.set(makeTransform("odom", "base_link", 10)), Status::Ok);
    ASSERT_EQ(buffer.set(makeTransform("odom", "base_link", 20)), Status::Ok);

    EXPECT_EQ(buffer.lookup("odom", "base_link", {9, 0}).status, Status::ExtrapolationPast);
    EXPECT_EQ(buffer.lookup("odom", "base_link", {21, 0}).status, Status::ExtrapolationFuture);
    EXPECT_FALSE(buffer.can("odom", "base_link", {21, 0}));
}

TEST(LPSS_tf_Buffer, dynamic_history_is_sorted_replaced_and_pruned) {
    Buffer buffer(10s);
    ASSERT_EQ(buffer.set(makeTransform("odom", "base_link", 10, 10.0)), Status::Ok);
    ASSERT_EQ(buffer.set(makeTransform("odom", "base_link", 5, 5.0)), Status::Ok);
    ASSERT_EQ(buffer.set(makeTransform("odom", "base_link", 10, 12.0)), Status::Ok);

    auto lookup = buffer.lookup("odom", "base_link", {10, 0});
    ASSERT_TRUE(lookup);
    EXPECT_NEAR(lookup.transform.transform.translation.x, 12.0, kEps);

    ASSERT_EQ(buffer.set(makeTransform("odom", "base_link", 20, 20.0)), Status::Ok);
    EXPECT_EQ(buffer.lookup("odom", "base_link", {5, 0}).status, Status::ExtrapolationPast);
    EXPECT_EQ(buffer.set(makeTransform("odom", "base_link", 9)), Status::TimestampOutOfRange);

    buffer.setCacheDuration(-1s);
    EXPECT_EQ(buffer.cacheDuration(), 0ns);
    EXPECT_EQ(buffer.lookup("odom", "base_link", {10, 0}).status, Status::ExtrapolationPast);
    EXPECT_TRUE(buffer.lookup("odom", "base_link", {20, 0}));
}

TEST(LPSS_tf_Buffer, rejects_multiple_parents_cycles_and_edge_mode_changes) {
    Buffer buffer;
    ASSERT_EQ(buffer.setStatic(makeTransform("map", "odom", 0)), Status::Ok);
    EXPECT_EQ(buffer.setStatic(makeTransform("world", "odom", 0)), Status::MultipleParents);
    EXPECT_EQ(buffer.setStatic(makeTransform("odom", "map", 0)), Status::CycleDetected);
    EXPECT_EQ(buffer.set(makeTransform("map", "odom", 1)), Status::StaticDynamicConflict);
    EXPECT_EQ(buffer.size(), 1u);
}

TEST(LPSS_tf_Buffer, rejects_invalid_inputs) {
    Buffer buffer;
    EXPECT_EQ(buffer.set(makeTransform("", "base_link", 1)), Status::InvalidArgument);
    EXPECT_EQ(buffer.set(makeTransform("map", "map", 1)), Status::InvalidArgument);

    auto invalid_time = makeTransform("map", "base_link", 1);
    invalid_time.header.stamp.nsec = 1'000'000'000u;
    EXPECT_EQ(buffer.set(invalid_time), Status::InvalidArgument);

    auto zero_quaternion = makeTransform("map", "base_link", 1);
    zero_quaternion.transform.rotation = {};
    EXPECT_EQ(buffer.set(zero_quaternion), Status::InvalidTransform);

    auto nan_translation = makeTransform("map", "base_link", 1);
    nan_translation.transform.translation.x = std::numeric_limits<double>::quiet_NaN();
    EXPECT_EQ(buffer.set(nan_translation), Status::InvalidTransform);
}

TEST(LPSS_tf_Buffer, distinguishes_unknown_and_disconnected_frames) {
    Buffer buffer;
    ASSERT_EQ(buffer.setStatic(makeTransform("map", "odom", 0)), Status::Ok);
    ASSERT_EQ(buffer.setStatic(makeTransform("world", "camera", 0)), Status::Ok);

    EXPECT_EQ(buffer.lookup("map", "missing").status, Status::FrameNotFound);
    EXPECT_EQ(buffer.lookup("map", "camera").status, Status::ConnectivityError);

    const auto identity = buffer.lookup("map", "map", {7, 8});
    ASSERT_TRUE(identity);
    EXPECT_EQ(identity.transform.header.stamp.sec, 7);
    EXPECT_EQ(identity.transform.header.stamp.nsec, 8u);
    expectIdentity(identity.transform.transform);

    buffer.clear();
    EXPECT_EQ(buffer.size(), 0u);
    EXPECT_EQ(buffer.lookup("map", "map").status, Status::FrameNotFound);
}

TEST(LPSS_tf_Buffer, inserts_dynamic_and_static_batches) {
    Buffer buffer;
    msg::TF dynamic{};
    dynamic.transforms = {
        makeTransform("map", "odom", 10, 1.0),
        makeTransform("odom", "base_link", 10, 2.0),
    };
    msg::TF fixed{};
    fixed.transforms = {makeTransform("base_link", "camera", 0, 0.5)};

    EXPECT_EQ(buffer.set(dynamic), Status::Ok);
    EXPECT_EQ(buffer.setStatic(fixed), Status::Ok);
    const auto result = buffer.lookup("map", "camera", {10, 0});
    ASSERT_TRUE(result);
    EXPECT_NEAR(result.transform.transform.translation.x, 3.5, kEps);
}

TEST(LPSS_tf_transport, sync_endpoints_are_released) {
    lpss::Node node("tf_sync_lifecycle", 45);
    Buffer buffer;
    {
        lpss::tf::Listener listener("robot", node, buffer);
        Broadcaster broadcaster("robot", node);
        StaticBroadcaster static_broadcaster("robot", node);
        EXPECT_FALSE(listener.invalid());
        EXPECT_FALSE(broadcaster.invalid());
        EXPECT_FALSE(static_broadcaster.invalid());
        broadcaster.send(msg::TF{});
        static_broadcaster.send(msg::TF{});
    }

    auto dynamic_pub = node.createPublisher<msg::TF>("robot/tf");
    auto static_pub = node.createPublisher<msg::TF>("robot/tf_static");
    auto dynamic_sub = node.createSubscriber<msg::TF>("robot/tf", [](const msg::TF &) {});
    auto static_sub = node.createSubscriber<msg::TF>("robot/tf_static", [](const msg::TF &) {});
    EXPECT_FALSE(dynamic_pub.invalid());
    EXPECT_FALSE(static_pub.invalid());
    EXPECT_FALSE(dynamic_sub.invalid());
    EXPECT_FALSE(static_sub.invalid());
    node.destroySubscriber(dynamic_sub);
    node.destroySubscriber(static_sub);
    node.destroyPublisher(dynamic_pub);
    node.destroyPublisher(static_pub);
}

#if __cplusplus >= 202002L

TEST(LPSS_tf_transport, async_endpoints_are_released) {
    lpss::async::Node node("tf_async_lifecycle", 46);
    Buffer buffer;
    {
        lpss::tf::Listener listener("robot", node, buffer);
        Broadcaster broadcaster("robot", node);
        StaticBroadcaster static_broadcaster("robot", node);
        EXPECT_FALSE(listener.invalid());
        EXPECT_FALSE(broadcaster.invalid());
        EXPECT_FALSE(static_broadcaster.invalid());
        broadcaster.send(msg::TF{});
        static_broadcaster.send(msg::TF{});
    }

    auto dynamic_pub = node.createPublisher<msg::TF>("robot/tf");
    auto static_pub = node.createPublisher<msg::TF>("robot/tf_static");
    auto dynamic_sub = node.createSubscriber<msg::TF>("robot/tf", [](const msg::TF &) {});
    auto static_sub = node.createSubscriber<msg::TF>("robot/tf_static", [](const msg::TF &) {});
    ASSERT_NE(dynamic_pub, nullptr);
    ASSERT_NE(static_pub, nullptr);
    ASSERT_NE(dynamic_sub, nullptr);
    ASSERT_NE(static_sub, nullptr);
    EXPECT_FALSE(dynamic_pub->invalid());
    EXPECT_FALSE(static_pub->invalid());
    EXPECT_FALSE(dynamic_sub->invalid());
    EXPECT_FALSE(static_sub->invalid());
    node.destroySubscriber(dynamic_sub);
    node.destroySubscriber(static_sub);
    node.destroyPublisher(dynamic_pub);
    node.destroyPublisher(static_pub);
}

#endif

} // namespace rm_test

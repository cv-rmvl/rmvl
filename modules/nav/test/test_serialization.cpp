/**
 * @file test_serialization.cpp
 * @author zhaoxi (535394140@qq.com)
 * @brief 导航消息序列化单元测试
 * @version 1.0
 * @date 2026-08-17
 *
 * @copyright Copyright 2026 (c), zhaoxi
 *
 */

#include <cstdint>
#include <limits>
#include <string>

#include <gtest/gtest.h>

#include "nlohmann/json.hpp"

#include "rmvlmsg/nav/occupancy_grid.hpp"
#include "rmvlmsg/nav/occupancy_grid_update.hpp"
#include "rmvlmsg/nav/odometry.hpp"
#include "rmvlmsg/nav/path.hpp"

namespace rm_test {

using namespace rm;

template <typename Message>
static json jsonRoundTrip(const Message &message) {
    return json::parse(json::parse(message.json()).dump());
}

TEST(Nav_serialization, odometry) {
    msg::Odometry source;
    source.header.stamp = {1000, 42};
    source.header.frame_id = "odom";
    source.child_frame_id = "base_link";
    source.pose.pose.position = {4.0, 5.0, 0.0};
    source.pose.pose.orientation.w = 1.0;
    source.pose.covariance[0] = 0.02;
    source.twist.twist.linear.x = 0.8;
    source.twist.twist.angular.z = 0.1;
    source.twist.covariance[35] = 0.03;

    const auto data = source.serialize();
    const auto decoded = msg::Odometry::deserialize(data.data());
    EXPECT_EQ(data.size(), source.compact_size());
    EXPECT_EQ(decoded.header.frame_id, "odom");
    EXPECT_EQ(decoded.child_frame_id, "base_link");
    EXPECT_DOUBLE_EQ(decoded.pose.pose.position.y, 5.0);
    EXPECT_DOUBLE_EQ(decoded.twist.twist.linear.x, 0.8);
    EXPECT_DOUBLE_EQ(decoded.twist.covariance[35], 0.03);

    const auto j = jsonRoundTrip(source);
    EXPECT_EQ(j["child_frame_id"].get<std::string>(), "base_link");
    EXPECT_DOUBLE_EQ(j["twist"]["twist"]["angular"]["z"].get<double>(), 0.1);
}

TEST(Nav_serialization, path) {
    msg::Path source;
    source.header.stamp = {2000, 0};
    source.header.frame_id = "map";
    source.poses.resize(2);
    source.poses[0].header.frame_id = "map";
    source.poses[0].pose.position = {1.0, 2.0, 0.0};
    source.poses[0].pose.orientation.w = 1.0;
    source.poses[1].header.frame_id = "map";
    source.poses[1].header.stamp = {2001, 0};
    source.poses[1].pose.position = {3.0, 4.0, 0.0};
    source.poses[1].pose.orientation.w = 1.0;

    const auto data = source.serialize();
    const auto decoded = msg::Path::deserialize(data.data());
    EXPECT_EQ(data.size(), source.compact_size());
    ASSERT_EQ(decoded.poses.size(), 2u);
    EXPECT_EQ(decoded.poses[0].header.frame_id, "map");
    EXPECT_DOUBLE_EQ(decoded.poses[1].pose.position.x, 3.0);

    const auto j = jsonRoundTrip(source);
    ASSERT_EQ(j["poses"].size(), 2u);
    EXPECT_EQ(j["poses"][1]["header"]["frame_id"].get<std::string>(), "map");
}

TEST(Nav_serialization, map_meta_data_max_dimensions) {
    msg::MapMetaData source;
    source.map_load_time = {3000, 999};
    source.resolution = 0.05f;
    source.width = std::numeric_limits<uint32_t>::max();
    source.height = std::numeric_limits<uint32_t>::max();
    source.origin.position = {-10.0, -20.0, 0.0};
    source.origin.orientation.w = 1.0;

    const auto data = source.serialize();
    const auto decoded = msg::MapMetaData::deserialize(data.data());
    EXPECT_EQ(data.size(), source.compact_size());
    EXPECT_EQ(decoded.width, std::numeric_limits<uint32_t>::max());
    EXPECT_EQ(decoded.height, std::numeric_limits<uint32_t>::max());
    EXPECT_FLOAT_EQ(decoded.resolution, 0.05f);
    EXPECT_DOUBLE_EQ(decoded.origin.position.x, -10.0);

    const auto j = jsonRoundTrip(source);
    EXPECT_EQ(j["width"].get<uint32_t>(), std::numeric_limits<uint32_t>::max());
    EXPECT_EQ(j["height"].get<uint32_t>(), std::numeric_limits<uint32_t>::max());
}

TEST(Nav_serialization, occupancy_grid) {
    msg::OccupancyGrid source;
    source.header.stamp = {4000, 0};
    source.header.frame_id = "map";
    source.info.map_load_time = {3900, 0};
    source.info.resolution = 0.1f;
    source.info.width = 3;
    source.info.height = 2;
    source.info.origin.orientation.w = 1.0;
    source.revision = 17;
    source.data = {-1, 0, 25, 50, 75, 100};

    const auto data = source.serialize();
    const auto decoded = msg::OccupancyGrid::deserialize(data.data());
    EXPECT_EQ(data.size(), source.compact_size());
    EXPECT_EQ(decoded.revision, 17u);
    EXPECT_EQ(decoded.data.size(), static_cast<std::size_t>(decoded.info.width) * decoded.info.height);
    ASSERT_EQ(decoded.data.size(), 6u);
    EXPECT_EQ(decoded.data.front(), -1);
    EXPECT_EQ(decoded.data.back(), 100);

    const auto j = jsonRoundTrip(source);
    ASSERT_EQ(j["data"].size(), 6u);
    EXPECT_TRUE(j["data"][0].is_number_integer());
    EXPECT_EQ(j["data"][0].get<int>(), -1);
    EXPECT_EQ(j["data"][5].get<int>(), 100);
}

TEST(Nav_serialization, occupancy_grid_update) {
    msg::OccupancyGridUpdate source;
    source.header.stamp = {5000, 0};
    source.header.frame_id = "map";
    source.base_revision = 17;
    source.revision = 18;
    source.x = 4;
    source.y = 5;
    source.width = 2;
    source.height = 2;
    source.data = {-1, 0, 80, 100};

    const auto data = source.serialize();
    const auto decoded = msg::OccupancyGridUpdate::deserialize(data.data());
    EXPECT_EQ(data.size(), source.compact_size());
    EXPECT_EQ(decoded.base_revision, 17u);
    EXPECT_EQ(decoded.revision, 18u);
    EXPECT_EQ(decoded.x, 4u);
    EXPECT_EQ(decoded.y, 5u);
    EXPECT_EQ(decoded.data.size(), static_cast<std::size_t>(decoded.width) * decoded.height);
    ASSERT_EQ(decoded.data.size(), 4u);
    EXPECT_EQ(decoded.data.front(), -1);
    EXPECT_EQ(decoded.data.back(), 100);

    const auto j = jsonRoundTrip(source);
    ASSERT_EQ(j["data"].size(), 4u);
    EXPECT_TRUE(j["data"][0].is_number_integer());
    EXPECT_EQ(j["data"][0].get<int>(), -1);
    EXPECT_EQ(j["base_revision"].get<uint64_t>(), 17u);
    EXPECT_EQ(j["revision"].get<uint64_t>(), 18u);
}

} // namespace rm_test

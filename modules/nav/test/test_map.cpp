/**
 * @file test_map.cpp
 * @author zhaoxi (535394140@qq.com)
 * @brief 二维栅格地图与代价地图单元测试
 * @version 1.0
 * @date 2026-08-16
 *
 * @copyright Copyright 2026 (c), zhaoxi
 *
 */

#include <cmath>
#include <limits>

#include <gtest/gtest.h>

#include "rmvl/nav/map.hpp"

namespace rm_test {

using namespace rm;
using namespace rm::nav;

namespace {

constexpr double kEps = 1e-9;

msg::OccupancyGrid makeGrid(uint32_t width, uint32_t height, int8_t value = -1, double resolution = 1.0) {
    msg::OccupancyGrid result{};
    result.header.frame_id = "map";
    result.info.resolution = static_cast<float>(resolution);
    result.info.width = width;
    result.info.height = height;
    result.info.origin.orientation.w = 1.0;
    result.data.assign(static_cast<std::size_t>(width) * height, value);
    return result;
}

msg::Pose planarPose(double x, double y, double yaw = 0.0) {
    msg::Pose result{};
    result.position = {x, y, 0.0};
    result.orientation.z = std::sin(yaw * 0.5);
    result.orientation.w = std::cos(yaw * 0.5);
    return result;
}

} // namespace

TEST(Nav_GridMap, validates_full_map_without_replacing_previous_state) {
    EXPECT_STREQ(to_string(MapStatus::InvalidResolution), "invalid map resolution");
    GridMap map;
    EXPECT_FALSE(map.valid());

    auto valid = makeGrid(3, 2, 0, 0.5);
    valid.revision = 7;
    ASSERT_EQ(map.reset(valid), MapStatus::Ok);
    EXPECT_TRUE(map.valid());
    EXPECT_EQ(map.revision(), 7u);

    auto invalid = valid;
    invalid.info.resolution = 0.0f;
    EXPECT_EQ(map.reset(invalid), MapStatus::InvalidResolution);
    EXPECT_EQ(map.revision(), 7u);

    invalid = valid;
    invalid.data.pop_back();
    EXPECT_EQ(map.reset(invalid), MapStatus::InvalidDimensions);

    invalid = valid;
    invalid.data[0] = 101;
    EXPECT_EQ(map.reset(invalid), MapStatus::InvalidValue);

    invalid = valid;
    invalid.info.origin.orientation = {0.1, 0.0, 0.0, 1.0};
    EXPECT_EQ(map.reset(invalid), MapStatus::InvalidOrigin);
    EXPECT_EQ(map.message().data.size(), 6u);
}

TEST(Nav_GridMap, converts_rotated_world_and_grid_coordinates) {
    constexpr double pi = 3.14159265358979323846;
    auto source = makeGrid(4, 3, 0);
    source.info.origin = planarPose(10.0, 20.0, pi / 2.0);
    GridMap map(source);
    ASSERT_TRUE(map.valid());

    const auto world = map.mapToWorld(0, 0);
    ASSERT_TRUE(world);
    EXPECT_NEAR(world->x, 9.5, kEps);
    EXPECT_NEAR(world->y, 20.5, kEps);

    const auto cell = map.worldToMap(world->x, world->y);
    ASSERT_TRUE(cell);
    EXPECT_EQ(cell->x, 0u);
    EXPECT_EQ(cell->y, 0u);
    EXPECT_FALSE(map.worldToMap(11.0, 20.0));
    EXPECT_FALSE(map.mapToWorld(4, 0));
}

TEST(Nav_GridMap, updates_occupancy_probability_and_revision) {
    auto source = makeGrid(2, 1);
    source.revision = 10;
    GridMap map(source);

    ASSERT_EQ(map.updateProbability(0, 0, 0.7), MapStatus::Ok);
    EXPECT_EQ(map.at(0, 0), 70);
    EXPECT_EQ(map.revision(), 11u);
    ASSERT_EQ(map.updateProbability(0, 0, 0.7), MapStatus::Ok);
    EXPECT_EQ(map.at(0, 0), 84);
    EXPECT_EQ(map.revision(), 12u);

    ASSERT_EQ(map.set(1, 0, 100), MapStatus::Ok);
    EXPECT_EQ(map.revision(), 13u);
    EXPECT_EQ(map.set(1, 0, 100), MapStatus::Ok);
    EXPECT_EQ(map.revision(), 13u);
    EXPECT_EQ(map.set(1, 0, -2), MapStatus::InvalidValue);
    EXPECT_EQ(map.updateProbability(1, 0, std::numeric_limits<double>::quiet_NaN()), MapStatus::InvalidProbability);
    EXPECT_EQ(map.updateProbability(2, 0, 0.5), MapStatus::OutOfBounds);
    ASSERT_EQ(map.updateProbability(1, 0, 0.3), MapStatus::Ok);
    EXPECT_LT(*map.at(1, 0), 100);
    EXPECT_EQ(map.revision(), 14u);
}

TEST(Nav_GridMap, applies_rectangular_updates_atomically) {
    auto source = makeGrid(4, 3, 0);
    source.revision = 5;
    GridMap map(source);

    msg::OccupancyGridUpdate update{};
    update.header.frame_id = "map";
    update.header.stamp = {20, 30};
    update.base_revision = 5;
    update.revision = 6;
    update.x = 1;
    update.y = 1;
    update.width = 2;
    update.height = 2;
    update.data = {10, 20, 30, 40};
    ASSERT_EQ(map.apply(update), MapStatus::Ok);
    EXPECT_EQ(map.at(1, 1), 10);
    EXPECT_EQ(map.at(2, 1), 20);
    EXPECT_EQ(map.at(1, 2), 30);
    EXPECT_EQ(map.at(2, 2), 40);
    EXPECT_EQ(map.revision(), 6u);

    EXPECT_FALSE(map.pendingUpdate());

    auto invalid = update;
    invalid.base_revision = 4;
    EXPECT_EQ(map.apply(invalid), MapStatus::RevisionMismatch);
    invalid = update;
    invalid.base_revision = 6;
    invalid.revision = 6;
    EXPECT_EQ(map.apply(invalid), MapStatus::InvalidRevision);
    invalid.revision = 7;
    invalid.x = 3;
    EXPECT_EQ(map.apply(invalid), MapStatus::OutOfBounds);
    EXPECT_EQ(map.revision(), 6u);
}

TEST(Nav_GridMap, tracks_local_changes_as_a_consistent_dirty_rectangle) {
    auto source = makeGrid(5, 4, 0);
    source.revision = 8;
    GridMap map(source);
    ASSERT_EQ(map.set(1, 1, 10), MapStatus::Ok);
    ASSERT_EQ(map.set(3, 2, 20), MapStatus::Ok);

    const auto update = map.pendingUpdate();
    ASSERT_TRUE(update);
    EXPECT_EQ(update->base_revision, 8u);
    EXPECT_EQ(update->revision, 10u);
    EXPECT_EQ(update->x, 1u);
    EXPECT_EQ(update->y, 1u);
    EXPECT_EQ(update->width, 3u);
    EXPECT_EQ(update->height, 2u);
    EXPECT_EQ(update->data, (std::vector<int8_t>{10, 0, 0, 0, 0, 20}));

    GridMap receiver(source);
    ASSERT_EQ(receiver.apply(*update), MapStatus::Ok);
    EXPECT_EQ(receiver.message().data, map.message().data);
    EXPECT_EQ(receiver.revision(), map.revision());

    map.clearPendingUpdate();
    EXPECT_FALSE(map.pendingUpdate());
}

TEST(Nav_GridMap, rejects_revision_overflow_and_invalid_remote_updates) {
    auto source = makeGrid(2, 2, 0);
    source.revision = std::numeric_limits<uint64_t>::max();
    GridMap map(source);
    EXPECT_EQ(map.set(0, 0, 1), MapStatus::InvalidRevision);
    EXPECT_EQ(map.at(0, 0), 0);
    EXPECT_FALSE(map.pendingUpdate());

    source.revision = 3;
    ASSERT_EQ(map.reset(source), MapStatus::Ok);
    msg::OccupancyGridUpdate update{};
    update.header.frame_id = "odom";
    update.base_revision = 3;
    update.revision = 4;
    update.width = 1;
    update.height = 1;
    update.data = {100};
    EXPECT_EQ(map.apply(update), MapStatus::FrameMismatch);
    update.header.frame_id = "map";
    update.data = {101};
    EXPECT_EQ(map.apply(update), MapStatus::InvalidValue);
    EXPECT_EQ(map.at(0, 0), 0);
    EXPECT_EQ(map.revision(), 3u);
}

TEST(Nav_GridMap, integrates_clipped_sensor_rays) {
    GridMap map(makeGrid(5, 3));
    ASSERT_EQ(map.integrateRay(-2.0, 1.5, 3.5, 1.5), MapStatus::Ok);
    EXPECT_EQ(map.revision(), 1u);
    EXPECT_EQ(map.at(0, 1), 30);
    EXPECT_EQ(map.at(1, 1), 30);
    EXPECT_EQ(map.at(2, 1), 30);
    EXPECT_EQ(map.at(3, 1), 70);
    EXPECT_EQ(map.at(4, 1), -1);

    EXPECT_EQ(map.integrateRay(-2.0, -2.0, -1.0, -1.0), MapStatus::OutOfBounds);
    ASSERT_EQ(map.integrateRay(3.5, 1.5, 8.0, 1.5, true), MapStatus::Ok);
    EXPECT_LT(*map.at(3, 1), 70);
    EXPECT_EQ(map.at(4, 1), 30);
}

TEST(Nav_Costmap, builds_static_layer_and_inflates_lethal_obstacles) {
    auto source = makeGrid(7, 7, 0);
    source.data[3 * 7 + 3] = 100;
    source.data[0] = -1;
    GridMap grid(source);
    CostmapOptions options{};
    options.inflation_radius = 2.0;
    options.inscribed_radius = 1.0;
    options.cost_scaling_factor = 1.0;
    Costmap costmap(grid, options);
    ASSERT_TRUE(costmap.valid());

    EXPECT_EQ(costmap.at(3, 3), Lethal);
    EXPECT_EQ(costmap.at(4, 3), Inscribed);
    ASSERT_TRUE(costmap.at(5, 3));
    EXPECT_GT(*costmap.at(5, 3), Free);
    EXPECT_LT(*costmap.at(5, 3), Inscribed);
    EXPECT_EQ(costmap.at(6, 6), Free);
    EXPECT_EQ(costmap.at(0, 0), Unknown);
}

TEST(Nav_Costmap, maintains_local_obstacle_layer_and_ray_clearing) {
    GridMap grid(makeGrid(6, 3, 0));
    CostmapOptions options{};
    options.inflation_radius = 0.0;
    options.inscribed_radius = 0.0;
    Costmap costmap(grid, options);

    ASSERT_EQ(costmap.markObstacle(Cell{2, 1}), MapStatus::Ok);
    EXPECT_EQ(costmap.at(2, 1), Free);
    costmap.updateCosts();
    EXPECT_EQ(costmap.at(2, 1), Lethal);

    ASSERT_EQ(costmap.clearRay(-1.0, 1.5, 4.5, 1.5), MapStatus::Ok);
    costmap.updateCosts();
    EXPECT_EQ(costmap.at(2, 1), Free);
    EXPECT_EQ(costmap.markObstacle(20.0, 20.0), MapStatus::OutOfBounds);

    ASSERT_EQ(costmap.markObstacle(Cell{5, 2}), MapStatus::Ok);
    ASSERT_EQ(costmap.clearObstacles(), MapStatus::Ok);
    costmap.updateCosts();
    EXPECT_EQ(costmap.at(5, 2), Free);
}

TEST(Nav_Costmap, updates_static_layer_only_for_matching_geometry) {
    GridMap grid(makeGrid(3, 2, 0));
    CostmapOptions options{};
    options.inflation_radius = 0.0;
    options.inscribed_radius = 0.0;
    Costmap costmap(grid, options);

    auto updated = grid;
    ASSERT_EQ(updated.set(1, 1, 100), MapStatus::Ok);
    ASSERT_EQ(costmap.setStaticMap(updated), MapStatus::Ok);
    EXPECT_EQ(costmap.at(1, 1), Lethal);

    auto shifted_message = updated.message();
    shifted_message.info.origin.position.x = 1.0;
    GridMap shifted(shifted_message);
    EXPECT_EQ(costmap.setStaticMap(shifted), MapStatus::GeometryMismatch);

    options.inscribed_radius = 1.0;
    options.inflation_radius = 0.5;
    EXPECT_EQ(costmap.reset(grid, options), MapStatus::InvalidOptions);
}

TEST(Nav_Costmap, detects_footprint_collision_and_map_boundary) {
    GridMap grid(makeGrid(10, 10, 0));
    CostmapOptions options{};
    options.inflation_radius = 0.0;
    options.inscribed_radius = 0.0;
    Costmap costmap(grid, options);
    ASSERT_EQ(costmap.markObstacle(Cell{5, 5}), MapStatus::Ok);
    costmap.updateCosts();

    const std::vector<msg::Point> footprint{{-0.4, -0.4, 0.0}, {0.4, -0.4, 0.0},
                                            {0.4, 0.4, 0.0}, {-0.4, 0.4, 0.0}};
    EXPECT_TRUE(costmap.collides(footprint, planarPose(5.5, 5.5)));
    EXPECT_FALSE(costmap.collides(footprint, planarPose(3.5, 3.5)));
    EXPECT_FALSE(costmap.collides(footprint, planarPose(3.5, 3.5, 1.0)));
    EXPECT_TRUE(costmap.collides(footprint, planarPose(0.2, 0.2)));
    EXPECT_TRUE(costmap.collides({}, planarPose(3.5, 3.5)));
}

TEST(Nav_Costmap, applies_unknown_space_collision_policy) {
    GridMap grid(makeGrid(4, 4));
    CostmapOptions options{};
    options.inflation_radius = 0.0;
    options.inscribed_radius = 0.0;
    options.unknown_is_lethal = false;
    Costmap costmap(grid, options);
    const std::vector<msg::Point> footprint{{-0.2, -0.2, 0.0}, {0.2, -0.2, 0.0},
                                            {0.2, 0.2, 0.0}, {-0.2, 0.2, 0.0}};
    EXPECT_FALSE(costmap.collides(footprint, planarPose(2.0, 2.0)));

    options.unknown_is_lethal = true;
    ASSERT_EQ(costmap.reset(grid, options), MapStatus::Ok);
    EXPECT_TRUE(costmap.collides(footprint, planarPose(2.0, 2.0)));
}

TEST(Nav_Costmap, exports_costs_as_occupancy_grid) {
    GridMap grid(makeGrid(3, 1, 0));
    CostmapOptions options{};
    options.inflation_radius = 1.0;
    options.inscribed_radius = 0.0;
    options.cost_scaling_factor = 1.0;
    Costmap costmap(grid, options);
    ASSERT_EQ(costmap.markObstacle(Cell{1, 0}), MapStatus::Ok);
    costmap.updateCosts();

    const auto message = costmap.message();
    ASSERT_EQ(message.data.size(), 3u);
    EXPECT_GT(message.data[0], 0);
    EXPECT_LT(message.data[0], 100);
    EXPECT_EQ(message.data[1], 100);
    EXPECT_GT(message.revision, grid.revision());
}

} // namespace rm_test

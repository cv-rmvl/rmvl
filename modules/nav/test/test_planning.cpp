/**
 * @file test_planning.cpp
 * @author zhaoxi (535394140@qq.com)
 * @brief 路径规划、跟踪与碰撞刹停单元测试
 * @version 1.0
 * @date 2026-08-17
 *
 * @copyright Copyright 2026 (c), zhaoxi
 *
 */

#include <cmath>
#include <cstdint>
#include <initializer_list>
#include <limits>
#include <utility>
#include <vector>

#include <gtest/gtest.h>

#include "rmvl/nav/controller.hpp"
#include "rmvl/nav/planner.hpp"

namespace rm_test {

using namespace rm;
using namespace rm::nav;

namespace {

msg::OccupancyGrid makeGrid(uint32_t width, uint32_t height, int8_t value = 0) {
    msg::OccupancyGrid grid{};
    grid.header.frame_id = "map";
    grid.info.resolution = 1.0F;
    grid.info.width = width;
    grid.info.height = height;
    grid.info.origin.orientation.w = 1.0;
    grid.data.assign(static_cast<std::size_t>(width) * height, value);
    return grid;
}

Costmap makeCostmap(msg::OccupancyGrid grid) {
    CostmapOptions options{};
    options.inflation_radius = 0.0;
    options.inscribed_radius = 0.0;
    return Costmap(GridMap(std::move(grid)), options);
}

msg::Pose pose(double x, double y, double yaw = 0.0) {
    msg::Pose result{};
    result.position = {x, y, 0.0};
    result.orientation.z = std::sin(yaw * 0.5);
    result.orientation.w = std::cos(yaw * 0.5);
    return result;
}

msg::Path path(std::initializer_list<msg::Point> points) {
    msg::Path result{};
    result.header.frame_id = "map";
    for (const auto &point : points) {
        msg::PoseStamped stamped{};
        stamped.header.frame_id = "map";
        stamped.pose.position = point;
        stamped.pose.orientation.w = 1.0;
        result.poses.push_back(std::move(stamped));
    }
    return result;
}

const std::vector<msg::Point> kFootprint{{-0.2, -0.2, 0.0}, {0.2, -0.2, 0.0},
                                          {0.2, 0.2, 0.0}, {-0.2, 0.2, 0.0}};

} // namespace

TEST(Nav_ToString, overloads_planning_and_tracking_status) {
    EXPECT_STREQ(to_string(PlanningStatus::NoPath), "no path");
    EXPECT_STREQ(to_string(TrackingStatus::GoalReached), "goal reached");
}

TEST(Nav_AStar, plans_through_wall_opening_and_keeps_path_traversable) {
    auto grid = makeGrid(10, 8);
    for (uint32_t y = 0; y < grid.info.height; ++y)
        if (y != 4)
            grid.data[static_cast<std::size_t>(y) * grid.info.width + 5] = 100;
    auto costmap = makeCostmap(std::move(grid));
    AStarOptions options{};
    options.simplify = false;
    options.smoothing_iterations = 0;
    AStarPlanner planner(options);
    const auto result = planner.plan(costmap, pose(1.5, 1.5), pose(8.5, 6.5));
    ASSERT_TRUE(result) << to_string(result.status);
    ASSERT_GE(result.path.poses.size(), 2u);
    EXPECT_EQ(result.path.header.frame_id, "map");
    EXPECT_DOUBLE_EQ(result.path.poses.front().pose.position.x, 1.5);
    EXPECT_DOUBLE_EQ(result.path.poses.back().pose.position.x, 8.5);
    bool crosses_opening = false;
    for (const auto &stamped : result.path.poses) {
        const auto cell = costmap.worldToMap(stamped.pose.position.x, stamped.pose.position.y);
        ASSERT_TRUE(cell);
        const auto cost = costmap.at(cell->x, cell->y);
        ASSERT_TRUE(cost);
        EXPECT_LT(*cost, Inscribed);
        crosses_opening = crosses_opening || (cell->x == 5 && cell->y == 4);
    }
    EXPECT_TRUE(crosses_opening);
    EXPECT_GT(result.expanded, 0u);
}

TEST(Nav_AStar, reports_blocked_endpoints_and_disconnected_goal) {
    auto grid = makeGrid(5, 5);
    grid.data[0] = 100;
    auto costmap = makeCostmap(grid);
    AStarPlanner planner;
    EXPECT_EQ(planner.plan(costmap, pose(0.5, 0.5), pose(4.5, 4.5)).status, PlanningStatus::StartBlocked);

    grid.data[0] = 0;
    for (uint32_t y = 0; y < 5; ++y)
        grid.data[static_cast<std::size_t>(y) * 5 + 2] = 100;
    costmap = makeCostmap(std::move(grid));
    EXPECT_EQ(planner.plan(costmap, pose(0.5, 2.5), pose(4.5, 2.5)).status, PlanningStatus::NoPath);
}

TEST(Nav_AStar, forbids_corner_cutting_unless_explicitly_enabled) {
    auto grid = makeGrid(2, 2);
    grid.data[1] = 100;
    grid.data[2] = 100;
    auto costmap = makeCostmap(std::move(grid));
    AStarOptions options{};
    options.simplify = false;
    options.smoothing_iterations = 0;
    AStarPlanner safe_planner(options);
    EXPECT_EQ(safe_planner.plan(costmap, pose(0.5, 0.5), pose(1.5, 1.5)).status, PlanningStatus::NoPath);

    options.allow_corner_cutting = true;
    AStarPlanner permissive_planner(options);
    EXPECT_TRUE(permissive_planner.plan(costmap, pose(0.5, 0.5), pose(1.5, 1.5)));
}

TEST(Nav_AStar, simplifies_open_space_path_and_validates_options) {
    auto costmap = makeCostmap(makeGrid(8, 8));
    AStarOptions raw_options{};
    raw_options.simplify = false;
    raw_options.smoothing_iterations = 0;
    const auto raw = AStarPlanner(raw_options).plan(costmap, pose(0.5, 0.5), pose(7.5, 5.5));
    const auto simplified = AStarPlanner().plan(costmap, pose(0.5, 0.5), pose(7.5, 5.5));
    ASSERT_TRUE(raw);
    ASSERT_TRUE(simplified);
    EXPECT_GT(raw.path.poses.size(), simplified.path.poses.size());
    EXPECT_EQ(simplified.path.poses.size(), 2u);

    raw_options.cost_weight = -1.0;
    EXPECT_EQ(AStarPlanner(raw_options).plan(costmap, pose(0.5, 0.5), pose(7.5, 5.5)).status,
              PlanningStatus::InvalidOptions);
}

TEST(Nav_AStar, smooths_grid_turns_without_entering_blocked_cells) {
    auto costmap = makeCostmap(makeGrid(8, 8));
    AStarOptions options{};
    options.simplify = false;
    options.smoothing_iterations = 0;
    const auto unsmoothed = AStarPlanner(options).plan(costmap, pose(0.5, 0.5), pose(7.5, 5.5));
    options.smoothing_iterations = 4;
    const auto smoothed = AStarPlanner(options).plan(costmap, pose(0.5, 0.5), pose(7.5, 5.5));
    ASSERT_TRUE(unsmoothed);
    ASSERT_TRUE(smoothed);
    ASSERT_EQ(unsmoothed.path.poses.size(), smoothed.path.poses.size());
    bool changed = false;
    for (std::size_t i = 0; i < smoothed.path.poses.size(); ++i) {
        const auto &before = unsmoothed.path.poses[i].pose.position;
        const auto &after = smoothed.path.poses[i].pose.position;
        changed = changed || std::hypot(after.x - before.x, after.y - before.y) > 1e-9;
        const auto cell = costmap.worldToMap(after.x, after.y);
        ASSERT_TRUE(cell);
        const auto cost = costmap.at(cell->x, cell->y);
        ASSERT_TRUE(cost);
        EXPECT_LT(*cost, Inscribed);
    }
    EXPECT_TRUE(changed);
}

TEST(Nav_PurePursuit, tracks_straight_and_curved_paths) {
    PurePursuit controller;
    const auto straight = controller.compute(pose(0.0, 0.0), path({{0.0, 0.0, 0.0}, {1.0, 0.0, 0.0}, {2.0, 0.0, 0.0}}));
    ASSERT_TRUE(straight);
    EXPECT_EQ(straight.status, TrackingStatus::Ok);
    EXPECT_GT(straight.command.linear.x, 0.0);
    EXPECT_NEAR(straight.command.angular.z, 0.0, 1e-12);

    const auto curved = controller.compute(pose(0.0, 0.0), path({{0.0, 0.0, 0.0}, {1.0, 1.0, 0.0}, {2.0, 1.0, 0.0}}));
    ASSERT_TRUE(curved);
    EXPECT_GT(curved.command.linear.x, 0.0);
    EXPECT_GT(curved.command.angular.z, 0.0);
}

TEST(Nav_PurePursuit, stops_at_goal_and_rejects_invalid_inputs) {
    PurePursuit controller;
    const auto route = path({{0.0, 0.0, 0.0}, {1.0, 0.0, 0.0}});
    const auto reached = controller.compute(pose(0.95, 0.0), route);
    ASSERT_TRUE(reached);
    EXPECT_EQ(reached.status, TrackingStatus::GoalReached);
    EXPECT_DOUBLE_EQ(reached.command.linear.x, 0.0);

    auto invalid_pose = pose(0.0, 0.0);
    invalid_pose.position.x = std::numeric_limits<double>::quiet_NaN();
    EXPECT_EQ(controller.compute(invalid_pose, route).status, TrackingStatus::InvalidPose);
    EXPECT_EQ(controller.compute(pose(0.0, 0.0), {}).status, TrackingStatus::InvalidPath);
}

TEST(Nav_CollisionStop, filters_commands_that_enter_an_obstacle) {
    auto costmap = makeCostmap(makeGrid(8, 5));
    ASSERT_EQ(costmap.markObstacle(Cell{3, 2}), MapStatus::Ok);
    costmap.updateCosts();
    CollisionStopOptions options{};
    options.prediction_horizon = 4.0;
    CollisionStop stop(options);
    msg::Twist forward{};
    forward.linear.x = 1.0;
    const auto blocked = stop.filter(costmap, kFootprint, pose(0.5, 2.5), forward);
    EXPECT_TRUE(blocked.stopped);
    EXPECT_DOUBLE_EQ(blocked.command.linear.x, 0.0);

    options.prediction_horizon = 1.0;
    const auto safe = CollisionStop(options).filter(costmap, kFootprint, pose(0.5, 2.5), forward);
    EXPECT_FALSE(safe.stopped);
    EXPECT_DOUBLE_EQ(safe.command.linear.x, 1.0);
}

TEST(Nav_CollisionStop, fails_closed_for_invalid_footprint_or_command) {
    auto costmap = makeCostmap(makeGrid(4, 4));
    CollisionStop stop;
    msg::Twist command{};
    command.linear.x = 0.2;
    EXPECT_TRUE(stop.filter(costmap, {}, pose(1.5, 1.5), command).stopped);
    command.linear.x = std::numeric_limits<double>::infinity();
    EXPECT_TRUE(stop.filter(costmap, kFootprint, pose(1.5, 1.5), command).stopped);
}

} // namespace rm_test

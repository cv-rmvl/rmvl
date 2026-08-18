/**
 * @file perf_planning.cpp
 * @brief 路径规划、跟踪与碰撞刹停性能测试
 */

#include <cmath>
#include <cstddef>
#include <cstdint>
#include <utility>
#include <vector>

#include <benchmark/benchmark.h>

#include "rmvl/nav/controller.hpp"
#include "rmvl/nav/planner.hpp"

namespace rm_test {

using namespace rm;
using namespace rm::nav;

namespace {

msg::OccupancyGrid makeGrid(uint32_t size, bool walls) {
    msg::OccupancyGrid grid{};
    grid.header.frame_id = "map";
    grid.info.resolution = 0.05F;
    grid.info.width = size;
    grid.info.height = size;
    grid.info.origin.orientation.w = 1.0;
    grid.data.assign(static_cast<std::size_t>(size) * size, 0);
    if (!walls)
        return grid;

    bool upper_gap = true;
    for (uint32_t x = 16; x + 1 < size; x += 16) {
        const uint32_t gap_begin = upper_gap ? size * 3 / 4 : size / 4;
        for (uint32_t y = 0; y < size; ++y)
            if (y < gap_begin || y >= gap_begin + 3)
                grid.data[static_cast<std::size_t>(y) * size + x] = 100;
        upper_gap = !upper_gap;
    }
    return grid;
}

Costmap makeCostmap(uint32_t size, bool walls = false) {
    CostmapOptions options{};
    options.inflation_radius = 0.0;
    options.inscribed_radius = 0.0;
    return Costmap(GridMap(makeGrid(size, walls)), options);
}

msg::Pose pose(double x, double y, double yaw = 0.0) {
    msg::Pose result{};
    result.position = {x, y, 0.0};
    result.orientation.z = std::sin(yaw * 0.5);
    result.orientation.w = std::cos(yaw * 0.5);
    return result;
}

msg::Path makePath(std::size_t count) {
    msg::Path path{};
    path.header.frame_id = "map";
    path.poses.reserve(count);
    for (std::size_t i = 0; i < count; ++i) {
        msg::PoseStamped stamped{};
        stamped.header.frame_id = "map";
        stamped.pose.position = {static_cast<double>(i) * 0.05,
                                 std::sin(static_cast<double>(i) * 0.02), 0.0};
        stamped.pose.orientation.w = 1.0;
        path.poses.push_back(std::move(stamped));
    }
    return path;
}

void runAStar(benchmark::State &state, bool walls) {
    const auto size = static_cast<uint32_t>(state.range(0));
    const auto costmap = makeCostmap(size, walls);
    const AStarPlanner planner{};
    const double end = (static_cast<double>(size) - 1.5) * 0.05;
    const auto start = pose(0.075, 0.075);
    const auto goal = pose(end, end);

    for (auto _ : state) {
        auto result = planner.plan(costmap, start, goal);
        if (!result) {
            state.SkipWithError(to_string(result.status));
            break;
        }
        benchmark::DoNotOptimize(result.path.poses.data());
        benchmark::DoNotOptimize(result.expanded);
    }
    state.counters["cells"] = static_cast<double>(size) * size;
}

} // namespace

static void BM_AStarOpen(benchmark::State &state) { runAStar(state, false); }

BENCHMARK(BM_AStarOpen)->Arg(64)->Arg(128)->Arg(256);

static void BM_AStarWalls(benchmark::State &state) { runAStar(state, true); }

BENCHMARK(BM_AStarWalls)->Arg(64)->Arg(128)->Arg(256);

static void BM_PurePursuitCompute(benchmark::State &state) {
    const auto count = static_cast<std::size_t>(state.range(0));
    const auto path = makePath(count);
    const auto current = pose(static_cast<double>(count) * 0.05 / 3.0, 0.0);
    const PurePursuit controller{};

    for (auto _ : state) {
        auto result = controller.compute(current, path);
        if (!result) {
            state.SkipWithError(to_string(result.status));
            break;
        }
        benchmark::DoNotOptimize(result);
    }
    state.SetItemsProcessed(state.iterations() * static_cast<int64_t>(count));
}

BENCHMARK(BM_PurePursuitCompute)->Arg(32)->Arg(256)->Arg(2048);

static void BM_CollisionStopFilter(benchmark::State &state) {
    constexpr double linear_step = 0.02;
    const auto samples = static_cast<std::size_t>(state.range(0));
    const auto costmap = makeCostmap(256);
    const std::vector<msg::Point> footprint{
        {-0.20, -0.15, 0.0}, {0.20, -0.15, 0.0}, {0.20, 0.15, 0.0}, {-0.20, 0.15, 0.0}};
    CollisionStopOptions options{};
    options.linear_step = linear_step;
    options.prediction_horizon = static_cast<double>(samples) * linear_step;
    const CollisionStop filter(options);
    const auto current = pose(1.0, 3.0);
    msg::Twist command{};
    command.linear.x = 1.0;
    command.angular.z = 0.2;

    for (auto _ : state) {
        auto result = filter.filter(costmap, footprint, current, command);
        if (result.stopped) {
            state.SkipWithError("unexpected collision stop");
            break;
        }
        benchmark::DoNotOptimize(result);
    }
    state.SetItemsProcessed(state.iterations() * static_cast<int64_t>(samples));
}

BENCHMARK(BM_CollisionStopFilter)->Arg(20)->Arg(50)->Arg(100);

} // namespace rm_test

/**
 * @file perf_map.cpp
 * @brief 二维栅格地图与代价地图性能测试
 */

#include <cstddef>
#include <cstdint>
#include <utility>
#include <vector>

#include <benchmark/benchmark.h>

#include "rmvl/nav/map.hpp"

namespace rm_test {

using namespace rm;
using namespace rm::nav;

namespace {

msg::OccupancyGrid makeGrid(uint32_t width, uint32_t height, double resolution = 0.05) {
    msg::OccupancyGrid grid{};
    grid.header.frame_id = "map";
    grid.info.resolution = static_cast<float>(resolution);
    grid.info.width = width;
    grid.info.height = height;
    grid.info.origin.orientation.w = 1.0;
    grid.data.assign(static_cast<std::size_t>(width) * height, 0);
    return grid;
}

} // namespace

static void BM_GridMapApply(benchmark::State &state) {
    constexpr uint32_t map_size = 256;
    const auto update_size = static_cast<uint32_t>(state.range(0));
    GridMap map(makeGrid(map_size, map_size));
    msg::OccupancyGridUpdate occupied{};
    occupied.header.frame_id = "map";
    occupied.x = (map_size - update_size) / 2;
    occupied.y = (map_size - update_size) / 2;
    occupied.width = update_size;
    occupied.height = update_size;
    occupied.data.assign(static_cast<std::size_t>(update_size) * update_size, 100);
    auto free = occupied;
    free.data.assign(occupied.data.size(), 0);
    bool write_occupied = true;

    for (auto _ : state) {
        auto &update = write_occupied ? occupied : free;
        update.base_revision = map.revision();
        update.revision = update.base_revision + 1;
        auto status = map.apply(update);
        if (status != MapStatus::Ok) {
            state.SkipWithError(to_string(status));
            break;
        }
        benchmark::DoNotOptimize(status);
        benchmark::ClobberMemory();
        write_occupied = !write_occupied;
    }
    state.SetItemsProcessed(state.iterations() * static_cast<int64_t>(occupied.data.size()));
}

BENCHMARK(BM_GridMapApply)->Arg(8)->Arg(32)->Arg(128);

static void BM_CostmapUpdateCosts(benchmark::State &state) {
    const auto size = static_cast<uint32_t>(state.range(0));
    auto grid = makeGrid(size, size);
    for (uint32_t y = 8; y < size; y += 16)
        for (uint32_t x = 8; x < size; x += 16)
            grid.data[static_cast<std::size_t>(y) * size + x] = 100;
    CostmapOptions options{};
    options.inflation_radius = 0.55;
    options.inscribed_radius = 0.20;
    Costmap costmap(GridMap(std::move(grid)), options);
    if (!costmap.valid()) {
        state.SkipWithError("failed to create costmap");
        return;
    }

    for (auto _ : state) {
        costmap.updateCosts();
        benchmark::ClobberMemory();
    }
    state.SetItemsProcessed(state.iterations() * static_cast<int64_t>(size) * size);
}

BENCHMARK(BM_CostmapUpdateCosts)->Arg(64)->Arg(128)->Arg(256);

} // namespace rm_test

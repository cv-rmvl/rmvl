路径规划与导航 {#tutorial_modules_nav}
============

@author 赵曦
@date 2026/08/17
@version 1.0
@brief 二维地图、路径规划、路径跟踪与安全控制

@prev_tutorial{tutorial_modules_lpss_robotdemo}

@next_tutorial{tutorial_modules_camera}

@tableofcontents

------

相关模块： @ref nav 及 @ref rmvlmsg

## 1 地图

### 1.1 二维栅格地图

`rm::nav::GridMap` 是 `msg::OccupancyGrid` 的可校验内存模型，负责带偏航角原点的世界/栅格坐标转换、贝叶斯占据概率更新、越界射线裁剪和 `OccupancyGridUpdate` 版本合并。本地修改会自动累计最小脏矩形；全量地图适合低频或按需发布，局部观测则可通过 `pendingUpdate()` 只发布变化区域。

### 1.2 代价地图

`rm::nav::Costmap` 在相同几何信息上维护静态层和局部障碍层。传感器一帧内可以连续标记障碍、执行射线清除，最后统一调用 `updateCosts()` 合成图层并计算膨胀，避免每个观测点都重算整张地图。代价值 `0`、`253`、`254`、`255` 分别表示自由、内切区域、致命障碍和未知空间。

```cpp
#include <rmvl/nav/map.hpp>

using namespace rm;

void updateMap(msg::OccupancyGrid full_map,
               const std::vector<msg::Point> &footprint,
               const msg::Pose &robot_pose) {
    nav::GridMap grid(std::move(full_map));
    if (!grid.valid())
        return;

    // 从地图外进入的射线也会自动裁剪；沿途更新为空闲，地图内终点更新为占据
    grid.integrateRay(-1.0, 0.5, 3.2, 0.5, true);
    if (auto update = grid.pendingUpdate()) {
        // 通过 lpss::Publisher<msg::OccupancyGridUpdate> 发布 *update
        grid.clearPendingUpdate();
    }

    nav::CostmapOptions options;
    options.inflation_radius = 0.55;
    options.inscribed_radius = 0.20;
    nav::Costmap costmap(grid, options);

    // 融合一帧局部障碍观测；射线清除只影响局部障碍层，不会清除静态墙体
    costmap.markObstacle(2.0, 1.0);
    costmap.clearRay(robot_pose.position.x, robot_pose.position.y, 4.0, 1.0);
    costmap.updateCosts();

    if (costmap.collides(footprint, robot_pose)) {
        // 触发刹停或拒绝该位姿
    }
}
```

`GridMap::apply()` 仅接受 `base_revision` 等于本地当前版本、`revision` 严格递增、坐标系一致且矩形完全位于地图内的增量；任何校验失败都不会产生部分写入。`Costmap` 的局部障碍修改采用批处理语义，在读取主代价或执行碰撞检测前应调用 `updateCosts()`。

## 2 规划

### 2.1 A* 路径规划

`rm::nav::AStarPlanner` 在 `Costmap` 上搜索 `msg::Path`。默认启用八邻域移动、禁止从障碍夹角穿过，并拒绝未知、内切和致命栅格。移动代价同时考虑几何距离和膨胀代价，因此在存在空间时会倾向远离障碍。

搜索完成后，规划器先使用栅格可见性删除冗余折点，再执行受碰撞约束的迭代平滑。可以通过 `AStarOptions::simplify` 和 `smoothing_iterations` 分别关闭这两个步骤。

```cpp
#include <rmvl/nav/planner.hpp>

nav::AStarOptions planner_options;
planner_options.allow_diagonal = true;
planner_options.allow_corner_cutting = false;
planner_options.traverse_unknown = false;

nav::AStarPlanner planner(planner_options);
const auto plan = planner.plan(costmap, robot_pose, goal_pose);
if (!plan) {
    // nav::to_string(plan.status) 可用于日志或状态上报
    return;
}

// plan.path 可通过 lpss::Publisher<msg::Path> 发布给控制节点和 LViz
```

规划前必须完成 `Costmap::updateCosts()`。起点、终点和输出路径都位于 `costmap.frameId()` 指定的坐标系中，调用方应先通过 TF 将机器人和目标位姿转换到该坐标系。

### 2.2 路径跟踪与安全刹停

`rm::nav::PurePursuit` 根据当前机器人位姿和 `msg::Path` 计算平面 `msg::Twist`。机器人偏离路径方向较大或前视点位于后方时，控制器会先原地转向；接近终点时按照 `slowdown_distance` 线性降速，进入 `goal_tolerance` 后返回 `TrackingStatus::GoalReached` 和零指令。

`rm::nav::CollisionStop` 是独立于跟踪算法的末级安全过滤器。它按照待发送速度在短时间内积分机器人轨迹，并在每个采样位姿调用 `Costmap::collides()`。预测到 footprint 碰撞，或者输入、配置无效时，均采用故障安全策略返回零指令。

```cpp
#include <rmvl/nav/controller.hpp>

nav::PurePursuit follower;
nav::CollisionStop safety;

const auto tracking = follower.compute(robot_pose, plan.path);
if (!tracking)
    return;

const auto safe_command = safety.filter(costmap, footprint, robot_pose, tracking.command);
sendToBase(safe_command.command);
```

安全过滤器只是一条无状态的最后防线，不能替代局部规划、动态障碍预测、制动距离建模和路径重规划。实际系统应根据底盘速度与制动能力调整 `prediction_horizon`，并确保局部代价地图在每个控制周期之前完成更新。

## 3 性能基准

安装 Google Benchmark 后，可以单独构建并运行 Nav 性能测试：

```bash
cmake -S . -B build -DBUILD_PERF_TESTS=ON
cmake --build build --target rmvl_nav_perf_test
./build/bin/rmvl_nav_perf_test
```

基准覆盖矩形地图增量、代价地图合成与膨胀、开放及障碍地图 A*、不同路径长度的 Pure Pursuit，以及不同预测采样数的碰撞刹停。夹具初始化位于计时循环之外，报告时间只包含对应算法调用。

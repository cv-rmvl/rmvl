路径规划与导航 {#tutorial_modules_nav}
============

@author 赵曦
@date 2026/08/17
@version 1.0
@brief 视觉定位、二维地图、路径规划、路径跟踪与安全控制

@prev_tutorial{tutorial_modules_lpss_robotdemo}

@next_tutorial{tutorial_modules_ort}

@tableofcontents

------

相关模块： @ref nav 及 @ref rmvlmsg

@remark 本教程中的 LPSS 通信示例统一使用 C++20 @ref rm::lpss::async::Node 。地图、规划和控制算法本身是同步计算，消息输入输出通过异步节点创建的 Publisher、Subscriber 和 TF Broadcaster 完成。异步 Publisher 的 `publish()` 会将发送任务交给节点的 IOContext 调度，因此调用处不需要 `co_await`。

## 1 架构与能力边界

Nav 将硬件、第三方定位算法与二维导航算法解耦，完整数据流如下。图中的节点均可点击，并会跳转到对应的类型文档。

@dot
digraph nav_architecture {
    graph [rankdir=TB, newrank=true, bgcolor="transparent", pad="0.06", nodesep="0.18", ranksep="0.48"];
    node [shape=box, style="rounded,filled", fontname="sans-serif", fontsize=11,
          color="#546e7a", fontcolor="#263238", fillcolor="#eceff1",
          width=2.0, height=0.72, fixedsize=true, margin="0.13,0.08"];
    edge [fontname="sans-serif", fontsize=10, color="#607d8b", fontcolor="#455a64", arrowsize=0.7];

    sensors [group="c1", label="传感器输入\nImage / IMU / Odometry",
             URL="\ref rm::nav::SensorFrame", tooltip="统一传感器帧"];
    sync [group="c2", label="FrameSynchronizer\n采集时间同步", fillcolor="#e3f2fd", color="#1e88e5",
          URL="\ref rm::nav::FrameSynchronizer", tooltip="多传感器帧同步器"];
    backend [group="c3", label="VO / SLAM Backend\n用户按场景选配", fillcolor="#fff3e0", color="#fb8c00",
             URL="\ref rm::nav::OdometryBackend", tooltip="可替换定位后端"];
    pipeline [group="c4", label="LocalizationPipeline\n校验并整理标准输出", fillcolor="#e3f2fd", color="#1e88e5",
              URL="\ref rm::nav::LocalizationPipeline", tooltip="定位输出流水线"];
    localization [group="c4", label="Odometry / TF / Path", fillcolor="#eceff1",
                  URL="\ref rm::nav::LocalizationResult", tooltip="标准定位输出"];
    mapper [group="c4", label="MapperBackend\n可选二维建图后端", fillcolor="#fff3e0", color="#fb8c00",
            URL="\ref rm::nav::MapperBackend", tooltip="可替换地图后端"];
    grid [group="c4", label="GridMap\n占据栅格与增量", fillcolor="#e3f2fd", color="#1e88e5",
          URL="\ref rm::nav::GridMap", tooltip="二维占据栅格"];
    costmap [group="c3", label="Costmap\n图层合成与障碍膨胀", fillcolor="#e3f2fd", color="#1e88e5",
             URL="\ref rm::nav::Costmap", tooltip="二维分层代价地图"];
    planner [group="c2", label="AStarPlanner\n全局路径规划", fillcolor="#e3f2fd", color="#1e88e5",
             URL="\ref rm::nav::AStarPlanner", tooltip="A* 全局规划器"];
    controller [group="c1", label="PurePursuit\n路径跟踪 + 当前位姿", fillcolor="#e3f2fd", color="#1e88e5",
                URL="\ref rm::nav::PurePursuit", tooltip="Pure Pursuit 跟踪器"];
    safety [group="c1", label="CollisionStop\nCostmap 安全过滤", fillcolor="#e3f2fd", color="#1e88e5",
            URL="\ref rm::nav::CollisionStop", tooltip="预测碰撞刹停"];

    { rank=same; sensors; sync; backend; pipeline; }
    { rank=same; safety; mapper; }
    { rank=same; controller; planner; costmap; grid; }

    sensors -> sync;
    sync -> backend [tooltip="输出已同步的 SensorFrame"];
    backend -> pipeline [tooltip="返回统一 BackendEstimate"];
    localization -> pipeline [dir=back];
    pipeline -> mapper [style=dashed, tooltip="配置 Mapper 时才执行"];
    mapper -> grid [tooltip="输出全量地图或矩形增量"];
    costmap -> grid [dir=back];
    planner -> costmap [dir=back];
    controller -> planner [dir=back, tooltip="输出待跟踪 Path"];
    safety -> controller [dir=back, tooltip="输出待过滤 Twist"];
    safety -> mapper [style=invis, weight=10];
}
@enddot

蓝色节点是 RMVL 内置组件，橙色节点是用户按传感器、算力和许可证选择的算法后端，灰色节点表示通用消息输入或输出。实线表示主数据流，虚线表示可选处理或运行时依赖。

- 定位主链将同步后的 SensorFrame 交给后端，再由 LocalizationPipeline 生成统一的 Odometry、TF 和 Path
- 配置 Mapper 时，Pipeline 额外生成全量或增量 OccupancyGrid，并交给 GridMap 和 Costmap
- 控制器同时依赖当前定位和规划路径，CollisionStop 还会使用 Costmap 完成 footprint 碰撞查询

RMVL 内置的是通用接入契约与二维导航算法，不绑定具体相机 SDK 和 SLAM 实现。

| 能力 | 当前提供方 | 说明 |
| ---- | :--------: | ---- |
| 图像、IMU、外部里程计同步 | RMVL | 使用消息采集时间，支持单目、双目和 RGB-D |
| Odometry、TF、Path 标准输出 | RMVL | 由 @ref rm::nav::LocalizationPipeline 统一整理 |
| 特征提取、位姿估计、回环和图优化 | 用户选配后端 | 由 @ref rm::nav::OdometryBackend 或 @ref rm::nav::SlamBackend 接入 |
| 深度投影和二维障碍建图 | 用户选配后端 | 通过 @ref rm::nav::MapperBackend 接入 |
| 占据栅格与分层代价地图 | RMVL | 提供全量地图、增量、局部障碍和膨胀 |
| 全局路径规划 | RMVL | 当前提供 A*、路径简化和平滑 |
| 路径跟踪和末级刹停 | RMVL | 当前提供 Pure Pursuit 和短时预测碰撞过滤 |
| 局部轨迹规划、动态障碍预测和恢复行为 | 尚未内置 | CollisionStop 不能替代这些功能 |

### 1.1 时间与坐标系

导航推荐使用以下坐标树

\f[\texttt{map}\to\texttt{odom}\to\texttt{base_link}\to\texttt{camera_link}\to\texttt{camera_optical_frame}\f]

- @p map 是允许因回环或重定位发生修正的全局参考系
- @p odom 是短时连续但允许长期漂移的局部参考系
- @p base_link 是机器人本体坐标系
- 相机、IMU 和其他传感器通过静态 TF 连接至 @p base_link

定位后端通常连续输出 \f$\texttt{odom}\to\texttt{base_link}\f$，具有全局修正能力的 SLAM 后端还可输出 \f$\texttt{map}\to\texttt{odom}\f$。所有定位消息和 TF 使用主图像的采集时间，不使用消息到达时间。规划前应在传感器数据对应的时间查询 TF，不应直接使用 `lpss::now()` 猜测尚未到达的变换。

## 2 地图

### 2.1 二维栅格地图

@ref rm::nav::GridMap 是 @ref rm::msg::OccupancyGrid 的可校验内存模型，负责带偏航角原点的世界/栅格坐标转换、贝叶斯占据概率更新、越界射线裁剪和 @ref rm::msg::OccupancyGridUpdate 版本合并。本地修改会自动累计最小脏矩形。全量地图适合低频或按需发布，局部观测则可通过 @ref rm::nav::GridMap::pendingUpdate "GridMap::pendingUpdate()" 只发布变化区域。

### 2.2 代价地图

@ref rm::nav::Costmap 在相同几何信息上维护静态层和局部障碍层。传感器一帧内可以连续标记障碍、执行射线清除，最后统一调用 @ref rm::nav::Costmap::updateCosts "Costmap::updateCosts()" 合成图层并计算膨胀，避免每个观测点都重算整张地图。代价值 `0`、`253`、`254`、`255` 分别表示自由、内切区域、致命障碍和未知空间。

```cpp
#include <rmvl/lpss/node.hpp>
#include <rmvl/nav/map.hpp>

using namespace rm;

using OccupancyGridUpdatePub = lpss::async::Publisher<msg::OccupancyGridUpdate>::ptr;

void updateMap(msg::OccupancyGrid full_map, const std::vector<msg::Point> &footprint,
               const msg::Pose &robot_pose, const OccupancyGridUpdatePub &update_pub) {
    nav::GridMap grid(std::move(full_map));
    if (!grid.valid())
        return;

    // 从地图外进入的射线也会自动裁剪，沿途更新为空闲，地图内终点更新为占据
    grid.integrateRay(-1.0, 0.5, 3.2, 0.5, true);
    if (auto update = grid.pendingUpdate()) {
        update_pub->publish(*update);
        grid.clearPendingUpdate();
    }

    nav::CostmapOptions options;
    options.inflation_radius = 0.55;
    options.inscribed_radius = 0.20;
    nav::Costmap costmap(grid, options);

    // 融合一帧局部障碍观测，射线清除只影响局部障碍层，不会清除静态墙体
    costmap.markObstacle(2.0, 1.0);
    costmap.clearRay(robot_pose.position.x, robot_pose.position.y, 4.0, 1.0);
    costmap.updateCosts();

    if (costmap.collides(footprint, robot_pose)) {
        // 触发刹停或拒绝该位姿
    }
}
```

@ref rm::nav::GridMap::apply "GridMap::apply()" 仅接受 `base_revision` 等于本地当前版本、`revision` 严格递增、坐标系一致且矩形完全位于地图内的增量。任何校验失败都不会产生部分写入。Costmap 的局部障碍修改采用批处理语义，在读取主代价或执行碰撞检测前应调用 @ref rm::nav::Costmap::updateCosts "Costmap::updateCosts()"。

## 3 规划与控制

### 3.1 A* 路径规划

@ref rm::nav::AStarPlanner 在 @ref rm::nav::Costmap 上搜索 @ref rm::msg::Path 。默认启用八邻域移动、禁止从障碍夹角穿过，并拒绝未知、内切和致命栅格。移动代价同时考虑几何距离和膨胀代价，因此在存在空间时会倾向远离障碍。

搜索完成后，规划器先使用栅格可见性删除冗余折点，再执行受碰撞约束的迭代平滑。可以通过 @ref rm::nav::AStarOptions::simplify "AStarOptions::simplify" 和 @ref rm::nav::AStarOptions::smoothing_iterations "AStarOptions::smoothing_iterations" 分别关闭这两个步骤。

```cpp
#include <rmvl/lpss/node.hpp>
#include <rmvl/nav/planner.hpp>

using namespace rm;

void planPath(const nav::Costmap &costmap, const msg::Pose &robot_pose,
              const msg::Pose &goal_pose,
              const lpss::async::Publisher<msg::Path>::ptr &path_pub) {
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

    path_pub->publish(plan.path);
}
```

规划前必须完成 @ref rm::nav::Costmap::updateCosts "Costmap::updateCosts()"。起点、终点和输出路径都位于 `costmap.frameId()` 指定的坐标系中，调用方应先通过 TF 将机器人和目标位姿转换到该坐标系。PlanningResult::cost 是栅格几何移动代价与代价地图惩罚之和，不直接表示米制路径长度。

### 3.2 路径跟踪与安全刹停

@ref rm::nav::PurePursuit 根据当前机器人位姿和 @ref rm::msg::Path 计算平面 @ref rm::msg::Twist 。机器人偏离路径方向较大或前视点位于后方时，控制器会先原地转向。接近终点时按照 `slowdown_distance` 线性降速，进入 `goal_tolerance` 后返回 @ref rm::nav::TrackingStatus::GoalReached 和零指令。

@ref rm::nav::CollisionStop 是独立于跟踪算法的末级安全过滤器。它按照待发送速度在短时间内积分机器人轨迹，并在每个采样位姿调用 @ref rm::nav::Costmap::collides "Costmap::collides()"。预测到 footprint 碰撞，或者输入、配置无效时，均采用故障安全策略返回零指令。

```cpp
#include <rmvl/lpss/node.hpp>
#include <rmvl/nav/controller.hpp>

using namespace rm;

void followPath(const nav::Costmap &costmap, const std::vector<msg::Point> &footprint,
                const msg::Pose &robot_pose, const msg::Path &path,
                const lpss::async::Publisher<msg::Twist>::ptr &command_pub) {
    nav::PurePursuit follower;
    nav::CollisionStop safety;

    const auto tracking = follower.compute(robot_pose, path);
    if (!tracking)
        return;

    const auto safe_cmd = safety.filter(costmap, footprint, robot_pose, tracking.command);
    command_pub->publish(safe_cmd.command);
}
```

安全过滤器只是一条无状态的最后防线，不能替代局部规划、动态障碍预测、制动距离建模和路径重规划。实际系统应根据底盘速度与制动能力调整 `prediction_horizon`，并确保局部代价地图在每个控制周期之前完成更新。

## 4 视觉里程计与 SLAM

### 4.1 无硬件依赖的接入边界

RMVL 不要求相机驱动、里程计驱动和定位算法运行在同一个进程，也不依赖 ROS 运行时。相机 SDK 或数据集读取器只需生成通用消息，视觉定位进程通过 LPSS 接收这些消息，再将结果转换为导航栈已经使用的 @ref rm::msg::Odometry "Odometry"、TF 和 @ref rm::msg::Path "Path"。

| 数据 | RMVL 消息 | 必需性 | 时间与坐标语义 |
| --- | --- | --- | --- |
| 主图像 | @ref rm::msg::Image | 必需 | `header.stamp` 为采集时间，`frame_id` 为光学坐标系 |
| 主相机标定 | @ref rm::msg::CameraInfo | 必需 | 尺寸和 `frame_id` 必须与主图像一致 |
| 右目或深度图 | @ref rm::msg::Image | 双目、RGB-D 必需 | 在同步容差内与主图像匹配，深度可使用 `16UC1` 或 `32FC1` |
| 辅助相机标定 | @ref rm::msg::CameraInfo | 双目、RGB-D 必需 | 对应右目或深度光学坐标系 |
| IMU | @ref rm::msg::Imu | 可选 | 每帧收到上一视觉帧之后的全部样本 |
| 轮速计或其他里程计 | @ref rm::msg::Odometry | 可选 | 选择采集时间最接近当前图像的样本 |

多传感器采集时间同步器 @ref rm::nav::FrameSynchronizer 参考 ROS 2 `message_filters` 的成熟语义，按消息采集时间而不是到达时间同步。单目模式只需要主相机，双目和 RGB-D 模式会在 @ref rm::nav::FrameSyncOptions::tolerance "FrameSyncOptions::tolerance" 内选择时间最接近的辅助图像。CameraInfo 作为低频状态缓存，不需要与每张图像同时发布。缓存有固定上限，过期、乱序和无法匹配的图像会被丢弃并计入 @ref rm::nav::FrameSyncStatistics "FrameSyncStatistics"，不会无限积压。

@note `16UC1` 只声明无符号 16 位单通道存储，实际深度比例由相机适配器或后端约定。`32FC1` 深度建议统一使用米。Image 当前采用紧密行排列，SDK 图像存在行填充时应在适配层重新打包。

相机和机器人本体之间的外参继续由 @ref rm::lpss::tf::Buffer 管理。后端适配器应将相机位姿转换为 `base_link` 位姿，不要把光学坐标系直接交给二维控制器。双目左右目、RGB 与深度未硬件对齐时，也应使用各自的静态外参完成投影或位姿换算。

### 4.2 可替换后端与标准输出

@ref rm::nav::OdometryBackend 是视觉里程计和 SLAM 共用的最小跟踪接口。@ref rm::nav::SlamBackend 在此基础上提供可选重定位入口，并允许通过 @ref rm::nav::BackendEstimate::map_to_odom "BackendEstimate::map_to_odom" 输出回环或重定位产生的全局修正。@ref rm::nav::MapperBackend 独立处理深度建图，避免把定位用稀疏特征地图误当成可用于避障的占据栅格。当前 @ref rm::nav::SensorFrame 尚未携带通用点云，点云 Mapper 需要自行订阅和同步外部点云数据。

具体实现可以封装相机 SDK、公开数据集读取器或第三方 VO/SLAM 库。SDK、OpenCV 和具体算法只出现在适配后端，Nav 公开接口始终使用 RMVL 消息和 STL 类型。没有真实硬件时，可直接从公开数据集构造相同消息，使用消息时间戳离线回放。

下面是最小后端骨架。适配器负责调用第三方算法、换算相机外参，并填充统一结果。示例中的算法调用部分需要替换为实际 SDK 或 SLAM API。

```cpp
#include <rmvl/nav/localization.hpp>

using namespace rm;

class MySlamBackend final : public nav::SlamBackend {
public:
    nav::BackendEstimate track(const nav::SensorFrame &frame) override {
        nav::BackendEstimate estimate;

        // 1. 将 frame.primary、frame.secondary 和 frame.imu 转换为第三方算法输入
        // 2. 运行跟踪并将相机位姿通过静态外参换算为 base_link 位姿
        // 3. 初始化或丢失跟踪时返回对应状态，不要填充伪造位姿

        estimate.status = nav::LocalizationStatus::Initializing;
        return estimate;
    }

    void reset() override {
        // 清理第三方算法的轨迹、关键帧和融合状态
    }

    nav::LocalizationStatus relocalize(const msg::PoseWithCovariance &pose,
                                       std::string_view frame_id) override {
        // 将全局初始位姿转交给第三方算法
        return nav::LocalizationStatus::Unsupported;
    }
};
```

后端成功时至少填写 `status`、`reference_frame`、`child_frame`、`pose` 和 `twist`。Pipeline 会把输出时间统一设置为主图像采集时间，并归一化位姿四元数。若提供 `map_to_odom`，其 `child_frame_id` 必须等于 `reference_frame`。当前 Pipeline 不转发 `relocalize()`，应用层可通过具体后端对象或独立 LPSS 服务处理重定位请求。

@ref rm::nav::LocalizationPipeline 串行调用后端，并统一完成以下输出：

- `msg::Odometry`，位姿位于后端指定的 `odom` 或 `map` 参考系，速度由子坐标系表达
- 与 Odometry 位姿等价的动态 `msg::TransformStamped`
- SLAM 后端可选的 `map` \f$\to\f$ `odom` 修正
- 容量有上限的历史 `msg::Path`
- Mapper 后端可选的全量 `OccupancyGrid` 或 `OccupancyGridUpdate`

下面展示输出处理方式。LPSS 订阅回调可以使用 `pushPrimary()`、`pushSecondary()`、`pushImu()` 和 `pushOdometry()` 写入数据。成功输出后，通过异步 Publisher 和由异步节点创建的 TF Broadcaster 发布即可。

```cpp
#include <memory>

#include <rmvl/lpss/node.hpp>
#include <rmvl/lpss/transform.hpp>
#include <rmvl/nav/localization.hpp>

using namespace rm;

void processFrame(nav::FrameSynchronizer &synchronizer, nav::LocalizationPipeline &pipeline,
                  const lpss::async::Publisher<msg::Odometry>::ptr &odometry_pub,
                  const lpss::async::Publisher<msg::Path>::ptr &path_pub,
                  const lpss::async::Publisher<msg::OccupancyGrid>::ptr &map_pub,
                  const lpss::async::Publisher<msg::OccupancyGridUpdate>::ptr &map_update_pub,
                  lpss::tf::Broadcaster &tf_pub) {
    auto synced = synchronizer.next();
    if (!synced)
        return;

    auto output = pipeline.process(synced.frame);
    if (!output)
        return;

    odometry_pub->publish(output.odometry);
    if (!output.path.poses.empty())
        path_pub->publish(output.path);
    tf_pub.send(output.transform);
    if (output.map_to_odom)
        tf_pub.send(*output.map_to_odom);

    if (!output.mapping || output.mapping->status != nav::LocalizationStatus::Ok)
        return;
    if (output.mapping->map)
        map_pub->publish(*output.mapping->map);
    if (output.mapping->update)
        map_update_pub->publish(*output.mapping->update);
}
```

`LocalizationResult` 转换为 `true` 只表示 Odometry 和动态 TF 有效，不能代表 Mapper 成功。全量地图与增量同时存在时，消费端应先装载全量地图，再按 `base_revision` 和 `revision` 顺序应用增量。所有远端地图消息仍应交给 @ref rm::nav::GridMap 校验。

### 4.3 async 节点接线

下面的节点展示如何创建异步 Subscriber、Publisher、TF Broadcaster 和处理定时器。为缩短篇幅，只列出 RGB-D 必需的话题，IMU 和外部里程计可用相同方式接入。

```cpp
#include <chrono>
#include <memory>

#include <rmvl/lpss/node.hpp>
#include <rmvl/lpss/transform.hpp>
#include <rmvl/nav/localization.hpp>

using namespace rm;
using namespace std::chrono_literals;

class LocalizationNode final : public lpss::async::Node {
public:
    explicit LocalizationNode(std::unique_ptr<nav::OdometryBackend> backend)
        : lpss::async::Node("localization"),
          _sync(nav::FrameSyncOptions{nav::CameraMode::Rgbd}),
          _pipeline(std::move(backend)), _tf("demo", *this) {
        _image_sub = createSubscriber<msg::Image>("demo/camera/image", [this](const auto &msg) {
            _sync.pushPrimary(msg);
        });
        _depth_sub = createSubscriber<msg::Image>("demo/camera/depth", [this](const auto &msg) {
            _sync.pushSecondary(msg);
        });
        _camera_info_sub = createSubscriber<msg::CameraInfo>("demo/camera/info", [this](const auto &msg) {
            _sync.setPrimaryInfo(msg);
        });
        _depth_info_sub = createSubscriber<msg::CameraInfo>("demo/camera/depth_info", [this](const auto &msg) {
            _sync.setSecondaryInfo(msg);
        });

        _odometry_pub = createPublisher<msg::Odometry>("demo/odometry");
        _path_pub = createPublisher<msg::Path>("demo/path");
        _map_pub = createPublisher<msg::OccupancyGrid>("demo/map");
        _map_update_pub = createPublisher<msg::OccupancyGridUpdate>("demo/map_updates");
        _process_timer = createTimer(1ms, [this] {
            processFrame(_sync, _pipeline, _odometry_pub, _path_pub,
                         _map_pub, _map_update_pub, _tf);
        });
    }

private:
    nav::FrameSynchronizer _sync;
    nav::LocalizationPipeline _pipeline;
    lpss::tf::Broadcaster _tf;
    lpss::async::Subscriber<msg::Image>::ptr _image_sub;
    lpss::async::Subscriber<msg::Image>::ptr _depth_sub;
    lpss::async::Subscriber<msg::CameraInfo>::ptr _camera_info_sub;
    lpss::async::Subscriber<msg::CameraInfo>::ptr _depth_info_sub;
    lpss::async::Publisher<msg::Odometry>::ptr _odometry_pub;
    lpss::async::Publisher<msg::Path>::ptr _path_pub;
    lpss::async::Publisher<msg::OccupancyGrid>::ptr _map_pub;
    lpss::async::Publisher<msg::OccupancyGridUpdate>::ptr _map_update_pub;
    lpss::async::Timer::ptr _process_timer;
};

int main() {
    auto node = LocalizationNode(std::make_unique<MySlamBackend>());
    node.spin();
    return 0;
}
```

该示例使用以下话题。`lpss::tf::Broadcaster("demo", node)` 自动使用 `demo/tf`，相机外参等静态变换应使用同一前缀的 @ref rm::lpss::tf::StaticBroadcaster 发布至 `demo/tf_static`。

| 话题 | 消息 | 方向 |
| --- | --- | --- |
| `demo/camera/image` | Image | 输入 |
| `demo/camera/depth` | Image | 输入 |
| `demo/camera/info` | CameraInfo | 输入 |
| `demo/camera/depth_info` | CameraInfo | 输入 |
| `demo/odometry` | Odometry | 输出 |
| `demo/path` | Path | 输出 |
| `demo/map` | OccupancyGrid | 可选输出 |
| `demo/map_updates` | OccupancyGridUpdate | 可选输出 |
| `demo/tf`、`demo/tf_static` | TF | 输出 |

该示例用于展示接线关系。实际 SLAM 的特征提取、优化和回环检测耗时较长时，订阅回调仍应只负责写入同步器，再由专用工作线程调用 `LocalizationPipeline::process()`，避免长时间占用 LPSS 通信线程。

### 4.4 典型部署方式

已有高精地图时，不必启用在线建图。系统可以使用连续里程计维护 \f$\texttt{odom}\to\texttt{base_link}\f$，由全局重定位后端维护 \f$\texttt{map}\to\texttt{odom}\f$，再将已有 OccupancyGrid 交给 Costmap、A* 和控制器。此时 Mapper 可以省略，只需持续融合局部障碍观测。

需要在线建图时，应同时配置定位后端和 Mapper。SLAM 稀疏特征地图只服务于定位和回环，不能直接表示自由空间、障碍高度和机器人可通行区域。Mapper 应将深度观测变换到机器人或地图坐标系，再按高度范围投影至 GridMap 和 Costmap。

### 4.5 当前能力边界

这一层完成的是硬件与算法无关的接入契约、时间同步和导航输出，不包含内置 VO/SLAM 算法。单目输入没有可靠绝对尺度时，不应直接用于米制路径控制。希望尽快形成可导航闭环时，应优先接 RGB-D 或双目后端，再融合轮速计和可选 IMU。

当前没有内置局部轨迹规划、动态障碍预测、恢复行为、地图持久化和完整的全局重定位流程。点云也尚未纳入通用 SensorFrame。上述能力可以通过新后端或后续 Nav 组件扩展，不应由 CollisionStop 或定位用稀疏特征地图代替。

## 5 性能基准

安装 Google Benchmark 后，可以单独构建并运行 Nav 性能测试：

```bash
cmake -S . -B build -DBUILD_PERF_TESTS=ON
cmake --build build --target rmvl_nav_perf_test
./build/bin/rmvl_nav_perf_test
```

基准覆盖矩形地图增量、代价地图合成与膨胀、开放及障碍地图 A*、不同路径长度的 Pure Pursuit，以及不同预测采样数的碰撞刹停。夹具初始化位于计时循环之外，报告时间只包含对应算法调用。

当前基准尚未覆盖 FrameSynchronizer 的队列匹配和 LocalizationPipeline 的消息整理开销。视觉后端性能受具体第三方实现、图像分辨率和硬件影响，应在后端仓库中使用公开数据集单独报告跟踪耗时、轨迹误差和丢帧率。

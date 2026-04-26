机器人运动规划 {#tutorial_modules_lpss_robotpln}
============

@author 赵曦
@date 2026/04/29
@version 1.0
@brief 本教程介绍了 LPSS 机器人扩展中的运动规划功能，展示了如何使用该功能进行机器人运动规划。

@prev_tutorial{tutorial_modules_lpss}

@next_tutorial{tutorial_modules_lpss_robotctl}

@tableofcontents

------

相关模块： @ref lpss_robot

## 1 概述

`rm::lpss::RobotPlanner` 提供 URDF 加载、正/逆运动学求解、以及轨迹规划功能。使用者可以直接指定关节角度或末端位姿来规划平滑的运动轨迹。

### 1.1 URDF 文件说明

URDF（Unified Robot Description Format）是 ROS/ROS2 标准的机器人描述格式，用 XML 定义机器人的连杆、关节和动力学属性。可以使用 **SW2URDF** 插件从 SolidWorks 导出 URDF 文件，或参考 ROS 官方文档手工编写。RMVL 完全兼容标准 URDF 格式，支持加载 ROS/ROS2 生态中的机器人模型。

## 2 快速开始

### 2.1 创建规划器

首先需要创建一个 `RobotPlanner` 对象，加载 URDF 模型：

```cpp
#include <rmvl/lpss/robot.hpp>

using namespace rm;

int main() {
    // 创建规划器，加载 URDF 文件
    lpss::RobotPlanner planner("/path/to/robot.urdf");
    
    // 可选：指定网格文件路径前缀，用于某些需要可视化的场景
    // lpss::RobotPlanner planner("/path/to/robot.urdf", "/path/to/meshes");
    
    return 0;
}
```

### 2.2 更新关节状态

在进行轨迹规划前，需要更新规划器中的关节状态（通常是当前机器人的实际关节角度）：

```cpp
// 创建关节状态消息
msg::JointState current_state;
current_state.name = {"joint1", "joint2", "joint3"};
current_state.position = {0.0, 0.0, 0.0};  // 当前关节角度
current_state.velocity = {0.0, 0.0, 0.0};  // 当前关节速度
current_state.effort = {0.0, 0.0, 0.0};    // 当前关节力矩

// 更新规划器
planner.update(current_state);

// 或获取当前状态
const auto &state = planner.joints();
```

### 2.3 关节空间规划

在已知目标关节角的情况下，可以使用关节空间规划快速生成轨迹：

```cpp
// 设置目标关节状态
msg::JointState target;
target.name = {"joint1", "joint2", "joint3"};
target.position = {1.57, 0.785, -0.785};  // 目标关节角
target.velocity = {0.0, 0.0, 0.0};
target.effort = {0.0, 0.0, 0.0};

// 规划轨迹
msg::JointTrajectory traj = planner.plan(target);

// 检查规划结果
if (traj.points.empty()) {
    std::cout << "轨迹规划失败！" << std::endl;
} else {
    std::cout << "规划成功，轨迹包含 " << traj.points.size() << " 个点" << std::endl;
}
```

### 2.4 笛卡尔空间规划（点到点）

指定末端执行器的目标位姿，规划器会自动进行逆运动学求解：

```cpp
// 创建目标位姿（相对于基坐标系）
msg::Pose target_pose;
target_pose.position.x = 0.3;
target_pose.position.y = 0.4;
target_pose.position.z = 0.5;

// 目标姿态（使用四元数表示，此为 z 轴旋转 90 度）
target_pose.orientation.x = 0.0;
target_pose.orientation.y = 0.0;
target_pose.orientation.z = 0.7071;  // sin(45°)
target_pose.orientation.w = 0.7071;  // cos(45°)

// 规划轨迹（目标连杆为 "end_effector"）
msg::JointTrajectory traj = planner.plan("end_effector", target_pose);

if (!traj.points.empty()) {
    std::cout << "轨迹总时间：" << traj.points.back().time_from_start << " ms" << std::endl;
}
```

### 2.5 笛卡尔空间规划（多点）

规划末端执行器通过多个途经点的轨迹：

```cpp
std::vector<msg::Pose> waypoints;

msg::Pose p1;
p1.position.x = 0.2;
p1.position.y = 0.3;
p1.position.z = 0.4;
p1.orientation.x = 0.0;
p1.orientation.y = 0.0;
p1.orientation.z = 0.0;
p1.orientation.w = 1.0;
waypoints.push_back(p1);

msg::Pose p2;
p2.position.x = 0.3;
p2.position.y = 0.4;
p2.position.z = 0.5;
p2.orientation.x = 0.0;
p2.orientation.y = 0.0;
p2.orientation.z = 0.7071;
p2.orientation.w = 0.7071;
waypoints.push_back(p2);

msg::JointTrajectory traj = planner.plan("end_effector", waypoints);
```

### 2.6 正运动学查询

```cpp
msg::Pose link_pose = planner.linkpose("link2");
```

### 2.7 调整执行速度

```cpp
planner.setMaxVelocityScalingFactor(0.5); // 设为 50%
msg::JointTrajectory traj = planner.plan(target);
planner.setMaxVelocityScalingFactor(1.0); // 恢复 100%
```
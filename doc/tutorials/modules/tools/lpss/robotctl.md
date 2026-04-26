机器人控制 {#tutorial_modules_lpss_robotctl}
============

@author 赵曦
@date 2026/04/29
@version 1.0
@brief 本教程介绍了 LPSS 机器人扩展中的控制功能，展示了如何使用该功能进行机器人控制。

@prev_tutorial{tutorial_modules_lpss_robotpln}

@next_tutorial{tutorial_modules_lpss_robotdemo}

@tableofcontents

------

相关模块： @ref lpss_robot

## 1 概述

`rm::lpss::RobotController` 负责轨迹的提交、校验和采样，将规划好的轨迹转化为实时的关节控制指令。

## 2 快速开始

### 2.1 创建控制器

```cpp
#include <rmvl/lpss/robot.hpp>

using namespace rm;

// 创建控制器，指定要控制的关节
lpss::RobotController controller({"joint1", "joint2", "joint3"});
```

### 2.2 提交轨迹

```cpp
// 获取规划的轨迹
msg::JointTrajectory traj = planner.plan(target);

// 提交轨迹
if (!controller.submit(traj)) {
    std::cerr << "轨迹提交失败" << std::endl;
    return;
}
```

### 2.3 实时采样控制指令

在控制循环中按实时时钟采样轨迹，下文的 `sendCommand` 函数代表将控制指令发送到电机驱动器的接口，需要根据实际硬件接口进行实现：

<div class="tabbed">

- <b class="tab-title">异步模式（C++20）</b>

  ```cpp
  class ControlNode : public lpss::async::Node {
  public:
      ControlNode() : lpss::async::Node("controller") {
          // 提交轨迹
          /* submit trajectory */
          // 100Hz 控制频率
          _timer = this->createTimer(10ms, [this]() {
              // 获取反馈
              /* get feedback */
              // 采样当前时刻的控制指令
              msg::JointState cmd = _controller.sample(_feedback);
              // 发送到电机驱动器
              sendCommand(cmd);
          });
      }
  private:
      lpss::RobotController _controller{{"joint1", "joint2", "joint3"}};
      msg::JointState _feedback;
  };
  ```

- <b class="tab-title">同步模式</b>

  ```cpp
  // 提交轨迹
  /* submit trajectory */
  while (true) {
      // 获取反馈状态
      /* get feedback */
      // 采样当前时刻的控制指令
      msg::JointState cmd = controller.sample(feedback);
      // 发送到电机驱动器
      sendCommand(cmd);
      // 100 Hz 控制频率
      std::this_thread::sleep_for(10ms);
  }
  ```

</div>

### 2.4 重置控制器

```cpp
controller.reset();  // 清空缓存，准备新轨迹
```

## 3 状态发布（可选）

### 3.1 发布机器人状态用于可视化

<div class="tabbed">

- <b class="tab-title">异步模式（C++20）</b>

  ```cpp
  class StateNode : public lpss::async::Node {
  public:
      StateNode() : lpss::async::Node("robot_state") {
          _planner.load("/path/to/robot.urdf");
          _pub = lpss::async::RobotStatePublisher::create("robot", *this, _planner, 50);
        
          // 定期更新机器人状态
          _timer = this->createTimer(40ms, [this]() {
              msg::JointState state;
              state.name = {"joint1", "joint2", "joint3"};
              state.position = {/* 当前角度 */};
            
              _planner.update(state);
          });
      }

  private:
      lpss::RobotPlanner _planner;
      lpss::async::RobotStatePublisher::ptr _pub;
      lpss::async::Timer::ptr _timer;
  };

  int main() {
      auto node = StateNode();
      node.spin();
  }
  ```

- <b class="tab-title">同步模式</b>

  ```cpp
  int main() {
      auto node = lpss::Node("robot_state");
      lpss::RobotPlanner planner("/path/to/robot.urdf");

      // 创建状态发布者，50ms 周期发布 TF
      lpss::RobotStatePublisher pub("robot", node, planner, 50);

      while (true) {
          // 从传感器获取当前关节状态
          msg::JointState state;
          // 更新状态
          planner.update(state);
      }
  }
  ```

</div>

## 4 工程部署建议

与 `ros2_control` 不同，`RobotController` 不一定需要部署在硬件近端的实时系统中。根据实际应用场景，可以将 `RobotController` 部署在控制端，采样后通过 LPSS 发布订阅机制与硬件执行层设备进行通信，传输实际的关节状态和控制指令，也可以将 `RobotController` 部署在执行端，采样后直接通过本地接口控制电机驱动器。
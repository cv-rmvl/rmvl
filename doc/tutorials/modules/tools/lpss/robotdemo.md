机器人扩展完整示例 {#tutorial_modules_lpss_robotdemo}
============

@author 赵曦
@date 2026/04/29
@version 1.0
@brief 本教程提供了一个完整的 LPSS 机器人应用示例，展示了如何结合运动规划和控制功能实现简单的机器人演示程序。

@prev_tutorial{tutorial_modules_lpss_robotctl}

@next_tutorial{tutorial_modules_camera}

@tableofcontents

------

相关模块： @ref lpss_robot

## 控制层部署控制器示例

### 概述

对于运算性能相对受限的执行层，一般会将 `RobotController` 部署在控制层的计算节点上，自行完成轨迹采样，并通过 LPSS 将控制指令（一般是 `msg::JointState`）发送到执行层。

总的来说，遵循如下架构：
- **控制层** ：运行 `RobotPlanner` 和 `RobotController`，周期性规划轨迹、采样控制指令并通过 LPSS 发布
- **执行层** ：订阅控制指令消息，根据命令驱动电机，并发布关节状态反馈

### 控制层

<div class="tabbed">

- <b class="tab-title">异步模式（C++20）</b>

  ```cpp
  #include <rmvl/lpss/robot.hpp>
  
  using namespace rm;
  using namespace std::chrono_literals;
  
  class ControllerNode : public lpss::async::Node {
  public:
      ControllerNode() : lpss::async::Node("controller") {
          // 加载机器人模型
          _planner.load("/path/to/robot.urdf");
          
          // 初始化当前关节状态（从执行层获取或设定初值）
          msg::JointState init_state;
          init_state.name = {"joint1", "joint2", "joint3"};
          init_state.position = {0.0, 0.0, 0.0};
          init_state.velocity = {0.0, 0.0, 0.0};
          init_state.effort = {0.0, 0.0, 0.0};
          _planner.update(init_state);
          
          // 订阅执行层发送的关节状态反馈
          _state_sub = this->createSubscriber<msg::JointState>("/robot/cur/joints", [this](const msg::JointState &state) {
              _fb_state = state;
              _planner.update(state);
          });
          
          // 发布控制指令给执行层
          _cmd_pub = this->createPublisher<msg::JointState>("/robot/tar/joints");
          
          // 定期规划轨迹和采样控制指令（100Hz）
          _control_timer = this->createTimer(10ms, [this]() {
              // 从执行层接收反馈状态
              _planner.update(_fb_state);
              
              // 如果还没有轨迹，规划一个新轨迹，实际应用中可以根据需要调整规划条件
              if (!_trajectory_received) {
                  msg::JointState target;
                  target.name = {"joint1", "joint2", "joint3"};
                  target.position = {1.57, 0.785, -0.785};  // 目标关节角
                  target.velocity = {0.0, 0.0, 0.0};
                  target.effort = {0.0, 0.0, 0.0};
                  
                  auto traj = _planner.plan(target);
                  if (!traj.points.empty()) {
                      if (!_controller.submit(traj)) {
                        printf("轨迹提交失败\n");
                          return;
                      }
                      _trajectory_received = true;
                  }
              }
              
              // 采样当前时刻的控制指令
              auto cmd = _controller.sample(_fb_state);
              
              // 发布控制指令到执行层
              _cmd_pub->publish(cmd);
              
              // 若轨迹已完成，重置控制器准备下一条轨迹
          });
      }
  
  private:
      lpss::RobotPlanner _planner;
      lpss::RobotController _controller{{"joint1", "joint2", "joint3"}};
      msg::JointState _fb_state;
      lpss::async::Publisher<msg::JointState>::ptr _cmd_pub;
      lpss::async::Subscriber<msg::JointState>::ptr _state_sub;
      lpss::async::Timer::ptr _control_timer;
      bool _trajectory_received = false;
  };
  
  int main() {
      auto node = std::make_shared<ControllerNode>();
      node->spin();
      return 0;
  }
  ```

- <b class="tab-title">同步模式</b>

  ```cpp
  #include <rmvl/lpss/robot.hpp>
  
  using namespace rm;
  using namespace std::chrono_literals;
  
  int main() {
      // 创建节点
      lpss::Node node("controller");
      
      // 加载机器人模型
      lpss::RobotPlanner planner("/path/to/robot.urdf");
      
      // 初始化关节状态
      msg::JointState init_state;
      init_state.name = {"joint1", "joint2", "joint3"};
      init_state.position = {0.0, 0.0, 0.0};
      init_state.velocity = {0.0, 0.0, 0.0};
      init_state.effort = {0.0, 0.0, 0.0};
      planner.update(init_state);
      
      // 创建控制器
      lpss::RobotController controller({"joint1", "joint2", "joint3"});
      
      // 订阅执行层的关节状态反馈
      msg::JointState feedback_state = init_state;
      auto sub = node.createSubscriber<msg::JointState>("/robot/cur/joints", [&](const msg::JointState &state) {
          feedback_state = state;
      });
      
      // 发布控制指令
      auto pub = node.createPublisher<msg::JointState>("/robot/tar/joints");
      
      // 规划第一条轨迹，实际应用中可以根据需要调整规划条件
      msg::JointState target;
      target.name = {"joint1", "joint2", "joint3"};
      target.position = {1.57, 0.785, -0.785};
      target.velocity = {0.0, 0.0, 0.0};
      target.effort = {0.0, 0.0, 0.0};
      
      auto traj = planner.plan(target);
      if (traj.points.empty()) {
          printf("轨迹规划失败\n");
          return 1;
      }
      
      // 提交轨迹
      if (!controller.submit(traj)) {
          printf("轨迹提交失败\n");
          return 1;
      }
      
      // 控制循环（100Hz）
      bool running = true;
      std::thread spin_thread([&]() { node.spin(); });
      
      while (running) {
          // 更新规划器状态
          planner.update(feedback_state);
          
          // 采样控制指令
          auto cmd = controller.sample(feedback_state);
          
          // 发布到执行层
          pub->publish(cmd);
          
          // 示例中仅执行一条轨迹后结束
          running = false;
          
          // 100Hz 控制频率
          std::this_thread::sleep_for(10ms);
      }
      
      node.shutdown();
      spin_thread.join();
      
      return 0;
  }
  ```

</div>

### 执行层

<div class="tabbed">

- <b class="tab-title">异步模式（C++20）</b>

  ```cpp
  #include <rmvl/lpss/robot.hpp>
  
  using namespace rm;
  using namespace std::chrono_literals;
  
  class ExecutorNode : public lpss::async::Node {
  public:
      ExecutorNode() : lpss::async::Node("executor") {
          // 订阅控制层发送的控制指令
          _cmd_sub = this->createSubscriber<msg::JointState>("/robot/tar/joints", [this](const msg::JointState &cmd) {
              _current_cmd = cmd;
          });
          
          // 发布关节状态反馈给控制层
          _state_pub = this->createPublisher<msg::JointState>("/robot/cur/joints");
          
          // 执行层控制循环（200Hz，更高频率确保实时性）
          _exec_timer = this->createTimer(5ms, [this]() {
              // 根据控制指令驱动电机
              for (size_t i = 0; i < _current_cmd.name.size(); ++i) {
                  // 示例：发送控制指令到电机驱动器
                  sendMotorCommand(_current_cmd.name[i], _current_cmd.position[i]);
              }
              
              // 获取电机反馈状态
              msg::JointState state;
              state.name = _current_cmd.name;
              state.position.resize(state.name.size());
              state.velocity.resize(state.name.size());
              state.effort.resize(state.name.size());
              
              for (size_t i = 0; i < state.name.size(); ++i) {
                  // 示例：从电机驱动器读取反馈
                  state.position[i] = getMotorPosition(state.name[i]);
                  state.velocity[i] = getMotorVelocity(state.name[i]);
                  state.effort[i] = getMotorEffort(state.name[i]);
              }
              
              // 发布状态反馈
              _state_pub->publish(state);
          });
      }
  
  private:
      msg::JointState _current_cmd;
      lpss::async::Publisher<msg::JointState>::ptr _state_pub;
      lpss::async::Subscriber<msg::JointState>::ptr _cmd_sub;
      lpss::async::Timer::ptr _exec_timer;
      
      // 模拟函数，实际应根据硬件接口实现
      void sendMotorCommand(const std::string &joint_name, double position) {
          // TODO：根据实际硬件接口实现
          printf("发送指令到 %s，位置: %.2f\n", joint_name.c_str(), position);
      }
      
      double getMotorPosition(const std::string &joint_name) {
          // TODO：从硬件读取电机位置
          return 0.0;
      }
      
      double getMotorVelocity(const std::string &joint_name) {
          // TODO：从硬件读取电机速度
          return 0.0;
      }
      
      double getMotorEffort(const std::string &joint_name) {
          // TODO：从硬件读取电机扭矩
          return 0.0;
      }
  };
  
  int main() {
      auto node = std::make_shared<ExecutorNode>();
      node->spin();
      return 0;
  }
  ```

- <b class="tab-title">同步模式</b>

  ```cpp
  #include <rmvl/lpss/robot.hpp>
  
  using namespace rm;
  using namespace std::chrono_literals;
  
  // 模拟函数，实际应根据硬件接口实现
  void sendMotorCommand(const std::string &joint_name, double position) {
      printf("发送指令到 %s，位置: %.2f\n", joint_name.c_str(), position);
  }
  
  double getMotorPosition(const std::string &joint_name) {
      return 0.0;  // TODO：从硬件读取电机位置
  }
  
  double getMotorVelocity(const std::string &joint_name) {
      return 0.0;  // TODO：从硬件读取电机速度
  }
  
  double getMotorEffort(const std::string &joint_name) {
      return 0.0;  // TODO：从硬件读取电机扭矩
  }
  
  int main() {
      // 创建节点
      lpss::Node node("executor");
      
      // 订阅控制指令
      msg::JointState current_cmd;
      auto sub = node.createSubscriber<msg::JointState>("/robot/tar/joints", [&](const msg::JointState &cmd) {
          current_cmd = cmd;
      });
      
      // 发布关节状态
      auto pub = node.createPublisher<msg::JointState>("/robot/cur/joints");
      
      // 启动节点的事件循环
      std::thread spin_thread([&]() { node.spin(); });
      
      bool running = true;
      
      // 执行层控制循环（200Hz）
      while (running) {
          // 根据控制指令驱动电机
          for (size_t i = 0; i < current_cmd.name.size(); ++i) {
              sendMotorCommand(current_cmd.name[i], current_cmd.position[i]);
          }
          
          // 获取电机反馈状态
          msg::JointState state;
          state.name = current_cmd.name;
          state.position.resize(state.name.size());
          state.velocity.resize(state.name.size());
          state.effort.resize(state.name.size());
          
          for (size_t i = 0; i < state.name.size(); ++i) {
              state.position[i] = getMotorPosition(state.name[i]);
              state.velocity[i] = getMotorVelocity(state.name[i]);
              state.effort[i] = getMotorEffort(state.name[i]);
          }
          
          // 发布状态反馈
          pub->publish(state);
          
          // 200Hz 执行频率
          std::this_thread::sleep_for(5ms);
      }
      
      node.shutdown();
      spin_thread.join();
      
      return 0;
  }
  ```

</div>

## 执行端部署控制器示例

### 概述

对于执行层计算资源较为充足的场景，推荐在执行层直接部署 `RobotController`，控制层仅负责轨迹规划，将规划好的完整轨迹通过 LPSS 发送到执行层，由执行层的控制器进行采样和控制指令生成。

架构如下：
- **控制层** ：运行 `RobotPlanner`，规划轨迹并通过 LPSS 发布完整轨迹消息
- **执行层** ：订阅轨迹消息，使用 `RobotController` 采样控制指令并驱动电机，发布关节状态反馈

### 控制层

<div class="tabbed">

- <b class="tab-title">异步模式（C++20）</b>

  ```cpp
  #include <rmvl/lpss/robot.hpp>
  
  using namespace rm;
  using namespace std::chrono_literals;
  
  class PlannerNode : public lpss::async::Node {
  public:
      PlannerNode() : lpss::async::Node("planner") {
          // 加载机器人模型
          _planner.load("/path/to/robot.urdf");
          
          // 初始化关节状态
          msg::JointState init_state;
          init_state.name = {"joint1", "joint2", "joint3"};
          init_state.position = {0.0, 0.0, 0.0};
          init_state.velocity = {0.0, 0.0, 0.0};
          init_state.effort = {0.0, 0.0, 0.0};
          _planner.update(init_state);
          
          // 发布轨迹消息给执行层
          _traj_pub = this->createPublisher<msg::JointTrajectory>("joint_trajectory");
          
          // 规划周期（例如 1Hz，可根据需要调整）
          _plan_timer = this->createTimer(1000ms, [this]() {
              // 定义规划目标
              std::vector<msg::Pose> waypoints;
              
              // 第一个途经点
              msg::Pose p1;
              p1.position.x = 0.2;
              p1.position.y = 0.3;
              p1.position.z = 0.4;
              p1.orientation.x = 0.0;
              p1.orientation.y = 0.0;
              p1.orientation.z = 0.0;
              p1.orientation.w = 1.0;
              waypoints.push_back(p1);
              
              // 第二个途经点
              msg::Pose p2;
              p2.position.x = 0.3;
              p2.position.y = 0.4;
              p2.position.z = 0.5;
              p2.orientation.x = 0.0;
              p2.orientation.y = 0.0;
              p2.orientation.z = 0.7071;
              p2.orientation.w = 0.7071;
              waypoints.push_back(p2);
              
              // 规划轨迹
              auto traj = _planner.plan("end_effector", waypoints);
              
              if (!traj.points.empty()) {
                  printf("规划成功：%zu 个点，总耗时 %lld ms\n", traj.points.size(), traj.points.back().time_from_start);
                  
                  // 发布轨迹消息
                  _traj_pub->publish(traj);
              } else {
                  printf("规划失败\n");
              }
          });
      }
  
  private:
      lpss::RobotPlanner _planner;
      lpss::async::Publisher<msg::JointTrajectory>::ptr _traj_pub;
      lpss::async::Timer::ptr _plan_timer;
  };
  
  int main() {
      auto node = std::make_shared<PlannerNode>();
      node->spin();
      return 0;
  }
  ```

- <b class="tab-title">同步模式</b>

  ```cpp
  #include <rmvl/lpss/robot.hpp>
  
  using namespace rm;
  using namespace std::chrono_literals;
  
  int main() {
      // 创建节点
      lpss::Node node("planner");
      
      // 加载机器人模型
      lpss::RobotPlanner planner("/path/to/robot.urdf");
      
      // 初始化关节状态
      msg::JointState init_state;
      init_state.name = {"joint1", "joint2", "joint3"};
      init_state.position = {0.0, 0.0, 0.0};
      init_state.velocity = {0.0, 0.0, 0.0};
      init_state.effort = {0.0, 0.0, 0.0};
      planner.update(init_state);
      
      // 发布轨迹消息
      auto pub = node.createPublisher<msg::JointTrajectory>("joint_trajectory");
      
      // 启动节点的事件循环
      std::thread spin_thread([&]() { node.spin(); });
      
      // 规划循环
      int plan_count = 0;
      bool running = true;
      
      while (running) {
          // 定义规划目标
          std::vector<msg::Pose> waypoints;
          
          // 第一个途经点
          msg::Pose p1;
          p1.position.x = 0.2;
          p1.position.y = 0.3;
          p1.position.z = 0.4;
          p1.orientation.x = 0.0;
          p1.orientation.y = 0.0;
          p1.orientation.z = 0.0;
          p1.orientation.w = 1.0;
          waypoints.push_back(p1);
          
          // 第二个途经点
          msg::Pose p2;
          p2.position.x = 0.3;
          p2.position.y = 0.4;
          p2.position.z = 0.5;
          p2.orientation.x = 0.0;
          p2.orientation.y = 0.0;
          p2.orientation.z = 0.7071;
          p2.orientation.w = 0.7071;
          waypoints.push_back(p2);
          
          // 规划轨迹
          auto traj = planner.plan("end_effector", waypoints);
          
          if (!traj.points.empty()) {
              printf("规划成功：%zu 个点，总耗时 %lld ms\n", traj.points.size(), traj.points.back().time_from_start);
              
              // 发布轨迹消息
              pub->publish(traj);
          } else {
              printf("规划失败\n");
          }
          
          plan_count++;
          if (plan_count >= 5) {  // 规划 5 次后退出
              running = false;
          }
          
          // 1Hz 规划频率
          std::this_thread::sleep_for(1000ms);
      }
      
      node.shutdown();
      spin_thread.join();
      
      return 0;
  }
  ```

</div>

### 执行层

<div class="tabbed">

- <b class="tab-title">异步模式（C++20）</b>

  ```cpp
  #include <rmvl/lpss/robot.hpp>
  
  using namespace rm;
  using namespace std::chrono_literals;
  
  class ExecutorNode : public lpss::async::Node {
  public:
      ExecutorNode() : lpss::async::Node("executor") {
          // 订阅轨迹消息
          _traj_sub = this->createSubscriber<msg::JointTrajectory>("joint_trajectory", [this](const msg::JointTrajectory &traj) {
              if (_controller.submit(traj)) {
                  printf("轨迹已提交，包含 %zu 个点\n", traj.points.size());
              } else {
                  printf("轨迹提交失败\n");
              }
          });
          
          // 发布关节状态反馈
          _state_pub = this->createPublisher<msg::JointState>("/robot/cur/joints");
          
          // 执行层控制循环（200Hz）
          _exec_timer = this->createTimer(5ms, [this]() {
              // 从传感器或驱动器获取当前关节状态
              msg::JointState feedback;
              feedback.name = {"joint1", "joint2", "joint3"};
              feedback.position.resize(feedback.name.size());
              feedback.velocity.resize(feedback.name.size());
              feedback.effort.resize(feedback.name.size());
              
              for (size_t i = 0; i < feedback.name.size(); ++i) {
                  feedback.position[i] = getMotorPosition(feedback.name[i]);
                  feedback.velocity[i] = getMotorVelocity(feedback.name[i]);
                  feedback.effort[i] = getMotorEffort(feedback.name[i]);
              }
              
              // 采样控制指令
              auto cmd = _controller.sample(feedback);
              
              // 根据控制指令驱动电机
              for (size_t i = 0; i < cmd.name.size(); ++i) {
                  sendMotorCommand(cmd.name[i], cmd.position[i]);
              }
              
              // 发布状态反馈
              _state_pub->publish(feedback);
          });
      }
  
  private:
      lpss::RobotController _controller{{"joint1", "joint2", "joint3"}};
      lpss::async::Publisher<msg::JointState>::ptr _state_pub;
      lpss::async::Subscriber<msg::JointTrajectory>::ptr _traj_sub;
      lpss::async::Timer::ptr _exec_timer;
      
      void sendMotorCommand(const std::string &joint_name, double position) {
          // TODO：根据实际硬件接口实现
      }
      
      double getMotorPosition(const std::string &joint_name) {
          // TODO：从硬件读取电机位置
          return 0.0;
      }
      
      double getMotorVelocity(const std::string &joint_name) {
          // TODO：从硬件读取电机速度
          return 0.0;
      }
      
      double getMotorEffort(const std::string &joint_name) {
          // TODO：从硬件读取电机扭矩
          return 0.0;
      }
  };
  
  int main() {
      auto node = std::make_shared<ExecutorNode>();
      node->spin();
      return 0;
  }
  ```

- <b class="tab-title">同步模式</b>

  ```cpp
  #include <rmvl/lpss/robot.hpp>
  
  using namespace rm;
  using namespace std::chrono_literals;
  
  // 模拟函数，实际应根据硬件接口实现
  void sendMotorCommand(const std::string &joint_name, double position) {
      // TODO：根据实际硬件接口实现
  }
  
  double getMotorPosition(const std::string &joint_name) {
      // TODO：从硬件读取电机位置
      return 0.0;
  }
  
  double getMotorVelocity(const std::string &joint_name) {
      // TODO：从硬件读取电机速度
      return 0.0;
  }
  
  double getMotorEffort(const std::string &joint_name) {
      // TODO：从硬件读取电机扭矩
      return 0.0;
  }
  
  int main() {
      // 创建节点
      lpss::Node node("executor");
      
      // 创建控制器
      lpss::RobotController controller({"joint1", "joint2", "joint3"});
      
      // 订阅轨迹消息
      auto sub = node.createSubscriber<msg::JointTrajectory>("joint_trajectory", [&](const msg::JointTrajectory &traj) {
          if (controller.submit(traj)) {
              printf("轨迹已提交，包含 %zu 个点\n", traj.points.size());
          } else {
              printf("轨迹提交失败\n");
          }
      });
      
      // 发布关节状态
      auto pub = node.createPublisher<msg::JointState>("/robot/cur/joints");
      
      // 启动节点的事件循环
      std::thread spin_thread([&]() { node.spin(); });
      
      bool running = true;
      
      // 执行层控制循环（200Hz）
      while (running) {
          // 从传感器或驱动器获取当前关节状态
          msg::JointState feedback;
          feedback.name = {"joint1", "joint2", "joint3"};
          feedback.position.resize(feedback.name.size());
          feedback.velocity.resize(feedback.name.size());
          feedback.effort.resize(feedback.name.size());
          
          for (size_t i = 0; i < feedback.name.size(); ++i) {
              feedback.position[i] = getMotorPosition(feedback.name[i]);
              feedback.velocity[i] = getMotorVelocity(feedback.name[i]);
              feedback.effort[i] = getMotorEffort(feedback.name[i]);
          }
          
          // 采样控制指令
          auto cmd = controller.sample(feedback);
          
          // 根据控制指令驱动电机
          for (size_t i = 0; i < cmd.name.size(); ++i) {
              sendMotorCommand(cmd.name[i], cmd.position[i]);
          }
          
          // 发布状态反馈
          pub->publish(feedback);
          
          // 200Hz 执行频率
          std::this_thread::sleep_for(5ms);
      }
      
      node.shutdown();
      spin_thread.join();
      
      return 0;
  }
  ```

</div>
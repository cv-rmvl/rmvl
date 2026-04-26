/**
 * @file test_ctl.cpp
 * @author zhaoxi (535394140@qq.com)
 * @brief 控制律组件单元测试
 * @version 1.0
 * @date 2026-04-28
 *
 * @copyright Copyright 2026 (c), zhaoxi
 *
 */

#include <gtest/gtest.h>

#include "rmvl/lpss/ctl/ff.hpp"
#include "rmvl/lpss/ctl/pid.hpp"
#include "rmvl/lpss/robot.hpp"

namespace rm_test {

using namespace rm;
using namespace lpss;

static constexpr double kEps = 1e-6;
static constexpr int32_t kPeriod = 10; // 10ms

// 单位传递函数
TEST(LPSS_ctl, unit_tf_basic) {
    auto ctl_law = ctl::UnitTF::create(ctl::basic_pos_imapping, ctl::basic_pos_omapping);
    ASSERT_NE(ctl_law, nullptr);

    // 创建期望和反馈状态
    msg::JointState desired{}, fb{}, cmd{};
    desired.name = {"joint1", "joint2"};
    desired.position = {1.0, 2.0};
    desired.velocity = {0.5, 1.5};
    desired.effort = {10.0, 20.0};

    fb.name = {"joint1", "joint2"};
    fb.position = {0.9, 1.95};
    fb.velocity = {0.45, 1.45};
    fb.effort = {9.5, 19.5};

    // 位置模式：输出应该等于期望
    auto res = ctl_law->compute(desired, fb, kPeriod, cmd);
    EXPECT_EQ(res, ctl::ControlStatus::Ok);
    EXPECT_EQ(cmd.position, desired.position);

    // 速度模式：输出应该等于期望
    auto ctl_law_vel = ctl::UnitTF::create(ctl::basic_vel_imapping, ctl::basic_vel_omapping);
    res = ctl_law_vel->compute(desired, fb, kPeriod, cmd);
    EXPECT_EQ(res, ctl::ControlStatus::Ok);
    EXPECT_EQ(cmd.velocity, desired.velocity);

    // 力矩模式：输出应该等于期望
    auto ctl_law_eff = ctl::UnitTF::create(ctl::basic_eff_imapping, ctl::basic_eff_omapping);
    res = ctl_law_eff->compute(desired, fb, kPeriod, cmd);
    EXPECT_EQ(res, ctl::ControlStatus::Ok);
    EXPECT_EQ(cmd.effort, desired.effort);
}

// 前馈
TEST(LPSS_ctl, feedforward_basic) {
    // 前馈系数：a0=2.0, a1=0, a2=0 （纯增益，乘以2）
    std::vector<double> a0 = {2.0};
    std::vector<double> a1 = {0.0};
    std::vector<double> a2 = {0.0};

    auto ctl_law = ctl::FeedForward::create(a0, a1, a2, ctl::basic_vel_imapping, ctl::basic_vel_omapping);
    ASSERT_NE(ctl_law, nullptr);

    msg::JointState desired{}, fb{}, cmd{};
    desired.name = {"joint1"};
    desired.velocity = {1.0};

    fb.name = {"joint1"};
    fb.velocity = {0.0};

    // 第一步：期望速度为 1.0 m/s，输出应该是 a0 * 1.0 = 2.0
    auto res = ctl_law->compute(desired, fb, kPeriod, cmd);
    EXPECT_EQ(res, ctl::ControlStatus::Ok);
    EXPECT_NEAR(cmd.velocity[0], 2.0, kEps);

    // 第二步：期望速度为 2.0 m/s，输出应该是 a0 * 2.0 = 4.0（纯增益不受微分影响）
    desired.velocity = {2.0};
    res = ctl_law->compute(desired, fb, kPeriod, cmd);
    EXPECT_EQ(res, ctl::ControlStatus::Ok);
    EXPECT_NEAR(cmd.velocity[0], 4.0, kEps);
}

// PID
TEST(LPSS_ctl, pid_basic) {
    std::vector<double> kp = {10.0};
    std::vector<double> ki = {2.0};
    std::vector<double> kd = {1.0};

    auto ctl_law = ctl::PID::create(kp, ki, kd);
    ASSERT_NE(ctl_law, nullptr);

    msg::JointState desired{}, fb{}, cmd{};
    desired.name = {"joint1"};
    desired.position = {1.0};

    fb.name = {"joint1"};
    fb.position = {0.0}; // 初始位置偏差为 1.0

    // 第一步：误差 = 1.0
    auto res = ctl_law->compute(desired, fb, kPeriod, cmd);
    EXPECT_EQ(res, ctl::ControlStatus::Ok);
    // 输出应该是正的，用于消除误差
    EXPECT_GT(cmd.position[0], 0.0);

    double prev_cmd = cmd.position[0];

    // 第二步：反馈改善，误差变为 0.5
    fb.position = {0.5};
    res = ctl_law->compute(desired, fb, kPeriod, cmd);
    EXPECT_EQ(res, ctl::ControlStatus::Ok);
    // 输出应该减小（因为误差减小）
    EXPECT_LT(cmd.position[0], prev_cmd);
}

TEST(LPSS_ctl, pid_steady_state) {
    // 测试 PID 稳态行为：纯比例控制
    std::vector<double> kp = {5.0};
    std::vector<double> ki = {0.0}; // 无积分
    std::vector<double> kd = {0.0}; // 无微分

    auto ctl_law = ctl::PID::create(kp, ki, kd, ctl::basic_pos_imapping, ctl::basic_pos_omapping);

    msg::JointState desired{}, fb{}, cmd{};
    desired.name = {"joint1"};
    desired.position = {10.0};

    fb.name = {"joint1"};
    fb.position = {5.0}; // 误差为 5.0

    auto res = ctl_law->compute(desired, fb, kPeriod, cmd);
    EXPECT_EQ(res, ctl::ControlStatus::Ok);
    // 输出应该 = kp * error = 5.0 * 5.0 = 25.0
    EXPECT_NEAR(cmd.position[0], 25.0, kEps);
}

// ============================================================================
// UnitTF + RobotController 集成测试
// ============================================================================

TEST(LPSS_ctl, robot_controller_with_unitTF) {
    auto ctl_law = ctl::UnitTF::create(ctl::basic_pos_imapping, ctl::basic_pos_omapping);

    RobotController controller({"joint1", "joint2"}, std::move(ctl_law));

    // 创建简单轨迹
    msg::JointTrajectory traj{};
    traj.joint_names = {"joint1", "joint2"};

    msg::JointTrajectoryPoint pt1{};
    pt1.time_from_start = 0;
    pt1.positions = {0.0, 0.0};
    pt1.velocities = {0.0, 0.0};

    msg::JointTrajectoryPoint pt2{};
    pt2.time_from_start = 100; // 100ms
    pt2.positions = {1.0, 2.0};
    pt2.velocities = {0.1, 0.2};

    traj.points = {pt1, pt2};

    // 提交轨迹
    ASSERT_TRUE(controller.submit(traj));

    // 采样反馈
    msg::JointState feedback{};
    feedback.name = {"joint1", "joint2"};
    feedback.position = {0.0, 0.0};
    feedback.velocity = {0.0, 0.0};

    auto cmd = controller.sample(feedback);
    EXPECT_EQ(cmd.name.size(), 2u);
    // UnitTF 应该直接输出期望状态
    // 在 t=0，期望位置为 {0.0, 0.0}
    EXPECT_NEAR(cmd.position[0], 0.0, kEps);
    EXPECT_NEAR(cmd.position[1], 0.0, kEps);
}

TEST(LPSS_ctl, robot_controller_velocity_tracking) {
    // 创建 PID 控制器用于速度跟踪
    std::vector<double> kp = {1.0};
    std::vector<double> ki = {0.1};
    std::vector<double> kd = {0.0};

    auto ctl_law = ctl::PID::create(kp, ki, kd, ctl::basic_vel_imapping, ctl::basic_vel_omapping);

    RobotController controller({"joint1"}, std::move(ctl_law));

    // 创建轨迹
    msg::JointTrajectory traj{};
    traj.joint_names = {"joint1"};

    msg::JointTrajectoryPoint pt1{};
    pt1.time_from_start = 0;
    pt1.positions = {0.0};
    pt1.velocities = {1.0}; // 期望速度 1.0 m/s

    msg::JointTrajectoryPoint pt2{};
    pt2.time_from_start = 100;
    pt2.positions = {0.1};
    pt2.velocities = {1.0}; // 保持速度 1.0 m/s

    traj.points = {pt1, pt2};

    ASSERT_TRUE(controller.submit(traj));

    // 反馈：速度偏低
    msg::JointState feedback{};
    feedback.name = {"joint1"};
    feedback.position = {0.0};
    feedback.velocity = {0.8}; // 实际速度 0.8 m/s

    auto cmd = controller.sample(feedback);

    // PID 应该输出正的速度命令来加速
    EXPECT_GT(cmd.velocity[0], 0.0);
    // 误差 = 1.0 - 0.8 = 0.2，输出应该 ≈ kp * 0.2 = 0.2
    EXPECT_NEAR(cmd.velocity[0], 0.2, 0.01); // 允许一点误差
}

} // namespace rm_test

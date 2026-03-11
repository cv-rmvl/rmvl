/**
 * @file test_robot.cpp
 * @author zhaoxi (535394140@qq.com)
 * @brief RobotPlanner 单元测试
 * @version 1.0
 * @date 2026-03-04
 *
 * @copyright Copyright 2026 (c), zhaoxi
 *
 */

#include <fstream>

#include <gtest/gtest.h>

#include "rmvl/algorithm/math.hpp"

#include "rmvl/lpss/robot.hpp"

namespace rm_test {

using namespace rm;
using namespace lpss;

// 简单 2-DOF 机械臂 URDF：base_link -> link1 (revolute, Z轴) -> link2 (revolute, Z轴)
// link1 原点距 base_link (0, 0, 0.5)，link2 原点距 link1 (1.0, 0, 0)
static constexpr const char *k_urdf_2dof = R"(<?xml version="1.0"?>
<robot name="test_2dof">
  <link name="base_link"/>
  <link name="link1"/>
  <link name="link2"/>

  <joint name="joint1" type="revolute">
    <parent link="base_link"/>
    <child link="link1"/>
    <origin xyz="0 0 0.5" rpy="0 0 0"/>
    <axis xyz="0 0 1"/>
    <limit lower="-3.14" upper="3.14"/>
  </joint>

  <joint name="joint2" type="revolute">
    <parent link="link1"/>
    <child link="link2"/>
    <origin xyz="1.0 0 0" rpy="0 0 0"/>
    <axis xyz="0 0 1"/>
    <limit lower="-3.14" upper="3.14"/>
  </joint>
</robot>
)";

// 含 fixed 关节的 URDF
static constexpr const char *k_urdf_with_fixed = R"(<?xml version="1.0"?>
<robot name="test_fixed">
  <link name="base_link"/>
  <link name="link1"/>
  <link name="tool"/>

  <joint name="joint1" type="revolute">
    <parent link="base_link"/>
    <child link="link1"/>
    <origin xyz="0 0 1" rpy="0 0 0"/>
    <axis xyz="0 0 1"/>
    <limit lower="-3.14" upper="3.14"/>
  </joint>

  <joint name="tool_joint" type="fixed">
    <parent link="link1"/>
    <child link="tool"/>
    <origin xyz="0.5 0 0" rpy="0 0 0"/>
  </joint>
</robot>
)";

//! 辅助：写临时 URDF 文件，返回路径
static std::string writeTempURDF(const char *content, const char *name) {
#ifdef _WIN32
    std::string path = std::string(std::getenv("TEMP")) + "\\" + name;
#else
    std::string path = std::string("/tmp/") + name;
#endif
    std::ofstream ofs(path);
    ofs << content;
    ofs.close();
    return path;
}

static constexpr double kEps = 1e-9;

// 加载 / 基础属性
TEST(LPSS_robotctl, load_and_joint_count) {
    auto path = writeTempURDF(k_urdf_2dof, "test_2dof.urdf");
    RobotPlanner rc(path);

    // 应有 2 个活动关节
    const auto &js = rc.joints();
    ASSERT_EQ(js.name.size(), 2u);
    EXPECT_EQ(js.name[0], "joint1");
    EXPECT_EQ(js.name[1], "joint2");

    // 零位初始化
    for (auto p : js.position)
        EXPECT_NEAR(p, 0.0, kEps);
}

TEST(LPSS_robotctl, load_with_fixed_joint) {
    auto path = writeTempURDF(k_urdf_with_fixed, "test_fixed.urdf");
    RobotPlanner rc(path);

    // fixed 关节不算活动关节，只有 joint1
    const auto &js = rc.joints();
    ASSERT_EQ(js.name.size(), 1u);
    EXPECT_EQ(js.name[0], "joint1");
}

// TF 树
TEST(LPSS_robotctl, tf_at_zero_position) {
    auto path = writeTempURDF(k_urdf_2dof, "test_2dof.urdf");
    RobotPlanner rc(path);

    const auto &tf = rc.tf();
    // 2 个关节 -> 2 个 TransformStamped
    ASSERT_EQ(tf.transforms.size(), 2u);

    // base_link -> link1
    const auto &ts0 = tf.transforms[0];
    EXPECT_EQ(ts0.header.frame_id, "base_link");
    EXPECT_EQ(ts0.child_frame_id, "link1");
    EXPECT_NEAR(ts0.transform.translation.x, 0.0, kEps);
    EXPECT_NEAR(ts0.transform.translation.y, 0.0, kEps);
    EXPECT_NEAR(ts0.transform.translation.z, 0.5, kEps);

    // link1 -> link2
    const auto &ts1 = tf.transforms[1];
    EXPECT_EQ(ts1.header.frame_id, "link1");
    EXPECT_EQ(ts1.child_frame_id, "link2");
    EXPECT_NEAR(ts1.transform.translation.x, 1.0, kEps);
    EXPECT_NEAR(ts1.transform.translation.y, 0.0, kEps);
    EXPECT_NEAR(ts1.transform.translation.z, 0.0, kEps);
}

// ========================== update + 正运动学 ==========================

TEST(LPSS_robotctl, update_and_get_link_pose_zero) {
    auto path = writeTempURDF(k_urdf_2dof, "test_2dof.urdf");
    RobotPlanner rc(path);

    // 零位时 link2 的位置应为 base_link 原点 + (0,0,0.5) + (1,0,0) = (1, 0, 0.5)
    auto pose = rc.linkpose("link2");
    EXPECT_NEAR(pose.position.x, 1.0, kEps);
    EXPECT_NEAR(pose.position.y, 0.0, kEps);
    EXPECT_NEAR(pose.position.z, 0.5, kEps);
}

TEST(LPSS_robotctl, update_joint1_90deg) {
    auto path = writeTempURDF(k_urdf_2dof, "test_2dof.urdf");
    RobotPlanner rc(path);

    // joint1 旋转 90° (π/2)，joint2 保持 0
    // link1 相对 base_link: origin (0,0,0.5)，绕 Z 轴转 90° -> link1 位置不变 (0,0,0.5)
    // link2 相对 link1: origin (1,0,0)，但 link1 已绕 Z 转 90°
    // 所以 link2 在世界坐标系下 = (0,0,0.5) + R_z(90°)*(1,0,0) = (0,0,0.5) + (0,1,0) = (0,1,0.5)
    msg::JointState js;
    js.name = {"joint1", "joint2"};
    js.position = {rm::PI / 2.0, 0.0};
    js.velocity = {0, 0};
    js.effort = {0, 0};
    rc.update(js);

    auto pose = rc.linkpose("link2");
    EXPECT_NEAR(pose.position.x, 0.0, 1e-6);
    EXPECT_NEAR(pose.position.y, 1.0, 1e-6);
    EXPECT_NEAR(pose.position.z, 0.5, 1e-6);
}

TEST(LPSS_robotctl, update_both_joints_90deg) {
    auto path = writeTempURDF(k_urdf_2dof, "test_2dof.urdf");
    RobotPlanner rc(path);

    // joint1 = 90°, joint2 = 90°
    // link1 在世界: (0, 0, 0.5)，朝向绕 Z 转 90°
    // link2 相对 link1: origin (1,0,0)，再绕 Z 转 90°
    //   link2 在 link1 坐标系转 90° 后：R_z(90°)*(1,0,0) = (0,1,0)
    //   但这是 joint2 的旋转对 link2 之后连杆的影响，link2 本身位置由 joint2 的 origin 决定
    //   link2 世界位置 = link1 世界位 + R_link1_world * joint2_origin
    //                  = (0,0,0.5) + R_z(90°)*(1,0,0) = (0,1,0.5)
    msg::JointState js;
    js.name = {"joint1", "joint2"};
    js.position = {rm::PI / 2.0, rm::PI / 2.0};
    js.velocity = {0, 0};
    js.effort = {0, 0};
    rc.update(js);

    auto pose = rc.linkpose("link2");
    EXPECT_NEAR(pose.position.x, 0.0, 1e-6);
    EXPECT_NEAR(pose.position.y, 1.0, 1e-6);
    EXPECT_NEAR(pose.position.z, 0.5, 1e-6);
}

// ========================== linkpose 根连杆 ==========================

TEST(LPSS_robotctl, get_root_link_pose_is_identity) {
    auto path = writeTempURDF(k_urdf_2dof, "test_2dof.urdf");
    RobotPlanner rc(path);

    auto pose = rc.linkpose("base_link");
    EXPECT_NEAR(pose.position.x, 0.0, kEps);
    EXPECT_NEAR(pose.position.y, 0.0, kEps);
    EXPECT_NEAR(pose.position.z, 0.0, kEps);
    // 单位四元数
    EXPECT_NEAR(pose.orientation.w, 1.0, kEps);
    EXPECT_NEAR(pose.orientation.x, 0.0, kEps);
    EXPECT_NEAR(pose.orientation.y, 0.0, kEps);
    EXPECT_NEAR(pose.orientation.z, 0.0, kEps);
}

// ========================== linkpose 不存在的连杆 ==========================

TEST(LPSS_robotctl, get_unknown_link_returns_zero) {
    auto path = writeTempURDF(k_urdf_2dof, "test_2dof.urdf");
    RobotPlanner rc(path);

    auto pose = rc.linkpose("nonexistent_link");
    EXPECT_NEAR(pose.position.x, 0.0, kEps);
    EXPECT_NEAR(pose.position.y, 0.0, kEps);
    EXPECT_NEAR(pose.position.z, 0.0, kEps);
}

// ========================== fixed 关节不影响活动关节 ==========================

TEST(LPSS_robotctl, fixed_joint_tool_pose) {
    auto path = writeTempURDF(k_urdf_with_fixed, "test_fixed.urdf");
    RobotPlanner rc(path);

    // 零位: base_link -> link1 (0,0,1) -> tool (0.5,0,0)
    // tool 世界位置 = (0.5, 0, 1)
    auto pose = rc.linkpose("tool");
    EXPECT_NEAR(pose.position.x, 0.5, kEps);
    EXPECT_NEAR(pose.position.y, 0.0, kEps);
    EXPECT_NEAR(pose.position.z, 1.0, kEps);

    // joint1 旋转 90° -> tool 世界位置 = (0,0,1) + R_z(90°)*(0.5,0,0) = (0, 0.5, 1)
    msg::JointState js;
    js.name = {"joint1"};
    js.position = {rm::PI / 2.0};
    js.velocity = {0};
    js.effort = {0};
    rc.update(js);

    pose = rc.linkpose("tool");
    EXPECT_NEAR(pose.position.x, 0.0, 1e-6);
    EXPECT_NEAR(pose.position.y, 0.5, 1e-6);
    EXPECT_NEAR(pose.position.z, 1.0, 1e-6);
}

// ========================== reload 测试 ==========================

TEST(LPSS_robotctl, reload_changes_model) {
    auto path_2dof = writeTempURDF(k_urdf_2dof, "test_2dof.urdf");
    auto path_fixed = writeTempURDF(k_urdf_with_fixed, "test_fixed.urdf");

    RobotPlanner rc(path_2dof);
    EXPECT_EQ(rc.joints().name.size(), 2u);

    // 重新加载另一个 URDF
    rc.load(path_fixed);
    EXPECT_EQ(rc.joints().name.size(), 1u);
    EXPECT_EQ(rc.joints().name[0], "joint1");

    // 关节状态应重置为零
    for (auto p : rc.joints().position)
        EXPECT_NEAR(p, 0.0, kEps);
}

// ========================== plan ==========================

TEST(LPSS_robotctl, plan_returns_correct_joint_names) {
    auto path = writeTempURDF(k_urdf_2dof, "test_2dof.urdf");
    RobotPlanner rc(path);

    msg::Pose target;
    target.position = {1.0, 0.0, 0.5};
    target.orientation = {0, 0, 0, 1};

    auto traj = rc.plan("link2", target);
    // plan 尚未实现，但 joint_names 应正确填充
    ASSERT_EQ(traj.joint_names.size(), 2u);
    EXPECT_EQ(traj.joint_names[0], "joint1");
    EXPECT_EQ(traj.joint_names[1], "joint2");
    // points 为空（未实现）
    EXPECT_TRUE(traj.points.empty());
}

} // namespace rm_test

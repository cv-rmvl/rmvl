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

#include "../src/robot_impl.hpp"
#include "test_robot_def.hpp"

namespace rm_test {

using namespace rm;
using namespace lpss;

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

// 基础加载功能
TEST(LPSS_robotctl, urdf_basic_load) {
    auto path = writeTempURDF(k_urdf_2dof, "test_2dof.urdf");
    RobotPlanner rp(path);

    // 应有 2 个活动关节
    const auto &js = rp.joints();
    ASSERT_EQ(js.name.size(), 2u);
    EXPECT_EQ(js.name[0], "joint1");
    EXPECT_EQ(js.name[1], "joint2");

    // 零位初始化
    for (auto p : js.position)
        EXPECT_NEAR(p, 0.0, kEps);
}

// 带固定关节的 URDF 加载
TEST(LPSS_robotctl, urdf_load_with_fixed_joint) {
    auto path = writeTempURDF(k_urdf_with_fixed, "test_fixed.urdf");
    RobotPlanner rp(path);

    // fixed 关节不算活动关节，只有 joint1
    const auto &js = rp.joints();
    ASSERT_EQ(js.name.size(), 1u);
    EXPECT_EQ(js.name[0], "joint1");
}

// max_velocity 解析
TEST(LPSS_robotctl, urdf_parse_max_velocity) {
    lpss::URDFModel model;
    model.parse(k_urdf_with_dynamics);

    ASSERT_EQ(model.active_joint_indices.size(), 2u);
    const auto &j1 = model.joints[model.active_joint_indices[0]];
    const auto &j2 = model.joints[model.active_joint_indices[1]];

    EXPECT_NEAR(j1.max_velocity, 2.0, kEps);
    EXPECT_NEAR(j2.max_velocity, 1.5, kEps);
}

TEST(LPSS_robotctl, urdf_parse_max_velocity_default) {
    // k_urdf_2dof 的 <limit> 没有 velocity 属性，应为默认值 3.14
    lpss::URDFModel model;
    model.parse(k_urdf_2dof);

    for (auto idx : model.active_joint_indices)
        EXPECT_NEAR(model.joints[idx].max_velocity, 3.14, kEps);
}

// max_effort 解析
TEST(LPSS_robotctl, urdf_parse_max_effort) {
    lpss::URDFModel model;
    model.parse(k_urdf_with_dynamics);

    const auto &j1 = model.joints[model.active_joint_indices[0]];
    const auto &j2 = model.joints[model.active_joint_indices[1]];

    EXPECT_NEAR(j1.max_effort, 50.0, kEps);
    EXPECT_NEAR(j2.max_effort, 20.0, kEps);
}

TEST(LPSS_robotctl, urdf_parse_max_effort_default) {
    // k_urdf_2dof 的 <limit> 没有 effort 属性，应为默认值 0
    lpss::URDFModel model;
    model.parse(k_urdf_2dof);

    for (auto idx : model.active_joint_indices)
        EXPECT_NEAR(model.joints[idx].max_effort, 0.0, kEps);
}

// inertial 解析
TEST(LPSS_robotctl, urdf_parse_link_inertial) {
    lpss::URDFModel model;
    model.parse(k_urdf_with_dynamics);

    // link1
    auto it1 = model.link_index.find("link1");
    ASSERT_NE(it1, model.link_index.end());
    const auto &l1 = model.links[it1->second];
    EXPECT_NEAR(l1.mass, 2.0, kEps);
    EXPECT_NEAR(l1.inertia[0], 0.1, kEps);  // ixx
    EXPECT_NEAR(l1.inertia[3], 0.1, kEps);  // iyy
    EXPECT_NEAR(l1.inertia[5], 0.5, kEps);  // izz

    // link2
    auto it2 = model.link_index.find("link2");
    ASSERT_NE(it2, model.link_index.end());
    const auto &l2 = model.links[it2->second];
    EXPECT_NEAR(l2.mass, 1.0, kEps);
    EXPECT_NEAR(l2.inertia[5], 0.2, kEps);  // izz
}

TEST(LPSS_robotctl, urdf_parse_link_no_inertial_defaults_zero) {
    // k_urdf_2dof 的 link 没有 <inertial>，mass 和 inertia 应为 0
    lpss::URDFModel model;
    model.parse(k_urdf_2dof);

    for (const auto &l : model.links) {
        EXPECT_NEAR(l.mass, 0.0, kEps);
        for (double v : l.inertia)
            EXPECT_NEAR(v, 0.0, kEps);
    }
}

// max_acceleration = effort / inertia（旋转关节, Z 轴）
TEST(LPSS_robotctl, urdf_max_acceleration_revolute_z_axis) {
    lpss::URDFModel model;
    model.parse(k_urdf_with_dynamics);

    // joint1: effort=50, izz=0.5, axis=(0,0,1) => a = 50/0.5 = 100
    const auto &j1 = model.joints[model.active_joint_indices[0]];
    EXPECT_NEAR(j1.max_acceleration, 100.0, 1e-6);

    // joint2: effort=20, izz=0.2, axis=(0,0,1) => a = 20/0.2 = 100
    const auto &j2 = model.joints[model.active_joint_indices[1]];
    EXPECT_NEAR(j2.max_acceleration, 100.0, 1e-6);
}

// max_acceleration = effort / inertia（旋转关节, Y 轴）
TEST(LPSS_robotctl, urdf_max_acceleration_revolute_y_axis) {
    lpss::URDFModel model;
    model.parse(k_urdf_y_axis);

    // axis=(0,1,0), I_axis = iyy = 0.6, effort=30 => a = 30/0.6 = 50
    const auto &j = model.joints[model.active_joint_indices[0]];
    EXPECT_NEAR(j.max_acceleration, 50.0, 1e-6);
}

// max_acceleration（非轴对齐旋转轴）
TEST(LPSS_robotctl, urdf_max_acceleration_revolute_diagonal_axis) {
    lpss::URDFModel model;
    model.parse(k_urdf_diagonal_axis);

    // axis ≈ (1/√2, 1/√2, 0), effort=40
    // I_axis = 0.2*0.5 + 0.8*0.5 + 0*0 + 2*(0.1*0.5 + 0 + 0) = 0.6
    // a = 40 / 0.6 ≈ 66.667
    const auto &j = model.joints[model.active_joint_indices[0]];
    EXPECT_NEAR(j.max_acceleration, 40.0 / 0.6, 1e-3);
}

// max_acceleration = effort / mass（移动关节）
TEST(LPSS_robotctl, urdf_max_acceleration_prismatic) {
    lpss::URDFModel model;
    model.parse(k_urdf_prismatic);

    // effort=100, mass=5.0 => a = 100/5 = 20
    const auto &j = model.joints[model.active_joint_indices[0]];
    EXPECT_NEAR(j.max_acceleration, 20.0, 1e-6);
}

// effort 缺失 => 保留默认 10
TEST(LPSS_robotctl, urdf_max_acceleration_default_no_effort) {
    lpss::URDFModel model;
    model.parse(k_urdf_no_effort);

    const auto &j = model.joints[model.active_joint_indices[0]];
    EXPECT_NEAR(j.max_acceleration, 10.0, kEps);
}

// inertial 缺失 => 保留默认 10
TEST(LPSS_robotctl, urdf_max_acceleration_default_no_inertial) {
    lpss::URDFModel model;
    model.parse(k_urdf_no_inertial);

    // effort=50 但 link1 没有 <inertial>，izz=0 => I_axis=0 => 保留默认 10
    const auto &j = model.joints[model.active_joint_indices[0]];
    EXPECT_NEAR(j.max_acceleration, 10.0, kEps);
}

// TF 树
TEST(LPSS_robotctl, tf_at_zero_position) {
    auto path = writeTempURDF(k_urdf_2dof, "test_2dof.urdf");
    RobotPlanner rp(path);

    const auto &tf = rp.tf();
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
    RobotPlanner rp(path);

    // 零位时 link2 的位置应为 base_link 原点 + (0,0,0.5) + (1,0,0) = (1, 0, 0.5)
    auto pose = rp.linkpose("link2");
    EXPECT_NEAR(pose.position.x, 1.0, kEps);
    EXPECT_NEAR(pose.position.y, 0.0, kEps);
    EXPECT_NEAR(pose.position.z, 0.5, kEps);
}

TEST(LPSS_robotctl, update_joint1_90deg) {
    auto path = writeTempURDF(k_urdf_2dof, "test_2dof.urdf");
    RobotPlanner rp(path);

    // joint1 旋转 90° (π/2)，joint2 保持 0
    // link1 相对 base_link: origin (0,0,0.5)，绕 Z 轴转 90° -> link1 位置不变 (0,0,0.5)
    // link2 相对 link1: origin (1,0,0)，但 link1 已绕 Z 转 90°
    // 所以 link2 在世界坐标系下 = (0,0,0.5) + R_z(90°)*(1,0,0) = (0,0,0.5) + (0,1,0) = (0,1,0.5)
    msg::JointState js;
    js.name = {"joint1", "joint2"};
    js.position = {rm::PI / 2.0, 0.0};
    js.velocity = {0, 0};
    js.effort = {0, 0};
    rp.update(js);

    auto pose = rp.linkpose("link2");
    EXPECT_NEAR(pose.position.x, 0.0, 1e-6);
    EXPECT_NEAR(pose.position.y, 1.0, 1e-6);
    EXPECT_NEAR(pose.position.z, 0.5, 1e-6);
}

TEST(LPSS_robotctl, update_both_joints_90deg) {
    auto path = writeTempURDF(k_urdf_2dof, "test_2dof.urdf");
    RobotPlanner rp(path);

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
    rp.update(js);

    auto pose = rp.linkpose("link2");
    EXPECT_NEAR(pose.position.x, 0.0, 1e-6);
    EXPECT_NEAR(pose.position.y, 1.0, 1e-6);
    EXPECT_NEAR(pose.position.z, 0.5, 1e-6);
}

// ========================== linkpose 根连杆 ==========================

TEST(LPSS_robotctl, get_root_link_pose_is_identity) {
    auto path = writeTempURDF(k_urdf_2dof, "test_2dof.urdf");
    RobotPlanner rp(path);

    auto pose = rp.linkpose("base_link");
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
    RobotPlanner rp(path);

    auto pose = rp.linkpose("nonexistent_link");
    EXPECT_NEAR(pose.position.x, 0.0, kEps);
    EXPECT_NEAR(pose.position.y, 0.0, kEps);
    EXPECT_NEAR(pose.position.z, 0.0, kEps);
}

// ========================== fixed 关节不影响活动关节 ==========================

TEST(LPSS_robotctl, fixed_joint_tool_pose) {
    auto path = writeTempURDF(k_urdf_with_fixed, "test_fixed.urdf");
    RobotPlanner rp(path);

    // 零位: base_link -> link1 (0,0,1) -> tool (0.5,0,0)
    // tool 世界位置 = (0.5, 0, 1)
    auto pose = rp.linkpose("tool");
    EXPECT_NEAR(pose.position.x, 0.5, kEps);
    EXPECT_NEAR(pose.position.y, 0.0, kEps);
    EXPECT_NEAR(pose.position.z, 1.0, kEps);

    // joint1 旋转 90° -> tool 世界位置 = (0,0,1) + R_z(90°)*(0.5,0,0) = (0, 0.5, 1)
    msg::JointState js;
    js.name = {"joint1"};
    js.position = {rm::PI / 2.0};
    js.velocity = {0};
    js.effort = {0};
    rp.update(js);

    pose = rp.linkpose("tool");
    EXPECT_NEAR(pose.position.x, 0.0, 1e-6);
    EXPECT_NEAR(pose.position.y, 0.5, 1e-6);
    EXPECT_NEAR(pose.position.z, 1.0, 1e-6);
}

// ========================== reload 测试 ==========================

TEST(LPSS_robotctl, reload_changes_model) {
    auto path_2dof = writeTempURDF(k_urdf_2dof, "test_2dof.urdf");
    auto path_fixed = writeTempURDF(k_urdf_with_fixed, "test_fixed.urdf");

    RobotPlanner rp(path_2dof);
    EXPECT_EQ(rp.joints().name.size(), 2u);

    // 重新加载另一个 URDF
    rp.load(path_fixed);
    EXPECT_EQ(rp.joints().name.size(), 1u);
    EXPECT_EQ(rp.joints().name[0], "joint1");

    // 关节状态应重置为零
    for (auto p : rp.joints().position)
        EXPECT_NEAR(p, 0.0, kEps);
}

// ========================== plan ==========================

TEST(LPSS_robotctl, plan_onestep_jointstate) {
    auto path = writeTempURDF(k_urdf_2dof, "test_2dof.urdf");
    RobotPlanner rp(path);

    // 目标关节角
    msg::JointState target;
    target.name = {"joint1", "joint2"};
    target.position = {rm::PI / 4, rm::PI / 6};
    target.velocity = {0, 0};
    target.effort = {0, 0};

    auto traj = rp.plan(target);
    ASSERT_GT(traj.points.size(), 0u);

    // 终点关节角应等于目标关节角
    EXPECT_NEAR(traj.points.back().positions[0], rm::PI / 4, 1e-6);
    EXPECT_NEAR(traj.points.back().positions[1], rm::PI / 6, 1e-6);

    // 起点关节角应等于零位
    EXPECT_NEAR(traj.points.front().positions[0], 0.0, 1e-6);
    EXPECT_NEAR(traj.points.front().positions[1], 0.0, 1e-6);

    // 两端速度为零
    for (double v : traj.points.front().velocities)
        EXPECT_NEAR(v, 0.0, 1e-6);
    for (double v : traj.points.back().velocities)
        EXPECT_NEAR(v, 0.0, 1e-6);
}

#ifdef RMVL_LPSS_WITH_KDL

TEST(LPSS_robotctl, plan_onestep_pose_traj_names) {
    auto path = writeTempURDF(k_urdf_2dof, "test_2dof.urdf");
    RobotPlanner rp(path);

    msg::Pose target;
    target.position = {0.707107, 0.707107, 0.5};
    target.orientation = {0, 0, 0.3827, 0.9239};

    auto traj = rp.plan("link2", target);
    ASSERT_EQ(traj.joint_names.size(), 2u);
    EXPECT_EQ(traj.joint_names[0], "joint1");
    EXPECT_EQ(traj.joint_names[1], "joint2");
}

TEST(LPSS_robotctl, plan_onestep_pose_invalid_frame) {
    auto path = writeTempURDF(k_urdf_2dof, "test_2dof.urdf");
    RobotPlanner rp(path);

    msg::Pose target;
    target.orientation = {0, 0, 0.3827, 0.9239};

    auto traj = rp.plan("nonexistent_link", target);
    EXPECT_TRUE(traj.points.empty());
}

TEST(LPSS_robotctl, plan_onestep_pose_boundary_0v) {
    auto path = writeTempURDF(k_urdf_2dof, "test_2dof.urdf");
    RobotPlanner rp(path);

    msg::Pose target;
    target.position = {0.707107, 0.707107, 0.5};
    target.orientation = {0, 0, 0.3827, 0.9239};

    auto traj = rp.plan("link2", target);
    ASSERT_GT(traj.points.size(), 1u);
    for (double v : traj.points.front().velocities)
        EXPECT_NEAR(v, 0.0, 1e-6);
    for (double v : traj.points.back().velocities)
        EXPECT_NEAR(v, 0.0, 1e-6);
}

TEST(LPSS_robotctl, plan_ik_fk_consistency) {
    auto path = writeTempURDF(k_urdf_2dof, "test_2dof.urdf");
    RobotPlanner rp(path);

    // 5 个测试集
    const std::vector<std::pair<double, double>> test_cases = {
        {0.0, 0.0},
        {rm::PI / 4, 0.0},
        {rm::PI / 4, rm::PI / 4},
        {rm::PI / 6, -rm::PI / 3},
        {-rm::PI / 4, rm::PI / 6},
    };

    for (const auto &[q1, q2] : test_cases) {
        msg::JointState js_fk;
        js_fk.name = {"joint1", "joint2"};
        js_fk.position = {q1, q2};
        js_fk.velocity = {0, 0};
        js_fk.effort = {0, 0};
        rp.update(js_fk);
        msg::Pose target = rp.linkpose("link2");

        msg::JointState js_zero;
        js_zero.name = {"joint1", "joint2"};
        js_zero.position = {0, 0};
        js_zero.velocity = {0, 0};
        js_zero.effort = {0, 0};
        rp.update(js_zero);

        auto traj = rp.plan("link2", target);
        // IK 的重复误差
        ASSERT_GT(traj.points.size(), 0u) << "IK failed for q1=" << q1 << " q2=" << q2;

        msg::JointState js_result;
        js_result.name = {"joint1", "joint2"};
        js_result.position = traj.points.back().positions;
        js_result.velocity = {0, 0};
        js_result.effort = {0, 0};
        rp.update(js_result);

        auto result = rp.linkpose("link2");
        EXPECT_NEAR(result.position.x, target.position.x, 1e-4) << "x mismatch for q1=" << q1 << " q2=" << q2;
        EXPECT_NEAR(result.position.y, target.position.y, 1e-4) << "y mismatch for q1=" << q1 << " q2=" << q2;
        EXPECT_NEAR(result.position.z, target.position.z, 1e-4) << "z mismatch for q1=" << q1 << " q2=" << q2;
    }
}

TEST(LPSS_robotctl, plan_multistep_pose) {
    auto path = writeTempURDF(k_urdf_2dof, "test_2dof.urdf");
    RobotPlanner rp(path);

    // 用 FK 生成三个途经点
    const std::vector<std::pair<double, double>> configs = {
        {rm::PI / 6, 0.0},
        {rm::PI / 4, rm::PI / 6},
        {rm::PI / 3, -rm::PI / 6},
    };

    std::vector<msg::Pose> waypoints;
    for (const auto &[q1, q2] : configs) {
        msg::JointState js;
        js.name = {"joint1", "joint2"};
        js.position = {q1, q2};
        js.velocity = {0, 0};
        js.effort = {0, 0};
        rp.update(js);
        waypoints.push_back(rp.linkpose("link2"));
    }

    // 从零位出发
    msg::JointState js_zero;
    js_zero.name = {"joint1", "joint2"};
    js_zero.position = {0, 0};
    js_zero.velocity = {0, 0};
    js_zero.effort = {0, 0};
    rp.update(js_zero);

    auto traj = rp.plan("link2", waypoints);
    ASSERT_GT(traj.points.size(), 0u);

    // 时间戳单调递增
    for (std::size_t i = 1; i < traj.points.size(); ++i)
        EXPECT_GE(traj.points[i].time_from_start, traj.points[i - 1].time_from_start);

    // 验证轨迹终点对应最后一个途经点的 FK 位置
    msg::JointState js_result;
    js_result.name = {"joint1", "joint2"};
    js_result.position = traj.points.back().positions;
    js_result.velocity = {0, 0};
    js_result.effort = {0, 0};
    rp.update(js_result);

    auto result = rp.linkpose("link2");
    EXPECT_NEAR(result.position.x, waypoints.back().position.x, 1e-4);
    EXPECT_NEAR(result.position.y, waypoints.back().position.y, 1e-4);
    EXPECT_NEAR(result.position.z, waypoints.back().position.z, 1e-4);
}
#endif // RMVL_LPSS_WITH_KDL

// setMaxVelocityScalingFactor / setMaxAccelerationScalingFactor
TEST(LPSS_robotctl, scaling_factor_setter_getter) {
    auto path = writeTempURDF(k_urdf_2dof, "test_2dof.urdf");
    RobotPlanner rp(path);

    // 默认值
    EXPECT_NEAR(rp.getMaxVelocityScalingFactor(), 1.0, kEps);
    EXPECT_NEAR(rp.getMaxAccelerationScalingFactor(), 1.0, kEps);

    // 正常设置
    rp.setMaxVelocityScalingFactor(0.5);
    rp.setMaxAccelerationScalingFactor(0.3);
    EXPECT_NEAR(rp.getMaxVelocityScalingFactor(), 0.5, kEps);
    EXPECT_NEAR(rp.getMaxAccelerationScalingFactor(), 0.3, kEps);

    // 超出范围被截断
    rp.setMaxVelocityScalingFactor(2.0);
    EXPECT_NEAR(rp.getMaxVelocityScalingFactor(), 1.0, kEps);
    rp.setMaxAccelerationScalingFactor(-1.0);
    EXPECT_NEAR(rp.getMaxAccelerationScalingFactor(), 0.01, kEps);
}

TEST(LPSS_robotctl, scaling_factor_affects_duration) {
    auto path = writeTempURDF(k_urdf_with_dynamics, "test_dynamics.urdf");
    RobotPlanner rp(path);

    msg::JointState target;
    target.name = {"joint1", "joint2"};
    target.position = {rm::PI / 2, rm::PI / 4};
    target.velocity = {0, 0};
    target.effort = {0, 0};

    // 全速规划
    rp.setMaxVelocityScalingFactor(1.0);
    rp.setMaxAccelerationScalingFactor(1.0);
    auto traj_fast = rp.plan(target);

    // 半速规划，轨迹时间应更长
    rp.setMaxVelocityScalingFactor(0.5);
    rp.setMaxAccelerationScalingFactor(0.5);
    auto traj_slow = rp.plan(target);

    ASSERT_GT(traj_fast.points.size(), 0u);
    ASSERT_GT(traj_slow.points.size(), 0u);
    EXPECT_GT(traj_slow.points.back().time_from_start,
              traj_fast.points.back().time_from_start);
}

} // namespace rm_test
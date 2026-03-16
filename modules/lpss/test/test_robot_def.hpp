/**
 * @file test_robot_def.hpp
 * @author Nq139 (1585062440@qq.com)
 * @brief
 * @version 0.1
 * @date 2026-03-16
 *
 * @copyright Copyright (c) 2026
 *
 */

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

// 带完整 limit 和 inertial 的 2-DOF URDF
// joint1: effort=50, velocity=2.0, 绕 Z 轴, child_link=link1, link1 izz=0.5 => a = 50/0.5 = 100
// joint2: effort=20, velocity=1.5, 绕 Z 轴, child_link=link2, link2 izz=0.2 => a = 20/0.2 = 100
constexpr const char *k_urdf_with_dynamics = R"(<?xml version="1.0"?>
<robot name="test_dynamics">
  <link name="base_link"/>

  <link name="link1">
    <inertial>
      <mass value="2.0"/>
      <origin xyz="0 0 0" rpy="0 0 0"/>
      <inertia ixx="0.1" ixy="0" ixz="0" iyy="0.1" iyz="0" izz="0.5"/>
    </inertial>
  </link>

  <link name="link2">
    <inertial>
      <mass value="1.0"/>
      <origin xyz="0 0 0" rpy="0 0 0"/>
      <inertia ixx="0.05" ixy="0" ixz="0" iyy="0.05" iyz="0" izz="0.2"/>
    </inertial>
  </link>

  <joint name="joint1" type="revolute">
    <parent link="base_link"/>
    <child link="link1"/>
    <origin xyz="0 0 0.5" rpy="0 0 0"/>
    <axis xyz="0 0 1"/>
    <limit lower="-3.14" upper="3.14" effort="50" velocity="2.0"/>
  </joint>

  <joint name="joint2" type="revolute">
    <parent link="link1"/>
    <child link="link2"/>
    <origin xyz="1.0 0 0" rpy="0 0 0"/>
    <axis xyz="0 0 1"/>
    <limit lower="-3.14" upper="3.14" effort="20" velocity="1.5"/>
  </joint>
</robot>
)";

// 无 effort 的 URDF（max_acceleration 应为默认值 10）
constexpr const char *k_urdf_no_effort = R"(<?xml version="1.0"?>
<robot name="test_no_effort">
  <link name="base_link"/>
  <link name="link1">
    <inertial>
      <mass value="1.0"/>
      <inertia ixx="0.1" ixy="0" ixz="0" iyy="0.1" iyz="0" izz="0.5"/>
    </inertial>
  </link>

  <joint name="joint1" type="revolute">
    <parent link="base_link"/>
    <child link="link1"/>
    <origin xyz="0 0 0.5" rpy="0 0 0"/>
    <axis xyz="0 0 1"/>
    <limit lower="-3.14" upper="3.14" velocity="2.0"/>
  </joint>
</robot>
)";

// 无 inertial 的 URDF（max_acceleration 应为默认值 10）
constexpr const char *k_urdf_no_inertial = R"(<?xml version="1.0"?>
<robot name="test_no_inertial">
  <link name="base_link"/>
  <link name="link1"/>

  <joint name="joint1" type="revolute">
    <parent link="base_link"/>
    <child link="link1"/>
    <origin xyz="0 0 0.5" rpy="0 0 0"/>
    <axis xyz="0 0 1"/>
    <limit lower="-3.14" upper="3.14" effort="50" velocity="2.0"/>
  </joint>
</robot>
)";

// 非对角惯性张量 + 非 Z 轴旋转的 URDF
// joint1 绕 Y 轴, effort=30, link1: ixx=0.3, iyy=0.6, izz=0.4, ixy=0.1, ixz=0.05, iyz=0.02
// I_axis = axis^T * I * axis, axis=(0,1,0) => I_axis = iyy = 0.6
// max_acceleration = 30 / 0.6 = 50
constexpr const char *k_urdf_y_axis = R"(<?xml version="1.0"?>
<robot name="test_y_axis">
  <link name="base_link"/>
  <link name="link1">
    <inertial>
      <mass value="3.0"/>
      <inertia ixx="0.3" ixy="0.1" ixz="0.05" iyy="0.6" iyz="0.02" izz="0.4"/>
    </inertial>
  </link>

  <joint name="joint1" type="revolute">
    <parent link="base_link"/>
    <child link="link1"/>
    <origin xyz="0 0 1" rpy="0 0 0"/>
    <axis xyz="0 1 0"/>
    <limit lower="-3.14" upper="3.14" effort="30" velocity="1.0"/>
  </joint>
</robot>
)";

// 移动关节 URDF: a = F / m
// effort=100, mass=5.0 => a = 100/5 = 20
constexpr const char *k_urdf_prismatic = R"(<?xml version="1.0"?>
<robot name="test_prismatic">
  <link name="base_link"/>
  <link name="slider">
    <inertial>
      <mass value="5.0"/>
      <inertia ixx="0.1" ixy="0" ixz="0" iyy="0.1" iyz="0" izz="0.1"/>
    </inertial>
  </link>

  <joint name="slide_joint" type="prismatic">
    <parent link="base_link"/>
    <child link="slider"/>
    <origin xyz="0 0 0" rpy="0 0 0"/>
    <axis xyz="1 0 0"/>
    <limit lower="0" upper="1.0" effort="100" velocity="0.5"/>
  </joint>
</robot>
)";

// 非轴对齐旋转轴的 URDF
// axis = (1/√2, 1/√2, 0), effort=40
// link1: ixx=0.2, iyy=0.8, izz=0.5, ixy=0.1, ixz=0, iyz=0
// I_axis = ixx*ax² + iyy*ay² + izz*0 + 2*(ixy*ax*ay + 0 + 0)
//        = 0.2*0.5 + 0.8*0.5 + 2*0.1*0.5 = 0.1 + 0.4 + 0.1 = 0.6
// max_acceleration = 40 / 0.6 ≈ 66.667
constexpr const char *k_urdf_diagonal_axis = R"(<?xml version="1.0"?>
<robot name="test_diag_axis">
  <link name="base_link"/>
  <link name="link1">
    <inertial>
      <mass value="2.0"/>
      <inertia ixx="0.2" ixy="0.1" ixz="0" iyy="0.8" iyz="0" izz="0.5"/>
    </inertial>
  </link>

  <joint name="joint1" type="revolute">
    <parent link="base_link"/>
    <child link="link1"/>
    <origin xyz="0 0 0" rpy="0 0 0"/>
    <axis xyz="0.7071068 0.7071068 0"/>
    <limit lower="-3.14" upper="3.14" effort="40" velocity="2.0"/>
  </joint>
</robot>
)";
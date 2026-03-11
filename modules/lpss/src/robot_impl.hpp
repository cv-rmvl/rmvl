/**
 * @file robot_impl.hpp
 * @author zhaoxi (535394140@qq.com)
 * @brief RobotPlanner::Impl 定义及运动学辅助函数
 * @version 1.0
 * @date 2026-03-04
 *
 * @copyright Copyright 2026 (c), zhaoxi
 *
 */

#pragma once

#include <cmath>
#include <unordered_map>

#include "rmvl/lpss/robot.hpp"

namespace rm::lpss {

//! 关节类型
enum class JointType : uint8_t {
    Fixed,      //!< 固定关节
    Revolute,   //!< 旋转关节
    Continuous, //!< 连续旋转关节（无限位）
    Prismatic,  //!< 移动关节
};

//! 单个关节描述
struct JointInfo {
    std::string name;                    //!< 关节名称
    JointType type{JointType::Fixed};    //!< 关节类型
    std::string parent_link;             //!< 父连杆名称
    std::string child_link;              //!< 子连杆名称
    std::array<double, 3> axis{0, 0, 1}; //!< 旋转/移动轴（在关节坐标系下）
    std::array<double, 3> origin_xyz{};  //!< 关节原点在父连杆坐标系下的平移
    std::array<double, 3> origin_rpy{};  //!< 关节原点在父连杆坐标系下的旋转（RPY）
    double lower{0}, upper{0};           //!< 关节限位
};

//! 单个连杆描述
struct LinkInfo {
    std::string name; //!< 连杆名称
};

//! URDF 运动学模型
struct URDFModel {
    std::string root_link;                                    //!< 根连杆名称
    std::vector<LinkInfo> links;                              //!< 所有连杆
    std::vector<JointInfo> joints;                            //!< 所有关节
    std::unordered_map<std::string, std::size_t> link_index;  //!< 连杆名 -> links 下标
    std::unordered_map<std::string, std::size_t> joint_index; //!< 关节名 -> joints 下标
    std::vector<std::size_t> active_joint_indices;            //!< 活动关节（非 Fixed）在 joints 中的下标

    //! [父连杆名:该连杆作为 parent 的所有关节下标列表]（用于正运动学递推）
    std::unordered_map<std::string, std::vector<std::size_t>> children_joints;

    /**
     * @brief 从 URDF 原始 XML 字符串解析运动学模型
     * @param[in] urdf_str URDF 原始 XML 字符串
     */
    void parse(std::string_view urdf_str);

    //! 清空所有模型数据
    void clear();
};

class RobotPlanner::Impl {
public:
    URDFModel model{};             //!< URDF 运动学模型
    msg::URDF urdf{};              //!< URDF 原始消息
    msg::JointState joint_state{}; //!< 当前关节状态
    msg::TF tf{};                  //!< 当前 TF 树

    //! 根据当前 joint_state 计算并更新 TF 树（正运动学）
    void updateTF();

    //! 将 RPY 角 + 平移 + 关节角组合成 TransformStamped
    static msg::TransformStamped computeJointTransform(const JointInfo &joint, double q);

    //! 初始化 joint_state 为零位
    void resetJointState();
};

} // namespace rm::lpss

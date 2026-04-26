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

#include "rmvl/lpss/robot.hpp"

#ifdef RMVL_LPSS_WITH_KDL
#include "chain.hpp"
#include "jntarray.hpp"
#endif

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
    double max_velocity{3.14};           //!< 最大关节速度 (rad/s 或 m/s)，来自 URDF `<limit velocity="...">`
    double max_effort{0};                //!< 最大关节力矩 (N·m 或 N)，来自 URDF `<limit effort="...">`
    double max_acceleration{10.0};       //!< 最大关节加速度 (rad/s² 或 m/s²)，由 effort / 转动惯量 估算，默认 10
};

//! 单个连杆描述
struct LinkInfo {
    std::string name;                                //!< 连杆名称
    double mass{0};                                  //!< 连杆质量 (kg)
    std::array<double, 6> inertia{0, 0, 0, 0, 0, 0}; //!< 惯性张量 [ixx, ixy, ixz, iyy, iyz, izz]
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

//! 将 RPY 角 + 平移 + 关节角组合成 TransformStamped
msg::TransformStamped computeJointTransform(const JointInfo &joint, double q);

/**
 * @brief 五次多项式轨迹插值
 * @param[in] q_start C 风格的指向起始关节角数组的指针
 * @param[in] q_end C 风格的指向终止关节角数组的指针
 * @param[in] n 数组长度
 * @param[in] t_start 起始时间戳（毫秒）
 * @param[in] duration 运动时长（毫秒）
 * @param[in] n_seg 插值段数
 * @return traj 轨迹
 */
std::vector<msg::JointTrajectoryPoint> interpolate(const double *q_start, const double *q_end, std::size_t n, int64_t t_start, int64_t duration, std::size_t n_seg);

/**
 * @brief 五次多项式轨迹插值
 * @param[in] q_start 起始关节角
 * @param[in] q_end 终止关节角
 * @param[in] t_start 起始时间戳（毫秒）
 * @param[in] duration 运动时长（毫秒）
 * @param[in] n_seg 插值段数
 * @return traj 轨迹
 */
inline std::vector<msg::JointTrajectoryPoint> interpolate(const std::vector<double> &q_start, const std::vector<double> &q_end, int64_t t_start, int64_t duration, std::size_t n_seg) {
    return interpolate(q_start.data(), q_end.data(), q_start.size(), t_start, duration, n_seg);
}

/**
 * @brief 根据关节速度/加速度限制自动估算运动时长
 *
 * @details 基于五次多项式 `p(s) = 10s³ - 15s⁴ + 6s⁵` 的解析峰值（速度系数 1.875，加速度系数 5.7735），
 *          对每个关节分别由速度约束和加速度约束求 T 的下界，取所有关节中最大值作为运动时长。
 *          关节的 `max_acceleration` 由 URDF 的 `effort / 子连杆转动惯量` 估算，缺失时默认 10 rad/s²。
 *          复杂度 O(n)，n 为活动关节数。
 *
 * @param[in] q_start C 风格的起始关节角数组指针
 * @param[in] q_end C 风格的终止关节角数组指针
 * @param[in] joints 活动关节信息指针列表（仅含非 Fixed 关节）
 * @param[in] velocity_scale 速度缩放因子 (0, 1]
 * @param[in] acceleration_scale 加速度缩放因子 (0, 1]
 * @return 运动时长（毫秒）
 */
int64_t estimateDuration(const double *q_start, const double *q_end, const std::vector<const JointInfo *> &joints,
                         double velocity_scale, double acceleration_scale);

/**
 * @brief 根据关节速度/加速度限制自动估算运动时长（vector 重载）
 * @see estimateDuration(const double *, const double *, const std::vector<const JointInfo *> &, double, double)
 */
inline int64_t estimateDuration(const std::vector<double> &q_start, const std::vector<double> &q_end,
                                const std::vector<const JointInfo *> &joints,
                                double velocity_scale, double acceleration_scale) {
    return estimateDuration(q_start.data(), q_end.data(), joints, velocity_scale, acceleration_scale);
}

class RobotPlanner::Impl {
public:
    URDFModel model{};             //!< URDF 运动学模型
    msg::URDF urdf{};              //!< URDF 原始消息
    msg::JointState joint_state{}; //!< 当前关节状态
    msg::TF tf{};                  //!< 当前 TF 树

    double velocity_scale{1.0};     //!< 最大速度缩放因子 (0, 1]
    double acceleration_scale{1.0}; //!< 最大加速度缩放因子 (0, 1]

    //! 根据当前 joint_state 计算并更新 TF 树（正运动学）
    void updateTF();

    //! 初始化 joint_state 为零位
    void resetJointState();

#ifdef RMVL_LPSS_WITH_KDL
    /**
     * @brief 从 URDF 模型构建 KDL::Chain
     * @param[in] frame 目标连杆名称
     * @param[out] chain 构建好的运动链
     * @param[out] path 从根连杆到目标连杆的关节路径（中间关联的连杆）
     * @return 是否成功找到路径
     */
    bool buildChain(std::string_view frame, KDL::Chain &chain, std::vector<const JointInfo *> &path) const;

    /**
     * @brief LMA 逆运动学求解
     * @param[in] chain 运动链
     * @param[in] path 关节路径
     * @param[in] target 目标位姿
     * @param[out] q_out 求解结果
     * @return 是否求解成功
     */
    bool solveIK(const KDL::Chain &chain, const std::vector<const JointInfo *> &path,
                 const msg::Pose &target, KDL::JntArray &q_out) const;
#endif // RMVL_LPSS_WITH_KDL
};

} // namespace rm::lpss

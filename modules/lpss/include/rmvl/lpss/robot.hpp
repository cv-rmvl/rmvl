/**
 * @file robot.hpp
 * @author zhaoxi (535394140@qq.com)
 * @brief LPSS 机器人相关功能扩展
 * @version 1.0
 * @date 2026-03-04
 *
 * @copyright Copyright 2026 (c), zhaoxi
 *
 */

#pragma once

#include "rmvl/lpss/node.hpp"

#include "rmvlmsg/geometry/pose.hpp"
#include "rmvlmsg/motion/joint_trajectory.hpp"
#include "rmvlmsg/motion/tf.hpp"
#include "rmvlmsg/motion/urdf.hpp"
#include "rmvlmsg/sensor/joint_state.hpp"

namespace rm {

//! @addtogroup lpss
//! @{

//! @defgroup lpss_robot 机器人功能扩展
//! @{
//! @brief 机器人运动学模型、状态发布、轨迹规划等功能扩展
//! @details 该模块提供了
//! - 机器人运动学模型规划器 lpss::RobotPlanner ，支持 URDF 解析、正/逆运动学求解、轨迹规划等功能
//! - 机器人状态发布者，通过传入 lpss::RobotPlanner 对象和 lpss::Node （或 lpss::async::Node ）节点对象周期性发布 TF 和 URDF 消息，供其他模块订阅使用
//! - 相关消息类型转换函数，如四元数乘法、变换合并等
//! @image html lpss/robotmodel.svg "机器人功能扩展模块示意图" width=75%
//! @}

namespace lpss {

//! @addtogroup lpss_robot
//! @{

/**
 * @brief 四元数乘法：q1 * q2
 *
 * @param[in] q1 msg::Quaternion 表示的四元数 1
 * @param[in] q2 msg::Quaternion 表示的四元数 2
 * @return 四元数乘积
 */
msg::Quaternion operator*(const msg::Quaternion &q1, const msg::Quaternion &q2);

/**
 * @brief 对向量执行旋转操作
 *
 * @param[in] q msg::Quaternion 表示的旋转四元数
 * @param[in] v msg::Vector3 表示的向量
 * @return msg::Vector3 表示的旋转后的向量
 */
msg::Vector3 rotate(const msg::Quaternion &q, const msg::Vector3 &v);

/**
 * @brief 合并两个变换
 * @details 具体来说符合以下规则：
 * - 在最初的坐标系下应用 t2 变换，再在原来的坐标系下应用 t1 变换，等价于在最初的坐标系下应用 t1 * t2 变换，即 “复合变换”
 * - 在最初的坐标系下应用 t1 变换，再在 t1 变换后的坐标系下应用 t2 变换，等价于在最初的坐标系下应用 t1 * t2 变换，即 “相对于坐标系的变换”
 *
 * @param[in] t1 msg::Transform 表示的变换 1
 * @param[in] t2 msg::Transform 表示的变换 2
 * @return msg::Transform 表示的合并后的变换
 */
msg::Transform operator*(const msg::Transform &t1, const msg::Transform &t2);

//! 机器人规划模块，提供 URDF 解析、正/逆运动学求解、轨迹规划等运动学功能
class RobotPlanner {
public:
    /**
     * @brief 构造机器人规划对象
     * @details 构造时以零位初始化所有关节状态，并生成对应的 TF 树
     *
     * @param[in] urdf_path 机器人 URDF 文件路径
     * @param[in] mesh_path 机器人网格文件路径前缀（可选），一般用于解析 URDF 中的 mesh 路径
     */
    RobotPlanner(std::string_view urdf_path, std::string_view mesh_path = {});

    RobotPlanner(RobotPlanner &&) noexcept;
    RobotPlanner &operator=(RobotPlanner &&) noexcept;
    ~RobotPlanner();

    /**
     * @brief 重新加载机器人模型
     * @details 解析新的 URDF 文件，重新构建运动学树，并以零位重新初始化所有关节状态和 TF 树
     *
     * @param[in] urdf_path 机器人 URDF 文件路径
     * @param[in] mesh_path 机器人网格文件路径前缀（可选），一般用于解析 URDF 中的 mesh 路径
     */
    void load(std::string_view urdf_path, std::string_view mesh_path = {});

    /**
     * @brief 更新关节状态，并根据正运动学重新计算 TF 树
     *
     * @param[in] joint_state 新的关节状态（名称、位置、速度、力矩）
     */
    void update(const msg::JointState &joint_state);

    /**
     * @brief 获取当前关节状态
     *
     * @return 当前关节状态的常引用
     */
    const msg::JointState &joints() const noexcept;

    //! 获取当前 URDF 消息
    const msg::URDF &urdf() const noexcept;

    //! 获取当前 TF 消息
    const msg::TF &tf() const noexcept;

    /**
     * @brief 关节空间点到点规划
     * @details
     * - 已知目标关节角，从当前关节角出发，用五次多项式插值生成平滑轨迹。
     * - 不需要逆运动学，适合直接指定关节角度的场景。
     *
     * @param[in] target 目标关节状态（仅使用 position 字段）
     * @return 关节轨迹，若关节数为 0 则返回空轨迹
     */
    msg::JointTrajectory plan(const msg::JointState &target) const;

    /**
     * @brief 笛卡尔空间点到点规划
     * @details
     * - 已知末端目标位姿，先用逆运动学（LMA 迭代）求出对应关节角，再从当前关节角出发做五次多项式插值。
     * - 若目标位姿超出工作空间或逆运动学不收敛，返回空轨迹。
     *
     * @param[in] frame 目标连杆名称（末端执行器所在连杆）
     * @param[in] target 目标位姿（位置 + 四元数姿态）
     * @return 关节轨迹，IK 失败或连杆不存在时返回空轨迹
     */
    msg::JointTrajectory plan(std::string_view frame, const msg::Pose &target) const;

    /**
     * @brief 笛卡尔空间多段途经点规划
     * @details
     * - 末端依次经过多个目标位姿，对每段分别做逆运动学 + 五次多项式插值，各段轨迹首尾拼接，每段根据关节速度/加速度限制自动估算时长。
     * - 若某段逆运动学失败，返回已成功规划的部分轨迹。
     *
     * @param[in] frame 目标连杆名称（末端执行器所在连杆）
     * @param[in] waypoints 途经位姿列表，按顺序依次到达
     * @return 关节轨迹，连杆不存在时返回空轨迹
     */
    msg::JointTrajectory plan(std::string_view frame, const std::vector<msg::Pose> &waypoints) const;

    /**
     * @brief 获取指定连杆相对于基坐标系的位姿（正运动学）
     *
     * @param[in] link_name 目标连杆名称
     * @return 该连杆在基坐标系下的位姿
     */
    msg::Pose linkpose(std::string_view link_name) const;

    /**
     * @brief 设置最大速度缩放因子
     * @details 轨迹规划时所有关节的最大速度将乘以该系数，值域 \f$(0,1]\f$。
     *
     * @param[in] factor 速度缩放因子，将被截断到\f$(0,1]\f$范围
     */
    void setMaxVelocityScalingFactor(double factor);

    /**
     * @brief 获取当前最大速度缩放因子
     * @return 当前速度缩放因子
     */
    double getMaxVelocityScalingFactor() const noexcept;

    /**
     * @brief 设置最大加速度缩放因子
     * @details 轨迹规划时所有关节的最大加速度将乘以该系数，值域\f$(0,1]\f$。
     *
     * @param[in] factor 加速度缩放因子，将被截断到\f$(0,1]\f$范围
     */
    void setMaxAccelerationScalingFactor(double factor);

    /**
     * @brief 获取当前最大加速度缩放因子
     * @return 当前加速度缩放因子
     */
    double getMaxAccelerationScalingFactor() const noexcept;

protected:
    class Impl;
    std::unique_ptr<Impl> _impl{};
};

/**
 * @brief 机器人状态发布者，周期性发布 TF 和 URDF 消息
 * @details 自动启动后台线程，用户需自行保证 `node` 的生命周期。
 */
class RobotStatePublisher {
public:
    /**
     * @brief 构造状态发布者
     * @details 启动后台线程，周期性发布 TF 和 URDF 数据。调用者修改 @p tf 或 @p urdf 数据时，应先通过 `mutex()` 加锁。
     *
     * @param[in] name 机器人名称，将用于构造发布主题名称前缀，如 `<name>/tf` 和 `<name>/robot_description`
     * @param[in] node LPSS 节点
     * @param[in] planner 机器人规划器对象，发布者将从中获取 TF 和 URDF 数据
     * @param[in] period TF 消息发布周期（单位：毫秒），URDF 消息发布周期将固定设置为 1s
     */
    RobotStatePublisher(std::string_view name, Node &node, RobotPlanner &planner, uint32_t period);

    ~RobotStatePublisher();

    RobotStatePublisher(const RobotStatePublisher &) = delete;
    RobotStatePublisher &operator=(const RobotStatePublisher &) = delete;

    //! 获取同步互斥锁的引用，调用者在修改 RobotPlanner 状态时需加锁
    std::mutex &mutex() noexcept { return _mtx; }

private:
    bool _running{true};
    std::reference_wrapper<Node> _node;
    std::reference_wrapper<RobotPlanner> _planner;

    Publisher<msg::URDF> _urdf_pub;
    Publisher<msg::TF> _tf_pub;

    std::thread _tf_thread;
    std::thread _urdf_thread;
    std::mutex _shutdown_mtx;
    std::condition_variable _shutdown_cv;
    std::mutex _mtx;
};

//! @} lpss_robot

#if __cplusplus >= 202002L

namespace async {

//! @addtogroup lpss_robot
//! @{

/**
 * @brief 异步机器人状态发布者，基于定时器周期性发布 TF 和 URDF 消息
 * @details 事件循环仍然由 `node` 管理，需要由用户自行调用 `node.spin()` 来驱动定时器回调的执行。
 */
class RobotStatePublisher {
public:
    /**
     * @brief 构造异步状态发布者
     * @details 基于事件循环定时器，周期性发布 TF 和 URDF 数据。
     *
     * @param[in] name 机器人名称，将用于构造发布主题名称前缀，如 `<name>/tf` 和 `<name>/robot_description`
     * @param[in] node LPSS 节点
     * @param[in] planner 机器人规划器对象，发布者将从中获取 TF 和 URDF 数据
     * @param[in] period TF 消息发布周期（单位：毫秒），URDF 消息发布周期将固定设置为 1s
     */
    RobotStatePublisher(std::string_view name, Node &node, RobotPlanner &planner, uint32_t period);

    ~RobotStatePublisher();

    RobotStatePublisher(const RobotStatePublisher &) = delete;
    RobotStatePublisher &operator=(const RobotStatePublisher &) = delete;

private:
    std::reference_wrapper<Node> _node;
    std::reference_wrapper<RobotPlanner> _planner;

    Publisher<msg::URDF>::ptr _urdf_pub{};
    Publisher<msg::TF>::ptr _tf_pub{};
    Timer::ptr _urdf_timer{};
    Timer::ptr _tf_timer{};
};

//! @} lpss_robot

} // namespace async

#endif

} // namespace lpss

//! @} lpss

} // namespace rm
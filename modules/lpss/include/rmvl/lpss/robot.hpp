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
#include "rmvlmsg/geometry/transform.hpp"
#include "rmvlmsg/sensor/joint_state.hpp"
#include "rmvlmsg/motion/joint_trajectory.hpp"
#include "rmvlmsg/motion/tf.hpp"
#include "rmvlmsg/motion/urdf.hpp"

#include "ctl/base.hpp"

namespace rm {

//! @addtogroup lpss
//! @{

//! @defgroup lpss_robot 机器人功能扩展
//! @{
//! @brief 机器人运动学模型、状态发布、轨迹规划等功能扩展
//! @details 该模块提供了
//! - 机器人运动学模型规划器 lpss::RobotPlanner ，支持 URDF 解析、正/逆运动学求解、轨迹规划等功能
//! - 机器人状态发布者，通过传入 lpss::RobotPlanner 对象和 lpss::Node （或 lpss::async::Node ）节点对象周期性发布 TF、URDF 和 JointTrajectory 消息，一般在调试时供其他模块订阅使用
//! - 相关消息类型转换函数，如四元数乘法、位姿变换、复合 SE(3) 变换等
//! <center><img src="robotmodel.png" alt="机器人功能扩展模块示意图" width="65%" /></center>
//! @}

namespace msg {

//! @addtogroup lpss_robot
//! @{

/**
 * @brief 四元数乘法：\f$q_1\times q_2\f$
 *
 * @param[in] q1 msg::Quaternion 表示的四元数 1
 * @param[in] q2 msg::Quaternion 表示的四元数 2
 * @return 四元数乘积
 */
msg::Quaternion operator*(const msg::Quaternion &q1, const msg::Quaternion &q2) noexcept;

/**
 * @brief 对向量执行旋转操作
 *
 * @param[in] q msg::Quaternion 表示的旋转四元数
 * @param[in] v msg::Vector3 表示的向量
 * @return msg::Vector3 表示的旋转后的向量
 */
msg::Vector3 rotate(const msg::Quaternion &q, const msg::Vector3 &v) noexcept;

/**
 * @brief 位姿变换
 * @details 将位姿 \f$p\f$ 从坐标系 \f$A\f$ 变换到坐标系 \f$B\f$，变换关系由 \f$T\f$ 定义，即 \f$B\f$ 相对于 \f$A\f$ 的变换。
 * @param[in] t msg::Transform 表示的变换
 * @param[in] p msg::Pose 表示的位姿
 * @return msg::Pose 表示的变换后的位姿
 */
msg::Pose operator*(const msg::Transform &t, const msg::Pose &p) noexcept;

/**
 * @brief 合并两个 SE(3) 变换
 * @details 具体来说符合以下规则：
 * - 在最初的坐标系下应用 \f$T_2\f$ 变换，再在原来的坐标系下应用 \f$T_1\f$ 变换，等价于在最初的坐标系下应用 \f$T_1T_2\f$ 变换，即 “复合变换”
 * - 在最初的坐标系下应用 \f$T_1\f$ 变换，再在 \f$T_1\f$ 变换后的坐标系下应用 \f$T_2\f$ 变换，等价于在最初的坐标系下应用 \f$T_1T_2\f$ 变换，即 “相对于坐标系的变换”
 *
 * @param[in] t1 msg::Transform 表示的变换 1
 * @param[in] t2 msg::Transform 表示的变换 2
 * @return msg::Transform 表示的合并后的变换
 */
msg::Transform operator*(const msg::Transform &t1, const msg::Transform &t2) noexcept;

//! @} lpss_robot

} // namespace msg

namespace lpss {

//! @addtogroup lpss_robot
//! @{

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
     * @param[in] target 相对于根连杆（一般是 `base_link`）的目标位姿，由 `msg::Pose` 表示
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
     * @param[in] waypoints 途经位姿列表，按顺序依次到达，每个位姿均相对于根连杆（一般是 `base_link`）
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
    void setMaxVelocityScalingFactor(double factor) noexcept;

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
    void setMaxAccelerationScalingFactor(double factor) noexcept;

    /**
     * @brief 获取当前最大加速度缩放因子
     * @return 当前加速度缩放因子
     */
    double getMaxAccelerationScalingFactor() const noexcept;

protected:
    class Impl;
    std::unique_ptr<Impl> _impl{};
};

//! 机器人控制模块
class RobotController {
public:
    /**
     * @brief 构造机器人控制器
     *
     * @param[in] joint_names 机器人关节名称列表，必须是 RobotPlanner 管理的关节集合的非空子集
     * @param[in] ctl_law 控制律对象指针，默认为单位传递函数配合位置映射
     */
    RobotController(const std::vector<std::string> &joint_names, ctl::ControlLawBase::ptr ctl_law = ctl::UnitTF::create(ctl::basic_pos_imapping, ctl::basic_pos_omapping));

    /**
     * @brief 提交待执行关节轨迹
     * @details 将自动完成轨迹校验，包括：
     * - 关节名校验：\f$S_{C}\subseteq S_{P}\f$，其中 \f$S_{C}\f$ 是控制器管理的关节集合，\f$S_{P}\f$ 是轨迹中出现的关节集合
     * - 时间轴校验：`points[i].time_from_start` 必须严格递增
     * - 维度校验：每个轨迹点的位置/速度/加速度向量长度与关节数量一致（字段为空则按策略补全）
     *
     * @param[in] traj 输入关节轨迹
     * @return 是否成功提交，失败时一般是由于轨迹校验未通过
     */
    bool submit(const msg::JointTrajectory &traj);

    //! 重置控制器状态，清空轨迹缓存，用于设置新轨迹前的状态清理
    void reset() noexcept;

    /**
     * @brief 采样并得到控制系统的输入值
     *
     * @param[in] feedback 当前反馈状态，具体是否需要传入由控制律实现决定
     * @return msg::JointState
     */
    msg::JointState sample(const msg::JointState &feedback = {}) noexcept;

private:
    //! 受控关节信息结构体
    struct ControlledJointInfo {
        std::string name;  //!< 关节名称
        std::size_t remap; //!< 轨迹关节名到受控关节名的索引映射
    };

    std::vector<ControlledJointInfo> _ctl_joints{}; //!< 受控关节
    msg::JointTrajectory _traj_cache{};             //!< 轨迹缓存
    ctl::ControlLawBase::ptr _ctl_law{};            //!< 控制律对象
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

    /**
     * @brief 更新轨迹显示，发布 JointTrajectory 消息以供可视化工具订阅
     *
     * @param[in] traj 需要发布的关节轨迹消息
     */
    void updateTrajectory(msg::JointTrajectory &&traj) noexcept;

    /**
     * @brief 更新轨迹显示，发布 JointTrajectory 消息以供可视化工具订阅
     *
     * @param[in] traj 需要发布的关节轨迹消息
     */
    void updateTrajectory(const msg::JointTrajectory &traj) noexcept;

    //! 获取同步互斥锁的引用，调用者在修改 RobotPlanner 状态时需加锁
    inline std::mutex &mutex() noexcept { return _mtx; }

private:
    bool _running{true};
    std::reference_wrapper<Node> _node;
    std::reference_wrapper<RobotPlanner> _planner;

    msg::JointTrajectory _traj_cache{}; //!< 轨迹缓存

    Publisher<msg::URDF> _urdf_pub;
    Publisher<msg::TF> _tf_pub;
    Publisher<msg::JointTrajectory> _traj_pub;

    std::mutex _shutdown_mtx;
    std::condition_variable _shutdown_cv;

    std::thread _tf_thread;
    std::thread _urdf_thread;
    std::thread _traj_thread;
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
    using ptr = std::shared_ptr<RobotStatePublisher>;

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

    /**
     * @brief 更新轨迹显示，发布 JointTrajectory 消息以供可视化工具订阅
     *
     * @param[in] traj 需要发布的关节轨迹消息
     */
    void updateTrajectory(msg::JointTrajectory &&traj) noexcept { _traj_cache = std::move(traj); }

    /**
     * @brief 更新轨迹显示，发布 JointTrajectory 消息以供可视化工具订阅
     *
     * @param[in] traj 需要发布的关节轨迹消息
     */
    void updateTrajectory(const msg::JointTrajectory &traj) noexcept { _traj_cache = traj; }

    /**
     * @brief 构造异步状态发布者共享指针
     * @see RobotStatePublisher::RobotStatePublisher
     *
     * @param[in] name 机器人名称，将用于构造发布主题名称前缀，如 `<name>/tf` 和 `<name>/robot_description`
     * @param[in] node LPSS 节点
     * @param[in] planner 机器人规划器对象，发布者将从中获取 TF 和 URDF 数据
     * @param[in] period TF 消息发布周期（单位：毫秒），URDF、Trajectory 消息发布周期将固定设置为 1s
     * @return RobotStatePublisher 共享指针
     */
    static inline ptr create(std::string_view name, Node &node, RobotPlanner &planner, uint32_t period) {
        return std::make_shared<RobotStatePublisher>(name, node, planner, period);
    }

private:
    std::reference_wrapper<Node> _node;
    std::reference_wrapper<RobotPlanner> _planner;

    msg::JointTrajectory _traj_cache{}; //!< 轨迹缓存

    Publisher<msg::URDF>::ptr _urdf_pub{};
    Publisher<msg::TF>::ptr _tf_pub{};
    Publisher<msg::JointTrajectory>::ptr _traj_pub{};
    Timer::ptr _low_timer{};
    Timer::ptr _high_timer{};
};

//! @} lpss_robot

} // namespace async

#endif

} // namespace lpss

//! @} lpss

} // namespace rm
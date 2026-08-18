/**
 * @file controller.hpp
 * @author zhaoxi (535394140@qq.com)
 * @brief 二维路径跟踪与碰撞刹停
 * @version 1.0
 * @date 2026-08-17
 *
 * @copyright Copyright 2026 (c), zhaoxi
 *
 */

#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

#include "rmvl/nav/map.hpp"
#include "rmvlmsg/geometry/twist.hpp"
#include "rmvlmsg/nav/path.hpp"

namespace rm::nav {

//! @defgroup nav_controller 二维路径跟踪与安全控制
//! @ingroup nav
//! @{
//! @brief 提供 Pure Pursuit 路径跟踪和独立的预测碰撞刹停过滤器

//! 路径跟踪状态
enum class TrackingStatus : uint8_t {
    Ok,             //!< 已生成有效控制指令
    GoalReached,    //!< 已进入目标容差，控制指令为零
    InvalidOptions, //!< 控制器配置无效
    InvalidPose,    //!< 当前位姿无效
    InvalidPath,    //!< 路径为空、坐标无效或坐标系不一致
};

/**
 * @brief 获取路径跟踪状态的稳定文本描述
 *
 * @param[in] status 路径跟踪状态
 * @return 指向静态文本的字符串指针
 */
const char *to_string(TrackingStatus status) noexcept;

//! Pure Pursuit 配置
struct PurePursuitOptions {
    double lookahead_distance{0.6};        //!< 前视距离，单位为米
    double target_speed{0.5};              //!< 目标线速度，单位为米每秒
    double max_angular_speed{1.5};         //!< 最大角速度绝对值，单位为弧度每秒
    double goal_tolerance{0.10};           //!< 目标位置容差，单位为米
    double slowdown_distance{1.0};         //!< 接近终点时开始线性降速的距离，单位为米
    double rotate_to_path_threshold{1.05}; //!< 超过该航向误差时原地转向，单位为弧度
    double heading_gain{2.0};              //!< 原地转向的比例增益
};

//! Pure Pursuit 计算结果
struct TrackingResult {
    TrackingStatus status{TrackingStatus::InvalidPath}; //!< 跟踪状态
    msg::Twist command{};                               //!< 平面速度指令
    msg::Point lookahead{};                             //!< 本次使用的前视点
    std::size_t lookahead_index{};                      //!< 前视点在路径中的索引

    //! @return 是否得到有效结果，GoalReached 也视为正常结果
    explicit operator bool() const noexcept {
        return status == TrackingStatus::Ok || status == TrackingStatus::GoalReached;
    }
};

/**
 * @brief Pure Pursuit 二维路径跟踪器
 * @note 当前位姿与路径必须位于同一坐标系；本类不执行 TF 查询。
 */
class PurePursuit {
public:
    /**
     * @brief 创建 Pure Pursuit 跟踪器
     *
     * @param[in] options 跟踪配置
     */
    explicit PurePursuit(PurePursuitOptions options = {}) noexcept;

    //! @return 当前跟踪配置
    const PurePursuitOptions &options() const noexcept;

    /**
     * @brief 根据当前位置和全局路径计算速度指令
     *
     * @param[in] pose 机器人在路径坐标系中的当前位姿
     * @param[in] path 待跟踪路径
     * @return 路径跟踪结果
     */
    TrackingResult compute(const msg::Pose &pose, const msg::Path &path) const;

private:
    PurePursuitOptions _options{};
};

//! 预测碰撞刹停配置
struct CollisionStopOptions {
    double prediction_horizon{1.0}; //!< 指令轨迹预测时间，单位为秒
    double linear_step{0.05};       //!< 相邻碰撞检测位姿的最大平移距离，单位为米
    double angular_step{0.10};      //!< 相邻碰撞检测位姿的最大旋转角度，单位为弧度
};

//! 碰撞刹停过滤结果
struct CollisionStopResult {
    msg::Twist command{}; //!< 安全时保留输入指令，存在风险时为零
    bool stopped{};       //!< 是否因无效输入或预测碰撞而刹停
};

/**
 * @brief 独立的短时预测碰撞刹停过滤器
 * @details 按输入速度积分平面运动轨迹，并使用 Costmap::collides() 检查每个采样位姿。
 * 任何配置、位姿、指令或 footprint 无效时均采用故障安全策略刹停。
 */
class CollisionStop {
public:
    /**
     * @brief 创建碰撞刹停过滤器
     *
     * @param[in] options 预测与采样配置
     */
    explicit CollisionStop(CollisionStopOptions options = {}) noexcept;

    //! @return 当前刹停过滤配置
    const CollisionStopOptions &options() const noexcept;

    /**
     * @brief 对速度指令执行预测碰撞过滤
     *
     * @param[in] costmap 已更新的代价地图
     * @param[in] footprint 机器人局部坐标系中的闭合多边形顶点，无需重复首顶点
     * @param[in] pose 机器人在代价地图坐标系中的当前位姿
     * @param[in] command 待过滤的平面速度指令
     * @return 过滤结果
     */
    CollisionStopResult filter(const Costmap &costmap, const std::vector<msg::Point> &footprint,
                               const msg::Pose &pose, const msg::Twist &command) const;

private:
    CollisionStopOptions _options{};
};

//! @} nav_controller

} // namespace rm::nav

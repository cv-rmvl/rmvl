/**
 * @file gyro_tracker.h
 * @author RoboMaster Vision Community
 * @brief 装甲板追踪器头文件
 * @version 0.1
 * @date 2022-12-10
 *
 * @copyright Copyright 2022 (c), RoboMaster Vision Community
 *
 */

#pragma once

#include "rmvl/combo/armor.h"
#include "rmvl/core/kalman.hpp"

#include "tracker.h"

namespace rm
{

//! @addtogroup gyro_tracker
//! @{

//! 整车状态追踪器
class GyroTracker final : public tracker
{
public:
    //! 消失状态
    enum VanishState : uint8_t
    {
        VANISH = 0U, //!< 丢失
        APPEAR = 1U  //!< 出现
    };

private:
    float _duration{}; //!< 采样帧差时间
    cv::Vec2f _pose;   //!< 修正后的装甲板姿态法向量
    float _rotspeed{}; //!< 绕 y 轴自转角速度（俯视顺时针为正，滤波数据，弧度）

    KF63f _center3d_filter; //!< 位置滤波器
    KF42f _pose_filter;     //!< 姿态滤波器

    std::deque<RobotType> _type_deque; //!< 装甲板状态队列（数字）

public:
    using ptr = std::shared_ptr<GyroTracker>;
    using const_ptr = std::shared_ptr<const GyroTracker>;

    GyroTracker() = delete;

    //! 初始化追踪器
    explicit GyroTracker(combo::ptr p_armor);

    /**
     * @brief 构建 GyroTracker
     *
     * @param[in] p_armor 第一帧装甲（不允许为空）
     */
    static inline GyroTracker::ptr make_tracker(combo::ptr p_armor) { return std::make_shared<GyroTracker>(p_armor); }

    /**
     * @brief 动态类型转换
     *
     * @param[in] p_tracker tracker::ptr 抽象指针
     * @return 派生对象指针
     */
    static inline GyroTracker::ptr cast(tracker::ptr p_tracker) { return std::dynamic_pointer_cast<GyroTracker>(p_tracker); }

    /**
     * @brief 动态类型转换
     *
     * @param[in] p_tracker tracker::const_ptr 抽象指针
     * @return 派生对象指针
     */
    static inline GyroTracker::const_ptr cast(tracker::const_ptr p_tracker) { return std::dynamic_pointer_cast<const GyroTracker>(p_tracker); }

    [[deprecated]] void update(double, const GyroData &) override {};

    /**
     * @brief 使用捕获的 `combo` 更新平面目标追踪器
     *
     * @param[in] p_combo 待传入 tracker 的平面目标，必须严格保证不为空
     */
    void update(combo::ptr p_combo) override;

    /**
     * @brief 更新消失状态
     *
     * @param[in] state 消失状态
     */
    inline void updateVanishState(VanishState state) { state == VANISH ? _vanish_num++ : _vanish_num = 0; }

    //! 获取帧差时间
    inline float getDuration() const { return _duration; }
    //! 获取修正后的装甲板姿态法向量
    inline const cv::Vec2f &getPose() const { return _pose; }
    //! 获取绕 y 轴的自转角速度（俯视顺时针为正，滤波数据，弧度）
    inline float getRotatedSpeed() const { return _rotspeed; }

private:
    /**
     * @brief 从 combo 中更新数据
     *
     * @param[in] p_combo Armor::ptr 共享指针
     */
    void updateFromCombo(combo::ptr p_combo);

    //! 初始化 tracker 的距离和运动滤波器
    void initFilter();

    /**
     * @brief 更新装甲板类型
     *
     * @param[in] stat 类型
     */
    void updateType(RMStatus stat);

    //! 更新位置滤波器
    void updatePositionFilter();

    //! 更新姿态滤波器
    void updatePoseFilter();

    /**
     * @brief 解算单个追踪器的角速度
     *
     * @return 角速度（俯视图逆时针为正）
     */
    float calcRotationSpeed();
};

//! @} gyro_tracker

} // namespace rm

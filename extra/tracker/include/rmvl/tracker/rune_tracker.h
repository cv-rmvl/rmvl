/**
 * @file rune_tracker.h
 * @author RoboMaster Vision Community
 * @brief 神符追踪器头文件
 * @version 1.0
 * @date 2021-09-21
 *
 * @copyright Copyright 2021 (c), RoboMaster Vision Community
 *
 */

#pragma once

#include <deque>

#include "rmvl/combo/rune.h"
#include "rmvl/core/kalman.hpp"
#include "tracker.h"

namespace rm
{

//! @addtogroup rune_tracker
//! @{

/**
 * @brief 神符时间序列
 * @note
 * - `getCenter()` 获取神符组合体修正的中心点，要获取神符旋转中心请访问对应特征
 * - `getAngle()` 考虑圈数的角度，即角度被映射至 \f$(-\infty,+\infty)\f$ 的范围，且不经过滤波
 */
class RuneTracker final : public tracker
{
    int _round{};              //!< 圈数
    float _rotated_speed;      //!< 神符旋转角速度
    std::deque<float> _angles; //!< 角度容器
    KF22f _filter;             //!< 神符的角度滤波器

public:
    using ptr = std::shared_ptr<RuneTracker>;
    using const_ptr = std::shared_ptr<const RuneTracker>;

    RuneTracker() = delete;

    /**
     * @brief 初始化 RuneTracker
     *
     * @param[in] p_rune 第一帧神符
     */
    explicit RuneTracker(combo::ptr p_rune);

    /**
     * @brief 构建 RuneTracker
     *
     * @param[in] p_rune 第一帧神符模块组合特征（不允许为空）
     */
    static inline RuneTracker::ptr make_tracker(combo::ptr p_rune) { return std::make_shared<RuneTracker>(p_rune); }

    /**
     * @brief 动态类型转换
     *
     * @param[in] p_tracker tracker::ptr 抽象指针
     * @return 派生对象指针
     */
    static inline RuneTracker::ptr cast(tracker::ptr p_tracker) { return std::dynamic_pointer_cast<RuneTracker>(p_tracker); }

    /**
     * @brief 动态类型转换
     *
     * @param[in] p_tracker tracker::const_ptr 抽象指针
     * @return 派生对象指针
     */
    static inline RuneTracker::const_ptr cast(tracker::const_ptr p_tracker) { return std::dynamic_pointer_cast<const RuneTracker>(p_tracker); }

    /**
     * @brief 更新时间序列
     *
     * @param[in] p_rune 神符共享指针
     * @param[in] tick 当前时间点
     * @param[in] gyro_data 云台数据
     */
    void update(combo::ptr p_rune, double tick, const GyroData &gyro_data) override;

    /**
     * @brief 滤波器初始化
     *
     * @param[in] init_angle 初始角度
     * @param[in] init_speed 初始速度
     */
    void initFilter(float init_angle = 0.f, float init_speed = 0.f);

    //! 获取神符滤波后的角速度（角度制）
    inline float getRotatedSpeed() { return _rotated_speed; }

private:
    /**
     * @brief 从 rune 中更新数据
     *
     * @param[in] p_combo 神符组合体
     */
    void updateFromRune(combo::ptr p_combo);

    /**
     * @brief 更新神符转动的圈数，并计算在考虑圈数时的完全值
     *
     * @return 考虑圈数的角度
     */
    float calculateTotalAngle();

    /**
     * @brief 更新角度滤波器
     *
     * @param[in] t 帧差时间
     */
    void updateRotateFilter(float t);

    /**
     * @brief 掉帧处理，更新滤波、估计后的考虑圈数的角度以及神符特征
     *
     * @param[in] tick 最新时间点
     * @param[in] gyro 最新陀螺仪数据
     */
    void vanishProcess(double tick, const GyroData &gyro_data);
};

//! @} rune_tracker

} // namespace rm

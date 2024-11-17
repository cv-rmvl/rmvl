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

#include "rmvl/algorithm/kalman.hpp"
#include "rmvl/combo/rune.h"
#include "tracker.h"

namespace rm
{

//! @addtogroup rune_tracker
//! @{

/**
 * @brief 神符时间序列
 * @note
 * - `center()` 获取神符组合体修正的中心点，要获取神符旋转中心请访问对应特征
 * - `angle()` 考虑圈数的角度，即角度被映射至 \f$(-\infty,+\infty)\f$ 的范围，且不经过滤波
 */
class RMVL_EXPORTS_W_DES RuneTracker final : public tracker
{
    int _round{};         //!< 圈数
    float _rotated_speed; //!< 神符旋转角速度
    KF21f _filter;        //!< 神符的角度滤波器

public:
    using ptr = std::shared_ptr<RuneTracker>;
    using const_ptr = std::shared_ptr<const RuneTracker>;

    //! @cond
    RuneTracker() = delete;
    explicit RuneTracker(combo::ptr p_rune);
    //! @endcond

    /**
     * @brief 构建 RuneTracker
     *
     * @param[in] p_rune 第一帧神符模块组合特征（不允许为空）
     */
    RMVL_W static inline ptr make_tracker(combo::ptr p_rune) { return std::make_shared<RuneTracker>(p_rune); }

    /**
     * @brief 从另一个追踪器进行构造
     *
     * @return 指向新追踪器的共享指针
     */
    RMVL_W tracker::ptr clone() override;

    RMVL_TRACKER_CAST(RuneTracker)

    /**
     * @brief 使用捕获的 `rm::Rune` 组合体更新追踪器
     *
     * @param[in] p_rune 神符共享指针
     */
    RMVL_W void update(combo::ptr p_rune) override;

    /**
     * @brief `rm::Rune` 目标丢失，使用时间点和 IMU 数据更新追踪器
     *
     * @param[in] tick 当前时间点
     * @param[in] imu_data 云台数据
     */
    RMVL_W void update(double tick, const ImuData &imu_data) override;

    //! 判断追踪器是否无效
    RMVL_W bool invalid() const override;

    //! 获取神符滤波后的角速度（角度制）
    RMVL_W inline float getRotatedSpeed() { return _rotated_speed; }

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
};

//! @} rune_tracker

} // namespace rm

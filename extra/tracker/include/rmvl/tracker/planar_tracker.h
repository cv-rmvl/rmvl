/**
 * @file planar_tracker.h
 * @author RoboMaster Vision Community
 * @brief 平面目标追踪器头文件
 * @version 1.0
 * @date 2021-08-21
 *
 * @copyright Copyright 2021 (c), RoboMaster Vision Community
 *
 */

#pragma once

#include "rmvl/core/kalman.hpp"
#include "rmvl/combo/armor.h"

#include "tracker.h"

namespace rm
{

//! @addtogroup planar_tracker
//! @{

//! 平面目标追踪器
class PlanarTracker final : public tracker
{
private:
    KF21f _distance_filter;             //!< 距离滤波器
    KF42f _motion_filter;               //!< 运动滤波器
    float _last_distance = 0;           //!< 目标上一帧的距离
    std::deque<float> _relative_speeds; //!< 图像速度的容器
    std::deque<RMStatus> _type_deque;   //!< 状态队列

public:
    using ptr = std::shared_ptr<PlanarTracker>;
    using const_ptr = std::shared_ptr<const PlanarTracker>;

    /**
     * @brief 构造，并初始化追踪器
     *
     * @param[in] p_combo 第一帧组合体
     */
    explicit PlanarTracker(combo::ptr p_combo);

    /**
     * @brief 构建 PlanarTracker
     *
     * @param[in] p_combo 第一帧平面目标组合特征（不允许为空）
     */
    static inline PlanarTracker::ptr make_tracker(combo::ptr p_combo) { return std::make_shared<PlanarTracker>(p_combo); }

    /**
     * @brief 丢失目标时，使用时间点和陀螺仪数据更新平面目标追踪器
     *
     * @param[in] tick 时间点
     * @param[in] gyro_data 陀螺仪数据
     */
    void update(double tick, const GyroData &gyro_data) override;

    /**
     * @brief 使用捕获的 `combo` 更新平面目标追踪器
     *
     * @param[in] p_combo 待传入 tracker 的平面目标，必须严格保证不为空
     */
    void update(combo::ptr p_combo) override;

private:
    /**
     * @brief 将 combo 中的数据更新至 tracker
     *
     * @param[in] p_combo combo::ptr 指针
     */
    void updateData(combo::ptr p_combo);

    //! 初始化 tracker 的距离和运动滤波器
    void initFilter();

    /**
     * @brief 更新状态类型
     *
     * @param[in] stat 类型
     */
    void updateType(RMStatus stat);

    /**
     * @brief 更新距离滤波器
     */
    void updateDistanceFilter();

    /**
     * @brief 更新运动滤波器
     * @note 将图像相对速度和陀螺仪速度融合后再做滤波的好处是，
     *       可以一定程度上减少时序不精准的问题
     * @note 帧差时间 t: (若只有一帧则取默认采样时间，否则取平均数值)
     */
    void updateMotionFilter();

    /**
     * @brief 掉帧处理（使用滤波器的预测数值补帧）同时更新整个 tracker 提供给外部的接口信息
     *
     * @param[in] tick 最新时间点
     * @param[in] gyro 最新陀螺仪数据
     */
    void vanishProcess(double tick, const GyroData &gyro);
};

//! @} planar_tracker

} // namespace rm

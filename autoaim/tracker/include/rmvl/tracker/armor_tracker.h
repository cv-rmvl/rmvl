/**
 * @file armor_tracker.h
 * @author RoboMaster Vision Community
 * @brief 装甲板追踪器头文件
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

//! @addtogroup armor_tracker
//! @{

//! 装甲模块追踪器
class ArmorTracker final : public tracker
{
private:
    KF22f _distance_filter;             //!< 距离滤波器
    KF44f _motion_filter;               //!< 运动滤波器
    float _last_distance = 0;           //!< 目标上一帧的距离
    std::deque<float> _relative_speeds; //!< 图像速度的容器
    std::deque<RobotType> _type_deque;  //!< 装甲板状态队列（数字）

public:
    /**
     * @brief 构造，并初始化追踪器
     *
     * @param[in] p_armor 第一帧组合体
     */
    explicit ArmorTracker(const combo_ptr &p_armor);

    /**
     * @brief 构建 ArmorTracker
     *
     * @param[in] p_armor 第一帧装甲模块组合特征（不允许为空）
     */
    static inline std::shared_ptr<ArmorTracker> make_tracker(const combo_ptr &p_armor) { return std::make_shared<ArmorTracker>(p_armor); }

    /**
     * @brief 更新时间序列
     *
     * @param[in] p_armor 传入tracker的装甲
     * @param[in] time 时间戳
     * @param[in] gyro_data 云台数据
     */
    void update(combo_ptr p_armor, int64 time, const GyroData &gyro_data) override;

private:
    /**
     * @brief 将 combo 中的数据更新至 tracker
     *
     * @param p_combo armor_ptr 指针
     */
    void updateData(const combo_ptr &p_combo);

    //! 初始化 tracker 的距离和运动滤波器
    void initFilter();

    /**
     * @brief 更新装甲板类型
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
     * @param[in] tick 最新时间戳
     * @param[in] gyro 最新陀螺仪数据
     */
    void vanishProcess(int64_t tick, const GyroData &gyro);
};

//! 装甲模块追踪器共享指针
using armor_tracker_ptr = std::shared_ptr<ArmorTracker>;

//! @} armor_tracker

} // namespace rm

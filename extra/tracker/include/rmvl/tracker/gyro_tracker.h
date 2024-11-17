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

#include "rmvl/algorithm/kalman.hpp"
#include "rmvl/combo/armor.h"

#include "tracker.h"

namespace rm
{

//! @addtogroup gyro_tracker
//! @{

//! 整车状态追踪器
class RMVL_EXPORTS_W_DES GyroTracker final : public tracker
{
public:
    using ptr = std::shared_ptr<GyroTracker>;
    using const_ptr = std::shared_ptr<const GyroTracker>;

    //! @cond
    GyroTracker() = delete;
    explicit GyroTracker(combo::ptr p_armor);
    //! @endcond

    /**
     * @brief 构建 GyroTracker
     *
     * @param[in] p_armor 第一帧装甲（不允许为空）
     */
    RMVL_W static inline ptr make_tracker(combo::ptr p_armor) { return std::make_shared<GyroTracker>(p_armor); }

    /**
     * @brief 从另一个追踪器进行构造
     *
     * @return 指向新追踪器的共享指针
     */
    RMVL_W tracker::ptr clone() override;

    RMVL_TRACKER_CAST(GyroTracker)

    [[deprecated]] void update(double, const ImuData &) override {};

    /**
     * @brief 使用捕获的 `combo` 更新平面目标追踪器
     *
     * @param[in] p_combo 待传入 tracker 的平面目标，必须严格保证不为空
     */
    RMVL_W void update(combo::ptr p_combo) override;

    /**
     * @brief 更新消失状态
     *
     * @param[in] is_vanish 是否消失
     */
    RMVL_W inline void updateVanishState(bool is_vanish) { is_vanish ? _vanish_num++ : _vanish_num = 0; }

    //! 获取帧差时间
    RMVL_W inline float getDuration() const { return _duration; }
    //! 获取修正后的装甲板姿态法向量
    RMVL_W inline const cv::Vec2f &getPose() const { return _pose; }
    //! 获取绕 y 轴的自转角速度（俯视顺时针为正，滤波数据，弧度）
    RMVL_W inline float getRotatedSpeed() const { return _rotspeed; }

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

    float _duration{}; //!< 采样帧差时间
    cv::Vec2f _pose;   //!< 修正后的装甲板姿态法向量
    float _rotspeed{}; //!< 绕 y 轴自转角速度（俯视顺时针为正，滤波数据，弧度）

    KF63f _center3d_filter; //!< 位置滤波器
    KF42f _pose_filter;     //!< 姿态滤波器

    std::deque<RobotType> _type_deque; //!< 装甲板状态队列（数字）
};

//! @} gyro_tracker

} // namespace rm

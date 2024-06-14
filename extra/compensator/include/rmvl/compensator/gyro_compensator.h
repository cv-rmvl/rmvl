/**
 * @file gyro_compensator.h
 * @author RoboMaster Vision Community
 * @brief 整车状态补偿模块接口声明
 * @version 1.0
 * @date 2021-08-28
 *
 * @copyright Copyright 2021 (c), RoboMaster Vision Community
 *
 */

#pragma once

#include "compensator.h"

namespace rm
{

//! @addtogroup gyro_compensator
//! @{

//! 整车状态补偿模块
class GyroCompensator final : public compensator
{
    float _yaw_static_com;   //!< yaw 轴静态补偿，方向与 yaw 一致
    float _pitch_static_com; //!< pitch 轴静态补偿，方向与 pitch 一致

public:
    //! 创建 GyroCompensator 对象
    GyroCompensator();

    //! 使用静态工厂函数创建 GyroCompensator 对象
    static inline auto make_compensator() { return std::make_unique<GyroCompensator>(); }

    /**
     * @brief 补偿函数，未考虑空气阻力，仅使用抛物线模型 \cite icra2019
     *
     * @param[in] groups 所有序列组
     * @param[in] shoot_speed 子弹射速 (m/s)
     * @param[in] com_flag 手动调节补偿标志
     * @return 补偿模块信息
     */
    CompensateInfo compensate(const std::vector<group::ptr> &groups, float shoot_speed, CompensateType com_flag) override;
};

//! @} compensator

} // namespace rm

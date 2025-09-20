/**
 * @file gravity_impl.h
 * @author zhaoxi (535394140@qq.com)
 * @brief
 * @version 1.0
 * @date 2024-03-03
 *
 * @copyright Copyright 2024 (c), zhaoxi
 *
 */

#pragma once

#include "rmvl/compensator/gravity_compensator.h"

#include "rmvl/algorithm/numcal.hpp"

namespace rm {

class GravityCompensator::Impl {
    float _yaw_static_com;   //!< yaw 轴静态补偿，方向与 yaw 一致
    float _pitch_static_com; //!< pitch 轴静态补偿，方向与 pitch 一致

public:
    Impl();

    //! 补偿函数，考虑空气阻力，使用 2 阶龙格库塔方法（中点公式）计算弹道
    CompensateInfo compensate(const std::vector<group::ptr> &groups, float shoot_speed, CompensateType com_flag);

private:
    /**
     * @brief 弹道模型
     *
     * @param[in] x 目标离相机的水平距离，单位 `m`
     * @param[in] v 枪口射速，单位 `m/s`
     * @param[in] angle 当前枪口仰角、俯角，`+` 表示仰角，`-` 表示俯角，单位 `rad`
     * @return 弹丸飞行至水平距离`x`时，落点的垂直距离即对应的子弹飞行时间
     * @note 返回值第一项是落点高度，为正表示落点在枪口上方，为负表示落点在枪口下方，单位 `m`
     */
    std::pair<double, double> bulletModel(double x, double v, double angle);

    /**
     * @brief 更新静态补偿
     *
     * @param[in] com_flag 手动调节补偿标志
     * @param[out] x_st 水平补偿
     * @param[out] y_st 垂直补偿
     */
    void updateStaticCom(CompensateType com_flag, float &x_st, float &y_st);

    /**
     * @brief 计算补偿角度以及子弹飞行时间
     * @note
     * - 需要严格满足相机对水平方向的夹角等于 `gyro_angle.y`
     *
     * @param[in] x 目标离相机的水平宽度
     * @param[in] y 目标离相机的铅垂高度
     * @param[in] velocity 枪口射速
     *
     * @return 补偿角度、子弹飞行时间的二元组
     */
    std::pair<double, double> calc(double x, double y, double velocity);

    std::unique_ptr<RungeKutta2> _rk; //!< 2 阶龙格库塔求解器
};

} // namespace rm

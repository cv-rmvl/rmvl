/**
 * @file gravity_compensator.h
 * @author RoboMaster Vision Community
 * @brief 重力模型补偿头文件
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

//! @addtogroup gravity_compensator
//! @{

//! 使用重力模型的补偿模块
class GravityCompensator final : public compensator
{
public:
    GravityCompensator() noexcept;
    ~GravityCompensator();

    //! 构造 GravityCompensator
    static inline auto make_compensator() { return std::make_unique<GravityCompensator>(); }

    /**
     * @brief 补偿函数，考虑空气阻力，使用 2 阶龙格库塔方法（中点公式）计算弹道
     *
     * @param[in] groups 所有序列组
     * @param[in] shoot_speed 子弹射速 (m/s)
     * @param[in] com_flag 手动调节补偿标志
     * @return 补偿模块信息
     */
    CompensateInfo compensate(const std::vector<group::ptr> &groups, float shoot_speed, CompensateType com_flag) override;

private:
    class Impl;
    Impl *_impl;
};

//! @} compensator

} // namespace rm

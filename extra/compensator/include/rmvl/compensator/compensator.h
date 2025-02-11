/**
 * @file compensator.h
 * @author RoboMaster Vision Community
 * @brief 抽象补偿类头文件
 * @version 1.0
 * @date 2021-08-24
 *
 * @copyright Copyright 2021 (c), RoboMaster Vision Community
 *
 */

#pragma once

#include <unordered_map>

#include "rmvl/group/group.h"

namespace rm
{

//! @addtogroup compensator
//! @{

/**
 * @brief 补偿模块信息
 * @note
 * - 作为弹道下坠补偿模块接口的返回值
 */
struct CompensateInfo
{
    /**
     * @brief 追踪器 - 补偿映射表
     * @note 记载每一个追踪器对应的补偿值，`x` 表示水平方向补偿增量，`y`
     *       表示垂直方向补偿增量
     */
    std::unordered_map<tracker::ptr, cv::Point2f> compensation;

    /**
     * @brief 飞行时间 (Time of Flying)
     * @note 记载每一个追踪器对应的飞行时间，表示弹丸击中目标所需要的子弹飞行时间
     */
    std::unordered_map<tracker::ptr, double> tof;
};

//! 强制补偿类型
enum class CompensateType : uint8_t
{
    UNKNOWN, //!< 未知
    UP,      //!< 向上强制补偿
    DOWN,    //!< 向下强制补偿
    LEFT,    //!< 向左强制补偿
    RIGHT,   //!< 向右强制补偿
};

//! 弹道下坠补偿模块
class compensator
{
public:
    using ptr = std::unique_ptr<compensator>;

    compensator() = default;
    virtual ~compensator() = default;

    /**
     * @brief 补偿核心函数，（补偿角度方向与 gyro_angle 方向一致）
     *
     * @param[in] groups 所有序列组
     * @param[in] shoot_speed 子弹射速 (m/s)
     * @param[in] com_flag 手动调节补偿标志
     * @return 补偿模块信息
     */
    virtual CompensateInfo compensate(const std::vector<group::ptr> &groups, float shoot_speed, CompensateType com_flag) = 0;
};

//! @} compensator

} // namespace rm

/**
 * @file common.hpp
 * @author zhaoxi (535394140@qq.com)
 * @brief 补偿模块公共头文件
 * @version 1.0
 * @date 2026-02-04
 *
 * @copyright Copyright 2026 (c), zhaoxi
 *
 */

#pragma once

#include "rmvl/tracker/tracker.h"

namespace rm {

/**
 * @brief 补偿模块信息
 * @note
 * - 作为弹道下坠补偿模块接口的返回值
 */
struct CompensateInfo {
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
enum class CompensateType : uint8_t {
    UNKNOWN, //!< 未知
    UP,      //!< 向上强制补偿
    DOWN,    //!< 向下强制补偿
    LEFT,    //!< 向左强制补偿
    RIGHT,   //!< 向右强制补偿
};

} // namespace rm
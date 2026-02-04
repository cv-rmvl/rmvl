/**
 * @file rune.hpp
 * @author zhaoxi (535394140@qq.com)
 * @brief 神符预测辅助函数
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
 * @brief 预测模块信息
 * @note
 * - 作为目标预测模块接口的返回值
 * @note
 * - 包含位置、角度、目标转角三类预测对象，同时包含静态响应、动态响应、射击延迟三种预测量类型
 * @see 关于预测量类型可参考 @ref tutorial_extra_gyro_predictor ，该预测模块对三种预测量类型均有涉及
 */
struct RunePredictorInfo {
    //! 静态响应预测增量 B
    std::unordered_map<tracker::const_ptr, double> static_prediction;
    //! 动态响应预测增量 Kt
    std::unordered_map<tracker::const_ptr, double> dynamic_prediction;
};

} // namespace rm

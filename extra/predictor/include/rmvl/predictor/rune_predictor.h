/**
 * @file rune_predictor.h
 * @author RoboMaster Vision Community
 * @brief 神符预测派生类头文件
 * @version 1.0
 * @date 2021-09-25
 *
 * @copyright Copyright 2021 (c), RoboMaster Vision Community
 *
 */

#pragma once

#include "rmvl/tracker/rune_tracker.h"

#include "details/rune.hpp"

namespace rm {

//! @addtogroup rune_predictor
//! @{

//! 神符预测模块
class RunePredictor final {
public:
    RunePredictor() = default;

    /**
     * @brief 神符预测核心函数
     * @note
     * - 静态响应预测量 `B`
     * @note
     * - 动态响应预测量 `Kt`
     *
     * @param[in] trackers 所有追踪器
     * @param[in] tof 每个追踪器对应的子弹飞行时间
     * @return 预测模块信息
     */
    RunePredictorInfo predict(const std::vector<RuneTracker::ptr> &trackers, const std::unordered_map<tracker::ptr, double> &tof);

    //! 构建 RunePredictor
    static inline std::unique_ptr<RunePredictor> make_predictor() { return std::make_unique<RunePredictor>(); }
};

//! @} rune_predictor

} // namespace rm

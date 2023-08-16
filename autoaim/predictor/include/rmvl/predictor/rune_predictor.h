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

#include <vector>

#include "predictor.h"

namespace rm
{

//! @addtogroup rune_predictor
//! @{

//! 神符预测模块
class RunePredictor final : public predictor
{
public:
    RunePredictor() = default;

    /**
     * @brief 神符预测核心函数
     * @note
     * - 静态响应预测量 `B` 生效: `ANG_Z`
     * @note
     * - 动态响应预测量 `Kt` 生效: `ANG_Z`
     *
     * @param[in] groups 所有序列组向量
     * @param[in] tof 每个追踪器对应的子弹飞行时间
     * @return 预测模块信息
     */
    PredictInfo predict(const std::vector<group_ptr> &groups,
                        const std::unordered_map<tracker_ptr, double> &tof) override;

    //! 构建 RunePredictor
    static inline std::unique_ptr<RunePredictor> make_predictor() { return std::make_unique<RunePredictor>(); }
};

//! @} rune_predictor

} // namespace rm

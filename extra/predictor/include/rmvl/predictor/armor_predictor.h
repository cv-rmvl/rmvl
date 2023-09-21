/**
 * @file armor_predictor.h
 * @author RoboMaster Vision Community
 * @brief 装甲板预测派生类头文件
 * @version 1.0
 * @date 2021-08-26
 *
 * @copyright Copyright 2021 (c), RoboMaster Vision Community
 *
 */

#pragma once

#include "predictor.h"

namespace rm
{

//! @addtogroup armor_predictor
//! @{

//! 装甲预测模块
class ArmorPredictor final : public predictor
{
public:
    //! 构建 ArmorPredictor
    static inline std::unique_ptr<ArmorPredictor> make_predictor() { return std::make_unique<ArmorPredictor>(); }

    /**
     * @brief 装甲板预测核心函数
     * @note
     * - 遍历所有 group 中所有 tracker，采用统一的预测方案：线性预测
     * @note
     * - 静态响应预测量 `B` 生效: `YAW`，`PITCH`
     * @note
     * - 动态响应预测量 `Kt` 生效: `YAW`，`PITCH`
     *
     * @param[in] groups 所有序列组
     * @param[in] tof 每个追踪器对应的子弹飞行时间
     * @return 预测模块信息
     */
    PredictInfo predict(const std::vector<group_ptr> &groups,
                        const std::unordered_map<tracker_ptr, double> &tof) override;
};

//! @} armor_predictor

} // namespace rm

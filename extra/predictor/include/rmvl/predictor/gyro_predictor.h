/**
 * @file gyro_predictor.h
 * @author RoboMaster Vision Community
 * @brief 整车状态预测派生类头文件
 * @version 0.1
 * @date 2023-01-09
 *
 * @copyright Copyright 2023 (c), RoboMaster Vision Community
 *
 */

#pragma once

#include "predictor.h"

namespace rm
{

//! @addtogroup gyro_predictor
//! @{

//! 整车状态预测模块
class GyroPredictor final : public predictor
{
public:
    //! 构建 GyroPredictor
    static inline std::unique_ptr<GyroPredictor> make_predictor() { return std::make_unique<GyroPredictor>(); }

    /**
     * @brief 装甲板预测核心函数
     * @note
     * - 遍历所有 group 中所有 tracker，采用统一的预测方案：三维预测
     * @note
     * - 静态响应预测量 `B` 生效: `POS_X`，`POS_Y`，`POS_Z`，`ANG_Y`
     * @note
     * - 动态响应预测量 `Kt` 生效: `POS_X`，`POS_Y`，`POS_Z`，`ANG_Y`
     * @note
     * - 射击延迟预测量 `Bs` 生效: `ANG_Y`
     *
     * @param[in] groups 所有序列组
     * @param[in] tof 每个追踪器对应的子弹飞行时间
     * @return 预测模块信息
     */
    PredictInfo predict(const std::vector<group::ptr> &groups,
                        const std::unordered_map<tracker::ptr, double> &tof) override;
};

//! @} gyro_predictor

} // namespace rm

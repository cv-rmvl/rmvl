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

#include "rmvl/tracker/tracker.h"

namespace rm {

//! @addtogroup armor_predictor
//! @{

//! 装甲预测模块
class ArmorPredictor final {
public:
    /**
     * @brief 预测模块信息
     * @note
     * - 作为目标预测模块接口的返回值
     * @note
     * - 包含位置、角度、目标转角三类预测对象，同时包含静态响应、动态响应、射击延迟三种预测量类型
     * @see 关于预测量类型可参考 @ref tutorial_extra_gyro_predictor ，该预测模块对三种预测量类型均有涉及
     */
    struct Info {
        //! 静态响应预测增量 B，每个 tracker 对应一个数组，数组元素依次为 YAW、PITCH
        std::unordered_map<tracker::const_ptr, std::array<double, 2>> static_prediction;
        //! 动态响应预测增量 Kt，每个 tracker 对应一个数组，数组元素依次为 YAW、PITCH
        std::unordered_map<tracker::const_ptr, std::array<double, 2>> dynamic_prediction;
    };

    //! 构建 ArmorPredictor
    static inline std::unique_ptr<ArmorPredictor> make_predictor() { return std::make_unique<ArmorPredictor>(); }

    /**
     * @brief 装甲板预测核心函数
     * @note
     * - 遍历所有 tracker，采用统一的预测方案：线性预测
     * @note
     * - 静态响应预测量 `B` 生效: `YAW`，`PITCH`
     * @note
     * - 动态响应预测量 `Kt` 生效: `YAW`，`PITCH`
     *
     * @param[in] trackers 所有追踪器
     * @param[in] tof 每个追踪器对应的子弹飞行时间
     * @return 预测模块信息
     */
    Info predict(const std::vector<tracker::ptr> &trackers, const std::unordered_map<tracker::ptr, double> &tof) const;
};

//! @} armor_predictor

} // namespace rm

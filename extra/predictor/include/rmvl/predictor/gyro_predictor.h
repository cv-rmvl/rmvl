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

#include "rmvl/group/group.h"

namespace rm {

//! @addtogroup gyro_predictor
//! @{

//! 整车状态预测模块
class GyroPredictor final {
public:
    //! 预测对象枚举
    enum : std::size_t {
        POS_X = 0, //!< X 方向位置预测增量
        POS_Y = 1, //!< Y 方向位置预测增量
        POS_Z = 2, //!< Z 方向位置预测增量
        ANG_Y = 3, //!< Y 轴旋转角度预测增量
    };

    /**
     * @brief 预测模块信息
     * @note
     * - 作为目标预测模块接口的返回值
     * @note
     * - 包含位置、角度、目标转角三类预测对象，同时包含静态响应、动态响应、射击延迟三种预测量类型
     * @see 关于预测量类型可参考 @ref tutorial_extra_gyro_predictor ，该预测模块对三种预测量类型均有涉及
     */
    struct Info {
        //! 静态响应预测增量 B，每个 tracker 对应一个数组，数组元素依次为 POS_X、POS_Y、POS_Z、ANG_Y
        std::unordered_map<tracker::const_ptr, std::array<double, 4>> static_prediction;
        //! 动态响应预测增量 Kt，每个 tracker 对应一个数组，数组元素依次为 POS_X、POS_Y、POS_Z、ANG_Y
        std::unordered_map<tracker::const_ptr, std::array<double, 4>> dynamic_prediction;
        //! 射击延迟预测增量 Bs，每个 tracker 对应 ANG_Y
        std::unordered_map<tracker::const_ptr, double> shoot_delay_prediction;
    };

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
    Info predict(const std::vector<group::ptr> &groups, const std::unordered_map<tracker::ptr, double> &tof);
};

//! @} gyro_predictor

} // namespace rm

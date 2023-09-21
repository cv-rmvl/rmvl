/**
 * @file predictor.h
 * @author RoboMaster Vision Community
 * @brief 抽象预测类头文件
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

//! @addtogroup predictor
//! @{

//! 预测对象枚举
enum : std::size_t
{
    POS_X = 0, //!< X 方向位置预测增量
    POS_Y = 1, //!< Y 方向位置预测增量
    POS_Z = 2, //!< Z 方向位置预测增量
    ANG_X = 3, //!< X 轴旋转角度预测增量
    ANG_Y = 4, //!< Y 轴旋转角度预测增量
    ANG_Z = 5, //!< Z 轴旋转角度预测增量
    YAW = 6,   //!< 目标转角 Yaw 预测增量
    PITCH = 7, //!< 目标转角 Pitch 预测增量
    ROLL = 8   //!< 目标转角 Roll 预测增量
};

/**
 * @brief 预测模块信息
 * @note
 * - 作为目标预测模块接口的返回值
 * @note
 * - 包含位置、角度、目标转角三类预测对象，同时包含静态响应、动态响应、射击延迟三种预测量类型
 * @see 关于预测量类型可参考 @ref tutorial_extra_gyro_predictor ，该预测模块对三种预测量类型均有涉及
 */
struct PredictInfo
{
    //! 静态响应预测增量 B
    std::unordered_map<tracker_ptr, cv::Vec<double, 9>> static_prediction;
    //! 动态响应预测增量 Kt
    std::unordered_map<tracker_ptr, cv::Vec<double, 9>> dynamic_prediction;
    //! 射击延迟预测增量 Bs
    std::unordered_map<tracker_ptr, cv::Vec<double, 9>> shoot_delay_prediction;
};

//! 目标预测模块
class predictor
{
public:
    predictor() = default;

    virtual ~predictor() = default;

    /**
     * @brief 预测核心函数
     *
     * @param[in] groups 所有序列组向量
     * @param[in] tof 每个追踪器对应的子弹飞行时间
     * @return 预测模块信息
     */
    virtual PredictInfo predict(const std::vector<group_ptr> &groups,
                                const std::unordered_map<tracker_ptr, double> &tof) = 0;
};

//! 抽象预测类非共享指针
using predict_ptr = std::unique_ptr<predictor>;

//! @} predictor

} // namespace rm

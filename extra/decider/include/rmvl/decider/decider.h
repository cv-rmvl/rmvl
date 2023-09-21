/**
 * @file decider.h
 * @author RoboMaster Vision Community
 * @brief 抽象决策类头文件
 * @version 1.0
 * @date 2021-08-24
 *
 * @copyright Copyright 2021 (c), RoboMaster Vision Community
 *
 */

#pragma once

#include "rmvl/compensator/compensator.h"
#include "rmvl/detector/detector.h"
#include "rmvl/predictor/predictor.h"

namespace rm
{

//! @addtogroup decider
//! @{

/**
 * @brief 决策模块信息
 * @note
 * - 作为目标决策模块接口的返回值
 */
struct DecideInfo
{
    tracker::ptr target;       //!< 目标追踪器
    cv::Point2f shoot_center; //!< 目标追踪器对应距离下的实时射击中心
    cv::Point2f exp_angle;    //!< 云台响应的期望角度偏移量
    cv::Point2f exp_center2d; //!< 像素坐标系下的期望目标点
    cv::Point3f exp_center3d; //!< 相机坐标系下的期望目标点
    bool can_shoot = false;   //!< 能否射击
};

//! 目标决策模块
class decider
{
public:
    using ptr = std::unique_ptr<decider>;

    decider() = default;

    virtual ~decider() = default;

    /**
     * @brief 决策核心函数
     *
     * @param[in] groups 所有序列组
     * @param[in] flag 决策状态模式
     * @param[in] last_target 历史目标追踪器，为空则默认自动判断
     * @param[in] detect_info 辅助决策的识别模块信息
     * @param[in] compensate_info 辅助决策的补偿模块信息
     * @param[in] predict_info 辅助决策的预测模块信息
     * @return 决策模块信息
     */
    virtual DecideInfo decide(const std::vector<group::ptr> &groups, RMStatus flag,
                              tracker::ptr last_target, const DetectInfo &detect_info,
                              const CompensateInfo &compensate_info, const PredictInfo &predict_info) = 0;
};

//! @} decider

} // namespace rm

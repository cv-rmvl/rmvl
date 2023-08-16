/**
 * @file translation_decider.h
 * @author RoboMaster Vision Community
 * @brief 平移目标决策器头文件
 * @version 1.0
 * @date 2021-09-30
 *
 * @copyright Copyright 2021 (c), RoboMaster Vision Community
 *
 */

#pragma once

#include "decider.h"

namespace rm
{

//! @addtogroup translation_decider
//! @{

//! 平移目标决策类
class TranslationDecider final : public decider
{
public:
    TranslationDecider() = default;

    //! 构造 TranslationDecider
    static inline std::unique_ptr<TranslationDecider> make_decider() { return std::make_unique<TranslationDecider>(); }

    /**
     * @brief 平移目标决策核心函数
     *
     * @param[in] groups 所有序列组
     * @param[in] flag 决策状态模式
     * @param[in] last_target 历史目标追踪器，为空则默认自动判断
     * @param[in] detect_info 辅助决策的识别模块信息
     * @param[in] compensate_info 辅助决策的补偿模块信息
     * @param[in] predict_info 辅助决策的预测模块信息
     * @return 决策模块信息
     */
    DecideInfo decide(const std::vector<group_ptr> &groups, RMStatus flag,
                      const tracker_ptr &last_target, const DetectInfo &detect_info,
                      const CompensateInfo &compensate_info, const PredictInfo &predict_info) override;

private:
    /**
     * @brief 获取离图像中心最近的目标序列
     *
     * @param[in] img 参考图像
     * @param[in] trackers 所有的装甲板时间序列追踪器
     * @return 离图像中心最近的目标序列
     */
    tracker_ptr getClosestTracker(cv::Mat img, const std::vector<tracker_ptr> &trackers);
};

//! @} translation_decider

} // namespace rm

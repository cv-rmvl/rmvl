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

#include "rmvl/compensator/details/common.hpp"
#include "rmvl/predictor/armor_predictor.h"

namespace rm {

//! @addtogroup translation_decider
//! @{

//! 平移目标决策类
class TranslationDecider final {
public:
    /**
     * @brief 决策模块信息
     * @note
     * - 作为目标决策模块接口的返回值
     */
    struct Info {
        tracker::ptr target;      //!< 目标追踪器
        cv::Point2f shoot_center; //!< 目标追踪器对应距离下的实时射击中心
        cv::Point2f exp_angle;    //!< 云台响应的期望角度偏移量
        cv::Point2f exp_center2d; //!< 像素坐标系下的期望目标点
        cv::Point3f exp_center3d; //!< 相机坐标系下的期望目标点
        bool can_shoot = false;   //!< 能否射击
    };

    TranslationDecider() = default;

    //! 构造 TranslationDecider
    static inline std::unique_ptr<TranslationDecider> make_decider() { return std::make_unique<TranslationDecider>(); }

    /**
     * @brief 平移目标决策核心函数
     *
     * @param[in] trackers 所有追踪器
     * @param[in] last_target 历史目标追踪器，为空则默认自动判断
     * @param[in] src 参考图像
     * @param[in] compensate_info 辅助决策的补偿模块信息
     * @param[in] predict_info 辅助决策的预测模块信息
     * @return 决策模块信息
     */
    Info decide(const std::vector<tracker::ptr> &trackers, tracker::ptr last_target, cv::Mat src,
                const CompensateInfo &compensate_info, const ArmorPredictor::Info &predict_info);

private:
    /**
     * @brief 获取离图像中心最近的目标序列
     *
     * @param[in] img 参考图像
     * @param[in] trackers 所有的装甲板时间序列追踪器
     * @return 离图像中心最近的目标序列
     */
    tracker::ptr getClosestTracker(cv::Mat img, const std::vector<tracker::ptr> &trackers);
};

//! @} translation_decider

} // namespace rm

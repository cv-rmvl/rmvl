/**
 * @file rune_decider.h
 * @author RoboMaster Vision Community
 * @brief 神符模块决策器头文件
 * @version 1.0
 * @date 2021-09-28
 *
 * @copyright Copyright 2021 (c), RoboMaster Vision Community
 *
 */

#pragma once

#include "rmvl/group/group.h"

#include "rmvl/compensator/details/common.hpp"
#include "rmvl/predictor/details/rune.hpp"

namespace rm {

//! @addtogroup rune_decider
//! @{

//! 神符决策模块
class RuneDecider final {
    bool _is_changed = true;         //!< 神符改变标志位
    double _start_tick{};            //!< 起始时间
    double _delta_time{};            //!< 累积时间
    int _send_times{};               //!< 发送次数
    float _miss_frequency{0.65f};    //!< 击打不成功最大间隔
    float _initial_frequency{0.17f}; //!< 初始击打间隔

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

    RuneDecider() = default;

    //! 构造 RuneDecider
    static inline std::unique_ptr<RuneDecider> make_decider() { return std::make_unique<RuneDecider>(); }

    /**
     * @brief 神符决策核心函数
     *
     * @param[in] groups 所有序列组
     * @param[in] flag 决策状态模式
     * @param[in] last_target 历史目标追踪器，为空则默认自动判断
     * @param[in] detect_info 辅助决策的识别模块信息
     * @param[in] compensate_info 辅助决策的补偿模块信息
     * @param[in] predict_info 辅助决策的预测模块信息
     * @return 决策模块信息
     */
    Info decide(group::ptr group, bool is_active, tracker::ptr last_target,
                const CompensateInfo &compensate_info, const RunePredictorInfo &predict_info);

private:
    /**
     * @brief 判断是否具有射击权限
     *
     * @param[in] target_tracker 目标追踪器
     * @param[in] is_active 神符是否激活
     * @param[in] comp 补偿增量
     * @param[in] center2d 像素坐标系下的期望目标点
     * @param[out] shoot_center 击打中心
     * @return 能否射击的标志
     */
    bool judgeShoot(tracker::ptr target_tracker, bool is_active, const cv::Point2f &comp,
                    const cv::Point2f &center2d, cv::Point2f &shoot_center);

    //! 触发初始化
    void triggerInit();
};

//! @} rune_decider

} // namespace rm

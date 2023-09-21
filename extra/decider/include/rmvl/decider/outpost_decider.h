/**
 * @file outpost_decider.h
 * @author RoboMaster Vision Community
 * @brief
 * @version 0.1
 * @date 2023-01-11
 *
 * @copyright Copyright 2023 (c), RoboMaster Vision Community
 *
 */

#pragma once

#include "decider.h"

namespace rm
{

//! @addtogroup rune_decider
//! @{

//! 前哨站决策模块
class OutpostDecider final : public decider
{
private:
    //! 目标装甲板信息 @todo 给类或结构体在声明处写上初始化数值
    struct TargetArmor
    {
        combo::ptr aim_armor;
        cv::Point2f predict_point;
        cv::Point2f angle; //!< `x` 为 total_yaw, `y` 为 total_pitch
        double fly_time;
        float height;
        float start_times;
        float time; // 击打等待时间
    };

    //! 清空模式
    enum class ClearMode
    {
        CURRENT, //!< 清空当前目标
        NEXT,    //!< 清空下一个目标
        REPLACE  //!< 清空所有
    };

    bool _is_next;       //!< 是否计算下一个装甲板标志位
    uint8_t _send_times; //!< 发送次数
    double _delta_times; //!< 累积时间
    float _velocity;     //!< 当前前哨站旋转速度
    float _width_bias;   //!< 灯条宽度差
    float _time;

    TargetArmor _current_aim;
    TargetArmor _next_aim;
    std::deque<double> _velocities;

public:
    OutpostDecider() = default; // 默认构造

    /**
     * @brief 静止建筑物模块决策核心函数
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
                              const CompensateInfo &compensate_info, const PredictInfo &predict_info) override;

    //! 构造 OutpostDecider
    static inline std::unique_ptr<OutpostDecider> make_decider() { return std::make_unique<OutpostDecider>(); }

private:
    /**
     * @brief 根据 ClearMode 初始化瞄准信息
     *
     * @param mode ClearMode 标志位
     */
    void init(ClearMode mode);

    /**
     * @brief 选择静止的目标序列
     *
     * @param trackers 当前所有追踪器
     * @return tracker::ptr
     */
    tracker::ptr getClosestTracker(const std::vector<tracker::ptr> &trackers);

    std::tuple<combo::ptr, float> getAimPoint(const std::vector<combo::ptr> &combos);

    cv::Point2f calculateData(tracker::ptr target_tracker, const CompensateInfo &compensate_info);

    void calculateInfo(tracker::ptr target_tracker, const CompensateInfo &compensate_info,
                       const cv::Point2f &horizon_center, bool is_next);
};

//! @} outpost_decider

} // namespace rm

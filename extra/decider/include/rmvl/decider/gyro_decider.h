/**
 * @file gyro_decider.h
 * @author RoboMaster Vision Community
 * @brief 装甲板模块决策器头文件
 * @version 0.1
 * @date 2023-01-11
 *
 * @copyright Copyright 2023 (c), RoboMaster Vision Community
 *
 */

#pragma once

#include "decider.h"
#include "rmvl/tracker/tracker.h"

namespace rm
{

//! @addtogroup gyro_decider
//! @{

//! 整车状态决策类
class GyroDecider final : public decider
{
    std::unordered_map<RobotType, int> _priority; //!< 数字决策优先级

public:
    GyroDecider();

    /**
     * @brief 装甲板决策核心函数
     * @note 优先选取上一帧的目标作为当前目标，同时考虑上 `TargetLockType`
     *       的信息作为目标 `group` 的选取。选取中，如果 `RobotType`
     *       优先级均相同，则选取 `GyroGroup::getCenter3D()` 更靠近 `(0, 0, 0)`
     *       的 `group` 作为目标。最后加上补偿、预测信息，以选择目标追踪器
     *       `target_tracker`，最后设置 `TransferData`。
     *
     * @param[in] groups 所有序列组
     * @param[in] flag 决策状态模式
     * @param[in] last_target 历史目标追踪器，为空则默认自动判断
     * @param[in] detect_info 辅助决策的识别模块信息
     * @param[in] compensate_info 辅助决策的补偿模块信息
     * @param[in] predict_info 辅助决策的预测模块信息
     * @return 决策模块信息
     */
    DecideInfo decide(const std::vector<group::ptr> &groups, RMStatus flag,
                      tracker::ptr last_target, const DetectInfo &detect_info,
                      const CompensateInfo &compensate_info, const PredictInfo &predict_info) override;

    //! 构造 GyroDecider
    static inline std::unique_ptr<GyroDecider> make_decider() { return std::make_unique<GyroDecider>(); }

private:
    /**
     * @brief 在指定的 group 中获取最近的目标追踪器
     * @note 参数 `p_group` 表示为目标序列组，通过调用此函数，可以得到目标追踪器
     *
     * @param[in] p_group 指定的序列组 group
     * @param[in] info 预测信息
     * @return 最近的目标追踪器
     */
    tracker::ptr getClosestTracker(group::ptr p_group, const PredictInfo &info);

    /**
     * @brief 获取优先级最高的目标序列组
     *
     * @param[in] groups 所有的装甲板序列组
     * @return 优先级最高的目标序列组
     */
    group::ptr getHighestPriorityGroup(const std::vector<group::ptr> &groups);
};

//! @} gyro_decider

} // namespace rm

/**
 * @file rune_decider.cpp
 * @author RoboMaster Vision Community
 * @brief
 * @version 1.0
 * @date 2021-09-28
 *
 * @copyright Copyright 2021 (c), RoboMaster Vision Community
 *
 */

#include "rmvl/decider/rune_decider.h"
#include "rmvl/combo/rune.h"
#include "rmvl/core/timer.hpp"
#include "rmvl/algorithm/transform.hpp"
#include "rmvl/algorithm/math.hpp"

#include "rmvlpara/camera/camera.h"
#include "rmvlpara/decider/rune_decider.h"

namespace rm
{

/**
 * @brief 从角度预测增量中获取实际目标转角增量、图像中的位置信息
 *
 * @param[in] d_predict `ANG_Z` 的角度预测增量
 * @param[in] ref_tracker 参考追踪器（预测点将以此为基准进行计算）
 * @param[out] d_angle 实际目标转角增量信息
 * @param[out] d_center 实际图像中的位置信息
 */
static void getPredictMsgFromAngleZ(float d_predict, tracker::ptr ref_tracker, cv::Point2f &d_angle, cv::Point2f &d_center)
{
    auto p_rune = Rune::cast(ref_tracker->front());
    // 特征距离
    cv::Point2f rune_center = p_rune->at(1)->getCenter();
    float feature_distance = p_rune->getFeatureDis();
    // 动态预测点
    cv::Point2f predict_center;
    predict_center.x = rune_center.x + feature_distance * cos(deg2rad(ref_tracker->getAngle() + d_predict));
    predict_center.y = rune_center.y - feature_distance * sin(deg2rad(ref_tracker->getAngle() + d_predict));
    // 平面修正
    auto correct_vec = Rune::verticalConvertToCamera(predict_center - rune_center,
                                                     ref_tracker->front()->getGyroData().rotation.pitch);
    d_center = rune_center + cv::Point2f(correct_vec);
    d_angle = calculateRelativeAngle(para::camera_param.cameraMatrix, rune_center + cv::Point2f(correct_vec)) -
              ref_tracker->getRelativeAngle();
}

DecideInfo RuneDecider::decide(const std::vector<group::ptr> &groups, RMStatus flag,
                               tracker::ptr last_target, const DetectInfo &,
                               const CompensateInfo &compensate_info, const PredictInfo &predict_info)
{
    // 决策信息
    DecideInfo info{};
    if (groups.size() != 1)
        RMVL_Error(RMVL_StsBadSize, "Size of the groups is not equal to \'1\'");
    // 需要被考虑的真实追踪器
    std::vector<tracker::ptr> true_trackers;
    true_trackers.reserve(5);

    for (auto &p_tracker : groups.front()->data())
        if (p_tracker->getType().RuneTypeID == flag.RuneTypeID)
            true_trackers.emplace_back(p_tracker);

    if (last_target != nullptr)
        for (auto &p_tracker : true_trackers)
            if (p_tracker == last_target)
                info.target = p_tracker;
    // 无目标 && 无神符
    if (!info.target && true_trackers.empty())
        return info;
    // 无目标 && 有神符
    else if (!info.target && !true_trackers.empty())
    {
        // 已激活神符决策
        if (flag.RuneTypeID == RuneType::ACTIVE)
            info.target = *min_element(true_trackers.begin(), true_trackers.end(), [&](const auto &lhs, const auto &rhs) {
                return lhs->getCenter().y < rhs->getCenter().y;
            });
        // 未激活神符决策
        else
        {
            triggerInit();
            // 记录出现神符开始时的时间
            info.target = true_trackers.back();
        }
    }
    // 有目标
    else
    {
        // 已激活神符决策
        if (flag.RuneTypeID == RuneType::ACTIVE)
        {
            // 寻找最下方神符
            info.target = *min_element(true_trackers.begin(), true_trackers.end(), [&](const auto &lhs, const auto &rhs) {
                return lhs->getCenter().y < rhs->getCenter().y;
            });
        }
        else
        {
            // 若正在追踪的目标不是最新的目标，则需要重置决策信息
            if (info.target != true_trackers.back())
            {
                triggerInit();
                info.target = true_trackers.back();
            }
        }
    }

    if (info.target != nullptr)
    {
        auto dKt = predict_info.dynamic_prediction.at(info.target)(ANG_Z);
        auto dB = predict_info.static_prediction.at(info.target)(ANG_Z);
        auto comp = compensate_info.compensation.at(info.target);

        cv::Point2f d_angle, center, unused_v;
        getPredictMsgFromAngleZ(dKt, info.target, unused_v, center);
        getPredictMsgFromAngleZ(dKt + dB, info.target, d_angle, unused_v);

        info.exp_angle = info.target->getRelativeAngle();
        info.exp_angle += d_angle; // 加上预测
        info.exp_angle += comp;    // 加上补偿
        info.exp_center2d = center;

        _delta_time = Timer::now() - _start_tick;
        // 判断能否射击
        if (judgeShoot(info.target, flag.RuneTypeID, comp, info.exp_center2d, info.shoot_center) &&
            _delta_time > _miss_frequency && !_is_changed)
        {
            if (_send_times < 5)
            {
                info.can_shoot = true;
                _send_times++;
            }
            else
            {
                triggerInit();
                _is_changed = false;
            }
        }
        else if (judgeShoot(info.target, flag.RuneTypeID, comp, info.exp_center2d, info.shoot_center) &&
                 _delta_time > _initial_frequency && _is_changed)
        {
            // 继续追踪 5 帧，作为靠近目标后的保险措施
            if (_send_times < 5)
                _send_times++;
            // 连续发送 5 次可以击打的信号
            else if (_send_times >= 5 && _send_times <= 10)
            {
                info.can_shoot = true;
                _send_times++;
            }
            // 击打完毕后重置的目标，并标记为 false 表示该目标已经击打过一次
            else
            {
                triggerInit();
                _is_changed = false;
            }
        }
    }
    return info;
}

bool RuneDecider::judgeShoot(tracker::ptr target_tracker, RuneType rune_mode,
                             const cv::Point2f &comp, const cv::Point2f &center2d, cv::Point2f &shoot_center)
{
    if (!target_tracker)
        return false;
    // 子弹落点转换为图像中的坐标点
    shoot_center = calculateRelativeCenter(para::camera_param.cameraMatrix, -comp);
    // 中心距离
    float center_dis = getDistance(center2d, shoot_center);
    // 判断
    switch (rune_mode)
    {
    case RuneType::ACTIVE: // 已激活
        return center_dis <= para::rune_decider_param.DISTURB_RADIUS_RATIO * target_tracker->front()->at(1)->getWidth();
    default: // 默认: 未激活
        return center_dis <= para::rune_decider_param.NORMAL_RADIUS_RATIO * target_tracker->front()->at(1)->getWidth();
    }
}

void RuneDecider::triggerInit()
{
    _is_changed = true;
    _start_tick = Timer::now();
    _initial_frequency = para::rune_decider_param.INIT_FREQUENCY;
    _miss_frequency = para::rune_decider_param.MISS_FREQUENCY;
    _send_times = 0;
}

} // namespace rm

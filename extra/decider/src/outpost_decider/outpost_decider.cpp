/**
 * @file outpost_decider.cpp
 * @author RoboMaster Vision Community
 * @brief
 * @version 0.1
 * @date 2023-01-11
 *
 * @copyright Copyright 2023 (c), RoboMaster Vision Community
 *
 */

#include "rmvl/decider/outpost_decider.h"

#include "rmvlpara/camera.hpp"
#include "rmvlpara/decider/outpost_decider.h"

using namespace rm;
using namespace para;
using namespace std;
using namespace cv;

void OutpostDecider::init(ClearMode mode)
{
    switch (mode)
    {
    case ClearMode::REPLACE:
        _is_next = false;
        _current_aim.height = _next_aim.height;
        _current_aim.start_times = _next_aim.start_times;
        _current_aim.aim_armor = _next_aim.aim_armor;
        _current_aim.angle = _next_aim.angle;
        _current_aim.time = _next_aim.time;
        _current_aim.fly_time = _next_aim.fly_time;
        break;
    case ClearMode::NEXT:
        _next_aim.time = 0;
        _next_aim.start_times = 0;
        _next_aim.aim_armor = nullptr;
        _next_aim.angle = Point2f(0, 0);
        _next_aim.fly_time = 0.f;
        _send_times = 0;
        break;
    case ClearMode::CURRENT:
        _is_next = false;
        _current_aim.time = 0;
        _current_aim.start_times = 0;
        _current_aim.aim_armor = nullptr;
        _current_aim.angle = Point2f(0, 0);
        _current_aim.fly_time = 0.f;
        break;
    }
}

DecideInfo OutpostDecider::decide(const std::vector<group::ptr> &groups, RMStatus,
                                  tracker::ptr last_target, const DetectInfo &detect_info,
                                  const CompensateInfo &compensate_info, const PredictInfo &)
{
    // 决策信息
    DecideInfo info{};
    if (groups.size() != 1)
        RMVL_Error(RMVL_StsBadSize, "Size of the groups must equal to \'1\'.");
    const vector<tracker::ptr> &trackers = groups.front()->data();
    // 获取、更新目标追踪器
    for (auto p_tracker : trackers)
        if (last_target == p_tracker && (p_tracker->getType().RobotTypeID == RobotType::OUTPOST ||
                                         p_tracker->getType().RobotTypeID == RobotType::SENTRY))
            info.target = p_tracker;
    // 当前帧无装甲板序列,并且并未储存击打信息,进入操作手模式
    if (!info.target && trackers.empty() && _current_aim.start_times == 0)
        return info;
    // 无追踪目标时，选择离图像中心最近的追踪器
    else if (!info.target && !trackers.empty())
    {
        init(ClearMode::CURRENT);
        init(ClearMode::NEXT);
        info.target = getClosestTracker(trackers);
    }

    Point2f feature_center;
    if (!detect_info.features.empty())
        feature_center = detect_info.features.front()->getCenter();

    // 如果当前有两个tracker并且上一个tracker消失帧数大于2
    if (trackers.size() > 1 && trackers[0]->getVanishNumber() > 2)
        _is_next = true;

    // 判断是否正在追踪
    if (info.target)
    {
        // 计算速度，并选出最佳的固定击打点
        calculateInfo(info.target, compensate_info, feature_center, _is_next);
    }
    // 计算运行时间, 判断是否需要击打
    _delta_times = static_cast<double>(getTickCount() - _current_aim.start_times) / getTickFrequency();
    // 跟丢目标或者变速则清空信息, 返回
    if (_delta_times > outpost_decider_param.LOST_TIME || (_current_aim.time == outpost_decider_param.TIMES_CHANGE))
    {
        init(ClearMode::NEXT);
        init(ClearMode::CURRENT);
        return info;
    }

    _time = outpost_decider_param.TIMES_1;

    if (_delta_times >= abs(_time - _current_aim.fly_time))
    {
        if (_send_times < outpost_decider_param.MAX_SEND_TIME)
        {
            info.exp_angle = _current_aim.angle;
            info.can_shoot = true;
            _send_times++;
        }
        if (_send_times > (outpost_decider_param.MAX_SEND_TIME - 1))
        {
            if (_next_aim.angle.x == 0)
                info.exp_angle = _current_aim.angle;
            else
            {
                init(ClearMode::REPLACE);
                init(ClearMode::NEXT);
                info.exp_angle = _current_aim.angle;
            }
        }
    }
    else
        info.exp_angle = _current_aim.angle;

    return info;
}

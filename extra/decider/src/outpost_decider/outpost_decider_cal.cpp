/**
 * @file outpost_decider_cal.cpp
 * @author RoboMaster Vision Community
 * @brief
 * @version 1.0
 * @date 2023-08-02
 *
 * @copyright Copyright 2023 (c), RoboMaster Vision Community
 *
 */

#include "rmvl/decider/outpost_decider.h"
#include "rmvlpara/camera.hpp"
#include "rmvlpara/decider/outpost_decider.h"

using namespace cv;
using namespace std;
using namespace para;
using namespace rm;

tracker::ptr OutpostDecider::getClosestTracker(const vector<tracker::ptr> &trackers)
{
    if (trackers.empty())
        return nullptr;
    return *min_element(trackers.begin(), trackers.end(),
                        [&](tracker::ptr lhs, tracker::ptr rhs)
                        {
                            return pow(lhs->front()->getRelativeAngle().x, 2) + pow(lhs->front()->getRelativeAngle().y, 2) <
                                   pow(rhs->front()->getRelativeAngle().x, 2) + pow(rhs->front()->getRelativeAngle().y, 2);
                        });
}

tuple<combo::ptr, float> OutpostDecider::getAimPoint(const vector<combo::ptr> &combos)
{
    unordered_map<combo::ptr, float> best_shoot_point; // 最佳射击点
    for (const auto &p_combo : combos)
    {
        // 找最小灯条宽度差->两灯条正对相机时宽度差最小
        float blob1_width = p_combo->at(0)->getWidth();
        float blob2_width = p_combo->at(1)->getWidth();
        [[maybe_unused]] float delta_width = abs(blob1_width - blob2_width);
        float angle = abs(p_combo->getAngle());
        best_shoot_point[p_combo] = angle;
    }
    auto armor = min_element(best_shoot_point.begin(), best_shoot_point.end(),
                             [&](const auto &pair_1, const auto &pair_2)
                             {
                                 return pair_1.second < pair_2.second;
                             })
                     ->first;

    float width_bias = best_shoot_point[armor];
    return {armor, width_bias};
}

Point2f OutpostDecider::calculateData(tracker::ptr target_tracker, const CompensateInfo &compensate_info)
{
    const auto &comp = compensate_info.compensation.at(target_tracker);
    // 加入预测、补偿
    return target_tracker->getRelativeAngle() + comp;
}

void OutpostDecider::calculateInfo(tracker::ptr target_tracker, const CompensateInfo &compensate_info,
                                   const Point2f &horizon_center, bool is_next)
{
    vector<combo::ptr> current_combos;
    for (size_t i = 0; i < target_tracker->size(); i++)
    {
        current_combos.emplace_back(target_tracker->at(i));
    }
    float distance = getDistance(target_tracker->front()->getCenter(), horizon_center) /
                     target_tracker->front()->getWidth();
    // 如果装甲板和水平灯条距离小于一定范围,则开始解算击打点
    if (distance < outpost_decider_param.OUTPOST_DISTANCE)
    {
        auto aimpoint_tuple = getAimPoint(current_combos);
        auto current_aim_armor = get<0>(aimpoint_tuple);
        auto current_width_bias = get<1>(aimpoint_tuple);

        // 第一次计算或者有误差更小时,更新
        if (_width_bias == 0 || _current_aim.aim_armor == nullptr || current_width_bias < _width_bias)
        {
            // (Warning) 此处if中的判断条件可能并不会按预期的进行
            if (is_next)
            {
                _width_bias = current_width_bias;
                _next_aim.aim_armor = current_aim_armor;
                _next_aim.start_times = current_aim_armor->getTick(); //! @bug int64->float
                // 更新位置数据
                _next_aim.height = target_tracker->getHeight();
                _next_aim.angle = calculateData(target_tracker, compensate_info);
                _next_aim.fly_time = compensate_info.tof.at(target_tracker);
            }
            else
            {
                _width_bias = current_width_bias;
                _current_aim.aim_armor = current_aim_armor;
                _current_aim.start_times = current_aim_armor->getTick(); //! @bug int64->float
                // 更新位置数据
                _current_aim.height = target_tracker->getHeight();
                _current_aim.angle = calculateData(target_tracker, compensate_info);
                _current_aim.fly_time = compensate_info.tof.at(target_tracker);
            }
        }
    }
    else
    {
        // 实时跟随
        if (is_next)
            _next_aim.aim_armor = target_tracker->front();
        else
            _current_aim.aim_armor = target_tracker->front();
    }
}

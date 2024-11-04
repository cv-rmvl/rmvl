/**
 * @file match.cpp
 * @author RoboMaster Vision Community
 * @brief match to tracker and group
 * @version 1.0
 * @date 2022-03-02
 *
 * @copyright Copyright 2023 (c), RoboMaster Vision Community
 *
 */

#include <unordered_set>

#include "rmvl/detector/armor_detector.h"

#include "rmvlpara/detector/armor_detector.h"
#include "rmvlpara/tracker/planar_tracker.h"

namespace rm
{

/**
 * @brief 判断是否突变
 *
 * @note 如果装甲板在一帧内位置变化特别大, 且速度发生较大突变，则创建新序列
 *
 * @param t_combo 时间序列中最新的装甲板
 * @param armor 待判断的装甲板
 * @param dis 测出的最小间距
 * @return 是否突变
 */
static inline bool isChange(const combo::ptr &t_combo, const combo::ptr &combo, float dis)
{
    return ((t_combo->angle() * combo->angle() < -80.f) || dis > para::armor_detector_param.MAX_TRACKER_DELTA_DIS);
}

void ArmorDetector::match(std::vector<group::ptr> &groups, const std::vector<combo::ptr> &combos)
{
    // combo 匹配 tracker
    matchArmors(groups.front()->data(), combos);
    // 删除因数字识别判断出的伪装甲板序列
    // eraseFakeTracker(groups.front()->data());
    // 删除 tracker
    eraseNullTracker(groups.front()->data());
}

void ArmorDetector::matchArmors(std::vector<tracker::ptr> &trackers, const std::vector<combo::ptr> &combos)
{
    // 如果 trackers 为空先为每个识别到的 combo 开辟序列
    if (trackers.empty())
    {
        for (const auto &p_combo : combos)
            trackers.emplace_back(PlanarTracker::make_tracker(p_combo));
        return;
    }

    // 如果当前帧识别到的装甲板数量 > 序列数量
    if (combos.size() > trackers.size())
    {
        // 初始化装甲板集合
        std::unordered_set<combo::ptr> armor_set(combos.begin(), combos.end());
        // 距离最近的装甲板匹配到相应的序列中, 并 update
        for (auto p_tracker : trackers)
        {
            // 离 p_tracker 最近的 combo 及其距离
            auto min_it = std::min_element(combos.begin(), combos.end(), [&](combo::const_ptr lhs, combo::const_ptr rhs) {
                return getDistance(lhs->center(), p_tracker->front()->center()) <
                       getDistance(rhs->center(), p_tracker->front()->center());
            });
            p_tracker->update(*min_it);
            armor_set.erase(*min_it);
        }
        // 没有匹配到的装甲板作为新的序列
        for (const auto &p_combo : armor_set)
            trackers.emplace_back(PlanarTracker::make_tracker(p_combo));
    }
    // 如果当前帧识别到的装甲板数量 < 序列数量
    else if (combos.size() < trackers.size())
    {
        // 初始化追踪器集合
        std::unordered_set<tracker::ptr> tracker_set(trackers.begin(), trackers.end());
        for (const auto &p_combo : combos)
        {
            // 离 armor 最近的 tracker 及其距离
            auto min_dis_tracker = std::min_element(trackers.begin(), trackers.end(), [&](tracker::const_ptr lhs, tracker::const_ptr rhs) {
                return getDistance(p_combo->center(), lhs->front()->center()) <
                       getDistance(p_combo->center(), rhs->front()->center());
            });
            min_dis_tracker->get()->update(p_combo);
            tracker_set.erase(*min_dis_tracker);
        }
        // 没有匹配到的序列传入 nullptr
        for (const auto &p_tracker : tracker_set)
            p_tracker->update(_tick, _gyro_data);
    }
    // 如果当前帧识别到的装甲板数量 = 序列数量
    else
    {
        // 初始化装甲板集合
        std::unordered_set<combo::ptr> armor_set(combos.begin(), combos.end());
        // 防止出现迭代器非法化的情况，此处使用下标访问
        size_t before_size = trackers.size(); // 存储原始 trackers 大小
        for (size_t i = 0; i < before_size; i++)
        {
            // 离 tracker 最近的 combo
            auto min_it = std::min_element(armor_set.begin(), armor_set.end(), [&](const combo::ptr &lhs, const combo::ptr &rhs) {
                return getDistance(lhs->center(), trackers[i]->front()->center()) <
                       getDistance(rhs->center(), trackers[i]->front()->center());
            });
            // 最短距离
            float min_dis = getDistance(min_it->get()->center(), trackers[i]->front()->center());
            // 判断是否突变
            if (isChange(trackers[i]->front(), *min_it, min_dis))
            {
                // 创建新序列，原来的序列打入 nullptr
                trackers[i]->update(_tick, _gyro_data);
                trackers.emplace_back(PlanarTracker::make_tracker(*min_it));
            }
            else
                trackers[i]->update(*min_it);
            armor_set.erase(*min_it);
        }
    }
}

void ArmorDetector::eraseNullTracker(std::vector<tracker::ptr> &trackers)
{
    // 删除
    trackers.erase(std::remove_if(trackers.begin(), trackers.end(), [](tracker::const_ptr p_tracker) {
                       return p_tracker->getVanishNumber() >= para::planar_tracker_param.TRACK_FRAMES;
                   }),
                   trackers.end());
}

void ArmorDetector::eraseFakeTracker(std::vector<tracker::ptr> &trackers)
{
    // 删除
    trackers.erase(std::remove_if(trackers.begin(), trackers.end(), [](tracker::const_ptr t1) {
                       return t1->type().RobotTypeID == RobotType::UNKNOWN;
                   }),
                   trackers.end());
}

} // namespace rm

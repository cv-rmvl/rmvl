/**
 * @file match.cpp
 * @author RoboMaster Vision Community
 * @brief match
 * @version 1.0
 * @date 2021-10-04
 *
 * @copyright Copyright 2021 (c), RoboMaster Vision Community
 *
 */

#include <unordered_set>

#include "rmvl/detector/rune_detector.h"
#include "rmvl/group/rune_group.h"

#include "rmvlpara/tracker/rune_tracker.h"

namespace rm
{

void RuneDetector::match(std::vector<tracker::ptr> &rune_trackers, const std::vector<combo::ptr> &combos)
{
    // 匹配
    this->matchRunes(rune_trackers, combos);
    // 删除
    rune_trackers.erase(remove_if(rune_trackers.begin(), rune_trackers.end(), [](tracker::const_ptr p_tracker) {
                            return p_tracker->getVanishNumber() >= para::rune_tracker_param.TRACK_FRAMES;
                        }),
                        rune_trackers.end());
}

void RuneDetector::matchRunes(std::vector<tracker::ptr> &trackers, const std::vector<combo::ptr> &combos)
{
    // 如果 trackers 为空先为每个识别到的 active_rune 开辟序列
    if (trackers.empty())
    {
        for (const auto &p_combo : combos)
            trackers.emplace_back(RuneTracker::make_tracker(p_combo));
        return;
    }
    // 如果当前帧识别到的神符数量 > 序列数量
    if (combos.size() > trackers.size())
    {
        // 初始化哈希表
        std::unordered_set<combo::ptr> combo_set(combos.begin(), combos.end());
        // 距离最近的神符匹配到相应序列中，并 update
        for (auto p_tracker : trackers)
        {
            // 离 p_tracker 最近的 active_rune
            combo::ptr closest_rune = *min_element(combos.begin(), combos.end(), [&p_tracker](combo::ptr lhs, combo::ptr rhs) {
                return getDeltaAngle(lhs->angle(), p_tracker->front()->angle()) <
                       getDeltaAngle(rhs->angle(), p_tracker->front()->angle());
            });
            p_tracker->update(closest_rune);
            combo_set.erase(closest_rune);
        }
        // 没有匹配到的神符作为新的序列，创建新的 tracker
        for (auto p_combo : combo_set)
            trackers.emplace_back(RuneTracker::make_tracker(p_combo));
    }
    // 如果当前帧识别到的神符数量 < 序列数量
    else if (combos.size() < trackers.size())
    {
        // 初始化哈希表
        std::unordered_set<tracker::ptr> tracker_set(trackers.begin(), trackers.end());
        for (auto p_combo : combos)
        {
            // 离 active_rune 最近的 tracker
            tracker::ptr closest_tracker = *min_element(trackers.begin(), trackers.end(), [&](tracker::const_ptr lhs, tracker::const_ptr rhs) {
                return getDeltaAngle(p_combo->angle(), lhs->front()->angle()) <
                       getDeltaAngle(p_combo->angle(), rhs->front()->angle());
            });
            closest_tracker->update(p_combo);
            tracker_set.erase(closest_tracker);
        }
        // 没有匹配到的序列则执行丢帧操作
        for (auto p_tracker : tracker_set)
            p_tracker->update(_tick, _gyro_data);
    }
    // 如果当前帧识别到的装甲板数量 = 序列数量
    else
    {
        // 初始化哈希表
        std::unordered_set<combo::ptr> combo_set(combos.begin(), combos.end());
        //! @note 防止出现迭代器非法化的情况，此处使用下标访问
        size_t trackers_size = trackers.size();
        for (size_t i = 0; i < trackers_size; i++)
        {
            // 离 tracker 最近的 p_combo
            combo::ptr closest_combo = *min_element(combo_set.begin(), combo_set.end(), [&](combo::const_ptr lhs, combo::const_ptr rhs) {
                return getDeltaAngle(lhs->angle(), trackers[i]->front()->angle()) <
                       getDeltaAngle(rhs->angle(), trackers[i]->front()->angle());
            });
            // 获取角度差
            float min_delta_angle = getDeltaAngle(closest_combo->angle(), trackers[i]->front()->angle());
            // 判断是否角度差过大
            if (min_delta_angle < 50.f)
                trackers[i]->update(closest_combo);
            else
            {
                trackers[i]->update(_tick, _gyro_data);
                trackers.emplace_back(RuneTracker::make_tracker(closest_combo));
            }
            combo_set.erase(closest_combo);
        }
    }
}

} // namespace rm

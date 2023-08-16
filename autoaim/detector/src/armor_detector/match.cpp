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
#include "rmvlpara/tracker/armor_tracker.h"

using namespace rm;
using namespace para;
using namespace std;
using namespace cv;

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
inline bool isChange(const combo_ptr &t_combo, const combo_ptr &combo, float dis)
{
    return ((t_combo->getAngle() * combo->getAngle() < -80.f) || dis > armor_detector_param.MAX_TRACKER_DELTA_DIS);
}

void ArmorDetector::match(vector<group_ptr> &groups, vector<combo_ptr> &combos)
{
    // combo 匹配 tracker
    matchArmors(groups.front()->data(), combos);
    // 删除因数字识别判断出的伪装甲板序列
    // eraseFakeTracker(groups.front()->data());
    // 删除 tracker
    eraseNullTracker(groups.front()->data());
}

void ArmorDetector::matchArmors(vector<tracker_ptr> &trackers, vector<combo_ptr> &combos)
{
    // 如果 trackers 为空先为每个识别到的 combo 开辟序列
    if (trackers.empty())
    {
        for (const auto &p_combo : combos)
            trackers.emplace_back(ArmorTracker::make_tracker(p_combo));
        return;
    }

    // 如果当前帧识别到的装甲板数量 > 序列数量
    if (combos.size() > trackers.size())
    {
        // 初始化装甲板集合
        unordered_set<combo_ptr> armor_set(combos.begin(), combos.end());
        // 距离最近的装甲板匹配到相应的序列中, 并 update
        for (auto p_tracker : trackers)
        {
            // 离 p_tracker 最近的 combo 及其距离
            auto min_it = min_element(combos.begin(), combos.end(),
                                      [&p_tracker](combo_ptr &lhs, combo_ptr &rhs)
                                      {
                                          return getDistance(lhs->getCenter(), p_tracker->front()->getCenter()) <
                                                 getDistance(rhs->getCenter(), p_tracker->front()->getCenter());
                                      });
            p_tracker->update(*min_it, _tick, _gyro_data);
            armor_set.erase(*min_it);
        }
        // 没有匹配到的装甲板作为新的序列
        for (const auto &p_combo : armor_set)
            trackers.emplace_back(ArmorTracker::make_tracker(p_combo));
    }
    // 如果当前帧识别到的装甲板数量 < 序列数量
    else if (combos.size() < trackers.size())
    {
        // 初始化追踪器集合
        unordered_set<tracker_ptr> tracker_set(trackers.begin(), trackers.end());
        for (const auto &p_combo : combos)
        {
            // 离 armor 最近的 tracker 及其距离
            auto min_dis_tracker =
                min_element(trackers.begin(), trackers.end(),
                            [&p_combo](tracker_ptr &lhs, tracker_ptr &rhs)
                            {
                                return getDistance(p_combo->getCenter(), lhs->front()->getCenter()) <
                                       getDistance(p_combo->getCenter(), rhs->front()->getCenter());
                            });
            min_dis_tracker->get()->update(p_combo, _tick, _gyro_data);
            tracker_set.erase(*min_dis_tracker);
        }
        // 没有匹配到的序列传入 nullptr
        for (const auto &p_tracker : tracker_set)
            p_tracker->update(nullptr, _tick, _gyro_data);
    }
    // 如果当前帧识别到的装甲板数量 = 序列数量
    else
    {
        // 初始化装甲板集合
        unordered_set<combo_ptr> armor_set(combos.begin(), combos.end());
        // 防止出现迭代器非法化的情况，此处使用下标访问
        size_t before_size = trackers.size(); // 存储原始 trackers 大小
        for (size_t i = 0; i < before_size; i++)
        {
            // 离 tracker 最近的 combo
            auto min_it =
                min_element(armor_set.begin(), armor_set.end(),
                            [&trackers, &i](const combo_ptr &lhs, const combo_ptr &rhs)
                            {
                                return getDistance(lhs->getCenter(), trackers[i]->front()->getCenter()) <
                                       getDistance(rhs->getCenter(), trackers[i]->front()->getCenter());
                            });
            // 最短距离
            float min_dis = getDistance(min_it->get()->getCenter(), trackers[i]->front()->getCenter());
            // 判断是否突变
            if (isChange(trackers[i]->front(), *min_it, min_dis))
            {
                // 创建新序列，原来的序列打入 nullptr
                trackers[i]->update(nullptr, _tick, _gyro_data);
                trackers.emplace_back(ArmorTracker::make_tracker(*min_it));
            }
            else
                trackers[i]->update(*min_it, _tick, _gyro_data);
            armor_set.erase(*min_it);
        }
    }
}

void ArmorDetector::eraseNullTracker(vector<tracker_ptr> &trackers)
{
    // 删除
    trackers.erase(remove_if(trackers.begin(), trackers.end(),
                             [&](tracker_ptr &p_tracker)
                             {
                                 return p_tracker->getVanishNumber() >= armor_tracker_param.TRACK_FRAMES;
                             }),
                   trackers.end());
}

void ArmorDetector::eraseFakeTracker(vector<tracker_ptr> &trackers)
{
    // 删除
    trackers.erase(remove_if(trackers.begin(), trackers.end(),
                             [&](tracker_ptr &t1)
                             {
                                 return t1->getType().RobotTypeID == RobotType::UNKNOWN;
                             }),
                   trackers.end());
}

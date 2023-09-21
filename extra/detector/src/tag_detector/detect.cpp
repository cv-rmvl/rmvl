/**
 * @file detect.cpp
 * @author zhaoxi (535394140@qq.com)
 * @brief
 * @version 1.0
 * @date 2023-05-08
 *
 * @copyright Copyright 2023 (c), zhaoxi
 *
 */

#include <unordered_set>

#include <opencv2/imgproc.hpp>

#include "rmvl/detector/tag_detector.h"
#include "rmvl/feature/tag.h"
#include "rmvl/tracker/planar_tracker.h"

#include "rmvlpara/detector/tag_detector.h"
#include "rmvlpara/tracker/planar_tracker.h"

#include "tag25h9.h"

using namespace para;
using namespace std;
using namespace cv;
using namespace rm;

TagDetector::TagDetector()
{
    _tf = tag25h9_create();
    _td = apriltag_detector_create();
    // 初始化 AprilTag 检测器
    apriltag_detector_add_family(_td, _tf);
}

TagDetector::~TagDetector()
{
    apriltag_detector_destroy(_td);
    tag25h9_destroy(_tf);
}

DetectInfo TagDetector::detect(vector<group::ptr> &groups, Mat &src, PixChannel,
                               const GyroData &gyro_data, int64 tick)
{
    DetectInfo info;
    info.src = src;
    _tick = tick;
    _gyro_data = gyro_data;
    // 初始化存储信息
    if (groups.empty())
        groups.emplace_back(DefaultGroup::make_group());

    cvtColor(src, info.gray, COLOR_BGR2GRAY);
    vector<vector<Point2f>> corners;
    vector<TagType> types;

    // 格式转换
    image_u8_t apriltag_img = {info.gray.cols,
                               info.gray.rows,
                               info.gray.cols,
                               info.gray.data};

    // 检测 AprilTag
    zarray_t *detections = apriltag_detector_detect(_td, &apriltag_img);
    int target_size = zarray_size(detections);
    corners.resize(target_size);
    types.resize(target_size);
    for (int i = 0; i < target_size; i++)
    {
        apriltag_detection_t *det = nullptr;
        zarray_get(detections, i, &det);
        corners[i].resize(4);
        for (int j = 0; j < 4; j++)
            corners[i][j] = Point2f(static_cast<float>(det->p[j][0]),
                                    static_cast<float>(det->p[j][1]));
        types[i] = static_cast<TagType>(det->id + 1);
    }

    // 释放资源
    apriltag_detections_destroy(detections);

    // 构造
    if (corners.size() != types.size())
        RMVL_Error(RMVL_StsBadSize, "Size of the corners and type are not equal");
    size_t feature_size = corners.size();
    info.features.reserve(feature_size);
    info.combos.reserve(feature_size);
    for (size_t i = 0; i < feature_size; ++i)
    {
        auto tag = Tag::make_feature(corners[i], types[i]);
        if (tag == nullptr)
            continue;
        info.features.push_back(tag);
        auto p_combo = DefaultCombo::make_combo(tag, _tick);
        if (p_combo == nullptr)
            continue;
        info.combos.push_back(p_combo);
    }

    group::ptr p_group = groups.front();
    auto &trackers = p_group->data();
    // combo 匹配 tracker
    match(trackers, info.combos);
    // 删除 tracker
    eraseNullTracker(trackers);
    return info;
}

/**
 * @brief 判断是否突变
 * @note 如果 Tag 在一帧内位置变化特别大, 且速度发生较大突变，则创建新序列
 *
 * @param dis 测出的最小间距
 * @return 是否突变
 */
inline bool isChange(float dis) { return dis > tag_detector_param.MAX_TRACKER_DELTA_DIS; }

void TagDetector::match(vector<tracker::ptr> &trackers, const vector<combo::ptr> &combos)
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
        unordered_set<combo::ptr> tag_set(combos.begin(), combos.end());
        // 距离最近的装甲板匹配到相应的序列中, 并 update
        for (auto p_tracker : trackers)
        {
            // 离 p_tracker 最近的 combo 及其距离
            auto min_it = min_element(combos.begin(), combos.end(),
                                      [&p_tracker](combo::ptr lhs, combo::ptr rhs) {
                                          return getDistance(lhs->getCenter(), p_tracker->front()->getCenter()) <
                                                 getDistance(rhs->getCenter(), p_tracker->front()->getCenter());
                                      });
            p_tracker->update(*min_it, _tick, _gyro_data);
            tag_set.erase(*min_it);
        }
        // 没有匹配到的装甲板作为新的序列
        for (const auto &p_combo : tag_set)
            trackers.emplace_back(PlanarTracker::make_tracker(p_combo));
    }
    // 如果当前帧识别到的装甲板数量 < 序列数量
    else if (combos.size() < trackers.size())
    {
        // 初始化追踪器集合
        unordered_set<tracker::ptr> tracker_set(trackers.begin(), trackers.end());
        for (const auto &p_combo : combos)
        {
            // 离 tag 最近的 tracker 及其距离
            auto min_dis_tracker =
                min_element(trackers.begin(), trackers.end(),
                            [&p_combo](tracker::ptr lhs, tracker::ptr rhs) {
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
        unordered_set<combo::ptr> tag_set(combos.begin(), combos.end());
        // 防止出现迭代器非法化的情况，此处使用下标访问
        size_t before_size = trackers.size(); // 存储原始 trackers 大小
        for (size_t i = 0; i < before_size; i++)
        {
            // 离 tracker 最近的 combo
            auto min_it =
                min_element(tag_set.begin(), tag_set.end(),
                            [&trackers, &i](const combo::ptr &combo_1, const combo::ptr &combo_2) -> bool {
                                return getDistance(combo_1->getCenter(), trackers[i]->front()->getCenter()) <
                                       getDistance(combo_2->getCenter(), trackers[i]->front()->getCenter());
                            });
            // 最短距离
            float min_dis = getDistance(min_it->get()->getCenter(), trackers[i]->front()->getCenter());
            // 判断是否突变
            //! @todo 这段掉帧处理需要增加其他信息，保证 tracker 的匹配正确
            if (isChange(min_dis))
            {
                // 创建新序列，原来的序列打入 nullptr
                trackers[i]->update(nullptr, _tick, _gyro_data);
                trackers.emplace_back(PlanarTracker::make_tracker(*min_it));
            }
            else
                trackers[i]->update(*min_it, _tick, _gyro_data);
            tag_set.erase(*min_it);
        }
    }
}

void TagDetector::eraseNullTracker(vector<tracker::ptr> &trackers)
{
    // 删除
    trackers.erase(remove_if(trackers.begin(), trackers.end(),
                             [&](tracker::ptr &p_tracker) {
                                 return p_tracker->getVanishNumber() >= planar_tracker_param.TRACK_FRAMES;
                             }),
                   trackers.end());
}
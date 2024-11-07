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

namespace rm
{

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

DetectInfo TagDetector::detect(std::vector<group::ptr> &groups, const cv::Mat &src, PixChannel, const ImuData &imu_data, double tick)
{
    DetectInfo info;
    info.src = src;
    _tick = tick;
    _imu_data = imu_data;
    // 初始化存储信息
    if (groups.empty())
        groups.emplace_back(DefaultGroup::make_group());

    cvtColor(src, info.gray, cv::COLOR_BGR2GRAY);
    std::vector<std::array<cv::Point2f, 4>> corners;
    std::vector<TagType> types;

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
        for (int j = 0; j < 4; j++)
            corners[i][j] = cv::Point2f(static_cast<float>(det->p[j][0]),
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
static inline bool isChange(float dis) { return dis > para::tag_detector_param.MAX_TRACKER_DELTA_DIS; }

void TagDetector::match(std::vector<tracker::ptr> &trackers, const std::vector<combo::ptr> &combos)
{
    // 如果 trackers 为空先为每个识别到的 combo 开辟序列
    if (trackers.empty())
    {
        for (const auto &p_combo : combos)
            trackers.emplace_back(PlanarTracker::make_tracker(p_combo));
        return;
    }

    // 如果当前帧识别到的视觉标签 `rm::Tag` 数量 > 序列数量
    if (combos.size() > trackers.size())
    {
        // 初始化视觉标签 `rm::Tag` 集合
        std::unordered_set<combo::ptr> tag_set(combos.begin(), combos.end());
        // 距离最近的视觉标签 `rm::Tag` 匹配到相应的序列中, 并 update
        for (auto p_tracker : trackers)
        {
            // 离 p_tracker 最近的 combo 及其距离
            auto min_it = min_element(combos.begin(), combos.end(), [&](combo::const_ptr lhs, combo::const_ptr rhs) {
                return getDistance(lhs->center(), p_tracker->front()->center()) <
                       getDistance(rhs->center(), p_tracker->front()->center());
            });
            p_tracker->update(*min_it);
            tag_set.erase(*min_it);
        }
        // 没有匹配到的视觉标签 `rm::Tag` 作为新的序列
        for (const auto &p_combo : tag_set)
            trackers.emplace_back(PlanarTracker::make_tracker(p_combo));
    }
    // 如果当前帧识别到的视觉标签 `rm::Tag` 数量 < 序列数量
    else if (combos.size() < trackers.size())
    {
        // 初始化追踪器集合
        std::unordered_set<tracker::ptr> tracker_set(trackers.begin(), trackers.end());
        for (const auto &p_combo : combos)
        {
            // 离视觉标签最近的 tracker 及其距离
            auto min_dis_tracker = *min_element(trackers.begin(), trackers.end(), [&](tracker::const_ptr lhs, tracker::const_ptr rhs) {
                return getDistance(p_combo->center(), lhs->front()->center()) <
                       getDistance(p_combo->center(), rhs->front()->center());
            });
            min_dis_tracker->update(p_combo);
            tracker_set.erase(min_dis_tracker);
        }
        // 没有匹配到的序列传入 nullptr
        for (auto p_tracker : tracker_set)
            p_tracker->update(_tick, _imu_data);
    }
    // 如果当前帧识别到的视觉标签 `rm::Tag` 数量 = 序列数量
    else
    {
        // 初始化视觉标签 `rm::Tag` 集合
        std::unordered_set<combo::ptr> tag_set(combos.begin(), combos.end());
        // 防止出现迭代器非法化的情况，此处使用下标访问
        size_t before_size = trackers.size(); // 存储原始 trackers 大小
        for (size_t i = 0; i < before_size; i++)
        {
            // 离 tracker 最近的 combo
            auto min_it = *min_element(tag_set.begin(), tag_set.end(), [&](combo::const_ptr combo_1, combo::const_ptr combo_2) {
                return getDistance(combo_1->center(), trackers[i]->front()->center()) <
                       getDistance(combo_2->center(), trackers[i]->front()->center());
            });
            // 最短距离
            float min_dis = getDistance(min_it->center(), trackers[i]->front()->center());
            // 判断是否突变
            //! @todo 这段掉帧处理需要增加其他信息，保证 tracker 的匹配正确
            if (isChange(min_dis))
            {
                // 创建新序列，原来的序列打入 nullptr
                trackers[i]->update(_tick, _imu_data);
                trackers.emplace_back(PlanarTracker::make_tracker(min_it));
            }
            else
                trackers[i]->update(min_it);
            tag_set.erase(min_it);
        }
    }
}

void TagDetector::eraseNullTracker(std::vector<tracker::ptr> &trackers)
{
    // 删除
    trackers.erase(remove_if(trackers.begin(), trackers.end(), [](tracker::const_ptr p_tracker) {
                       return p_tracker->getVanishNumber() >= para::planar_tracker_param.TRACK_FRAMES;
                   }),
                   trackers.end());
}

} // namespace rm

/**
 * @file detect.cpp
 * @author RoboMaster Vision Community
 * @brief detect
 * @version 1.0
 * @date 2021-09-21
 *
 * @copyright Copyright 2021 (c), RoboMaster Vision Community
 *
 */

#include "rmvl/algorithm/pretreat.hpp"
#include "rmvl/detector/rune_detector.h"
#include "rmvl/group/rune_group.h"

#include "rmvlpara/detector/rune_detector.h"

namespace rm {

RuneDetectorInfo RuneDetector::detect(group::ptr &group, const cv::Mat &src, uint8_t color, const ImuData &imu_data, double tick) {
    RuneDetectorInfo info{};
    info.src = src;
    _tick = tick;
    _imu_data = imu_data;
    // 初始化存储信息
    if (group == nullptr)
        group = RuneGroup::make_group();
    // 二值化处理图像
    PixChannel ch_minus = color == RED ? BLUE : RED;
    int thesh = color == RED ? para::rune_detector_param.GRAY_THRESHOLD_RED : para::rune_detector_param.GRAY_THRESHOLD_BLUE;
    info.bin = binary(info.src, color, ch_minus, thesh);
    // 寻找神符
    find(info.bin, info.features, info.combos);
    // 匹配
    auto &rune_trackers = group->data();
    match(rune_trackers, info.combos);

    // 完成原始角度数据同步
    if (rune_trackers.empty())
        group = RuneGroup::make_group();
    else
        group->sync(_imu_data, _tick);

    return info;
}

} // namespace rm

/**
 * @file detect.cpp
 * @author RoboMaster Vision Community
 * @brief detect
 * @version 1.0
 * @date 2021-08-18
 *
 * @copyright Copyright 2021 (c), RoboMaster Vision Community
 *
 */

#include "rmvl/detector/armor_detector.h"

#include "rmvlpara/detector/armor_detector.h"

namespace rm
{

DetectInfo ArmorDetector::detect(std::vector<group::ptr> &groups, const cv::Mat &src, PixChannel color, const ImuData &imu_data, double tick)
{
    DetectInfo info{};
    info.src = src;
    _tick = tick;
    _imu_data = imu_data;
    // 初始化存储信息
    if (groups.empty())
        groups.emplace_back(DefaultGroup::make_group());
    // 二值化处理图像
    PixChannel ch_minus = color == RED ? BLUE : RED;
    int thesh = color == RED ? para::armor_detector_param.GRAY_THRESHOLD_RED : para::armor_detector_param.GRAY_THRESHOLD_BLUE;
    info.bin = binary(src, color, ch_minus, thesh);

    // 找到所有的灯条和装甲板
    find(info.bin, info.features, info.combos, info.rois);
    // 将目标匹配进序列组
    match(groups, info.combos);
    return info;
}

} // namespace rm

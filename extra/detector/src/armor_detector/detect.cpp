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

#include "rmvl/algorithm/pretreat.hpp"
#include "rmvl/detector/armor_detector.h"

#include "rmvlpara/detector/armor_detector.h"

namespace rm {

ArmorDetector::ArmorDetector(std::string_view model) {
    _ort = std::make_unique<ClassificationNet>(model);
    for (int i = 0; i < 9; ++i)
        _robot_t[i] = static_cast<RobotType>(i);
}

ArmorDetectorInfo ArmorDetector::detect(std::vector<tracker::ptr> &trackers, const cv::Mat &src, uint8_t color, const ImuData &imu_data, double tick) {
    ArmorDetectorInfo info{};
    info.src = src;
    _tick = tick;
    _imu_data = imu_data;
    // 二值化处理图像
    PixChannel ch_minus = color == RED ? BLUE : RED;
    int thesh = color == RED ? para::armor_detector_param.GRAY_THRESHOLD_RED : para::armor_detector_param.GRAY_THRESHOLD_BLUE;
    info.bin = binary(src, color, ch_minus, thesh);

    // 找到所有的灯条和装甲板
    find(info.bin, info.features, info.combos, info.rois);
    // 将目标匹配进时间序列
    match(trackers, info.combos);
    return info;
}

} // namespace rm

/**
 * @file detect.cpp
 * @author RoboMaster Vision Community
 * @brief
 * @version 0.1
 * @date 2022-12-10
 *
 * @copyright Copyright 2022 (c), RoboMaster Vision Community
 *
 */

#include "rmvl/detector/gyro_detector.h"

#include "rmvlpara/detector/gyro_detector.h"

namespace rm {

GyroDetector::GyroDetector(std::string_view model, int armor_num) : _armor_num(armor_num) {
    _ort = std::make_unique<ClassificationNet>(model);
    for (int i = 0; i < 9; ++i)
        _robot_t[i] = static_cast<RobotType>(i);
}

DetectInfo GyroDetector::detect(std::vector<group::ptr> &groups, const cv::Mat &src, uint8_t color, const ImuData &imu_data, double tick) {
    // 识别信息
    DetectInfo info{};
    info.src = src;
    _tick = tick;
    _imu_data = imu_data;

    // 二值化处理图像
    PixChannel ch_minus = color == RED ? BLUE : RED;
    int thesh = color == RED ? para::gyro_detector_param.GRAY_THRESHOLD_RED : para::gyro_detector_param.GRAY_THRESHOLD_BLUE;
    info.bin = binary(info.src, color, ch_minus, thesh);

    // 找到所有的灯条和装甲板
    find(info.bin, info.features, info.combos, info.rois);
    // 将目标匹配进序列组
    match(groups, info.combos);
    return info;
}

} // namespace rm

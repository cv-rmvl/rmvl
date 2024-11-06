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
#include "rmvl/group/gyro_group.h"

#include "rmvlpara/detector/gyro_detector.h"

namespace rm
{

DetectInfo GyroDetector::detect(std::vector<group::ptr> &groups, cv::Mat &src, PixChannel color,
                                const ImuData &imu_data, double tick)
{
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

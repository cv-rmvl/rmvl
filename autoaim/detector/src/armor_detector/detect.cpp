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

using namespace rm;
using namespace para;
using namespace std;
using namespace cv;

DetectInfo ArmorDetector::detect(vector<group_ptr> &groups, Mat &src, PixChannel color,
                                 const GyroData &gyro_data, int64 record_time)
{
    DetectInfo info{};
    info.src = src;
    _tick = record_time;
    _gyro_data = gyro_data;
    // 初始化存储信息
    if (groups.empty())
        groups.emplace_back(DefaultGroup::make_group());
    // 二值化处理图像
    PixChannel ch_minus = color == RED ? BLUE : RED;
    int thesh = color == RED ? armor_detector_param.GRAY_THRESHOLD_RED : armor_detector_param.GRAY_THRESHOLD_BLUE;
    info.bin = rm::binary(src, color, ch_minus, thesh);

    // 找到所有的灯条和装甲板
    find(info.bin, info.features, info.combos);
    // 将目标匹配进序列组
    match(groups, info.combos);
    return info;
}

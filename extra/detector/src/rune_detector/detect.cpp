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

#include "rmvl/detector/rune_detector.h"
#include "rmvl/group/rune_group.h"

#include "rmvlpara/detector/rune_detector.h"

using namespace rm;
using namespace para;
using namespace std;
using namespace cv;

DetectInfo RuneDetector::detect(vector<group::ptr> &groups, Mat &src, PixChannel color,
                                const GyroData &gyro_data, int64 record_time)
{
    if (groups.size() > 1)
        RMVL_Error(RMVL_StsBadArg, "Size of the argument \"groups\" is greater than 1");
    DetectInfo info{};
    info.src = src;
    _tick = record_time;
    _gyro_data = gyro_data;
    // 初始化存储信息
    if (groups.empty())
        groups.emplace_back(RuneGroup::make_group());
    auto rune_group = groups.front();
    // 二值化处理图像
    PixChannel ch_minus = color == RED ? BLUE : RED;
    int thesh = color == RED ? rune_detector_param.GRAY_THRESHOLD_RED : rune_detector_param.GRAY_THRESHOLD_BLUE;
    info.bin = binary(info.src, color, ch_minus, thesh);
    // 寻找神符
    find(info.bin, info.features, info.combos);
    // 匹配
    auto &rune_trackers = rune_group->data();
    match(rune_trackers, info.combos);

    // 完成原始角度数据同步
    if (rune_trackers.empty())
        groups = {RuneGroup::make_group()};
    else
        rune_group->sync(_gyro_data, _tick);

    return info;
}

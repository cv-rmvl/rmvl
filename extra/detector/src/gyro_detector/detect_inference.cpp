/**
 * @file inference.cpp
 * @author RoboMaster Vision Community
 * @brief 熵权法推理
 * @version 0.1
 * @date 2023-01-17
 *
 * @copyright Copyright 2023 (c), RoboMaster Vision Community
 *
 */

#include "rmvl/algorithm/math.hpp"
#include "rmvl/algorithm/pretreat.hpp"
#include "rmvl/detector/gyro_detector.h"
#include "rmvl/group/gyro_group.h"

#include "rmvlpara/detector/gyro_detector.h"

namespace rm {

GyroDetector::GyroDetector(std::string_view model, int armor_num) : _armor_num(armor_num) {
    _ort = std::make_unique<ClassificationNet>(model);
    for (int i = 0; i < 9; ++i)
        _robot_t[i] = static_cast<RobotType>(i);
}

GyroDetectorInfo GyroDetector::detect(std::vector<group::ptr> &groups, const cv::Mat &src, uint8_t color, const ImuData &imu_data, double tick) {
    // 识别信息
    GyroDetectorInfo info{};
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

std::unordered_map<size_t, size_t> GyroDetector::ewTopsisInference(group::ptr group, const std::vector<combo::ptr> &combos) {
    auto trackers = group->data();
    auto pGyroGroup = GyroGroup::cast(group);
    if (trackers.empty() || combos.empty())
        return {};
    // (a) 生成样本指标矩阵，（指标：距离，角度差）
    std::vector<std::vector<double>> samples(trackers.size() * combos.size());
    for (size_t c = 0; c < combos.size(); ++c) {
        for (size_t t = 0; t < trackers.size(); ++t) {
            // 2 表示两个指标
            samples[c * trackers.size() + t].resize(4);
            for (int i = 0; i < 4; i++)
                samples[c * trackers.size() + t][i] = -getDistance(combos[c]->corners()[i], trackers[t]->front()->corners()[i]);
        }
    }
    // (b) 运用熵权法推理
    EwTopsis ew(samples);
    auto arr = ew.inference();
    // (c) 数据导出并提取出指定的下标
    std::unordered_map<size_t, size_t> target;
    target.reserve(combos.size());
    // 每个 combos 都在 trackers 找到得分最高的一个作为目标
    for (size_t i = 0; i < combos.size(); ++i) {
        target[i] = (max_element(arr.begin() + trackers.size() * i,
                                 arr.begin() + trackers.size() * (i + 1)) -
                     arr.begin()) %
                    trackers.size();
    }
    return target;
}

} // namespace rm

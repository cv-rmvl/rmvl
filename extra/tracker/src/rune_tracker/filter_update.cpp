/**
 * @file filter_update.cpp
 * @author RoboMaster Vision Community
 *         zhaoxi
 * @brief 神符追踪器滤波处理
 * @version 1.0
 * @date 2022-08-25
 *
 * @copyright Copyright 2021 (c), RoboMaster Vision Community
 *
 */

#include "rmvl/tracker/rune_tracker.h"

#include "rmvlpara/tracker/rune_tracker.h"

namespace rm
{

void RuneTracker::initFilter(float init_angle, float init_speed)
{
    auto first_combo = _combo_deque.front();

    // 初始化旋转滤波器
    _filter.setR({para::rune_tracker_param.ROTATE_R});
    _filter.setQ(para::rune_tracker_param.ROTATE_Q);
    _filter.init({init_angle, init_speed}, 1e-2);
}

void RuneTracker::updateRotateFilter(float t)
{
    _filter.setA({1, t,
                  0, 1});
    // 预测
    _filter.predict();
    // 更新
    cv::Matx21f result = _filter.correct({_angle});
    _rotated_speed = result(1);
}

} // namespace rm

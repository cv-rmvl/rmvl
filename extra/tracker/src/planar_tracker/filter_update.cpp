/**
 * @file filter_update.cpp
 * @author RoboMaster Vision Community
 * @brief 平面目标追踪器 - 滤波更新
 * @version 1.0
 * @date 2022-08-24
 *
 * @copyright Copyright 2021 (c), RoboMaster Vision Community
 *
 */

#include "rmvl/tracker/planar_tracker.h"

#include "rmvlpara/tracker/planar_tracker.h"

namespace rm
{

void PlanarTracker::initFilter()
{
    combo::ptr first_combo = _combo_deque.front();
    // 初始化距离滤波器
    _distance_filter.setR({para::planar_tracker_param.DIS_R});
    _distance_filter.setQ(para::planar_tracker_param.DIS_Q);
    cv::Matx21f init_dis_vec = {first_combo->extrinsics().distance(), 0};
    _distance_filter.init(init_dis_vec, 1e5f);
    // 初始化运动滤波器
    _motion_filter.setR(para::planar_tracker_param.MOTION_R);
    _motion_filter.setQ(para::planar_tracker_param.MOTION_Q);
    cv::Matx41f init_move_vec = {first_combo->getRelativeAngle().x,
                                 first_combo->getRelativeAngle().y,
                                 0, 0};
    _motion_filter.init(init_move_vec, 1e5f);
}

void PlanarTracker::updateDistanceFilter()
{
    // 设置状态转移矩阵
    _distance_filter.setA({1, 1,
                           0, 1});
    float current_distance = _combo_deque.front()->extrinsics().distance();
    // 预测
    _distance_filter.predict();
    // 更新
    cv::Matx21f correct_vec = _distance_filter.correct({current_distance});
    _extrinsic.distance(correct_vec(0));
}

void PlanarTracker::updateMotionFilter()
{
    // 采样时间
    float t = 0.f;
    if (_combo_deque.size() >= 2)
        t = (_combo_deque.front()->tick() - _combo_deque.back()->tick()) / static_cast<double>(_combo_deque.size() - 1);
    else
        t = para::planar_tracker_param.SAMPLE_INTERVAL / 1000.;
    // 设置状态转移矩阵
    _motion_filter.setA({1, 0, t, 0,
                         0, 1, 0, t,
                         0, 0, 1, 0,
                         0, 0, 0, 1});
    // 预测
    _motion_filter.predict();
    // 更新
    cv::Matx41f xk = _motion_filter.correct(front()->getRelativeAngle());
    _relative_angle = {xk(0), xk(1)}; // 相对角度滤波数值
    _speed = {xk(2), xk(3)};          // 绝对速度滤波数值
}

} // namespace rm

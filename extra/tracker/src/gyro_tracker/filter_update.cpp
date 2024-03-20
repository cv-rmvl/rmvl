/**
 * @file filter_update.cpp
 * @author RoboMaster Vision Community
 * @brief
 * @version 0.1
 * @date 2023-04-16
 *
 * @copyright Copyright 2023 (c), RoboMaster Vision Community
 *
 */

#include "rmvl/tracker/gyro_tracker.h"

#include "rmvlpara/combo/armor.h"
#include "rmvlpara/tracker/gyro_tracker.h"

namespace rm
{

void GyroTracker::initFilter()
{
    auto first_combo = _combo_deque.front();
    // 初始化运动滤波器 相对角度和角速度
    _motion_filter.setR(para::gyro_tracker_param.MOTION_R);
    _motion_filter.setQ(para::gyro_tracker_param.MOTION_Q);
    const auto &relative_angle = first_combo->getRelativeAngle();
    cv::Matx41f init_move_vec = {relative_angle.x, relative_angle.y, 0, 0};
    _motion_filter.init(init_move_vec, 1e-2);
    // 初始化位置滤波器
    _center3d_filter.setR(para::gyro_tracker_param.POSITION_R);
    _center3d_filter.setQ(para::gyro_tracker_param.POSITION_Q);
    const auto &tvec = first_combo->getExtrinsics().tvec();
    cv::Matx61f init_position_vec = {tvec(0), tvec(1), tvec(2), 0, 0, 0};
    _center3d_filter.init(init_position_vec, 1e-2);
    // 初始化姿态滤波器
    _pose_filter.setR(para::gyro_tracker_param.POSE_R);
    _pose_filter.setQ(para::gyro_tracker_param.POSE_Q);
    const auto &pose = Armor::cast(first_combo)->getPose();
    cv::Vec4f init_pose_vec = {pose(0), pose(1), 0, 0};
    _pose_filter.init(init_pose_vec, 1e-2);
}

void GyroTracker::updateMotionFilter()
{
    float t = _duration;
    // 设置状态转移矩阵
    _motion_filter.setA({1, 0, t, 0,
                         0, 1, 0, t,
                         0, 0, 1, 0,
                         0, 0, 0, 1});
    // 预测
    _motion_filter.predict();
    // 更新
    auto result = _motion_filter.correct(front()->getRelativeAngle());
    _relative_angle = {result(0), result(1)}; // 相对角度滤波数值
    _speed = {result(2), result(3)};          // 相对速度滤波数值
}

void GyroTracker::updatePositionFilter()
{
    float t = _duration;
    // 设置状态转移矩阵
    _center3d_filter.setA({1, 0, 0, t, 0, 0,
                           0, 1, 0, 0, t, 0,
                           0, 0, 1, 0, 0, t,
                           0, 0, 0, 1, 0, 0,
                           0, 0, 0, 0, 1, 0,
                           0, 0, 0, 0, 0, 1});
    // 预测
    _center3d_filter.predict();
    // 更新
    cv::Vec3f tvec = at(0)->getExtrinsics().tvec();
    cv::Matx61f correct_position = _center3d_filter.correct(tvec);
    _extrinsic.tvec({correct_position(0), correct_position(1), correct_position(2)});
}

void GyroTracker::updatePoseFilter()
{
    float t = _duration;
    // 设置状态转移矩阵
    _pose_filter.setA({1, 0, t, 0,
                       0, 1, 0, t,
                       0, 0, 1, 0,
                       0, 0, 0, 1});
    // 预测
    _pose_filter.predict();
    // 更新
    cv::Vec2f pose = Armor::cast(front())->getPose();
    cv::Matx41f correct_pose = _pose_filter.correct(pose);
    _pose = {correct_pose(0), correct_pose(1)};
}

} // namespace rm

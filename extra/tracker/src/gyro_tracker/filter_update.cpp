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
    // 初始化位置滤波器
    _center3d_filter.setR(para::gyro_tracker_param.POSITION_R);
    _center3d_filter.setQ(para::gyro_tracker_param.POSITION_Q);
    const auto &tvec = first_combo->extrinsic().tvec();
    cv::Matx61f init_position_vec = {tvec(0), tvec(1), tvec(2), 0, 0, 0};
    _center3d_filter.init(init_position_vec, 1e5f);
    // 初始化姿态滤波器
    _pose_filter.setR(para::gyro_tracker_param.POSE_R);
    _pose_filter.setQ(para::gyro_tracker_param.POSE_Q);
    const auto &pose = Armor::cast(first_combo)->getPose();
    cv::Vec4f init_pose_vec = {pose(0), pose(1), 0, 0};
    _pose_filter.init(init_pose_vec, 1e5f);
}

void GyroTracker::updatePositionFilter()
{
    float t{_duration};
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
    cv::Vec3f tvec = at(0)->extrinsic().tvec();
    cv::Matx61f correct_position = _center3d_filter.correct(tvec);
    _extrinsic.tvec({correct_position(0), correct_position(1), correct_position(2)});
}

void GyroTracker::updatePoseFilter()
{
    float t{_duration};
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

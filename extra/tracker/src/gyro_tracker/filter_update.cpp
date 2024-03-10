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

#include <opencv2/calib3d.hpp>

#include "rmvl/tracker/gyro_tracker.h"

#include "rmvlpara/combo/armor.h"
#include "rmvlpara/tracker/gyro_tracker.h"

namespace rm
{

void GyroTracker::initFilter()
{
    auto first_combo = _combo_deque.front();
    // 初始化运动滤波器 相对角度和角速度
    _motion_filter.setR(para::gyro_tracker_param.MOTION_R); // 距离为两帧差，误差为两倍
    _motion_filter.setQ(para::gyro_tracker_param.MOTION_Q); // 距离的过程噪声仔细调整
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
    // Set the state transition matrix: A
    _motion_filter.setA(cv::Matx44f{
        1, 0, t, 0,
        0, 1, 0, t,
        0, 0, 1, 0,
        0, 0, 0, 1});
    // Kalman predict
    _motion_filter.predict();
    // 获取陀螺仪角速度
    cv::Point2f gyro_speed = {at(0)->getGyroData().rotation.yaw_speed,
                              at(0)->getGyroData().rotation.pitch_speed};
    // 相对角速度
    cv::Point2f relative_speed;
    if (_combo_deque.size() >= 4U)
    {
        relative_speed = (_combo_deque[0]->getRelativeAngle() +
                          _combo_deque[1]->getRelativeAngle() -
                          _combo_deque[2]->getRelativeAngle() -
                          _combo_deque[3]->getRelativeAngle()) /
                         (4.f * t);
    }
    // 队列长度不够长，则在获取速度时选择首尾取平均值
    else if (size() > 1 && size() < 4)
    {
        relative_speed = (front()->getRelativeAngle() -
                          back()->getRelativeAngle()) /
                         static_cast<float>(size() - 1) / t;
    }
    else
        relative_speed = -gyro_speed; // 确保获取的绝对速度为 0
    // 绝对速度
    _speed = relative_speed + gyro_speed;

    cv::Matx41f result = _motion_filter.correct({front()->getRelativeAngle().x, // 位置_x
                                                 front()->getRelativeAngle().y, // 位置_y
                                                 _speed.x,                      // 速度_x
                                                 _speed.y});                    // 速度_y

    _relative_angle = {result(0), result(1)}; // 相对角度滤波数值
    _speed = {result(2), result(3)};          // 绝对速度滤波数值
}

void GyroTracker::updatePositionFilter()
{
    float t = _duration;
    // 设置状态转移矩阵
    _center3d_filter.setA(cv::Matx66f{1, 0, 0, t, 0, 0,
                                      0, 1, 0, 0, t, 0,
                                      0, 0, 1, 0, 0, t,
                                      0, 0, 0, 1, 0, 0,
                                      0, 0, 0, 0, 1, 0,
                                      0, 0, 0, 0, 0, 1});
    // 距离变化速度
    cv::Vec3f delta_tvec = 0.f;
    if (_combo_deque.size() >= 4)
        delta_tvec = (at(0)->getExtrinsics().tvec() + at(1)->getExtrinsics().tvec() -
                      at(2)->getExtrinsics().tvec() - at(3)->getExtrinsics().tvec()) /
                     (4.f * t);
    else if (_combo_deque.size() > 1 && _combo_deque.size() < 4)
        delta_tvec = (front()->getExtrinsics().tvec() - back()->getExtrinsics().tvec()) /
                     (static_cast<float>(_combo_deque.size() - 1) * t);
    // 预测
    _center3d_filter.predict();
    // 更新
    cv::Vec3f tvec = at(0)->getExtrinsics().tvec();
    cv::Matx61f correct_position = _center3d_filter.correct({tvec[0], tvec[1], tvec[2],
                                                             delta_tvec[0], delta_tvec[1], delta_tvec[2]});
    cv::Vec3f correct_tvec = {correct_position(0), correct_position(1), correct_position(2)};
    _extrinsic.tvec(correct_tvec);
}

void GyroTracker::updatePoseFilter()
{
    float t = _duration;
    // 设置状态转移矩阵
    _pose_filter.setA(cv::Matx44f{1, 0, t, 0,
                                  0, 1, 0, t,
                                  0, 0, 1, 0,
                                  0, 0, 0, 1});
    // 姿态变化速度
    cv::Vec2f dpose = (Armor::cast(at(0))->getPose() - Armor::cast(at(1))->getPose()) / t;
    // 预测
    _pose_filter.predict();
    // 更新
    cv::Vec2f pose = Armor::cast(front())->getPose();
    cv::Matx41f correct_pose = _pose_filter.correct({pose(0), pose(1), dpose(0), dpose(1)});
    _pose = {correct_pose(0), correct_pose(1)};
}

} // namespace rm

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

using namespace rm;
using namespace para;
using namespace std;
using namespace cv;

void PlanarTracker::initFilter()
{
    combo::ptr first_combo = _combo_deque.front();
    // 初始化距离滤波器
    _distance_filter.setR(Matx22f::diag({0.01, 0.02})); // 距离为两帧差，误差为两倍
    _distance_filter.setQ(Matx22f::diag({0.01, 0.01})); // 距离的过程噪声随意调整
    Matx21f init_dis_vec = {first_combo->getExtrinsics().distance(), 0};
    _distance_filter.init(init_dis_vec, 1e-2);
    // 初始化运动滤波器，相对角度和角速度
    _motion_filter.setR(planar_tracker_param.R); // 距离为两帧差，误差为两倍
    _motion_filter.setQ(planar_tracker_param.Q); // 距离的过程噪声仔细调整
    Matx41f init_move_vec = {first_combo->getRelativeAngle().x,
                             first_combo->getRelativeAngle().y,
                             0, 0};
    _motion_filter.init(init_move_vec, 1e-2);
}

void PlanarTracker::updateDistanceFilter()
{
    // 设置状态转移矩阵
    _distance_filter.setA(Matx22f{1, 1,
                                  0, 1});
    // 距离变化速度 / 差分
    float current_distance = _combo_deque.front()->getExtrinsics().distance();
    float delta_distance = current_distance - _last_distance;
    // 预测
    _distance_filter.predict();
    // 更新
    Matx21f correct_vec = _distance_filter.correct({current_distance,
                                                    delta_distance});
    _extrinsic.distance(correct_vec(0));
    _last_distance = correct_vec(0);
}

void PlanarTracker::updateMotionFilter()
{
    // 帧差时间
    float t = 0.f;
    if (_combo_deque.size() >= 2)
        t = (_combo_deque.front()->getTick() - _combo_deque.back()->getTick()) / static_cast<double>(_combo_deque.size() - 1);
    else
        t = planar_tracker_param.SAMPLE_INTERVAL / 1000.;
    // Set the state transition matrix: A
    _motion_filter.setA(Matx44f{
        1, 0, t, 0,
        0, 1, 0, t,
        0, 0, 1, 0,
        0, 0, 0, 1});
    // Kalman predict
    _motion_filter.predict();
    // 获取陀螺仪角速度
    Point2f gyro_speed = Point2f(at(0)->getGyroData().rotation.yaw_speed,
                                 at(0)->getGyroData().rotation.pitch_speed);
    // 相对角速度
    Point2f relative_speed;
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

    Matx41f result = _motion_filter.correct({front()->getRelativeAngle().x, // 位置_x
                                             front()->getRelativeAngle().y, // 位置_y
                                             _speed.x,                      // 速度_x
                                             _speed.y});                    // 速度_y

    _relative_angle = {result(0), result(1)}; // 相对角度滤波数值
    _speed = {result(2), result(3)};          // 绝对速度滤波数值
}

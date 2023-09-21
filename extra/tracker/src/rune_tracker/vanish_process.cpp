/**
 * @file vanish_process.cpp
 * @author RoboMaster Vision Community
 * @brief 神符追踪器-掉帧处理文件
 * @version 1.0
 * @date 2022-08-25
 *
 * @copyright Copyright 2021 (c), RoboMaster Vision Community
 *
 */

#include "rmvl/rmath/transform.h"
#include "rmvl/tracker/rune_tracker.h"

#include "rmvlpara/camera.hpp"
#include "rmvlpara/tracker/rune_tracker.h"

using namespace rm;
using namespace para;
using namespace std;
using namespace cv;

/**
 * @brief 强制构造神符
 *
 * @param[in] ref_rune 参考神符组合体
 * @param[in] delta_angle 基于参考神符的角度增量（角度制）
 * @param[in] tick 最新时间戳
 * @param[in] gyro_data 最新陀螺仪数据
 * @return 构造出的神符
 */
rune_ptr runeConstructForced(rune_ptr ref_rune, float delta_angle, int64_t tick, const GyroData &gyro_data)
{
    auto p_rune_target = RuneTarget::cast(ref_rune->at(0));
    auto p_rune_center = RuneCenter::cast(ref_rune->at(1));
    // 获取中心点
    auto old_target = p_rune_target->getCenter();
    auto old_center = p_rune_center->getCenter();
    // 绕 center 旋转
    Vec2f target_vec(old_target.x - old_center.x,
                     old_target.y - old_center.y);
    Matx22f rot = {cos(deg2rad(delta_angle)), sin(deg2rad(delta_angle)),
                   -sin(deg2rad(delta_angle)), cos(deg2rad(delta_angle))};
    target_vec = rot * target_vec;
    // 获取新的神符中心图像中心点
    auto old_gyro_angle = Point2f(ref_rune->getGyroData().rotation.yaw,
                                  ref_rune->getGyroData().rotation.pitch);
    auto new_gyro_angle = Point2f(gyro_data.rotation.yaw, gyro_data.rotation.pitch);
    // -(new_gyro_angle - old_gyro_angle)
    auto dpoint = calculateRelativeCenter(camera_param.cameraMatrix, old_gyro_angle - new_gyro_angle) -
                  calculateRelativeCenter(camera_param.cameraMatrix, {});
    // 获取新的神符靶心图像中心点
    auto new_center = old_center + dpoint;
    auto new_target = new_center + Point2f(target_vec);
    // 强制构造
    auto p_new_rune_center = RuneCenter::make_feature(new_center);
    auto p_new_rune_target = RuneTarget::make_feature(new_target, p_rune_target->isActive());
    return Rune::make_combo(p_new_rune_target, p_new_rune_center, gyro_data, tick, true);
}

void RuneTracker::vanishProcess(int64 tick, const GyroData &gyro_data)
{
    // 判空
    if (_combo_deque.empty() || _vanish_num == 0)
        return;
    // 获取帧差时间
    float t = 0.f;
    if (_combo_deque.size() >= 2)
        t = (_combo_deque.front()->getTick() - _combo_deque.back()->getTick()) /
            static_cast<double>(_combo_deque.size() - 1) / getTickFrequency();
    else
        t = rune_tracker_param.SAMPLE_INTERVAL / 1000.f;
    _filter.setA(Matx22f{1, t,
                         0, 1});
    // 旋转状态先验估计
    auto rotate_pre = _filter.predict();

    rune_ptr p_rune = runeConstructForced(Rune::cast(_combo_deque.front()),
                                          rotate_pre(0) - _angle, tick, gyro_data);
    _angle = p_rune->getAngle();

    float rad_angle = deg2rad(p_rune->getAngle());
    float feature_dis = getDistance(p_rune->at(0)->getCenter(),
                                    p_rune->at(1)->getCenter());
    Point2f relative_center = {p_rune->at(1)->getCenter().x + cos(rad_angle) * feature_dis,
                               p_rune->at(1)->getCenter().y - sin(rad_angle) * feature_dis};

    _relative_angle = calculateRelativeAngle(camera_param.cameraMatrix, relative_center);
    // 直接更新后验估计
    _filter.correct(rotate_pre);
    _combo_deque.emplace_front(p_rune);
}

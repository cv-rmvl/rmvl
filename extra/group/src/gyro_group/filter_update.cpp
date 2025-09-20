/**
 * @file filter_update.cpp
 * @author RoboMaster Vision Community
 * @brief
 * @version 0.1
 * @date 2023-04-17
 *
 * @copyright Copyright 2023 (c), RoboMaster Vision Community
 *
 */

#include "rmvl/group/gyro_group.h"

#include "rmvlpara/group/gyro_group.h"

namespace rm {

void GyroGroup::updateCenter3DFilter(cv::Vec3f center, float t, cv::Vec3f &update_center, cv::Vec3f &update_speed) {
    // 设置状态转移矩阵
    _center3d_filter.setA({1, 0, 0, t, 0, 0,
                           0, 1, 0, 0, t, 0,
                           0, 0, 1, 0, 0, t,
                           0, 0, 0, 1, 0, 0,
                           0, 0, 0, 0, 1, 0,
                           0, 0, 0, 0, 0, 1});
    // -------------------- 预测 --------------------
    _center3d_filter.predict();
    // -------------------- 更新 --------------------
    cv::Matx61f correct_val = _center3d_filter.correct(center);
    update_center = {correct_val(0), correct_val(1), correct_val(2)};
    update_speed = {correct_val(3), correct_val(4), correct_val(5)};
}

void GyroGroup::updateRotationFilter(float rotspeed, float &update_rotspeed) {
    // 更新角度信息
    _rotspeed_deq.push_front(rotspeed);
    if (_rotspeed_deq.size() > para::gyro_group_param.ROTSPEED_SIZE)
        _rotspeed_deq.pop_back();
    // 此处采用最简单的移动平均滤波
    update_rotspeed = 0;
    for (auto val : _rotspeed_deq)
        update_rotspeed += val;
    update_rotspeed /= static_cast<float>(_rotspeed_deq.size());
}

} // namespace rm

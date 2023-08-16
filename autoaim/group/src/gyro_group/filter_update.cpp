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

#include "rmvlpara/tracker/gyro_tracker.h"

using namespace para;
using namespace rm;
using namespace std;
using namespace cv;

void GyroGroup::updateCenter3DFilter(cv::Vec3f center, float t, cv::Vec3f &update_center, cv::Vec3f &update_speed)
{
    // 更新位置信息
    _center3d_deq.push_front(center);
    if (_center3d_deq.size() > 16)
        _center3d_deq.pop_back();
    // 设置状态转移矩阵
    _center3d_filter.setA(Matx66f{1, 0, 0, t, 0, 0,
                                   0, 1, 0, 0, t, 0,
                                   0, 0, 1, 0, 0, t,
                                   0, 0, 0, 1, 0, 0,
                                   0, 0, 0, 0, 1, 0,
                                   0, 0, 0, 0, 0, 1});
    // -------------------- 预测 --------------------
    _center3d_filter.predict();
    // -------------------- 更新 --------------------
    // 获取速度
    if (_center3d_deq.empty())
        RMVL_Error(RMVL_StsBadSize, "\"_center3d_deq\" is empty");
    size_t center3d_num = _center3d_deq.size();
    Vec3f speed = center3d_num >= 4U
                      ? (_center3d_deq[0] + _center3d_deq[1] - _center3d_deq[2] - _center3d_deq[3]) / (4.f * t)
                      // 队列长度不够长，则在获取速度时选择首尾取平均值
                      : (_center3d_deq.front() - _center3d_deq.back()) / static_cast<float>(center3d_num - 1) / t;
    Matx61f correct_val = _center3d_filter.correct({center(0), center(1), center(2),
                                                     speed(0), speed(1), speed(2)});
    update_center = {correct_val(0), correct_val(1), correct_val(2)};
    update_speed = {correct_val(3), correct_val(4), correct_val(5)};
}

void GyroGroup::updateRotationFilter(float rotspeed, float &update_rotspeed)
{
    // 更新角度信息
    _rotspeed_deq.push_front(rotspeed);
    if (_rotspeed_deq.size() > gyro_group_param.ROTSPEED_SIZE)
        _rotspeed_deq.pop_back();
    // 此处采用最简单的移动平均滤波
    update_rotspeed = 0;
    for (auto val : _rotspeed_deq)
        update_rotspeed += val;
    update_rotspeed /= static_cast<float>(_rotspeed_deq.size());
}

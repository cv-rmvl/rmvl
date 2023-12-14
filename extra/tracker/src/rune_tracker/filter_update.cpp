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

using namespace cv;
using namespace std;
using namespace para;
using namespace rm;

void RuneTracker::initFilter(float init_angle, float init_speed)
{
    auto first_combo = _combo_deque.front();

    // 初始化旋转滤波器
    _filter.setR(rune_tracker_param.ROTATE_R);
    _filter.setQ(rune_tracker_param.ROTATE_Q);
    Matx21f init_rotate_vec = {init_angle, init_speed};
    _filter.init(init_rotate_vec, 1e-2);
}

void RuneTracker::updateRotateFilter(float t)
{
    // 匀速运动模型
    _filter.setA(Matx22f{1, t,
                         0, 1});
    auto pre_vec = _filter.predict();
    // 神符速度默认为卡尔曼上次根据过往数据的预测结果
    float raw_speed = pre_vec(1);
    // 更新速度
    if (_angles.size() > 1)
    {
        if (_angles.size() >= 4)
            raw_speed = (_angles[0] + _angles[1] - _angles[2] - _angles[3]) / (4 * t);
        else
            raw_speed = (_angles.front() - _angles.back()) / (static_cast<float>(_angles.size() - 1) * t);
    }

    Matx21f result = _filter.correct({_angle, raw_speed});
    // 更新速度
    _rotated_speed = result(1);
}

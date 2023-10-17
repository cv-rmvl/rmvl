/**
 * @file PlanarTracker.cpp
 * @author RoboMaster Vision Community
 * @brief 平面目标追踪器
 * @version 1.0
 * @date 2021-08-21
 *
 * @copyright Copyright 2021 (c), RoboMaster Vision Community
 *
 */

#include "rmvl/tracker/planar_tracker.h"

#include "rmvlpara/tracker/planar_tracker.h"

using namespace para;
using namespace std;
using namespace cv;
using namespace rm;

void PlanarTracker::updateData(combo::ptr p_combo)
{
    _height = p_combo->getHeight();
    _width = p_combo->getHeight();
    _angle = p_combo->getHeight();
    _corners = p_combo->getCorners();
    _center = p_combo->getCenter();
    _extrinsic = p_combo->getExtrinsics();
}

PlanarTracker::PlanarTracker(combo::ptr p_combo)
{
    if (p_combo == nullptr)
        RMVL_Error(RMVL_StsBadArg, "Pointer of the input argument combo::ptr is null pointer");
    _combo_deque.emplace_front(p_combo);
    _type = p_combo->getType(); // tracker 状态初始化
    _type_deque.emplace_front(_type);
    initFilter();
    _relative_angle = p_combo->getRelativeAngle();
    updateData(p_combo);
}

void PlanarTracker::update(combo::ptr p_combo, double tick, const GyroData &gyro_data)
{
    if (p_combo == nullptr)
    {
        ++_vanish_num;
        vanishProcess(tick, gyro_data);
    }
    else
    {
        updateData(p_combo);
        _combo_deque.emplace_front(p_combo);
        // 更新状态
        updateType(p_combo->getType());
        // 重置丢失帧数
        _vanish_num = 0;

        // 更新距离 KF
        updateDistanceFilter();
        // 更新平面运动轨迹 KF
        updateMotionFilter();
    }
    if (_combo_deque.size() >= 12U)
        _combo_deque.pop_back();
}


void PlanarTracker::updateType(RMStatus stat)
{
    _type = stat;
    // 仅更新 RobotType
    _type_deque.push_back(stat);
    if (_type_deque.size() > 12)
        _type_deque.pop_back();
    if (_type_deque.size() < 12)
        _type = calculateModeNum(_type_deque.begin(), _type_deque.end());
}

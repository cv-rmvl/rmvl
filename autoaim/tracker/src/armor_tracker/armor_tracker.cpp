/**
 * @file ArmorTracker.cpp
 * @author RoboMaster Vision Community
 * @brief 装甲板追踪器
 * @version 1.0
 * @date 2021-08-21
 *
 * @copyright Copyright 2021 (c), RoboMaster Vision Community
 *
 */

#include "rmvl/tracker/armor_tracker.h"

#include "rmvlpara/tracker/armor_tracker.h"

using namespace cv;
using namespace std;
using namespace para;
using namespace rm;

void ArmorTracker::updateData(const combo_ptr &p_combo)
{
    _height = p_combo->getHeight();
    _width = p_combo->getHeight();
    _angle = p_combo->getHeight();
    _corners = p_combo->getCorners();
    _center = p_combo->getCenter();
    _pnp_data = p_combo->getPNP();
}

ArmorTracker::ArmorTracker(const combo_ptr &p_armor)
{
    if (p_armor == nullptr)
        RMVL_Error(RMVL_StsBadArg, "Pointer of the input argument combo_ptr is null pointer");
    _combo_deque.emplace_front(p_armor);
    _type = p_armor->getType(); // tracker状态初始化
    _type_deque.emplace_front(_type.RobotTypeID);
    initFilter();
    _relative_angle = p_armor->getRelativeAngle();
    updateData(p_armor);
}

void ArmorTracker::update(combo_ptr p_armor, int64 tick, const GyroData &gyro_data)
{
    if (p_armor == nullptr)
    {
        ++_vanish_num;
        vanishProcess(tick, gyro_data);
    }
    else
    {
        updateData(p_armor);
        _combo_deque.emplace_front(p_armor);
        // Update the type of the armor
        updateType(p_armor->getType());
        // Reset the vanish number
        _vanish_num = 0;
        // Update as normal if the p_armor isn't nullptr
        // Update the distance kalman filter
        updateDistanceFilter();
        // Update the motion kalman filter
        updateMotionFilter();
    }
    if (_combo_deque.size() >= 12U)
        _combo_deque.pop_back();
}

void ArmorTracker::updateType(RMStatus stat)
{
    _type = stat;
    _type_deque.push_back(stat.RobotTypeID);
    if (_type_deque.size() > 12)
        _type_deque.pop_back();
    if (_type_deque.size() < 12)
        _type.RobotTypeID = calculateModeNum(_type_deque.begin(), _type_deque.end());
}
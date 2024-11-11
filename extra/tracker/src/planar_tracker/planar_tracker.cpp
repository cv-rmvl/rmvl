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

namespace rm
{

void PlanarTracker::updateData(combo::ptr p_combo)
{
    _height = p_combo->height();
    _width = p_combo->height();
    _angle = p_combo->height();
    _corners = p_combo->corners();
    _center = p_combo->center();
    _extrinsic = p_combo->extrinsic();
}

PlanarTracker::PlanarTracker(combo::ptr p_combo)
{
    if (p_combo == nullptr)
        RMVL_Error(RMVL_StsBadArg, "Pointer of the input argument combo::ptr is null pointer");
    _combo_deque.emplace_front(p_combo);
    _type = p_combo->type(); // tracker 状态初始化
    _type_deque.emplace_front(_type);
    initFilter();
    _relative_angle = p_combo->getRelativeAngle();
    updateData(p_combo);
}

bool PlanarTracker::invalid() const { return _vanish_num >= para::planar_tracker_param.TRACK_FRAMES; }

tracker::ptr PlanarTracker::clone()
{
    auto retval = std::make_shared<PlanarTracker>(*this);
    // 更新内部所有组合体
    for (auto &p_combo : retval->_combo_deque)
        p_combo = p_combo->clone(p_combo->tick());
    return retval;
}

void PlanarTracker::update(combo::ptr p_combo)
{
    if (p_combo == nullptr)
        RMVL_Error(RMVL_StsBadArg, "Pointer of the input argument combo::ptr is nullptr");

    updateData(p_combo);
    _combo_deque.emplace_front(p_combo);
    // 更新状态
    updateType(p_combo->type());
    // 重置丢失帧数
    _vanish_num = 0;

    // 更新距离 KF
    updateDistanceFilter();
    // 更新平面运动轨迹 KF
    updateMotionFilter();

    if (_combo_deque.size() >= 12U)
        _combo_deque.pop_back();
}

void PlanarTracker::update(double tick, [[maybe_unused]] const ImuData &imu_data)
{
    if (_combo_deque.empty())
        return;
    _vanish_num++;
    _combo_deque.push_back(_combo_deque.front()->clone(tick));
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

} // namespace rm

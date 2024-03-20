/**
 * @file default_tracker.cpp
 * @author RoboMaster Vision Community
 * @brief 默认追踪器
 * @version 1.0
 * @date 2023-09-17
 *
 * @copyright Copyright 2023 (c), RoboMaster Vision Community
 *
 */

#include "rmvl/tracker/tracker.h"

void rm::DefaultTracker::updateData(rm::combo::ptr p_combo)
{
    _height = p_combo->getHeight();
    _width = p_combo->getHeight();
    _angle = p_combo->getHeight();
    _corners = p_combo->getCorners();
    _center = p_combo->getCenter();
    _extrinsic = p_combo->getExtrinsics();
}

rm::DefaultTracker::DefaultTracker(rm::combo::ptr p_combo) : rm::tracker()
{
    if (p_combo == nullptr)
        RMVL_Error(RMVL_StsBadArg, "Pointer of the input argument \"combo::ptr\" is null pointer");
    _combo_deque = {p_combo};
    updateData(p_combo);
}

void rm::DefaultTracker::update(rm::combo::ptr p_combo)
{
    updateData(p_combo);
    _combo_deque.push_front(p_combo);
    if (_combo_deque.size() > 32U)
        _combo_deque.pop_back();
    _vanish_num = 0;
}

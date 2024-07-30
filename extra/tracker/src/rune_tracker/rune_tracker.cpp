/**
 * @file RuneTracker.cpp
 * @author RoboMaster Vision Community
 * @brief 神符追踪器
 * @version 1.0
 * @date 2021-09-21
 *
 * @copyright Copyright 2021 (c), RoboMaster Vision Community
 *
 */

#include "rmvl/tracker/rune_tracker.h"
#include "rmvlpara/tracker/rune_tracker.h"

namespace rm
{

void RuneTracker::updateFromRune(combo::ptr p_combo)
{
    _width = p_combo->getWidth();
    _height = p_combo->getHeight();
    _corners = p_combo->getCorners();
    _extrinsic = p_combo->getExtrinsics();
    _center = p_combo->getCenter();
    _type = p_combo->getType();
    _relative_angle = p_combo->getRelativeAngle();
}

RuneTracker::RuneTracker(combo::ptr p_rune)
{
    if (p_rune == nullptr)
        RMVL_Error(RMVL_StsBadArg, "Pointer of the input argument combo::ptr is null pointer");
    _combo_deque.emplace_front(p_rune);
    initFilter(p_rune->getAngle(), 0.f);
    _angle = p_rune->getAngle();
    _center = p_rune->getCenter();
    updateFromRune(p_rune);
}

tracker::ptr RuneTracker::clone()
{
    auto retval = std::make_shared<RuneTracker>(*this);
    // 更新内部所有组合体
    for (auto &p_combo : retval->_combo_deque)
        p_combo = p_combo->clone(p_combo->getTick());
    return retval;
}

void RuneTracker::update(combo::ptr p_rune)
{
    if (p_rune == nullptr)
        RMVL_Error(RMVL_StsBadArg, "Pointer of the input argument combo::ptr is nullptr");
    else
    {
        _combo_deque.push_front(p_rune);
        // 数据更新
        updateFromRune(p_rune);
        // 更新神符转动的圈数，并计算在考虑圈数时的完全值
        _angle = calculateTotalAngle();
        // 更新滤波器
        float t = 0.f;
        if (_combo_deque.size() >= 2)
            t = (_combo_deque.front()->getTick() - _combo_deque.back()->getTick()) / static_cast<double>(_combo_deque.size() - 1);
        else
            t = para::rune_tracker_param.SAMPLE_INTERVAL / 1000.;
        updateRotateFilter(t);
        // 重置消失帧数
        _vanish_num = 0;
    }
    if (_combo_deque.size() >= 8U)
        _combo_deque.pop_back();
}

float RuneTracker::calculateTotalAngle()
{
    // 若当前容器 size < 2 则圈数为默认0
    if (_combo_deque.size() < 2)
        return front()->getAngle();
    float current_angle = _combo_deque.at(0)->getAngle();
    float last_angle = _combo_deque.at(1)->getAngle();
    // 角度判断，计算圈数
    if (current_angle > 135.f && last_angle < -135.f) // 顺时针
        _round--;
    else if (current_angle < -135.f && last_angle > 135.f) // 逆时针
        _round++;

    // 角度范围修正至 (-180, 180] -> 循环修正
    while (fabs(current_angle) > 180.f)
        current_angle += (current_angle > 0.f) ? -360.f : 360.f;
    // 更新角度
    auto p_rune = Rune::cast(_combo_deque.front());
    RMVL_DbgAssert(p_rune != nullptr);
    return current_angle + 360.f * _round;
}

} // namespace rm

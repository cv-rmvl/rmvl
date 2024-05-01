/**
 * @file filter_update.cpp
 * @author RoboMaster Vision Community
 * @brief 平面目标追踪器 - 掉帧处理
 * @version 1.0
 * @date 2022-08-24
 *
 * @copyright Copyright 2021 (c), RoboMaster Vision Community
 *
 */

#include "rmvl/tracker/planar_tracker.h"

namespace rm
{

void PlanarTracker::update(double tick, [[maybe_unused]] const GyroData &gyro_data)
{
    if (_combo_deque.empty())
        return;
    _vanish_num++;
    _combo_deque.push_back(_combo_deque.front()->clone(tick));
}

} // namespace rm

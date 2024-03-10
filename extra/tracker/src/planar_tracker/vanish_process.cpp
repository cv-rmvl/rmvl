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

#include "rmvlpara/camera/camera.h"
#include "rmvlpara/tracker/planar_tracker.h"

namespace rm
{

void PlanarTracker::vanishProcess([[maybe_unused]] double tick, [[maybe_unused]] const GyroData &gyro_data)
{
    if (_combo_deque.empty() || _vanish_num == 0)
        return;
    //! @note 后续完成 #10 后再进行完善
    combo::ptr combo = _combo_deque.front();

    // //////////////////////// 构造新对象 ////////////////////////
    // // 补帧 combo 中心点
    // // Point2f vanish_center = calculateRelativeCenter(camera_param.cameraMatrix, _relative_angle);
    // // Point2f delta = vanish_center - last_combo->getCenter();
    // vector<Point2f> last_corners = last_combo->getCorners();

    // // Force to build 'LightBlob' object
    // LightBlob::ptr p_left = LightBlob::make_feature(last_corners[1],
    //                                                 last_corners[0],
    //                                                 last_combo->at(0)->getWidth());
    // LightBlob::ptr p_right = LightBlob::make_feature(last_corners[2],
    //                                                  last_corners[3],
    //                                                  last_combo->at(1)->getWidth());
    // combo::ptr combo = Armor::make_combo(p_left, p_right, gyro_data, tick, last_combo->getType().ArmorSizeTypeID);
    // // Correct
    // // _motion_filter.correct(result);
    _combo_deque.emplace_back(combo);
}

} // namespace rm

/**
 * @file filter_update.cpp
 * @author RoboMaster Vision Community
 * @brief 装甲板追踪器掉帧处理
 * @version 1.0
 * @date 2022-08-24
 *
 * @copyright Copyright 2021 (c), RoboMaster Vision Community
 *
 */

#include "rmvl/tracker/armor_tracker.h"
#include "rmvlpara/camera.hpp"
#include "rmvlpara/tracker/armor_tracker.h"

using namespace rm;
using namespace para;
using namespace std;
using namespace cv;

void ArmorTracker::vanishProcess(int64_t tick, const GyroData &gyro_data)
{
    if (_combo_deque.empty() || _vanish_num == 0)
        return;
    combo_ptr &last_armor = _combo_deque.front();

    //////////////////////// 构造新对象 ////////////////////////
    // 补帧 combo 中心点
    // Point2f vanish_center = calculateRelativeCenter(camera_param.cameraMatrix, _relative_angle);
    // Point2f delta = vanish_center - last_armor->getCenter();
    vector<Point2f> last_corners = last_armor->getCorners();

    // Force to build 'LightBlob' object
    light_blob_ptr p_left = LightBlob::make_feature(last_corners[1],
                                             last_corners[0],
                                             last_armor->at(0)->getWidth());
    light_blob_ptr p_right = LightBlob::make_feature(last_corners[2],
                                              last_corners[3],
                                              last_armor->at(1)->getWidth());
    armor_ptr armor = Armor::make_combo(p_left, p_right, gyro_data, tick, last_armor->getType().ArmorSizeTypeID);
    // Correct
    // _motion_filter.correct(result);
    _combo_deque.emplace_back(armor);
}

/**
 * @file rune.cpp
 * @author RoboMaster Vision Community
 * @brief
 * @version 0.1
 * @date 2023-01-21
 *
 * @copyright Copyright 2023 (c), RoboMaster Vision Community
 *
 */

#include "rmvl/combo/rune.h"
#include "rmvl/feature/rune/rune_logging.h"
#include "rmvl/rmath/transform.h"

#include "rmvlpara/camera.hpp"
#include "rmvlpara/combo/rune.h"

using namespace cv;
using namespace rm;
using namespace std;
using namespace para;

rune_ptr Rune::make_combo(const rune_target_ptr &p_target, const rune_center_ptr &p_center,
                          const GyroData &gyro_data, int64 tick, bool force)
{
    // ------------------------------【判空】------------------------------
    if (p_target == nullptr || p_center == nullptr)
        return nullptr;
    if (!force)
    {
        // ----------------------【特征面积之比是否合适】----------------------
        float rune_area_ratio = p_target->getArea() / p_center->getArea();
        DEBUG_RUNE_INFO_("rune 1.rune_area_ratio : %f", rune_area_ratio);
        DEBUG_RUNE_INFO_("rune 1.rune_active : %d", p_target->isActive());
        if (rune_area_ratio < rune_param.MIN_AREA_RATIO ||
            (!p_target->isActive() && rune_area_ratio > rune_param.MAX_AREA_RATIO))
        {
            DEBUG_RUNE_WARNING_("rune 1.rune_area_ratio : fail");
            return nullptr;
        }
        DEBUG_RUNE_PASS_("rune 1.rune_area_ratio : pass");
        // ------------------------【特征间距是否合适】------------------------
        // 使用神符装甲板的长宽信息代表组合体的长宽信息
        float rune_radius = getDistance(p_target->getCenter(), p_center->getCenter());
        float radius_ratio = rune_radius / p_center->getHeight();
        DEBUG_RUNE_INFO_("rune 2.radius_ratio : %f", radius_ratio);
        if (radius_ratio < rune_param.MIN_RADIUS_RATIO ||
            radius_ratio > rune_param.MAX_RADIUS_RATIO)
        {
            DEBUG_RUNE_WARNING_("rune 2.rune_radius_ratio : fail");
            return nullptr;
        }
        DEBUG_RUNE_PASS_("rune 2.rune_radius_ratio : pass");
    }

    return make_shared<Rune>(p_target, p_center, gyro_data, tick);
}

Rune::Rune(const rune_target_ptr &p_target, const rune_center_ptr &p_center, const GyroData &gyro_data, int64 tick)
{
    _gyro_data = gyro_data;
    _width = p_target->getWidth() + p_center->getWidth() + getDistance(p_target->getCenter(), p_center->getCenter());
    _height = p_target->getHeight();
    // ------------- 获取神符中心 -------------
    _center = p_target->getCenter();
    // ------------- 获取激活信息 -------------
    _is_active = p_target->isActive();
    // ----------- 计算相对目标转角 -----------
    _relative_angle = calculateRelativeAngle(camera_param.cameraMatrix, _center);
    // ------------- 计算直线距离 -------------
    // 计算 pitch 的绝对目标转角
    double absolute_angle = _relative_angle.y + _gyro_data.rotation.pitch;
    // 计算距离
    _pnp_data.distance(rune_param.RUNE_DISTANCE * sec(deg2rad(absolute_angle)));
    // ------------- 更新神符角度 -------------
    auto angle_vec = Rune::cameraConvertToVertical(p_target->getCenter() - p_center->getCenter(),
                                                   gyro_data.rotation.pitch);
    _angle = getHAngle(p_center->getCenter(), Point2f(angle_vec) + p_center->getCenter(), DEG);
    // ----------- 更新神符特征间距 -----------
    _feature_dis = getDistance(angle_vec, Vec2f{});
    // ---------- 设置组合体特征指针 ----------
    _features = {p_target, p_center};
    // ---------- 更新组合体类型信息 ----------
    _type.RuneTypeID = _is_active ? RuneType::ACTIVE : RuneType::INACTIVE;
    _tick = tick;
}

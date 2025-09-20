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

#include "rmvl/algorithm/transform.hpp"

#include "rmvl/combo/rune.h"

#include "rmvlpara/camera/camera.h"
#include "rmvlpara/combo/rune.h"

namespace rm {

Rune::ptr Rune::make_combo(RuneTarget::ptr p_target, RuneCenter::ptr p_center,
                           const ImuData &imu_data, double tick, bool force) {
    // ------------------------------【判空】------------------------------
    if (p_target == nullptr || p_center == nullptr)
        return nullptr;
    if (!force) {
        // ----------------------【特征面积之比是否合适】----------------------
        float rune_area_ratio = p_target->area() / p_center->area();
        DEBUG_INFO_("rune 1.rune_area_ratio : %f", rune_area_ratio);
        DEBUG_INFO_("rune 1.rune_active : %d", p_target->isActive());
        if (rune_area_ratio < para::rune_param.MIN_AREA_RATIO ||
            (!p_target->isActive() && rune_area_ratio > para::rune_param.MAX_AREA_RATIO)) {
            DEBUG_WARNING_("rune 1.rune_area_ratio : fail");
            return nullptr;
        }
        DEBUG_PASS_("rune 1.rune_area_ratio : pass");
        // ------------------------【特征间距是否合适】------------------------
        // 使用神符装甲板的长宽信息代表组合体的长宽信息
        float rune_radius = getDistance(p_target->center(), p_center->center());
        float radius_ratio = rune_radius / p_center->height();
        DEBUG_INFO_("rune 2.radius_ratio : %f", radius_ratio);
        if (radius_ratio < para::rune_param.MIN_RADIUS_RATIO ||
            radius_ratio > para::rune_param.MAX_RADIUS_RATIO) {
            DEBUG_WARNING_("rune 2.rune_radius_ratio : fail");
            return nullptr;
        }
        DEBUG_PASS_("rune 2.rune_radius_ratio : pass");
    }

    return std::make_shared<Rune>(p_target, p_center, imu_data, tick);
}

Rune::Rune(RuneTarget::ptr p_target, RuneCenter::ptr p_center, const ImuData &imu_data, double tick) {
    // 通用信息
    _imu_data = imu_data;
    _width = p_target->width() + p_center->width() + getDistance(p_target->center(), p_center->center());
    _height = p_target->height();
    _center = p_target->center();
    _relative_angle = calculateRelativeAngle(para::camera_param.cameraMatrix, _center);
    // 计算 pitch 的绝对目标转角
    double absolute_angle = _relative_angle.y + _imu_data.rotation.pitch;
    // 计算距离
    _extrinsic.distance(para::rune_param.RUNE_DISTANCE * sec(deg2rad(absolute_angle)));
    // 神符角度
    auto angle_vec = Rune::cameraConvertToVertical(p_target->center() - p_center->center(), imu_data.rotation.pitch);
    _angle = getHAngle(p_center->center(), cv::Point2f(angle_vec) + p_center->center(), DEG);
    _features = {p_target, p_center};
    _corners = {p_target->center(), p_center->center()};

    // 专属信息
    _is_active = p_target->isActive();
    _feature_dis = getDistance(angle_vec, cv::Vec2f{});
    _state["rune"] = _is_active ? "active" : "inactive";
    _tick = tick;
}

combo::ptr Rune::clone(double tick) {
    auto retval = std::make_shared<Rune>(*this);
    // 更新内部所有特征
    for (std::size_t i = 0; i < _features.size(); ++i)
        retval->_features[i] = _features[i]->clone();
    // 更新时间戳
    retval->_tick = tick;
    return retval;
}

cv::Vec2f Rune::cameraConvertToVertical(const cv::Vec2f &angle_vec, float diff_theta) {
    return {angle_vec(0), angle_vec(1) * sec(deg2rad(diff_theta))};
}

cv::Vec2f Rune::verticalConvertToCamera(const cv::Vec2f &angle_vec, float diff_theta) {
    return {angle_vec(0), angle_vec(1) * std::cos(deg2rad(diff_theta))};
}

} // namespace rm

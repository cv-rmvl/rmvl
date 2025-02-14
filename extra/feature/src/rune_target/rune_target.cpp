/**
 * @file rune_target.cpp
 * @author RoboMaster Vision Community
 * @brief
 * @version 0.1
 * @date 2023-04-09
 *
 * @copyright Copyright 2023 (c), RoboMaster Vision Community
 *
 */

#include <opencv2/imgproc.hpp>

#include "rmvl/core/util.hpp"
#include "rmvl/feature/rune_target.h"

#include "rmvlpara/feature/rune_target.h"

namespace rm
{

std::shared_ptr<RuneTarget> RuneTarget::make_feature(const std::vector<cv::Point> &contour, bool is_active)
{
    if (contour.size() < 5)
        return nullptr;
    // init
    cv::RotatedRect rotated_rect = fitEllipse(contour);
    // ------------------------ 比例判断 ------------------------
    float width = std::max(rotated_rect.size.width, rotated_rect.size.height);
    float height = std::min(rotated_rect.size.width, rotated_rect.size.height);
    float ratio = width / height;
    DEBUG_INFO_("target 1.ratio : %f", ratio);
    if (ratio > para::rune_target_param.MAX_RATIO || ratio < para::rune_target_param.MIN_RATIO)
    {
        DEBUG_WARNING_("target 1.ratio : fail");
        return nullptr;
    }
    DEBUG_PASS_("target 1.ratio : pass");

    // ---------------------- 面积比例判断 ----------------------
    float contour_area = contourArea(contour);  // 轮廓面积
    float rect_area = rotated_rect.size.area(); // 矩形面积
    float area_ratio = contour_area / rect_area;
    DEBUG_INFO_("target 2.area_ratio : %f", area_ratio);
    if (area_ratio > para::rune_target_param.MAX_AREA_RATIO || area_ratio < para::rune_target_param.MIN_AREA_RATIO)
    {
        DEBUG_WARNING_("target 2.area_ratio : fail");
        return nullptr;
    }
    DEBUG_PASS_("target 2.area_ratio : pass");

    // -------------------- 面积周长比例判断 --------------------
    float perimeter = arcLength(contour, true);
    float area_peri_ratio = contour_area / (perimeter * perimeter);
    DEBUG_INFO_("target 3.area_peri_ratio : %f", area_peri_ratio);
    if (area_peri_ratio > para::rune_target_param.MAX_AREA_PERI_RATIO ||
        area_peri_ratio < para::rune_target_param.MIN_AREA_PERI_RATIO)
    {
        DEBUG_WARNING_("target 3.area_peri_ratio : fail");
        return nullptr;
    }
    DEBUG_PASS_("target 3.area_peri_ratio : pass");

    // ---------------------- 绝对面积判断 ----------------------
    DEBUG_INFO_("target 4.contour_area : %f", contour_area);
    if (contour_area < para::rune_target_param.MIN_AREA)
    {
        DEBUG_WARNING_("target 4.contour_area : fail");
        return nullptr;
    }
    DEBUG_PASS_("target 4.contour_area : pass");

    auto retval = std::make_shared<RuneTarget>();
    retval->_angle = 0.f;
    retval->_center = rotated_rect.center;
    retval->_width = std::max(rotated_rect.size.width, rotated_rect.size.height);
    retval->_height = std::min(rotated_rect.size.width, rotated_rect.size.height);

    std::vector<cv::Point2f> corners;
    corners.reserve(4);
    float radius = (retval->_height + retval->_width) / 4;
    retval->_radius = radius;
    corners.emplace_back(rotated_rect.center + cv::Point2f(-radius, radius));
    corners.emplace_back(rotated_rect.center + cv::Point2f(-radius, -radius));
    corners.emplace_back(rotated_rect.center + cv::Point2f(radius, -radius));
    corners.emplace_back(rotated_rect.center + cv::Point2f(radius, radius));
    retval->_corners = corners;
    retval->_is_active = is_active;

    return retval;
}

RuneTarget::RuneTarget(const cv::Point &center, bool is_active) : _is_active(is_active)
{
    _center = center;
    _angle = 0.f;
    _width = 16.f;
    _height = 16.f;
}

} // namespace rm

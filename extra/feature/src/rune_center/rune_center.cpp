/**
 * @file rune_center.cpp
 * @author RoboMaster Vision Community
 * @brief
 * @version 1.0
 * @date 2021-09-10
 *
 * @copyright Copyright 2021 (c), RoboMaster Vision Community
 *
 */

#include <opencv2/imgproc.hpp>

#include "rmvl/feature/rune_center.h"

#include "rmvlpara/feature/rune_center.h"

namespace rm
{

RuneCenter::RuneCenter(const cv::Point2f &center)
{
    // 初始化构造形状信息
    _center = center;
    _width = 20;
    _height = 20;
    _ratio = 1;
    // 更新角点信息
    _corners = {center + cv::Point2f(-10, 10), center + cv::Point2f(-10, -10),
                 center + cv::Point2f(10, -10), center + cv::Point2f(10, 10)};
}

RuneCenter::RuneCenter(const std::vector<cv::Point> &contour, cv::RotatedRect &rotated_rect) : _rotated_rect(rotated_rect)
{
    _contour = contour;
    _center = _rotated_rect.center;
    _width = std::max(_rotated_rect.size.width, _rotated_rect.size.height);
    _height = std::min(_rotated_rect.size.width, _rotated_rect.size.height);
    _ratio = _width / _height;
    // 更新角点信息
    _corners.resize(4);
    _rotated_rect.points(_corners.data());
}

std::shared_ptr<RuneCenter> RuneCenter::make_feature(const std::vector<cv::Point> &contour)
{
    if (contour.size() < 6)
        return nullptr;
    // init
    cv::RotatedRect rotated_rect = fitEllipse(contour);

    // 1.绝对面积判断
    DEBUG_INFO_("center 1.rotated_rect_area : %f", rotated_rect.size.area());
    if (rotated_rect.size.area() < para::rune_center_param.MIN_AREA || rotated_rect.size.area() > para::rune_center_param.MAX_AREA)
    {
        DEBUG_WARNING_("center 1.rotated_rect_area : fail");
        return nullptr;
    }
    DEBUG_PASS_("center 1.rotated_rect_area : pass");

    // 2.比例判断
    float width = std::max(rotated_rect.size.width, rotated_rect.size.height);
    float height = std::min(rotated_rect.size.width, rotated_rect.size.height);
    float ratio = width / height;
    DEBUG_INFO_("center 2.center_ratio : %f", ratio);
    if (ratio > para::rune_center_param.MAX_RATIO || ratio < para::rune_center_param.MIN_RATIO)
    {
        DEBUG_WARNING_("center 2.center_ratio : fail");
        return nullptr;
    }
    DEBUG_PASS_("center 2.center_ratio : pass");

    return std::make_shared<RuneCenter>(contour, rotated_rect);
}

} // namespace rm

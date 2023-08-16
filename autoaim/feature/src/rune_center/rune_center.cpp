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
#include "rmvl/feature/rune/rune_logging.h"

#include "rmvlpara/feature/rune_center.h"

using namespace rm;
using namespace para;
using namespace std;
using namespace cv;

RuneCenter::RuneCenter(const Point2f &center)
{
    // 初始化构造形状信息
    _center = center;
    _width = 20;
    _height = 20;
    _ratio = 1;
    // 更新角点信息
    _corners = {center + Point2f(-10, 10), center + Point2f(-10, -10),
                 center + Point2f(10, -10), center + Point2f(10, 10)};
}

RuneCenter::RuneCenter(vector<Point> &contour, RotatedRect &rotated_rect) : _rotated_rect(rotated_rect)
{
    _contour = contour;
    _center = _rotated_rect.center;
    _width = max(_rotated_rect.size.width, _rotated_rect.size.height);
    _height = min(_rotated_rect.size.width, _rotated_rect.size.height);
    _ratio = _width / _height;
    // 更新角点信息
    Point2f sortedPoints[4];
    _rotated_rect.points(sortedPoints);
    // 预留4个角点的空间
    _corners.reserve(4);
    for (auto point : sortedPoints)
        _corners.push_back(point);
}

shared_ptr<RuneCenter> RuneCenter::make_feature(const Point2f &center)
{
    return make_shared<RuneCenter>(center);
}

shared_ptr<RuneCenter> RuneCenter::make_feature(vector<Point> &contour)
{
    if (contour.size() < 6)
        return nullptr;
    // init
    RotatedRect rotated_rect = fitEllipse(contour);

    // 1.绝对面积判断
    DEBUG_RUNE_INFO_("center 1.rotated_rect_area : %f", rotated_rect.size.area());
    if (rotated_rect.size.area() < rune_center_param.MIN_AREA || rotated_rect.size.area() > rune_center_param.MAX_AREA)
    {
        DEBUG_RUNE_WARNING_("center 1.rotated_rect_area : fail");
        return nullptr;
    }
    DEBUG_RUNE_PASS_("center 1.rotated_rect_area : pass");

    // 2.比例判断
    float width = max(rotated_rect.size.width, rotated_rect.size.height);
    float height = min(rotated_rect.size.width, rotated_rect.size.height);
    float ratio = width / height;
    DEBUG_RUNE_INFO_("center 2.center_ratio : %f", ratio);
    if (ratio > rune_center_param.MAX_RATIO || ratio < rune_center_param.MIN_RATIO)
    {
        DEBUG_RUNE_WARNING_("center 2.center_ratio : fail");
        return nullptr;
    }
    DEBUG_RUNE_PASS_("center 2.center_ratio : pass");

    return make_shared<RuneCenter>(contour, rotated_rect);
}

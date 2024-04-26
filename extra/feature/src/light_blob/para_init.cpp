/**
 * @file para_init.cpp
 * @author RoboMaster Vision Community
 * @brief 角点直接构造灯条
 * @version 1.0
 * @date 2022-05-30
 *
 * @copyright Copyright 2023 (c), RoboMaster Vision Community
 *
 */

#include "rmvl/feature/light_blob.h"
#include "rmvl/core/util.hpp"
#include "rmvl/core.hpp"

using namespace rm;
using namespace std;
using namespace cv;

/**
 * @brief 参数构造
 *
 * @param top 上顶点
 * @param bottom 下顶点
 * @param width 灯条宽
 */
LightBlob::LightBlob(const Point2f &top, const Point2f &bottom, float width)
{
    // 顶点构建不设置筛选，仅设置异常
    if (isnan(top.x) || isnan(top.y) || top.x < -10000.f || top.x > 10000.f || top.y < -10000.f || top.y > 10000.f)
        RMVL_Error_(RMVL_StsBadArg, "Argument \"top\" is invalid, x = %.5f, y = %.5f", top.x, top.y);
    if (isnan(bottom.x) || isnan(bottom.y) || bottom.x < -10000.f || bottom.x > 10000.f || bottom.y < -10000.f || bottom.y > 10000.f)
        RMVL_Error_(RMVL_StsBadArg, "Argument \"bottom\" is invalid, x = %.5f, y = %.5f", bottom.x, bottom.y);
    if (isnan(width) || width < 0 || width > 10000.f)
        RMVL_Error_(RMVL_StsBadArg, "Argument \"width\" is invalid, value is: %.5f", width);

    // 设置灯条参数
    _top = top;
    _bottom = bottom;
    _center = (top + bottom) / 2.f;
    _angle = getVAngle(_bottom, _top, DEG);
    _height = getDistance(_top, _bottom);
    _width = width;
    _corners.reserve(4);
    // 设置灯条角点
    Vec2f vertical_vec = {top.x - bottom.x,
                          top.y - bottom.y};
    vertical_vec /= sqrt(vertical_vec(0) * vertical_vec(0) +
                         vertical_vec(1) * vertical_vec(1));
    Point2f vertical_point(-vertical_vec(1) * width / 2.f,
                           vertical_vec(0) * width / 2.f);
    _corners = {_top + vertical_point,
                 _top - vertical_point,
                 _bottom + vertical_point,
                 _bottom - vertical_point};
}

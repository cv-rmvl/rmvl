/**
 * @file Pilot.cpp
 * @author RoboMaster Vision Community
 * @brief 引导灯类源文件
 * @version 1.0
 * @date 2021-12-14
 *
 * @copyright Copyright 2021 (c), RoboMaster Vision Community
 *
 */

#include <opencv2/imgproc.hpp>

#include "rmvl/rmath.hpp"
#include "rmvl/feature/pilot.h"

#include "rmvlpara/feature/pilot.h"

using namespace cv;
using namespace std;
using namespace para;
using namespace rm;

void Pilot::getTruePoint(vector<Point> &contour)
{
    Point left = _center;
    Point right = _center;
    // 遍历每一个轮廓点
    for (const auto &point : contour)
    {
        if (abs(point.y - _center.y) > _height / pilot_param.VERTEX_K)
            continue;
        // 小于或大于一定值时才对其进行计算比较，节省运算
        if (point.x < _center.x) // 找出轮廓中最左边的点作为左极点
        {
            if (point.x < left.x)
                left = point;
        }
        else // 找出轮廓中最右边的点作为右极点
        {
            if (point.x > right.x)
                right = point;
        }
    }
    // 提取轮廓点集中离拟合椭圆左右顶点最近的点作修正后的数据
    if (left.x != _center.x &&
        right.x != _center.x)
    {
        // 更新左右顶点
        _left = left;
        _right = right;
        float temp_width = getDistance(_left, _right);
        _height = _height * temp_width / _width;
        _width = temp_width;
        _angle = getHAngle(_left, _right);
    }
}

shared_ptr<Pilot> Pilot::make_feature(vector<Point> &contour, Mat &bin)
{
    RotatedRect rotated_rect = fitEllipse(contour);
    Point2f center = rotated_rect.center;
    if (center.x < 0 || center.x > bin.cols ||
        center.y < 0 || center.y > bin.rows)
        return nullptr;
    float width = max(rotated_rect.size.width, rotated_rect.size.height);
    float height = min(rotated_rect.size.width, rotated_rect.size.height);
    // 仅对二值图中绿色部分拟合形状进行筛选
    int row = center.y;
    int col_l = center.x - width / 3;
    int col_r = center.x + width / 3;
    if (bin.ptr<uchar>(row)[col_l] == 125 ||
        bin.ptr<uchar>(row)[col_r] == 125)
    {
        if (contour.empty())
            return nullptr;
        float wh_ratio = width / height;
        if (wh_ratio > pilot_param.MAX_RATIO ||
            wh_ratio < pilot_param.MIN_RATIO)
            return nullptr;
    }
    else
        return nullptr;

    return make_shared<Pilot>(contour, rotated_rect, width, height);
}

Pilot::Pilot(vector<Point> &contour, RotatedRect &rotated_rect, float width, float height)
    : _rotated_rect(rotated_rect)
{
    if (isnan(width) || width < 0 || width > 100000.f)
        RMVL_Error_(RMVL_StsBadArg, "Argument \"width\" is invalid, value is %.5f", width);
    if (isnan(height) || height < 0 || height > 100000.f)
        RMVL_Error_(RMVL_StsBadArg, "Argument \"height\" is invalid, value is %.5f", height);
    _width = width;
    _height = height;
    _center = _rotated_rect.center;
    _angle = _rotated_rect.angle;
    getTruePoint(contour);
    _corners = {_left, _right};
}

Pilot::Pilot(const float &last_width, const float &last_height, const Point2f &last_center, const float &last_angle, const vector<cv::Point2f> &last_corners)
{
    if (isnan(last_width) || last_width < 0 || last_width > 100000.f)
        RMVL_Error_(RMVL_StsBadArg, "Argument \"width\" is invalid, value is %.5f", last_width);
    if (isnan(last_height) || last_height < 0 || last_height > 100000.f)
        RMVL_Error_(RMVL_StsBadArg, "Argument \"height\" is invalid, value is %.5f", last_height);
    _width = last_width;
    _height = last_height;
    _center = last_center;
    _angle = last_angle;
    _corners = last_corners;
}
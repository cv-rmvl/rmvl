/**
 * @file light_blob.cpp
 * @author RoboMaster Vision Community
 * @brief 轮廓构造灯条
 * @version 1.0
 * @date 2022-11-16
 *
 * @copyright Copyright 2023 (c), RoboMaster Vision Community
 *
 */

#include <opencv2/imgproc.hpp>

#include "rmvl/feature/light_blob.h"
#include "rmvl/rmath.hpp"

#include "rmvlpara/feature/light_blob.h"

namespace rm
{

void LightBlob::calcAccurateInfo(float lw_ratio, const std::vector<cv::Point> &contour)
{
    // angle 向量
    const float rad_angle = deg2rad(_angle);
    const float cos_angle = cos(rad_angle);
    const float sin_angle = sin(rad_angle);
    // 轮廓分割
    std::vector<cv::Point> up_contour, down_contour;
    up_contour.reserve(contour.size() >> 1);
    down_contour.reserve(contour.size() >> 1);
    float height_bias = 0.25f * _height * cos_angle;
    for (auto &point : contour)
    {
        if (point.y < _center.y - height_bias)
            up_contour.emplace_back(point);
        else if (point.y > _center.y + height_bias)
            down_contour.emplace_back(point);
    }
    if (lw_ratio < para::light_blob_param.RATIO_THRESHOLD)
    {
        cv::Vec4f line_vec = {sin_angle, -cos_angle, _center.x, _center.y};
        _top = *min_element(up_contour.begin(), up_contour.end(), [&](const cv::Point &lhs, const cv::Point &rhs) {
            return getDistance(line_vec, lhs, false) < getDistance(line_vec, rhs, false);
        });
        _bottom = *min_element(down_contour.begin(), down_contour.end(), [&](const cv::Point &lhs, const cv::Point &rhs) {
            return getDistance(line_vec, lhs, false) < getDistance(line_vec, rhs, false);
        });
    }
    else
    {
        // 寻找上下顶点
        cv::Vec4f line_vec = {cos_angle, sin_angle, _center.x, _center.y}; // 方向为角度向量的法向量
        _top = *max_element(up_contour.begin(), up_contour.end(), [&](const cv::Point &lhs, const cv::Point &rhs) {
            return getDistance(line_vec, lhs, false) < getDistance(line_vec, rhs, false);
        });
        _bottom = *max_element(down_contour.begin(), down_contour.end(), [&](const cv::Point &lhs, const cv::Point &rhs) {
            return getDistance(line_vec, lhs, false) < getDistance(line_vec, rhs, false);
        });
    }
    // 更新灯条中点
    _center = (_top + _bottom) / 2.f;
    // 长度更新，宽度同比缩放
    float temp_height = getDistance(_top, _bottom);
    float temp_width = temp_height / _height * _width;
    _rotated_rect.size.height = _height = temp_height;
    _rotated_rect.size.width = _width = temp_width;
    // 角点更新
    float half_w = temp_width / 2;
    _corners = {_bottom + cv::Point2f(-half_w * cos_angle, -half_w * sin_angle),
                _top + cv::Point2f(-half_w * cos_angle, -half_w * sin_angle),
                _top + cv::Point2f(half_w * cos_angle, half_w * sin_angle),
                _bottom + cv::Point2f(half_w * cos_angle, half_w * sin_angle)};
}

std::shared_ptr<LightBlob> LightBlob::make_feature(const std::vector<cv::Point> &contour)
{
    // init
    if (contour.size() < 6)
        return nullptr;
    cv::RotatedRect rotated_rect = fitEllipse(contour);
    // 统一角度方向
    float angle = (rotated_rect.angle > 90 ? rotated_rect.angle - 180 : rotated_rect.angle);
    // 根据斜率排除误识别
    if (abs(angle) > para::light_blob_param.MAX_ANGLE)
        return nullptr;
    // 获取灯条长宽比例
    float lw_ratio = rotated_rect.size.height / rotated_rect.size.width;
    // 长宽比例过大
    if (lw_ratio > para::light_blob_param.MAX_RATIO)
        return nullptr;

    // 长度过小，排除
    if (rotated_rect.size.height < para::light_blob_param.MIN_LENGTH)
        return nullptr;
    //  判断为远处灯条
    if (rotated_rect.size.height < para::light_blob_param.CLOSE_LENGTH)
    {
        // 比例判断
        if (lw_ratio > para::light_blob_param.MAX_FAR_RATIO || lw_ratio < para::light_blob_param.MIN_FAR_RATIO)
            return nullptr;
    }
    // 判断为近处灯条
    else
    {
        // 比例判断
        if (lw_ratio > para::light_blob_param.MAX_CLOSE_RATIO)
            return nullptr;
    }
    // 构造返回
    return make_shared<LightBlob>(contour, rotated_rect, lw_ratio, angle);
}

LightBlob::LightBlob(const std::vector<cv::Point> &contour, cv::RotatedRect &rotated_rect, float lw_ratio, float angle)
    : _rotated_rect(rotated_rect)
{
    _angle = angle;
    // 记录包围矩形角点
    _corners.resize(4);
    _rotated_rect.points(_corners.data());
    // 点从上到下排序
    sort(_corners.begin(), _corners.end(),
         [](const cv::Point2f &lhs, const cv::Point2f &rhs) {
             return lhs.y < rhs.y;
         });

    // 获得粗略的上下顶点和长宽
    _top = (_corners[0] + _corners[1]) / 2;
    _bottom = (_corners[2] + _corners[3]) / 2;
    _height = _rotated_rect.size.height;
    _width = _rotated_rect.size.width;
    _center = _rotated_rect.center;
    // 修正灯条匹配误差，获得精准的上下顶点和长宽度
    calcAccurateInfo(lw_ratio, contour);
}

} // namespace rm

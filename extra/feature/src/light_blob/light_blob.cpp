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

#include "rmvl/algorithm/math.hpp"
#include "rmvl/feature/light_blob.h"

#include "rmvlpara/feature/light_blob.h"

namespace rm
{

static void calcAccurateInfo(float lw_ratio, const std::vector<cv::Point> &contour, float angle,
                             cv::Point2f &top, cv::Point2f &bottom, cv::Point2f &center,
                             float &width, float &height, std::vector<cv::Point2f> &corners)
{
    // angle 向量
    const float rad_angle = deg2rad(angle);
    const float cos_angle = std::cos(rad_angle);
    const float sin_angle = std::sin(rad_angle);
    // 轮廓分割
    std::vector<cv::Point> up_contour, down_contour;
    up_contour.reserve(contour.size() >> 1);
    down_contour.reserve(contour.size() >> 1);
    float height_bias = 0.25f * height * cos_angle;
    for (auto &point : contour)
    {
        if (point.y < center.y - height_bias)
            up_contour.emplace_back(point);
        else if (point.y > center.y + height_bias)
            down_contour.emplace_back(point);
    }
    if (lw_ratio < para::light_blob_param.RATIO_THRESHOLD)
    {
        cv::Vec4f line_vec = {sin_angle, -cos_angle, center.x, center.y};
        top = *min_element(up_contour.begin(), up_contour.end(), [&](const cv::Point &lhs, const cv::Point &rhs) {
            return getDistance(line_vec, lhs, false) < getDistance(line_vec, rhs, false);
        });
        bottom = *min_element(down_contour.begin(), down_contour.end(), [&](const cv::Point &lhs, const cv::Point &rhs) {
            return getDistance(line_vec, lhs, false) < getDistance(line_vec, rhs, false);
        });
    }
    else
    {
        // 寻找上下顶点
        cv::Vec4f line_vec = {cos_angle, sin_angle, center.x, center.y}; // 方向为角度向量的法向量
        top = *max_element(up_contour.begin(), up_contour.end(), [&](const cv::Point &lhs, const cv::Point &rhs) {
            return getDistance(line_vec, lhs, false) < getDistance(line_vec, rhs, false);
        });
        bottom = *max_element(down_contour.begin(), down_contour.end(), [&](const cv::Point &lhs, const cv::Point &rhs) {
            return getDistance(line_vec, lhs, false) < getDistance(line_vec, rhs, false);
        });
    }
    // 更新灯条中点
    center = (top + bottom) / 2.f;
    // 长度更新，宽度同比缩放
    float temp_height = getDistance(top, bottom);
    float temp_width = temp_height / height * width;
    height = temp_height;
    width = temp_width;
    // 角点更新
    float half_w = temp_width / 2;
    corners = {bottom + cv::Point2f(-half_w * cos_angle, -half_w * sin_angle),
               top + cv::Point2f(-half_w * cos_angle, -half_w * sin_angle),
               top + cv::Point2f(half_w * cos_angle, half_w * sin_angle),
               bottom + cv::Point2f(half_w * cos_angle, half_w * sin_angle)};
}

LightBlob::ptr LightBlob::make_feature(const std::vector<cv::Point> &contour)
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
    auto retval = std::make_shared<LightBlob>();

    retval->_angle = angle;
    // 记录包围矩形角点
    auto &corners = retval->_corners;
    corners.resize(4);
    rotated_rect.points(corners.data());
    // 点从上到下排序
    sort(corners.begin(), corners.end(), [](const cv::Point2f &lhs, const cv::Point2f &rhs) {
        return lhs.y < rhs.y;
    });

    // 获得粗略的上下顶点和长宽
    retval->_top = (corners[0] + corners[1]) / 2;
    retval->_bottom = (corners[2] + corners[3]) / 2;
    retval->_height = rotated_rect.size.height;
    retval->_width = rotated_rect.size.width;
    retval->_center = rotated_rect.center;
    // 修正灯条匹配误差，获得精准的上下顶点和长宽度
    calcAccurateInfo(lw_ratio, contour, angle, retval->_top, retval->_bottom, retval->_center, retval->_width, retval->_height, corners);

    return retval;
}

LightBlob::ptr LightBlob::make_feature(const cv::Point2f &top, const cv::Point2f &bottom, float width)
{
    // 顶点筛选
    if (std::isnan(top.x) || std::isnan(top.y) || top.x < -10000.f || top.x > 10000.f || top.y < -10000.f || top.y > 10000.f)
        return nullptr;
    if (std::isnan(bottom.x) || std::isnan(bottom.y) || bottom.x < -10000.f || bottom.x > 10000.f || bottom.y < -10000.f || bottom.y > 10000.f)
        return nullptr;
    if (std::isnan(width) || width < 0 || width > 10000.f)
        return nullptr;

    auto retval = std::make_shared<LightBlob>();

    // 设置灯条参数
    retval->_top = top;
    retval->_bottom = bottom;
    retval->_center = (top + bottom) / 2.f;
    retval->_angle = getVAngle(bottom, top, DEG);
    retval->_height = getDistance(top, bottom);
    retval->_width = width;
    retval->_corners.reserve(4);
    // 设置灯条角点
    cv::Vec2f vertical_vec = {top.x - bottom.x,
                              top.y - bottom.y};
    vertical_vec /= sqrt(vertical_vec(0) * vertical_vec(0) +
                         vertical_vec(1) * vertical_vec(1));
    cv::Point2f vertical_point(-vertical_vec(1) * width / 2.f,
                               vertical_vec(0) * width / 2.f);
    retval->_corners = {top + vertical_point,
                        top - vertical_point,
                        bottom + vertical_point,
                        bottom - vertical_point};
    return retval;
}

} // namespace rm

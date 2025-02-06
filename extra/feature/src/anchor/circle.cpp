/**
 * @file circle.cpp
 * @author zhaoxi (535394140@qq.com)
 * @brief 圆形轮廓特征点检测
 * @version 1.0
 * @date 2025-02-06
 *
 * @copyright Copyright 2025 (c), zhaoxi
 *
 */

#include <opencv2/imgproc.hpp>

#include "anchor_def.hpp"
#include "rmvlpara/feature/anchor.h"

namespace rm
{

// 判断椭圆拟合的平均拟合精度
static double fitEllipseAccuracy(const std::vector<cv::Point> &contour, const cv::RotatedRect &rrect)
{
    double totalError{};
    for (const auto &point : contour)
    {
        // 将点转换到椭圆坐标系中
        cv::Point2f center = rrect.center;
        double angle = rrect.angle * CV_PI / 180.0;
        double a = rrect.size.width / 2.0;  // 长轴
        double b = rrect.size.height / 2.0; // 短轴

        double cos_angle = std::cos(angle);
        double sin_angle = std::sin(angle);

        double x = point.x - center.x;
        double y = point.y - center.y;

        double x_ellipse = x * cos_angle + y * sin_angle;
        double y_ellipse = -x * sin_angle + y * cos_angle;

        // 计算点到椭圆的距离
        totalError += std::abs(std::sqrt((x_ellipse * x_ellipse) / (a * a) + (y_ellipse * y_ellipse) / (b * b)) - 1);
    }
    return totalError / (contour.size() + 1);
}

std::optional<CircleInfo> createAnchorFromCircleContour(const std::vector<cv::Point> &contour)
{
    // 椭圆拟合
    if (contour.size() < 5)
        return std::nullopt;
    auto rrect = cv::fitEllipse(contour);
    if (fitEllipseAccuracy(contour, rrect) > para::anchor_param.MAX_FIT_ACCURACY)
        return std::nullopt;
    // 离心率
    float a = std::max(rrect.size.width, rrect.size.height) / 2;
    float b = std::min(rrect.size.width, rrect.size.height) / 2;
    if (b < rm::para::anchor_param.MIN_RADIUS)
        return std::nullopt;
    if (a > rm::para::anchor_param.MAX_RADIUS)
        return std::nullopt;
    auto eccentricity = std::sqrt(1 - (b * b) / (a * a));

    return eccentricity < rm::para::anchor_param.MAX_ECCENTRICITY ? std::make_optional(CircleInfo{rrect.center, a, eccentricity}) : std::nullopt;
}

} // namespace rm

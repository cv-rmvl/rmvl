/**
 * @file square.cpp
 * @author zhaoxi (535394140@qq.com)
 * @brief 矩形轮廓特征点检测
 * @version 1.0
 * @date 2025-02-06
 *
 * @copyright Copyright 2025 (c), zhaoxi
 *
 */

#include <opencv2/imgproc.hpp>

#include "rmvl/algorithm/math.hpp"

#include "anchor_def.hpp"

namespace rm
{

/**
 * @brief 获取矩形轮廓特征的中心点
 *
 * @param[in] curve_points 多边形拟合得到的角点
 * @return 中心点
 */
static cv::Point2f getSquareCenter(const std::vector<cv::Point2f> &curve_points)
{
    RMVL_DbgAssert(curve_points.size() == 4);
    return ((curve_points[0].x * curve_points[2].y - curve_points[0].y * curve_points[2].x) * (curve_points[1] - curve_points[3]) -
            (curve_points[1].x * curve_points[3].y - curve_points[1].y * curve_points[3].x) * (curve_points[0] - curve_points[2])) /
           ((curve_points[0].x - curve_points[2].x) * (curve_points[1].y - curve_points[3].y) -
            (curve_points[1].x - curve_points[3].x) * (curve_points[0].y - curve_points[2].y));
}

/**
 * @brief 获取矩形轮廓特征的有序角点（最左边的点为第一个角点，按顺时针排列）
 *
 * @param[in] curve_points 多边形拟合得到的角点
 * @return 有序角点
 */
static std::vector<cv::Point2f> getSqaureOrderCorners(const std::vector<cv::Point2f> &curve_points)
{
    RMVL_DbgAssert(curve_points.size() == 4);
    auto it = std::min_element(curve_points.begin(), curve_points.end(), [](const cv::Point2f &lhs, const cv::Point2f &rhs) {
        return lhs.x < rhs.x;
    });

    std::size_t idx = std::distance(curve_points.begin(), it);
    std::vector<cv::Point2f> retval(4);
    for (int i = 0; i < 4; ++i)
        retval[i] = curve_points[(idx + i) % 4];
    // 通过 findContours 得到的轮廓点按照逆时针方向排列，需要逆序处理，以得到顺时针排列的角点
    std::swap(retval[1], retval[3]);
    return retval;
}

std::optional<SquareInfo> createAnchorFromSqaureContour(const std::vector<cv::Point> &contour)
{
    if (contour.size() < 5)
        return std::nullopt;

    // 多边形拟合
    double arc_length = cv::arcLength(contour, true); // 周长
    std::vector<cv::Point2f> curve_points;
    cv::approxPolyDP(contour, curve_points, 0.02 * arc_length, true);

    // 条件筛选
    if (curve_points.size() != 4)
        return std::nullopt;
    else
    {
        SquareInfo info;
        // 获取中心点
        info.center = getSquareCenter(curve_points);
        // 计算边长
        for (int i = 0; i < 4; ++i)
            info.length = std::max(info.length, static_cast<float>(getDistance(curve_points[i], curve_points[(i + i) % 4])));
        // 获取有序角点
        info.corners = getSqaureOrderCorners(curve_points);
        // 获取角度
        info.angle = (getHAngle(info.corners[0], info.corners[1], DEG) + getHAngle(info.corners[3], info.corners[2], DEG)) / 2.f;
        info.angle = info.angle > 45 ? info.angle - 90 : info.angle;

        return info;
    }
}

} // namespace rm

/**
 * @file cross.cpp
 * @author zhaoxi (535394140@qq.com)
 * @brief 十字交叉轮廓特征点检测
 * @version 1.0
 * @date 2025-02-08
 *
 * @copyright Copyright 2025 (c), zhaoxi
 *
 */

#include <opencv2/imgproc.hpp>

#include "rmvl/algorithm/math.hpp"
#include "rmvlpara/feature/anchor.h"

#include "anchor_def.hpp"

namespace rm
{

/**
 * @brief 获取十字交叉轮廓特征的角点
 *
 * @param[in] contour 轮廓
 * @param[in] center 中心点
 * @return 角点
 */
static std::vector<cv::Point2f> getCrossCorners(const std::vector<cv::Point> &contour, const cv::Point2f &center)
{
    // 寻找最远的一个点
    cv::Point2f p0 = *std::max_element(contour.begin(), contour.end(), [&center](const cv::Point &lhs, const cv::Point &rhs) {
        return getDistance(lhs, center) < getDistance(rhs, center);
    });

    // 以 center 为原点，寻找与 2 * (center - p1) + center 最近的点
    cv::Point2f tar = 2 * (center - p0) + center;
    cv::Point2f p2 = *std::min_element(contour.begin(), contour.end(), [&tar](const cv::Point &lhs, const cv::Point &rhs) {
        return getDistance(lhs, tar) < getDistance(rhs, tar);
    });

    // 计算与 p0p2 直线最远的点
    cv::Vec4f line(center.x - p0.x, center.y - p0.y, center.x, center.y);
    auto comp = [&line](const cv::Point &lhs, const cv::Point &rhs) {
        return getDistance(line, lhs) < getDistance(line, rhs);
    };
    cv::Point2f p1 = *std::max_element(contour.begin(), contour.end(), comp);
    cv::Point2f p3 = *std::min_element(contour.begin(), contour.end(), comp);

    std::array<cv::Point2f, 4> corners{p0, p1, p2, p3};

    // 保证最左边的点为第一个角点
    auto it = std::min_element(corners.begin(), corners.end(), [](const cv::Point2f &lhs, const cv::Point2f &rhs) {
        return lhs.x < rhs.x;
    });
    std::size_t idx = std::distance(corners.begin(), it);
    std::vector<cv::Point2f> retval(4);
    for (int i = 0; i < 4; ++i)
        retval[i] = corners[(idx + i) % 4];
    return retval;
}

std::optional<CrossInfo> createAnchorFromCrossContour(const std::vector<cv::Point> &contour)
{
    if (contour.size() < 10)
        return std::nullopt;
    cv::Moments m = cv::moments(contour);
    // 紧凑度筛选
    double arc_length = cv::arcLength(contour, true);
    if (m.m00 / (arc_length * arc_length) > para::anchor_param.MAX_COMPACTNESS)
        return std::nullopt;

    rm::CrossInfo info;
    // 获取轮廓中心点
    info.center = cv::Point2d{m.m10 / m.m00, m.m01 / m.m00};
    // 获取轮廓的角点
    info.corners = rm::getCrossCorners(contour, info.center);
    // 获取轮廓的长度
    info.length = (getDistance(info.corners[0], info.corners[2]) + getDistance(info.corners[1], info.corners[3])) / 2.f;
    // 获取轮廓角度
    info.angle = (getHAngle(info.corners[0], info.corners[2]) + getHAngle(info.corners[1], info.corners[3])) / 2.f;

    return info;
}

} // namespace rm

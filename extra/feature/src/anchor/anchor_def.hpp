/**
 * @file anchor_def.hpp
 * @author zhaoxi (535394140@qq.com)
 * @brief
 * @version 1.0
 * @date 2025-02-06
 *
 * @copyright Copyright 2025 (c), zhaoxi
 *
 */

#pragma once

#include <optional>

#include "rmvl/feature/anchor.h"

namespace rm
{

/////////////////////////////// 圆形特征点 ///////////////////////////////

// 圆形特征
struct CircleInfo
{
    cv::Point2f center{};  // 中心点
    float radius{};        // 半径
    double eccentricity{}; // 离心率
};

// 从圆形轮廓构造特征
std::optional<CircleInfo> createAnchorFromCircleContour(const std::vector<cv::Point> &contour);

/////////////////////////////// 矩形特征点 ///////////////////////////////

// 矩形特征
struct SquareInfo
{
    cv::Point2f center{};               // 中心点
    float length{};                     // 边长
    std::vector<cv::Point2f> corners{}; // 轮廓
    float angle{};                      // 角度
};

// 从矩形轮廓构造特征
std::optional<SquareInfo> createAnchorFromSqaureContour(const std::vector<cv::Point> &contour);

///////////////////////////// 十字交叉特征点 /////////////////////////////

// 十字交叉特征
struct CrossInfo
{
    cv::Point2f center{};               // 中心点
    float length{};                     // 长度
    std::vector<cv::Point2f> corners{}; // 轮廓
    float angle{};                      // 角度
};

// 从十字交叉轮廓构造特征
std::optional<CrossInfo> createAnchorFromCrossContour(const std::vector<cv::Point> &contour);

} // namespace rm

/**
 * @file anchor_test.cpp
 * @author zhaoxi (535394140@qq.com)
 * @brief 定位点特征单元测试
 * @version 1.0
 * @date 2025-02-06
 *
 * @copyright Copyright 2025 (c), zhaoxi
 *
 */

#include "rmvl/rmvl_modules.hpp"

#ifdef HAVE_RMVL_PILOT

#include <gtest/gtest.h>

#include <opencv2/calib3d.hpp>
#include <opencv2/imgproc.hpp>

#include "rmvl/feature/anchor.h"

namespace rm_test
{

static std::vector<std::vector<cv::Point>> getContours()
{
    auto src = cv::Mat(cv::Size(640, 480), CV_8UC1, cv::Scalar(0, 0, 0));
    // Square
    cv::rectangle(src, {90, 180}, {190, 260}, cv::Scalar(255, 255, 255), -1);
    // Circle
    cv::circle(src, {350, 250}, 50, cv::Scalar(255, 255, 255), -1);
    // Cross
    cv::line(src, {550, 100}, {550, 200}, cv::Scalar(255, 255, 255), 8);
    cv::line(src, {500, 150}, {600, 150}, cv::Scalar(255, 255, 255), 8);

    std::vector<cv::Vec2f> ptsrc = {{0, 0},
                                    {0, 100},
                                    {100, 100},
                                    {100, 0}};
    std::vector<cv::Vec2f> ptdst = {{5, 0},
                                    {0, 95},
                                    {100, 100},
                                    {95, 10}};
    auto M = cv::getPerspectiveTransform(ptsrc, ptdst);

    cv::Mat dst;
    cv::warpPerspective(src, dst, M, src.size());
    cv::threshold(dst, dst, 125, 255, cv::THRESH_BINARY);
    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(dst, contours, cv::RETR_LIST, cv::CHAIN_APPROX_NONE);
    std::sort(contours.begin(), contours.end(), [](const auto &lhs, const auto &rhs) {
        return lhs.front().x < rhs.front().x;
    });
    return contours;
}

TEST(BuildAnchorTest, BuildCircle)
{
    auto contours = getContours();
    auto &sqaure_contour = contours[0];
    auto &circle_contour = contours[1];
    auto &cross_contour = contours[2];

    auto p_circle_1 = rm::Anchor::make_feature(sqaure_contour, rm::AnchorType::Circle);
    EXPECT_EQ(p_circle_1, nullptr);
    auto p_circle_2 = rm::Anchor::make_feature(circle_contour, rm::AnchorType::Circle);
    EXPECT_NE(p_circle_2, nullptr);
    auto p_circle_3 = rm::Anchor::make_feature(cross_contour, rm::AnchorType::Circle);
    EXPECT_EQ(p_circle_3, nullptr);
}

TEST(BuildAnchorTest, BuildSqaure)
{
    auto contours = getContours();
    auto &sqaure_contour = contours[0];
    auto &circle_contour = contours[1];
    auto &cross_contour = contours[2];

    auto p_sqaure_1 = rm::Anchor::make_feature(sqaure_contour, rm::AnchorType::Square);
    EXPECT_NE(p_sqaure_1, nullptr);
    EXPECT_GE(p_sqaure_1->angle(), 0);
    auto p_sqaure_2 = rm::Anchor::make_feature(circle_contour, rm::AnchorType::Square);
    EXPECT_EQ(p_sqaure_2, nullptr);
    auto p_sqaure_3 = rm::Anchor::make_feature(cross_contour, rm::AnchorType::Square);
    EXPECT_EQ(p_sqaure_3, nullptr);
}

TEST(BuildAnchorTest, BuildCross)
{
    auto contours = getContours();
    auto &sqaure_contour = contours[0];
    auto &circle_contour = contours[1];
    auto &cross_contour = contours[2];

    auto p_cross_1 = rm::Anchor::make_feature(sqaure_contour, rm::AnchorType::Cross);
    EXPECT_EQ(p_cross_1, nullptr);
    auto p_cross_2 = rm::Anchor::make_feature(circle_contour, rm::AnchorType::Cross);
    EXPECT_EQ(p_cross_2, nullptr);
    auto p_cross_3 = rm::Anchor::make_feature(cross_contour, rm::AnchorType::Cross);
    EXPECT_NE(p_cross_3, nullptr);
    EXPECT_LE(p_cross_3->angle(), 0);
}

} // namespace rm_test

#endif // HAVE_RMVL_PILOT

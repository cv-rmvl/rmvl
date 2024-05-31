/**
 * @file rune_target_test.cpp
 * @author RoboMaster Vision Community
 * @brief
 * @version 0.1
 * @date 2023-04-25
 *
 * @copyright Copyright 2023 (c), RoboMaster Vision Community
 *
 */

#include "rmvl/rmvl_modules.hpp"

#ifdef HAVE_RMVL_RUNE_TARGET

#include <gtest/gtest.h>
#include <opencv2/imgproc.hpp>

#include "rmvl/feature/rune_target.h"

using namespace rm;

namespace rm_test
{

class BuildRuneTargetTest : public testing::Test
{
public:
    std::vector<cv::Point> contour;

    void SetUp() override
    {
        // 一般轮廓
        cv::Mat src = cv::Mat::zeros(cv::Size(1000, 1000), CV_8UC1);
        circle(src, cv::Point(500, 500), 45, cv::Scalar(255), 2);
        std::vector<std::vector<cv::Point>> contours;
        findContours(src, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_NONE);
        contour = contours.front();
    }

    void TearDown() override {}
};

TEST_F(BuildRuneTargetTest, normal_contourSize)
{
    // 一般个数轮廓点数，正常圆
    RuneTarget::ptr rt1 = RuneTarget::make_feature(contour, 0);
    EXPECT_TRUE(rt1);
}

TEST_F(BuildRuneTargetTest, contourShape)
{
    // 过宽轮廓
    cv::Mat img = cv::Mat::zeros(1000, 1000, CV_8UC1);
    rectangle(img, {400, 400}, {600, 500}, {255, 255, 255}, cv::FILLED);
    std::vector<std::vector<cv::Point>> contours;
    findContours(img, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_NONE);
    std::vector<cv::Point> contour_ = contours.front();
    RuneTarget::ptr rt3 = RuneTarget::make_feature(contour_, false);
    EXPECT_FALSE(rt3);

    // 过高轮廓
    img = cv::Mat::zeros(1000, 1000, CV_8UC1);
    rectangle(img, {400, 400}, {500, 600}, {255, 255, 255}, cv::FILLED);
    findContours(img, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_NONE);
    contour_ = contours.front();
    auto rt4 = RuneTarget::make_feature(contour_, 0);
    EXPECT_FALSE(rt4);
}

TEST_F(BuildRuneTargetTest, few_contourSize)
{
    // 非正常轮廓点数构建神符靶心
    std::vector<cv::Point> contour_4 = {{500, 455}, {455, 500}, {500, 545}, {545, 500}};
    auto rt = RuneTarget::make_feature(contour_4, 0);
    EXPECT_FALSE(rt);
}

} // namespace rm_test

#endif // HAVE_RMVL_RUNE_TARGET

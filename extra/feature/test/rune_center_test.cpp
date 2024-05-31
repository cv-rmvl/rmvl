/**
 * @file rune_center_test.cpp
 * @author RoboMaster Vision Community
 * @brief
 * @version 1.0
 * @date 2022-08-16
 *
 * @copyright Copyright 2023 (c), RoboMaster Vision Community
 *
 */

#include "rmvl/rmvl_modules.hpp"

#ifdef HAVE_RMVL_RUNE_CENTER

#include <gtest/gtest.h>
#include <opencv2/imgproc.hpp>

#include "rmvl/feature/rune_center.h"

using namespace rm;

namespace rm_test
{

class BuildRuneCenterTest : public testing::Test
{
public:
    std::vector<cv::Point> contour;
    std::vector<cv::Point> contour_6;
    cv::Vec4i hierarchy;

    void SetUp() override
    {
        // 6 点轮廓
        contour_6 = {{15, 0}, {28, 8}, {28, 23}, {15, 30}, {2, 23}, {2, 8}};
        // 一般轮廓
        cv::Mat src = cv::Mat::zeros(cv::Size(100, 100), CV_8UC1);
        circle(src, cv::Point(50, 50), 45, cv::Scalar(255), -1);
        std::vector<std::vector<cv::Point>> contours;
        findContours(src, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_NONE);
        contour = contours.front();
    }

    void TearDown() override {}
};

TEST_F(BuildRuneCenterTest, normal_contourSize)
{
    // 轮廓点数为6
    RuneCenter::ptr rc1 = RuneCenter::make_feature(contour_6);
    EXPECT_TRUE(rc1);
}

TEST_F(BuildRuneCenterTest, few_contourSize)
{
    // 非正常轮廓点数构建神符中心
    std::vector<cv::Point> contour_5 = {{28, 8}, {28, 23}, {15, 30}, {2, 23}, {2, 8}};
    RuneCenter::ptr rc = RuneCenter::make_feature(contour_5);
    EXPECT_FALSE(rc);
}

} // namespace rm_test

#endif // HAVE_RMVL_RUNE_CENTER

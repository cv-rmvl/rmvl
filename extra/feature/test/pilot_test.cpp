/**
 * @file pilot_test.cpp
 * @author RoboMaster Vision Community
 * @brief 引导灯
 * @version 1.0
 * @date 2022-12-15
 *
 * @copyright Copyright 2022 (c), RoboMaster Vision Community
 *
 */

#include "rmvl/rmvl_modules.hpp"

#ifdef HAVE_RMVL_PILOT

#include <gtest/gtest.h>
#include <opencv2/imgproc.hpp>

#include "rmvl/feature/pilot.h"

using namespace rm;

namespace rm_test
{

class BuildPilotTest : public testing::Test
{
public:
    Pilot::ptr pilot;
    cv::Mat src;
    std::vector<cv::Point> d_contour;

    void SetUp() override
    {
        d_contour = buildPilotContours(125, 10);
    }
    void TearDown() override {}

    std::vector<cv::Point> buildPilotContours(float color, float radius)
    {
        src = cv::Mat::zeros(cv::Size(500, 500), CV_8UC1);
        circle(src, cv::Point(60, 50), radius, cv::Scalar(color), cv::FILLED);
        std::vector<cv::Point> contour;
        std::vector<std::vector<cv::Point>> contours;
        findContours(src, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_NONE);
        return contour = contours.front();
    }
};

TEST_F(BuildPilotTest, wrong_color)
{
    std::vector<cv::Point> contour;
    // 错误的颜色值
    contour = buildPilotContours(127, 10);
    pilot = Pilot::make_feature(contour, src);
    EXPECT_FALSE(pilot != nullptr);
    // 正确的颜色值
    contour.clear();
    pilot.reset();
    contour = buildPilotContours(125, 10);
    pilot = Pilot::make_feature(contour, src);
    EXPECT_TRUE(pilot != nullptr);
}

TEST_F(BuildPilotTest, wrong_area)
{
    std::vector<cv::Point> contour;
    // 正确的面积大小
    contour.clear();
    pilot.reset();
    contour = buildPilotContours(125, 10);
    pilot = Pilot::make_feature(contour, src);
    EXPECT_TRUE(pilot != nullptr);
}

} // namespace rm_test

#endif // HAVE_RMVL_PILOT

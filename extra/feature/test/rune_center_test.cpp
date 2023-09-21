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

#include <gtest/gtest.h>
#include <opencv2/imgproc.hpp>

#include "rmvl/feature/rune_center.h"

using namespace rm;
using namespace std;
using namespace cv;

namespace rm_test
{

class BuildRuneCenterTest : public testing::Test
{
public:
    vector<Point> contour;
    vector<Point> contour_6;
    Vec4i hierarchy;

    void SetUp() override
    {
        // 6 点轮廓
        contour_6 = {Point(15, 0), Point(27.9, 7.5), Point(27.9, 22.5),
                     Point(15, 30), Point(2.1, 22.5), Point(2.1, 7.5)};
        // 一般轮廓
        Mat src = Mat::zeros(Size(100, 100), CV_8UC1);
        circle(src, Point(50, 50), 45, Scalar(255), -1);
        vector<vector<Point>> contours;
        findContours(src, contours, RETR_EXTERNAL, CHAIN_APPROX_NONE);
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
    vector<Point> contour_5 = {Point(27.9, 7.5), Point(27.9, 22.5),
                               Point(15, 30), Point(2.1, 22.5), Point(2.1, 7.5)};
    RuneCenter::ptr rc = RuneCenter::make_feature(contour_5);
    EXPECT_FALSE(rc);
}

} // namespace rm_test

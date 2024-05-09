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
using namespace std;
using namespace cv;

namespace rm_test
{

class BuildRuneTargetTest : public testing::Test
{
public:
    vector<Point> contour;

    void SetUp() override
    {
        // 一般轮廓
        Mat src = Mat::zeros(Size(1000, 1000), CV_8UC1);
        circle(src, Point(500, 500), 45, Scalar(255), 2);
        vector<vector<Point>> contours;
        findContours(src, contours, RETR_EXTERNAL, CHAIN_APPROX_NONE);
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
    Mat img = Mat::zeros(Size(1000, 1000), CV_8UC1);
    rectangle(img, Point(400, 400), Point(600, 500), Scalar(255, 255, 255), -1);
    vector<vector<Point>> contours;
    findContours(img, contours, RETR_EXTERNAL, CHAIN_APPROX_NONE);
    vector<Point> contour_ = contours.front();
    RuneTarget::ptr rt3 = RuneTarget::make_feature(contour_, 0);
    EXPECT_FALSE(rt3);

    // 过高轮廓
    img = Mat::zeros(Size(1000, 1000), CV_8UC1);
    rectangle(img, Point(400, 400), Point(500, 600), Scalar(255, 255, 255), -1);
    findContours(img, contours, RETR_EXTERNAL, CHAIN_APPROX_NONE);
    contour_ = contours.front();
    RuneTarget::ptr rt4 = RuneTarget::make_feature(contour_, 0);
    EXPECT_FALSE(rt4);
}

TEST_F(BuildRuneTargetTest, few_contourSize)
{
    // 非正常轮廓点数构建神符靶心
    vector<Point> contour_4 = {Point(500, 455), Point(455, 500), Point(500, 545), Point(545, 500)};
    RuneTarget::ptr rt = RuneTarget::make_feature(contour_4, 0);
    EXPECT_FALSE(rt);
}

} // namespace rm_test

#endif // HAVE_RMVL_RUNE_TARGET

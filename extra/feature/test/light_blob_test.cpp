/**
 * @file light_blob_test.cpp
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

#include "rmvl/feature/light_blob.h"
#include "rmvlpara/feature/light_blob.h"

using namespace rm;
using namespace para;
using namespace std;
using namespace cv;

namespace rm_test
{

TEST(BuildLightBlobTest, fitEllipse_angle_contourSize_judgeTable)
{
    // 数据准备
    vector<Point> contour;
    // 轮廓点数为 6，小倾角
    contour = {Point(100, 100), Point(105, 105), Point(95, 105),
               Point(100, 200), Point(105, 195), Point(95, 195)};
    LightBlob::ptr blob1 = LightBlob::make_feature(contour);
    EXPECT_TRUE(blob1 != nullptr);

    // 一般个数轮廓点数，小倾角
    contour.clear();
    Mat src = Mat::zeros(Size(150, 150), CV_8UC1);
    line(src, Point(75, 20), Point(75, 130), Scalar(255), 25);
    vector<vector<Point>> contours;
    findContours(src, contours, RETR_EXTERNAL, CHAIN_APPROX_NONE);
    contour = contours.front();
    LightBlob::ptr blob2 = LightBlob::make_feature(contour);
    EXPECT_TRUE(blob2 != nullptr);

    // 轮廓点数为 6，大倾角
    contour.clear();
    contour = {Point(100, 100), Point(105, 100), Point(100, 105),
               Point(170, 170), Point(165, 170), Point(170, 165)};
    LightBlob::ptr blob3 = LightBlob::make_feature(contour);
    EXPECT_FALSE(blob3 != nullptr);

    // 一般个数轮廓点数，大倾角
    contour.clear();
    src = Mat::zeros(Size(150, 150), CV_8UC1);
    line(src, Point(30, 30), Point(120, 120), Scalar(255), 25);
    contours.clear();
    findContours(src, contours, RETR_EXTERNAL, CHAIN_APPROX_NONE);
    contour = contours.front();
    LightBlob::ptr blob4 = LightBlob::make_feature(contour);
    EXPECT_FALSE(blob4 != nullptr);
}

TEST(BuildLightBlobTest, fitEllipse_few_contourSize)
{
    // 轮廓点数为 5
    vector<Point> contour = {Point(100, 100), Point(105, 105), Point(95, 105),
                             Point(100, 200), Point(105, 195)};
    LightBlob::ptr blob = LightBlob::make_feature(contour);
    EXPECT_FALSE(blob != nullptr);
}

TEST(BuildLightBlobTest, fitEllipse_width_contourSize_judgeTable)
{
    // 数据准备
    vector<Point> contour;

    // 一般个数轮廓点数，过细灯条，小倾角
    contour.clear();
    Mat src = Mat::zeros(Size(150, 150), CV_8UC1);
    line(src, Point(75, 20), Point(75, 130), Scalar(255), 3);
    vector<vector<Point>> contours;
    findContours(src, contours, RETR_EXTERNAL, CHAIN_APPROX_NONE);
    contour = contours.front();
    LightBlob::ptr blob1 = LightBlob::make_feature(contour);
    EXPECT_FALSE(blob1 != nullptr);

    // 一般个数轮廓点数，过细灯条，大倾角
    contour.clear();
    src = Mat::zeros(Size(150, 150), CV_8UC1);
    line(src, Point(30, 30), Point(120, 120), Scalar(255), 3);
    contours.clear();
    findContours(src, contours, RETR_EXTERNAL, CHAIN_APPROX_NONE);
    contour = contours.front();
    LightBlob::ptr blob2 = LightBlob::make_feature(contour);
    EXPECT_FALSE(blob2 != nullptr);
}

} // namespace rm_test

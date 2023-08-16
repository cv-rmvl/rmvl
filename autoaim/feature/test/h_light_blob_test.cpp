/**
 * @file h_light_blob_test.cpp
 * @author RoboMaster Vision Community
 * @brief 水平灯条
 * @version 1.0
 * @date 2022-12-16
 *
 * @copyright Copyright 2022 (c), RoboMaster Vision Community
 *
 */

#include <gtest/gtest.h>
#include <opencv2/imgproc.hpp>

#include "rmvl/feature/h_light_blob.h"
#include "rmvl/rmath.hpp"

using namespace rm;
using namespace std;
using namespace cv;

namespace rm_test
{

class BuildH_LightBlobTest : public testing::Test
{
public:
    h_light_blob_ptr h_light_blob;
    Mat src;
    vector<Point> d_contour;
    Point2f base = Point2f(50, 50);

    void SetUp() override {}
    void TearDown() override {}

    vector<Point> buildH_LightContours_angle(float angle, Point2f bias = Point2f())
    {
        // 改110
        Point2f blob_bias(static_cast<int>(110 * cos(deg2rad(angle))),
                          static_cast<int>(110 * sin(deg2rad(angle))));
        Mat src = Mat::zeros(Size(200 + bias.x + blob_bias.x, 200 + bias.y + blob_bias.y), CV_8UC1);
        line(src, base + bias, base + blob_bias + bias, Scalar(255), 10);
        vector<vector<Point>> contours;
        findContours(src, contours, RETR_EXTERNAL, CHAIN_APPROX_NONE);
        return contours.front();
    }

    vector<Point> buildH_LightContours_ratio(float delta_w, float delta_h)
    {
        Mat src = Mat::zeros(Size(200, 200), CV_8UC1);
        rectangle(src, base, base + Point2f(delta_w, delta_h), Scalar(255), FILLED);
        vector<vector<Point>> contours;
        findContours(src, contours, RETR_EXTERNAL, CHAIN_APPROX_NONE);
        return contours.front();
    }
};

TEST_F(BuildH_LightBlobTest, wrong_angel)
{
    vector<Point> contour;
    // 错误的角度
    contour = buildH_LightContours_angle(30, Point2f());
    h_light_blob = H_LightBlob::make_feature(contour);
    EXPECT_FALSE(h_light_blob != nullptr);
    // 正确的角度
    contour = buildH_LightContours_angle(20, Point2f());
    h_light_blob = H_LightBlob::make_feature(contour);
    EXPECT_FALSE(h_light_blob = nullptr);
}

TEST_F(BuildH_LightBlobTest, wrong_wh_ratio)
{
    vector<Point> contour;
    // 错误的长宽比
    contour = buildH_LightContours_ratio(100, 5);
    h_light_blob = H_LightBlob::make_feature(contour);
    EXPECT_FALSE(h_light_blob != nullptr);
    // 正确的长宽比
    contour = buildH_LightContours_ratio(100, 20);
    h_light_blob = H_LightBlob::make_feature(contour);
    EXPECT_FALSE(h_light_blob = nullptr);
}

} // namespace rm_test

/**
 * @file TopArmor_test.cpp
 * @author RoboMaster Vision Community
 * @brief 引导灯测试
 * @version 1.0
 * @date 2022-12-11
 *
 * @copyright Copyright 2022 (c), RoboMaster Vision Community
 *
 */

#include <gtest/gtest.h>
#include <opencv2/imgproc.hpp>

#include "rmvl/combo/top_armor.h"

using namespace rm;
using namespace std;
using namespace cv;

namespace rm_test
{

class BuildTopArmorTest : public testing::Test
{
public:
    pilot_ptr Pilot;
    h_light_blob_ptr h_light_blob;
    Point2f center;
    Point2f base = Point2f(50, 50);

    void SetUp() override
    {
        h_light_blob = buildBlob(0, Point2f(0, 0));
        Pilot = buildPilot(0.f, Point2f(0, 0), center, 110);
    }
    void TearDown() override {}

    /**
     * @brief 构造灯条
     *
     * @param angle 倾斜角
     * @param bias 偏置
     * @return light_blob_ptr
     */
    h_light_blob_ptr buildBlob(float angle, Point2f bias = Point2f())
    {
        // 改110
        Point2f blob_bias(static_cast<int>(110 * cos(deg2rad(angle))),
                          static_cast<int>(110 * sin(deg2rad(angle))));
        Mat src = Mat::zeros(Size(500 + bias.x + blob_bias.x, 500 + bias.y + blob_bias.y), CV_8UC1);
        line(src, base + bias, base + blob_bias + bias, Scalar(255), 10);
        center = ((base + bias) + (base + blob_bias + bias)) / 2;
        vector<vector<Point>> contours;
        findContours(src, contours, RETR_EXTERNAL, CHAIN_APPROX_NONE);
        return H_LightBlob::make_feature(contours.front());
    }

    /**
     * @brief
     *
     * @param angle 与构造灯条的角度成相反数
     * @param bias 偏置
     * @param center 水平灯条中心点
     * @return pilot_ptr
     */
    pilot_ptr buildPilot(float angle, Point2f bias, Point2f center, float distance)
    {
        Point2f pilot_bias(static_cast<int>(distance * sin(deg2rad(angle))),
                           static_cast<int>(distance * cos(deg2rad(angle))));
        Mat src = Mat::zeros(Size(200 + bias.x + pilot_bias.x, 200 + bias.y + pilot_bias.y), CV_8UC1);
        circle(src, center + pilot_bias, 10, Scalar(125), FILLED);
        vector<vector<Point>> contours;
        findContours(src, contours, RETR_EXTERNAL, CHAIN_APPROX_NONE);
        return Pilot::make_feature(contours.front(), src);
    }
};

TEST_F(BuildTopArmorTest, incorrect_h_blob)
{
    h_light_blob_ptr h_blob = nullptr;
    // 一个灯条为空
    top_armor_ptr TopArmor = TopArmor::make_combo(Pilot, h_blob, GyroData(), getTickCount());
    EXPECT_FALSE(TopArmor != nullptr);
    // 一个灯条为假
    vector<Point> null_contour;
    h_blob = H_LightBlob::make_feature(null_contour);
    TopArmor.reset();
    TopArmor = TopArmor::make_combo(Pilot, h_blob, GyroData(), getTickCount());
    EXPECT_FALSE(TopArmor != nullptr);
}

TEST_F(BuildTopArmorTest, different_distance)
{
    pilot_ptr Pilot_test;
    // 间距过大
    Pilot_test = buildPilot(0, Point2f(0, 0), center, 600);
    top_armor_ptr TopArmor = TopArmor::make_combo(Pilot_test, h_light_blob, GyroData(), getTickCount());
    EXPECT_FALSE(TopArmor != nullptr);
    // 正常间距
    TopArmor.reset();
    Pilot_test = buildPilot(0, Point2f(0, 0), center, 200);
    TopArmor = TopArmor::make_combo(Pilot_test, h_light_blob, GyroData(), getTickCount());
    EXPECT_FALSE(TopArmor != nullptr);
    // 间距过小
    TopArmor.reset();
    Pilot_test = buildPilot(0, Point2f(0, 0), center, 100);
    TopArmor = TopArmor::make_combo(Pilot_test, h_light_blob, GyroData(), getTickCount());
    EXPECT_FALSE(TopArmor = nullptr);
}

} // namespace rm_test

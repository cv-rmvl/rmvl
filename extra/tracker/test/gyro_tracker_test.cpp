/**
 * @file gyro_tracker_test.cpp
 * @author RoboMaster Vision Community
 * @brief 装甲板追踪器单元测试
 * @version 0.1
 * @date 2022-12-24
 *
 * @copyright Copyright 2022 (c), RoboMaster Vision Community
 *
 */

#include <gtest/gtest.h>
#include <opencv2/imgproc.hpp>

#include "rmvl/tracker/gyro_tracker.h"

#include "rmvlpara/camera.hpp"

using namespace rm;
using namespace para;
using namespace std;
using namespace cv;

namespace rm_test
{

class GyroTrackerTest : public testing::Test
{
    Mat src;

public:
    int64 tick = getTickCount();
    GyroData gyro_data = GyroData();

    void SetUp() override
    {
        camera_param.cameraMatrix = {1500, 0, 640,
                                     0, 1500, 512,
                                     0, 0, 1};
        camera_param.distCoeff = {-5.4755376911541398e-01,
                                  3.4944810516243208e-01,
                                  -5.7851122028406655e-03,
                                  2.1845982977120020e-03,
                                  0.};
    }

    void TearDown() override {}

    /**
     * @brief 构建 Armor
     *
     * @param center 装甲板中心点
     * @param angle 装甲板倾角
     * @return
     */
    armor_ptr buildArmor(Point center, float angle)
    {
        light_blob_ptr left_blob = buildBlob(angle, center - Point(125 * cos(deg2rad(angle)), 125 * sin(deg2rad(angle))));
        light_blob_ptr right_blob = buildBlob(angle, center + Point(125 * cos(deg2rad(angle)), 125 * sin(deg2rad(angle))));
        return Armor::make_combo(left_blob, right_blob, GyroData(), getTickCount());
    }

    /**
     * @brief 构造 LightBlob
     *
     * @param angle 倾斜角
     * @param center 中心点
     * @return
     */
    light_blob_ptr buildBlob(float angle, Point center)
    {
        src = Mat::zeros(Size(1280, 1024), CV_8UC1);
        Point base_bias(static_cast<int>(-110 * sin(deg2rad(angle))),
                        static_cast<int>(110 * cos(deg2rad(angle))));
        line(src, center - base_bias / 2, center + base_bias / 2, Scalar(255), 12);
        vector<vector<Point>> contours;
        findContours(src, contours, RETR_EXTERNAL, CHAIN_APPROX_NONE);
        return LightBlob::make_feature(contours.front());
    }
};

// 初始化构建功能验证
TEST_F(GyroTrackerTest, initial_build_function_test)
{
    // 传入真实装甲板
    armor_ptr armor = buildArmor(Point(500, 300), 8);
    tracker_ptr p_tracker = GyroTracker::make_tracker(armor);
    EXPECT_EQ(p_tracker->size(), 1);
    EXPECT_EQ(p_tracker->at(0), armor);
}

// 追踪器传入装甲板更新功能验证
TEST_F(GyroTrackerTest, tracker_update_with_1_armor)
{
    // 连续传入 2 个装甲板
    armor_ptr armor = buildArmor(Point(500, 300), 8);
    tracker_ptr p_tracker = GyroTracker::make_tracker(armor);
    armor_ptr armor2 = buildArmor(Point(505, 300), 8);
    p_tracker->update(armor2, tick, gyro_data);
    EXPECT_EQ(p_tracker->size(), 2);
    EXPECT_NE(p_tracker->getRelativeAngle(), armor2->getRelativeAngle());
}

} // namespace rm_test

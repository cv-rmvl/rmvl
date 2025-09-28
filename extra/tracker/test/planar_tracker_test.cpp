/**
 * @file planar_tracker_test.cpp
 * @author RoboMaster Vision Community
 * @brief 使用装甲板完成平面目标追踪器的单元测试
 * @version 1.0
 * @date 2022-08-28
 *
 * @copyright Copyright 2023 (c), RoboMaster Vision Community
 *
 */

#include "rmvl/rmvl_modules.hpp"

#ifdef HAVE_RMVL_PLANAR_TRACKER

#include <gtest/gtest.h>
#include <opencv2/imgproc.hpp>

#include "rmvl/algorithm/math.hpp"
#include "rmvl/core/timer.hpp"
#include "rmvl/tracker/planar_tracker.h"

#include "rmvlpara/camera/camera.h"

namespace rm_test {

class PlanarTrackerTest : public testing::Test {
    cv::Mat src;

public:
    double tick{rm::Timer::now()};
    rm::ImuData imu_data{};

    void SetUp() override {
        rm::para::camera_param.cameraMatrix = {1500, 0, 640,
                                               0, 1500, 512,
                                               0, 0, 1};
        rm::para::camera_param.distCoeffs = {-5.4755376911541398e-01,
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
    rm::Armor::ptr buildArmor(cv::Point center, float angle) {
        rm::LightBlob::ptr left_blob = buildBlob(angle, center - cv::Point(125 * cos(rm::deg2rad(angle)), 125 * sin(rm::deg2rad(angle))));
        rm::LightBlob::ptr right_blob = buildBlob(angle, center + cv::Point(125 * cos(rm::deg2rad(angle)), 125 * sin(rm::deg2rad(angle))));
        return rm::Armor::make_combo(left_blob, right_blob, rm::ImuData(), rm::Timer::now());
    }

    /**
     * @brief 构造 LightBlob
     *
     * @param angle 倾斜角
     * @param center 中心点
     * @return
     */
    rm::LightBlob::ptr buildBlob(float angle, cv::Point center) {
        src = cv::Mat::zeros(cv::Size(1280, 1024), CV_8UC1);
        cv::Point base_bias(static_cast<int>(-110 * sin(rm::deg2rad(angle))),
                            static_cast<int>(110 * cos(rm::deg2rad(angle))));
        cv::line(src, center - base_bias / 2, center + base_bias / 2, cv::Scalar(255), 12);
        std::vector<std::vector<cv::Point>> contours;
        cv::findContours(src, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_NONE);
        return rm::LightBlob::make_feature(contours.front());
    }
};

// 初始化构建功能验证
TEST_F(PlanarTrackerTest, initial_build_function_test) {
    // 传入真实装甲板
    rm::Armor::ptr armor = buildArmor(cv::Point(500, 300), 8);
    rm::tracker::ptr p_tracker = rm::PlanarTracker::make_tracker(armor);
    EXPECT_EQ(p_tracker->size(), 1);
    EXPECT_EQ(p_tracker->at(0), armor);
}

// 追踪器传入装甲板更新功能验证
TEST_F(PlanarTrackerTest, tracker_update_with_1_armor) {
    // 连续传入 2 个装甲板
    rm::Armor::ptr armor = buildArmor(cv::Point(500, 300), 8);
    rm::tracker::ptr p_tracker = rm::PlanarTracker::make_tracker(armor);
    rm::Armor::ptr armor2 = buildArmor(cv::Point(505, 300), 8);
    p_tracker->update(armor2);
    EXPECT_EQ(p_tracker->size(), 2);
    // Kalman Filter 初始化时，观测值为 armor2，初速度为 0，因此先验估计值为 armor
    // 最优估计应该更倾向于 armor2
    float da1 = rm::getDistance(p_tracker->getRelativeAngle(), armor->getRelativeAngle());
    float da2 = rm::getDistance(p_tracker->getRelativeAngle(), armor2->getRelativeAngle());
    EXPECT_GE(da1, da2);
}

// 追踪器掉帧处理功能验证
TEST_F(PlanarTrackerTest, tracker_update_with_none) {
    // 传入装甲板后传入空
    rm::Armor::ptr armor = buildArmor(cv::Point(500, 300), 8);
    rm::tracker::ptr p_tracker = rm::PlanarTracker::make_tracker(armor);
    p_tracker->update(tick, imu_data);
    EXPECT_EQ(p_tracker->size(), 2);
    EXPECT_EQ(p_tracker->front()->center(), armor->center());
    EXPECT_EQ(p_tracker->getVanishNumber(), 1);
}

} // namespace rm_test

#endif // HAVE_RMVL_PLANAR_TRACKER

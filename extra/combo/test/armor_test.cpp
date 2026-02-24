/**
 * @file armor_test.cpp
 * @author RoboMaster Vision Community
 * @brief 装甲板测试
 * @version 1.0
 * @date 2022-08-15
 *
 * @copyright Copyright 2023 (c), RoboMaster Vision Community
 *
 */

#include "rmvl/rmvl_modules.hpp"

#ifdef HAVE_RMVL_ARMOR

#include <gtest/gtest.h>
#include <opencv2/imgproc.hpp>

#include "rmvl/algorithm/math.hpp"
#include "rmvl/combo/armor.h"
#include "rmvl/core/timer.hpp"

using namespace rm;

namespace rm_test {

class BuildArmorTest : public testing::Test {
public:
    // x = 15 左灯条
    LightBlob::ptr left_blob;

    void SetUp() override { left_blob = buildBlob(0.f); }
    void TearDown() override {}

    /**
     * @brief 构造灯条
     *
     * @param angle 倾斜角
     * @param bias 偏置
     * @return LightBlob::ptr
     */
    LightBlob::ptr buildBlob(float angle, cv::Point bias = cv::Point()) {
        cv::Point base(500, 500);
        cv::Point base_bias(static_cast<int>(110 * sin(deg2rad(angle))),
                            static_cast<int>(110 * -cos(deg2rad(angle))));
        cv::Mat src = cv::Mat::zeros(cv::Size(2000, 2000), CV_8UC1);
        line(src, base + bias, base + base_bias + bias, cv::Scalar(255), 25);
        std::vector<std::vector<cv::Point>> contours;
        findContours(src, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_NONE);
        return LightBlob::make_feature(contours.front());
    }
};

TEST_F(BuildArmorTest, incorrect_blob) {
    LightBlob::ptr right_blob = nullptr;
    // 一个灯条为空
    Armor::ptr armor = Armor::make_combo(left_blob, right_blob, ImuData(), Time::now());
    EXPECT_FALSE(armor != nullptr);
    // 一个灯条为假
    std::vector<cv::Point> null_contour;
    right_blob = LightBlob::make_feature(null_contour);
    armor.reset();
    armor = Armor::make_combo(left_blob, right_blob, ImuData(), Time::now());
    EXPECT_FALSE(armor != nullptr);
}

TEST_F(BuildArmorTest, different_blob_distance) {
    LightBlob::ptr right_blob;
    // 小间距
    right_blob = buildBlob(0, cv::Point2f(30, 0));
    Armor::ptr armor1 = Armor::make_combo(left_blob, right_blob, ImuData(), Time::now());
    EXPECT_EQ(armor1, nullptr);
    // 正常间距
    right_blob = buildBlob(0, cv::Point2f(200, 0));
    Armor::ptr armor2 = Armor::make_combo(left_blob, right_blob, ImuData(), Time::now());
    EXPECT_NE(armor2, nullptr);
    // 大间距
    right_blob = buildBlob(0, cv::Point2f(1000, 0));
    Armor::ptr armor3 = Armor::make_combo(left_blob, right_blob, ImuData(), Time::now());
    EXPECT_EQ(armor3, nullptr);
}

TEST_F(BuildArmorTest, different_blob_tiltAngle) {
    left_blob.reset();
    LightBlob::ptr right_blob;
    // 小倾角
    left_blob = buildBlob(5);
    right_blob = buildBlob(5, cv::Point(200, 10));
    Armor::ptr armor1 = Armor::make_combo(left_blob, right_blob, ImuData(), Time::now());
    EXPECT_NE(armor1, nullptr);
    // 大倾角
    left_blob = buildBlob(45);
    right_blob = buildBlob(45, cv::Point(180, 180));
    Armor::ptr armor2 = Armor::make_combo(left_blob, right_blob, ImuData(), Time::now());
    EXPECT_EQ(armor2, nullptr);
}

TEST_F(BuildArmorTest, different_blob_angle) {
    LightBlob::ptr right_blob;
    // 小夹角
    right_blob = buildBlob(5, cv::Point(200, 0));
    Armor::ptr armor1 = Armor::make_combo(left_blob, right_blob, ImuData(), Time::now());
    EXPECT_NE(armor1, nullptr);
    // 大夹角
    right_blob = buildBlob(15, cv::Point(200, 0));
    Armor::ptr armor2 = Armor::make_combo(left_blob, right_blob, ImuData(), Time::now());
    EXPECT_EQ(armor2, nullptr);
}

} // namespace rm_test

#endif // HAVE_RMVL_ARMOR

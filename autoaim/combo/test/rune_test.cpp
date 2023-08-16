/**
 * @file rune_test.cpp
 * @author RoboMaster Vision Community
 * @brief
 * @version 0.1
 * @date 2023-04-26
 *
 * @copyright Copyright 2023 (c), RoboMaster Vision Community
 *
 */

#include <gtest/gtest.h>

#include <opencv2/imgproc.hpp>

#define private public
#define protected public

#include "rmvl/combo/rune.h"

#undef private
#undef protected

using namespace rm;
using namespace std;
using namespace cv;

namespace rm_test
{

TEST(BuildRuneTest, incorrect_rune_feature)
{
    // 均为空
    rune_ptr rune1 = Rune::make_combo(nullptr, nullptr, GyroData(), getTickCount());
    EXPECT_FALSE(rune1);
    // 均为假
    vector<Point> none_vec;
    rune_target_ptr rune_target = RuneTarget::make_feature(none_vec, false);
    rune_center_ptr rune_center = RuneCenter::make_feature(none_vec);
    combo_ptr combo_2 = Rune::make_combo(rune_target, rune_center, GyroData(), getTickCount());
    EXPECT_FALSE(combo_2);
}

TEST(BuildRuneTest, calculate_rune_angle_0)
{
    auto rune_target = RuneTarget::make_feature(Point2f(800, 500), false);
    auto rune_center = RuneCenter::make_feature(Point2f(500, 500));
    auto rune = Rune::make_combo(rune_target, rune_center, GyroData(), getTickCount());
    EXPECT_TRUE(rune != nullptr);
    // 角度判断
    EXPECT_LE(abs(rune->getAngle()), 1.f);
}

TEST(BuildRuneTest, calculate_rune_angle_45)
{
    auto rune_target = RuneTarget::make_feature(Point2f(712, 288), false);
    auto rune_center = RuneCenter::make_feature(Point2f(500, 500));
    auto rune = Rune::make_combo(rune_target, rune_center, GyroData(), getTickCount());
    EXPECT_TRUE(rune != nullptr);
    // 角度判断
    EXPECT_LE(abs(rune->getAngle() - 45.f), 5.f);
}

TEST(BuildRuneTest, calculate_rune_angle_90)
{
    auto rune_target = RuneTarget::make_feature(Point2f(500, 200), false);
    auto rune_center = RuneCenter::make_feature(Point2f(500, 500));
    auto rune = Rune::make_combo(rune_target, rune_center, GyroData(), getTickCount());
    EXPECT_TRUE(rune != nullptr);
    // 角度判断
    EXPECT_LE(abs(rune->getAngle() - 90.f), 1.f);
}

TEST(BuildRuneTest, calculate_rune_angle_minus_90)
{
    auto rune_target = RuneTarget::make_feature(Point2f(500, 800), false);
    auto rune_center = RuneCenter::make_feature(Point2f(500, 500));
    auto rune = Rune::make_combo(rune_target, rune_center, GyroData(), getTickCount());
    EXPECT_TRUE(rune != nullptr);
    // 角度判断
    EXPECT_LE(abs(rune->getAngle() + 90.f), 1.f);
}

} // namespace rm_test

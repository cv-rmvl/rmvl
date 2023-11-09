/**
 * @file armor_match_test.cpp
 * @author RoboMaster Vision Community
 * @brief 装甲板匹配单元测试
 * @version 1.0
 * @date 2022-08-30
 *
 * @copyright Copyright 2023 (c), RoboMaster Vision Community
 *
 */

#include "armor_detector_test.h"

using namespace rm_test;
using namespace rm;
using namespace std;
using namespace cv;

// 较多追踪器与较少装甲板匹配策略功能验证
TEST_F(ArmorDetectorTest, more_tracker_match_less_armor)
{
    buildArmorImg(Point(200, 800), -5.f);
    buildArmorImg(Point(600, 400), 5.f);
    p_detector->detect(groups, src, RED, GyroData(), timer.now());
    resetImg();
    buildArmorImg(Point(610, 405), 3.f);
    auto info = p_detector->detect(groups, src, RED, GyroData(), timer.now());

    auto &trackers = groups.front()->data();
    EXPECT_EQ(info.combos.size(), 1);
    EXPECT_EQ(trackers.size(), 2);

    EXPECT_EQ(trackers[0]->getVanishNumber(), 1);
    EXPECT_EQ(trackers[1]->getVanishNumber(), 0);
}

// 追踪器与同等装甲板匹配策略功能验证
TEST_F(ArmorDetectorTest, tracker_match_an_equal_number_of_armor)
{
    // 1 追踪器 1 帧间距离较近的装甲板
    buildArmorImg(Point(200, 800), -5.f);
    auto info = p_detector->detect(groups, src, RED, GyroData(), timer.now());
    resetImg();
    buildArmorImg(Point(210, 805), -7.f);
    p_detector->detect(groups, src, RED, GyroData(), timer.now());

    auto &trackers = groups.front()->data();

    EXPECT_EQ(trackers.front()->getVanishNumber(), 0);
    // 1 追踪器 1 帧间距离较远的装甲板
    resetImg();
    resetDetector();
    buildArmorImg(Point(200, 800), -5.f);
    info = p_detector->detect(groups, src, RED, GyroData(), timer.now());
    resetImg();
    buildArmorImg(Point(1000, 200), -7.f);
    info = p_detector->detect(groups, src, RED, GyroData(), timer.now());

    trackers = groups.front()->data();
    EXPECT_EQ(info.combos.size(), 1);
    EXPECT_EQ(trackers.size(), 2);

    EXPECT_EQ(trackers[0]->getVanishNumber(), 1);
    EXPECT_EQ(trackers[1]->getVanishNumber(), 0);
}

// 较少追踪器与较多装甲板匹配策略功能验证
TEST_F(ArmorDetectorTest, less_tracker_match_more_armor)
{
    buildArmorImg(Point(600, 400), 5.f);
    auto info = p_detector->detect(groups, src, RED, GyroData(), timer.now());
    resetImg();
    buildArmorImg(Point(200, 800), -5.f);
    buildArmorImg(Point(610, 405), 3.f);
    info = p_detector->detect(groups, src, RED, GyroData(), timer.now());

    auto &trackers = groups.front()->data();
    EXPECT_EQ(info.combos.size(), 2);
    EXPECT_EQ(trackers.size(), 2);

    EXPECT_EQ(trackers[0]->getVanishNumber(), 0);
    EXPECT_EQ(trackers[1]->getVanishNumber(), 0);
}

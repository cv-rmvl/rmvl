/**
 * @file rune_detector_test.cpp
 * @author RoboMaster Vision Community
 * @brief
 * @version 0.1
 * @date 2023-04-27
 *
 * @copyright Copyright 2023 (c), RoboMaster Vision Community
 *
 */

#include "rmvl/rmvl_modules.hpp"

#ifdef HAVE_RMVL_RUNE_DETECTOR

#include <gtest/gtest.h>

#include <opencv2/imgproc.hpp>

#include "rmvl/algorithm/math.hpp"
#include "rmvl/algorithm/pretreat.hpp"
#include "rmvl/combo/rune.h"
#include "rmvl/core/timer.hpp"
#include "rmvl/detector/rune_detector.h"

using namespace rm::numeric_literals;

namespace rm_test {

class RuneDetectorTest : public testing::Test {
public:
    cv::Mat src;
    rm::group::ptr group;
    rm::RuneDetector::ptr p_detector;

    void SetUp() override {
        p_detector = rm::RuneDetector::make_detector();
        src = cv::Mat::zeros(cv::Size(1278, 1024), CV_8UC3);
    }
    void TearDown() override {}

    inline void createRuneCenter(int center_x, int center_y) {
        putText(src, "R", cv::Point(center_x, center_y), cv::FONT_HERSHEY_COMPLEX, 1., cv::Scalar(0, 0, 255), 4);
    }

    /**
     * @brief 绘制神符靶心
     *
     * @param target_x 神符靶心x坐标
     * @param target_y 神符靶心y坐标
     * @param theta 神符角度(弧度制),从中心到靶心的角度,以x轴正方向为0，(-PI,PI]
     * @param active 神符是否激活
     * @param ring_num 激活神符环数
     */
    void createRuneTarget(int target_x, int target_y, float theta = 0.f, bool active = false, float ring_num = 9) {
        // 在图像上构建神符扇叶
        // 未激活神符
        if (!active) {
            // 神符靶心
            circle(src, cv::Point(target_x, target_y), 8, cv::Scalar(0, 0, 255), 2);
            // 神符辅助瞄准特征
            circle(src, cv::Point(target_x, target_y), 24, cv::Scalar(0, 0, 255), 2);
            circle(src, cv::Point(target_x, target_y), 44, cv::Scalar(0, 0, 255), 2);
            // 远（上）
            line(src, cv::Point(target_x + 20 * std::cos(theta), target_y - 20 * std::sin(theta)),
                 cv::Point(target_x + 52 * std::cos(theta), target_y - 52 * std::sin(theta)), cv::Scalar(0, 0, 255), 4);
            // 近（下）
            line(src, cv::Point(target_x - 20 * std::cos(theta), target_y + 20 * std::sin(theta)),
                 cv::Point(target_x - 52 * std::cos(theta), target_y + 52 * std::sin(theta)), cv::Scalar(0, 0, 255), 4);
            // 左
            line(src, cv::Point(target_x + 20 * std::cos(theta + 90_to_rad), target_y - 20 * std::sin(theta + 90_to_rad)),
                 cv::Point(target_x + 52 * std::cos(theta + 90_to_rad), target_y - 52 * std::sin(theta + 90_to_rad)), cv::Scalar(0, 0, 255), 4);
            // 右
            line(src, cv::Point(target_x - 20 * std::cos(theta + 90_to_rad), target_y + 20 * std::sin(theta + 90_to_rad)),
                 cv::Point(target_x - 52 * std::cos(theta + 90_to_rad), target_y + 52 * std::sin(theta + 90_to_rad)), cv::Scalar(0, 0, 255), 4);
        }
        // 已激活神符
        else {
            // 神符靶心
            circle(src, cv::Point(target_x, target_y), 5 * (11 - ring_num), cv::Scalar(0, 0, 255), 2);
        }
        // 神符支架
        // 外支架
        // 横
        line(src, cv::Point(target_x + 70 * std::cos(theta) + 60 * std::cos(theta + 90_to_rad), target_y - 70 * std::sin(theta) - 60 * std::sin(theta + 90_to_rad)),
             cv::Point(target_x + 70 * std::cos(theta) - 60 * std::cos(theta + 90_to_rad), target_y - 70 * std::sin(theta) + 60 * std::sin(theta + 90_to_rad)),
             cv::Scalar(0, 0, 255), 8);
        // 竖
        // 左
        line(src, cv::Point(target_x + 70 * std::cos(theta) + 60 * std::cos(theta + 90_to_rad), target_y - 70 * std::sin(theta) - 60 * std::sin(theta + 90_to_rad)),
             cv::Point(target_x + 25 * std::cos(theta) + 60 * std::cos(theta + 90_to_rad), target_y - 25 * std::sin(theta) - 60 * std::sin(theta + 90_to_rad)),
             cv::Scalar(0, 0, 255), 8);
        // 右
        line(src, cv::Point(target_x + 70 * std::cos(theta) - 60 * std::cos(theta + 90_to_rad), target_y - 70 * std::sin(theta) + 60 * std::sin(theta + 90_to_rad)),
             cv::Point(target_x + 25 * std::cos(theta) - 60 * std::cos(theta + 90_to_rad), target_y - 25 * std::sin(theta) + 60 * std::sin(theta + 90_to_rad)),
             cv::Scalar(0, 0, 255), 8);

        // 内支架
        // 横
        line(src, cv::Point(target_x - 70 * std::cos(theta) + 60 * std::cos(theta + 90_to_rad), target_y + 70 * std::sin(theta) - 60 * std::sin(theta + 90_to_rad)),
             cv::Point(target_x - 70 * std::cos(theta) - 60 * std::cos(theta + 90_to_rad), target_y + 70 * std::sin(theta) + 60 * std::sin(theta + 90_to_rad)),
             cv::Scalar(0, 0, 255), 8);
        // 竖
        // 左
        line(src, cv::Point(target_x - 70 * std::cos(theta) + 60 * std::cos(theta + 90_to_rad), target_y + 70 * std::sin(theta) - 60 * std::sin(theta + 90_to_rad)),
             cv::Point(target_x - 25 * std::cos(theta) + 60 * std::cos(theta + 90_to_rad), target_y + 25 * std::sin(theta) - 60 * std::sin(theta + 90_to_rad)),
             cv::Scalar(0, 0, 255), 8);
        // 右
        line(src, cv::Point(target_x - 70 * std::cos(theta) - 60 * std::cos(theta + 90_to_rad), target_y + 70 * std::sin(theta) + 60 * std::sin(theta + 90_to_rad)),
             cv::Point(target_x - 25 * std::cos(theta) - 60 * std::cos(theta + 90_to_rad), target_y + 25 * std::sin(theta) + 60 * std::sin(theta + 90_to_rad)),
             cv::Scalar(0, 0, 255), 8);

        // 内支架连接
        // 中心
        line(src, cv::Point(target_x - 78 * std::cos(theta), target_y + 78 * std::sin(theta)),
             cv::Point(target_x - 162 * std::cos(theta), target_y + 162 * std::sin(theta)), cv::Scalar(0, 0, 255), 16);
        // 未激活神符
        if (!active) {
            // 间断
            for (int i = 80; i < 190; i += 10)
                line(src, cv::Point(target_x - i * std::cos(theta) + 25 * std::cos(theta + 90_to_rad), target_y + i * std::sin(theta) - 25 * std::sin(theta + 90_to_rad)),
                     cv::Point(target_x - i * std::cos(theta) - 25 * std::cos(theta + 90_to_rad), target_y + i * std::sin(theta) + 25 * std::sin(theta + 90_to_rad)),
                     cv::Scalar(0, 0, 0), 4);
        }

        // 已激活神符
        else {
            // 左
            line(src, cv::Point(target_x - 70 * std::cos(theta) + 62 * std::cos(theta + 90_to_rad), target_y + 70 * std::sin(theta) - 62 * std::sin(theta + 90_to_rad)),
                 cv::Point(target_x - 120 * std::cos(theta) + 62 * std::cos(theta + 90_to_rad), target_y + 120 * std::sin(theta) - 62 * std::sin(theta + 90_to_rad)),
                 cv::Scalar(0, 0, 255), 4);
            line(src, cv::Point(target_x - 120 * std::cos(theta) + 62 * std::cos(theta + 90_to_rad), target_y + 120 * std::sin(theta) - 62 * std::sin(theta + 90_to_rad)),
                 cv::Point(target_x - 172 * std::cos(theta) + 25 * std::cos(theta + 90_to_rad), target_y + 172 * std::sin(theta) - 25 * std::sin(theta + 90_to_rad)),
                 cv::Scalar(0, 0, 255), 4);
            // 右
            line(src, cv::Point(target_x - 70 * std::cos(theta) - 62 * std::cos(theta + 90_to_rad), target_y + 70 * std::sin(theta) + 62 * std::sin(theta + 90_to_rad)),
                 cv::Point(target_x - 120 * std::cos(theta) - 62 * std::cos(theta + 90_to_rad), target_y + 120 * std::sin(theta) + 62 * std::sin(theta + 90_to_rad)),
                 cv::Scalar(0, 0, 255), 4);
            line(src, cv::Point(target_x - 120 * std::cos(theta) - 62 * std::cos(theta + 90_to_rad), target_y + 120 * std::sin(theta) + 62 * std::sin(theta + 90_to_rad)),
                 cv::Point(target_x - 172 * std::cos(theta) - 25 * std::cos(theta + 90_to_rad), target_y + 172 * std::sin(theta) + 25 * std::sin(theta + 90_to_rad)),
                 cv::Scalar(0, 0, 255), 4);
            // 内横
            line(src, cv::Point(target_x - 172 * std::cos(theta) + 25 * std::cos(theta + 90_to_rad), target_y + 172 * std::sin(theta) - 25 * std::sin(theta + 90_to_rad)),
                 cv::Point(target_x - 172 * std::cos(theta) - 25 * std::cos(theta + 90_to_rad), target_y + 172 * std::sin(theta) + 25 * std::sin(theta + 90_to_rad)),
                 cv::Scalar(0, 0, 255), 4);
        }
    }

    std::vector<rm::combo::ptr> detect() {
        auto info = p_detector->detect(group, src, rm::RED, rm::ImuData(), rm::Time::now());
        return info.combos;
    }
};

TEST_F(RuneDetectorTest, target1_center0) {
    SetUp();
    createRuneTarget(600, 370, 90_to_rad, false);
    auto combos = detect();
    EXPECT_TRUE(combos.empty());
}

TEST_F(RuneDetectorTest, target1_center1) {
    SetUp();
    createRuneTarget(600, 370, 90_to_rad, false);
    createRuneCenter(590, 610);

    auto combos = detect();
    EXPECT_EQ(combos.size(), 1);

    auto p_rune = rm::Rune::cast(combos.front());
    EXPECT_FALSE(p_rune->isActive());
}

TEST_F(RuneDetectorTest, target1_centerTilt) {
    SetUp();
    createRuneTarget(800, 200, 90_to_rad, false);
    createRuneCenter(590, 610);

    auto combos = detect();

    EXPECT_EQ(combos.size(), 0);
}

TEST_F(RuneDetectorTest, target1_center2) {
    SetUp();
    createRuneTarget(600, 370, 90_to_rad, false);
    createRuneCenter(590, 610);
    createRuneCenter(978, 520);

    auto combos = detect();
    EXPECT_EQ(combos.size(), 1);
    EXPECT_LE(combos.front()->center().x, 700);
}

TEST_F(RuneDetectorTest, 1_active_rune) {
    SetUp();
    createRuneTarget(600, 370, 90_to_rad, true);
    createRuneCenter(590, 610);

    auto combos = detect();
    EXPECT_EQ(combos.size(), 1);

    auto rune = rm::Rune::cast(combos.front());
    EXPECT_TRUE(rune->isActive());
}

TEST_F(RuneDetectorTest, 1_inactive_1_active) {
    SetUp();
    createRuneTarget(600, 370, 90_to_rad, false);
    createRuneTarget(600 + 230 * std::cos(18_to_rad), 600 - 230 * std::sin(18_to_rad), 18_to_rad, true);
    createRuneCenter(590, 610);

    auto combos = detect();
    EXPECT_EQ(combos.size(), 2);

    sort(combos.begin(), combos.end(),
         [](const rm::combo::ptr &lhs, const rm::combo::ptr &rhs) {
             return lhs->center().x < rhs->center().x;
         });

    auto rune1 = rm::Rune::cast(combos[0]);
    auto rune2 = rm::Rune::cast(combos[1]);
    EXPECT_FALSE(rune1->isActive());
    EXPECT_TRUE(rune2->isActive());
}

} // namespace rm_test

#endif // HAVE_RMVL_RUNE_DETECTOR

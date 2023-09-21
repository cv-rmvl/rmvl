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

#include <gtest/gtest.h>

#include <opencv2/imgproc.hpp>

#include "rmvl/detector/rune_detector.h"

using namespace rm;
using namespace std;
using namespace cv;

#include <gtest/gtest.h>

#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

#include "rmvl/detector/rune_detector.h"

using namespace std;
using namespace cv;
using namespace rm;

namespace rm_test
{

class RuneDetectorTest : public testing::Test
{
public:
    Mat src;
    vector<group_ptr> groups;
    detect_ptr detector;

    void SetUp() override
    {
        detector = RuneDetector::make_detector();
        src = Mat::zeros(Size(1278, 1024), CV_8UC3);
    }
    void TearDown() override {}

    inline void createRuneCenter(int center_x, int center_y)
    {
        putText(src, "R", Point(center_x, center_y), FONT_HERSHEY_COMPLEX, 1., Scalar(0, 0, 255), 4);
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
    void createRuneTarget(int target_x, int target_y, float theta = 0.f, bool active = false, float ring_num = 9)
    {
        // 在图像上构建神符扇叶
        // 未激活神符
        if (!active)
        {
            // 神符靶心
            circle(src, Point(target_x, target_y), 8, Scalar(0, 0, 255), 2);
            // 神符辅助瞄准特征
            circle(src, Point(target_x, target_y), 24, Scalar(0, 0, 255), 2);
            circle(src, Point(target_x, target_y), 44, Scalar(0, 0, 255), 2);
            // 远（上）
            line(src, Point(target_x + 20 * cos(theta), target_y - 20 * sin(theta)),
                 Point(target_x + 52 * cos(theta), target_y - 52 * sin(theta)), Scalar(0, 0, 255), 4);
            // 近（下）
            line(src, Point(target_x - 20 * cos(theta), target_y + 20 * sin(theta)),
                 Point(target_x - 52 * cos(theta), target_y + 52 * sin(theta)), Scalar(0, 0, 255), 4);
            // 左
            line(src, Point(target_x + 20 * cos(theta + 90_to_rad), target_y - 20 * sin(theta + 90_to_rad)),
                 Point(target_x + 52 * cos(theta + 90_to_rad), target_y - 52 * sin(theta + 90_to_rad)), Scalar(0, 0, 255), 4);
            // 右
            line(src, Point(target_x - 20 * cos(theta + 90_to_rad), target_y + 20 * sin(theta + 90_to_rad)),
                 Point(target_x - 52 * cos(theta + 90_to_rad), target_y + 52 * sin(theta + 90_to_rad)), Scalar(0, 0, 255), 4);
        }
        // 已激活神符
        else
        {
            // 神符靶心
            circle(src, Point(target_x, target_y), 5 * (11 - ring_num), Scalar(0, 0, 255), 2);
        }
        // 神符支架
        // 外支架
        // 横
        line(src, Point(target_x + 70 * cos(theta) + 60 * cos(theta + 90_to_rad), target_y - 70 * sin(theta) - 60 * sin(theta + 90_to_rad)),
             Point(target_x + 70 * cos(theta) - 60 * cos(theta + 90_to_rad), target_y - 70 * sin(theta) + 60 * sin(theta + 90_to_rad)),
             Scalar(0, 0, 255), 8);
        // 竖
        // 左
        line(src, Point(target_x + 70 * cos(theta) + 60 * cos(theta + 90_to_rad), target_y - 70 * sin(theta) - 60 * sin(theta + 90_to_rad)),
             Point(target_x + 25 * cos(theta) + 60 * cos(theta + 90_to_rad), target_y - 25 * sin(theta) - 60 * sin(theta + 90_to_rad)),
             Scalar(0, 0, 255), 8);
        // 右
        line(src, Point(target_x + 70 * cos(theta) - 60 * cos(theta + 90_to_rad), target_y - 70 * sin(theta) + 60 * sin(theta + 90_to_rad)),
             Point(target_x + 25 * cos(theta) - 60 * cos(theta + 90_to_rad), target_y - 25 * sin(theta) + 60 * sin(theta + 90_to_rad)),
             Scalar(0, 0, 255), 8);

        // 内支架
        // 横
        line(src, Point(target_x - 70 * cos(theta) + 60 * cos(theta + 90_to_rad), target_y + 70 * sin(theta) - 60 * sin(theta + 90_to_rad)),
             Point(target_x - 70 * cos(theta) - 60 * cos(theta + 90_to_rad), target_y + 70 * sin(theta) + 60 * sin(theta + 90_to_rad)),
             Scalar(0, 0, 255), 8);
        // 竖
        // 左
        line(src, Point(target_x - 70 * cos(theta) + 60 * cos(theta + 90_to_rad), target_y + 70 * sin(theta) - 60 * sin(theta + 90_to_rad)),
             Point(target_x - 25 * cos(theta) + 60 * cos(theta + 90_to_rad), target_y + 25 * sin(theta) - 60 * sin(theta + 90_to_rad)),
             Scalar(0, 0, 255), 8);
        // 右
        line(src, Point(target_x - 70 * cos(theta) - 60 * cos(theta + 90_to_rad), target_y + 70 * sin(theta) + 60 * sin(theta + 90_to_rad)),
             Point(target_x - 25 * cos(theta) - 60 * cos(theta + 90_to_rad), target_y + 25 * sin(theta) + 60 * sin(theta + 90_to_rad)),
             Scalar(0, 0, 255), 8);

        // 内支架连接
        // 中心
        line(src, Point(target_x - 78 * cos(theta), target_y + 78 * sin(theta)),
             Point(target_x - 162 * cos(theta), target_y + 162 * sin(theta)), Scalar(0, 0, 255), 16);
        // 未激活神符
        if (!active)
        {
            // 间断
            for (int i = 80; i < 190; i += 10)
                line(src, Point(target_x - i * cos(theta) + 25 * cos(theta + 90_to_rad), target_y + i * sin(theta) - 25 * sin(theta + 90_to_rad)),
                     Point(target_x - i * cos(theta) - 25 * cos(theta + 90_to_rad), target_y + i * sin(theta) + 25 * sin(theta + 90_to_rad)),
                     Scalar(0, 0, 0), 4);
        }

        // 已激活神符
        else
        {
            // 左
            line(src, Point(target_x - 70 * cos(theta) + 62 * cos(theta + 90_to_rad), target_y + 70 * sin(theta) - 62 * sin(theta + 90_to_rad)),
                 Point(target_x - 120 * cos(theta) + 62 * cos(theta + 90_to_rad), target_y + 120 * sin(theta) - 62 * sin(theta + 90_to_rad)),
                 Scalar(0, 0, 255), 4);
            line(src, Point(target_x - 120 * cos(theta) + 62 * cos(theta + 90_to_rad), target_y + 120 * sin(theta) - 62 * sin(theta + 90_to_rad)),
                 Point(target_x - 172 * cos(theta) + 25 * cos(theta + 90_to_rad), target_y + 172 * sin(theta) - 25 * sin(theta + 90_to_rad)),
                 Scalar(0, 0, 255), 4);
            // 右
            line(src, Point(target_x - 70 * cos(theta) - 62 * cos(theta + 90_to_rad), target_y + 70 * sin(theta) + 62 * sin(theta + 90_to_rad)),
                 Point(target_x - 120 * cos(theta) - 62 * cos(theta + 90_to_rad), target_y + 120 * sin(theta) + 62 * sin(theta + 90_to_rad)),
                 Scalar(0, 0, 255), 4);
            line(src, Point(target_x - 120 * cos(theta) - 62 * cos(theta + 90_to_rad), target_y + 120 * sin(theta) + 62 * sin(theta + 90_to_rad)),
                 Point(target_x - 172 * cos(theta) - 25 * cos(theta + 90_to_rad), target_y + 172 * sin(theta) + 25 * sin(theta + 90_to_rad)),
                 Scalar(0, 0, 255), 4);
            // 内横
            line(src, Point(target_x - 172 * cos(theta) + 25 * cos(theta + 90_to_rad), target_y + 172 * sin(theta) - 25 * sin(theta + 90_to_rad)),
                 Point(target_x - 172 * cos(theta) - 25 * cos(theta + 90_to_rad), target_y + 172 * sin(theta) + 25 * sin(theta + 90_to_rad)),
                 Scalar(0, 0, 255), 4);
        }
    }

    vector<combo_ptr> detect()
    {
        auto info = detector->detect(groups, src, RED, GyroData(), getTickCount());
        return info.combos;
    }
};

TEST_F(RuneDetectorTest, target1_center0)
{
    SetUp();
    createRuneTarget(600, 370, 90_to_rad, false);
    auto combos = detect();
    EXPECT_TRUE(combos.empty());
}

TEST_F(RuneDetectorTest, target1_center1)
{
    SetUp();
    createRuneTarget(600, 370, 90_to_rad, false);
    createRuneCenter(590, 610);
    // imshow("test", src);
    // waitKey(0);
    auto combos = detect();
    EXPECT_EQ(combos.size(), 1);

    rune_ptr p_rune = Rune::cast(combos.front());
    EXPECT_FALSE(p_rune->isActive());
}

TEST_F(RuneDetectorTest, target1_centerTilt)
{
    SetUp();
    createRuneTarget(800, 200, 90_to_rad, false);
    createRuneCenter(590, 610);
    // imshow("test", src);
    // waitKey(0);
    auto combos = detect();

    EXPECT_EQ(combos.size(), 0);
}

TEST_F(RuneDetectorTest, target1_center2)
{
    SetUp();
    createRuneTarget(600, 370, 90_to_rad, false);
    createRuneCenter(590, 610);
    createRuneCenter(978, 520);
    // imshow("test", src);
    // waitKey(0);
    auto combos = detect();
    EXPECT_EQ(combos.size(), 1);
    EXPECT_LE(combos.front()->getCenter().x, 700);
}

TEST_F(RuneDetectorTest, 1_active_rune)
{
    SetUp();
    createRuneTarget(600, 370, 90_to_rad, true);
    createRuneCenter(590, 610);
    // imshow("test", src);
    // waitKey(0);
    auto combos = detect();
    EXPECT_EQ(combos.size(), 1);

    rune_ptr rune = Rune::cast(combos.front());
    EXPECT_TRUE(rune->isActive());
}

TEST_F(RuneDetectorTest, 1_inactive_1_active)
{
    SetUp();
    createRuneTarget(600, 370, 90_to_rad, false);
    createRuneTarget(600 + 230 * cos(deg2rad(18.f)), 600 - 230 * sin(deg2rad(18.f)), deg2rad(18.f), true);
    createRuneCenter(590, 610);
    // imshow("test", src);
    // waitKey(0);

    auto combos = detect();
    EXPECT_EQ(combos.size(), 2);

    sort(combos.begin(), combos.end(),
         [](const combo_ptr &lhs, const combo_ptr &rhs)
         {
             return lhs->getCenter().x < rhs->getCenter().x;
         });

    rune_ptr rune1 = Rune::cast(combos[0]);
    rune_ptr rune2 = Rune::cast(combos[1]);
    EXPECT_FALSE(rune1->isActive());
    EXPECT_TRUE(rune2->isActive());
}

} // namespace rm_test

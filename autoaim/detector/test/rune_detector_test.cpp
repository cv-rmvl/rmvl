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

namespace rm_test
{

class RuneDetectorTest : public testing::Test
{
public:
    Mat src;
    vector<group_ptr> groups;
    rm::detect_ptr detector;

    void SetUp() override
    {
        detector = rm::RuneDetector::make_detector();
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
            circle(src, Point(target_x, target_y), 32, Scalar(0, 0, 255), 2);
            circle(src, Point(target_x, target_y), 44, Scalar(0, 0, 255), 2);
            // 远（上）
            line(src, Point(target_x + 24 * cosf(theta), target_y - 24 * sinf(theta)),
                 Point(target_x + 52 * cosf(theta), target_y - 52 * sinf(theta)), Scalar(0, 0, 255), 4);
            // 近（下）
            line(src, Point(target_x - 24 * cosf(theta), target_y + 24 * sinf(theta)),
                 Point(target_x - 52 * cosf(theta), target_y + 52 * sinf(theta)), Scalar(0, 0, 255), 4);
            // 左
            line(src, Point(target_x + 24 * cosf(theta + deg2rad(90.f)), target_y - 24 * sinf(theta + deg2rad(90.f))),
                 Point(target_x + 52 * cosf(theta + deg2rad(90.f)), target_y - 52 * sinf(theta + deg2rad(90.f))), Scalar(0, 0, 255), 4);
            // 右
            line(src, Point(target_x - 24 * cosf(theta + deg2rad(90.f)), target_y + 24 * sinf(theta + deg2rad(90.f))),
                 Point(target_x - 52 * cosf(theta + deg2rad(90.f)), target_y + 52 * sinf(theta + deg2rad(90.f))), Scalar(0, 0, 255), 4);
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
        line(src, Point(target_x + 70 * cosf(theta) + 60 * cosf(theta + deg2rad(90.f)), target_y - 70 * sinf(theta) - 60 * sinf(theta + deg2rad(90.f))),
             Point(target_x + 70 * cosf(theta) - 60 * cosf(theta + deg2rad(90.f)), target_y - 70 * sinf(theta) + 60 * sinf(theta + deg2rad(90.f))),
             Scalar(0, 0, 255), 8);
        // 竖
        // 左
        line(src, Point(target_x + 70 * cosf(theta) + 60 * cosf(theta + deg2rad(90.f)), target_y - 70 * sinf(theta) - 60 * sinf(theta + deg2rad(90.f))),
             Point(target_x + 25 * cosf(theta) + 60 * cosf(theta + deg2rad(90.f)), target_y - 25 * sinf(theta) - 60 * sinf(theta + deg2rad(90.f))),
             Scalar(0, 0, 255), 8);
        // 右
        line(src, Point(target_x + 70 * cosf(theta) - 60 * cosf(theta + deg2rad(90.f)), target_y - 70 * sinf(theta) + 60 * sinf(theta + deg2rad(90.f))),
             Point(target_x + 25 * cosf(theta) - 60 * cosf(theta + deg2rad(90.f)), target_y - 25 * sinf(theta) + 60 * sinf(theta + deg2rad(90.f))),
             Scalar(0, 0, 255), 8);

        // 内支架
        // 横
        line(src, Point(target_x - 70 * cosf(theta) + 60 * cosf(theta + deg2rad(90.f)), target_y + 70 * sinf(theta) - 60 * sinf(theta + deg2rad(90.f))),
             Point(target_x - 70 * cosf(theta) - 60 * cosf(theta + deg2rad(90.f)), target_y + 70 * sinf(theta) + 60 * sinf(theta + deg2rad(90.f))),
             Scalar(0, 0, 255), 8);
        // 竖
        // 左
        line(src, Point(target_x - 70 * cosf(theta) + 60 * cosf(theta + deg2rad(90.f)), target_y + 70 * sinf(theta) - 60 * sinf(theta + deg2rad(90.f))),
             Point(target_x - 25 * cosf(theta) + 60 * cosf(theta + deg2rad(90.f)), target_y + 25 * sinf(theta) - 60 * sinf(theta + deg2rad(90.f))),
             Scalar(0, 0, 255), 8);
        // 右
        line(src, Point(target_x - 70 * cosf(theta) - 60 * cosf(theta + deg2rad(90.f)), target_y + 70 * sinf(theta) + 60 * sinf(theta + deg2rad(90.f))),
             Point(target_x - 25 * cosf(theta) - 60 * cosf(theta + deg2rad(90.f)), target_y + 25 * sinf(theta) + 60 * sinf(theta + deg2rad(90.f))),
             Scalar(0, 0, 255), 8);

        // 内支架连接
        // 中心
        line(src, Point(target_x - 78 * cosf(theta), target_y + 78 * sinf(theta)),
             Point(target_x - 162 * cosf(theta), target_y + 162 * sinf(theta)), Scalar(0, 0, 255), 16);
        // 未激活神符
        if (!active)
        {
            // 间断
            for (int i = 80; i < 190; i += 10)
                line(src, Point(target_x - i * cosf(theta) + 25 * cosf(theta + deg2rad(90.f)), target_y + i * sinf(theta) - 25 * sinf(theta + deg2rad(90.f))),
                     Point(target_x - i * cosf(theta) - 25 * cosf(theta + deg2rad(90.f)), target_y + i * sinf(theta) + 25 * sinf(theta + deg2rad(90.f))),
                     Scalar(0, 0, 0), 4);
        }

        // 已激活神符
        else
        {
            // 左
            line(src, Point(target_x - 70 * cosf(theta) + 62 * cosf(theta + deg2rad(90.f)), target_y + 70 * sinf(theta) - 62 * sinf(theta + deg2rad(90.f))),
                 Point(target_x - 120 * cosf(theta) + 62 * cosf(theta + deg2rad(90.f)), target_y + 120 * sinf(theta) - 62 * sinf(theta + deg2rad(90.f))),
                 Scalar(0, 0, 255), 4);
            line(src, Point(target_x - 120 * cosf(theta) + 62 * cosf(theta + deg2rad(90.f)), target_y + 120 * sinf(theta) - 62 * sinf(theta + deg2rad(90.f))),
                 Point(target_x - 172 * cosf(theta) + 25 * cosf(theta + deg2rad(90.f)), target_y + 172 * sinf(theta) - 25 * sinf(theta + deg2rad(90.f))),
                 Scalar(0, 0, 255), 4);
            // 右
            line(src, Point(target_x - 70 * cosf(theta) - 62 * cosf(theta + deg2rad(90.f)), target_y + 70 * sinf(theta) + 62 * sinf(theta + deg2rad(90.f))),
                 Point(target_x - 120 * cosf(theta) - 62 * cosf(theta + deg2rad(90.f)), target_y + 120 * sinf(theta) + 62 * sinf(theta + deg2rad(90.f))),
                 Scalar(0, 0, 255), 4);
            line(src, Point(target_x - 120 * cosf(theta) - 62 * cosf(theta + deg2rad(90.f)), target_y + 120 * sinf(theta) + 62 * sinf(theta + deg2rad(90.f))),
                 Point(target_x - 172 * cosf(theta) - 25 * cosf(theta + deg2rad(90.f)), target_y + 172 * sinf(theta) + 25 * sinf(theta + deg2rad(90.f))),
                 Scalar(0, 0, 255), 4);
            // 内横
            line(src, Point(target_x - 172 * cosf(theta) + 25 * cosf(theta + deg2rad(90.f)), target_y + 172 * sinf(theta) - 25 * sinf(theta + deg2rad(90.f))),
                 Point(target_x - 172 * cosf(theta) - 25 * cosf(theta + deg2rad(90.f)), target_y + 172 * sinf(theta) + 25 * sinf(theta + deg2rad(90.f))),
                 Scalar(0, 0, 255), 4);
        }
    }

    vector<combo_ptr> detect()
    {
        auto info = detector->detect(groups, src, RED, GyroData(), getTickCount());
        return info.combos;
    }
};

TEST_F(RuneDetectorTest, armor1_center0)
{
    SetUp();
    createRuneTarget(600, 370, deg2rad(90.f), false);
    auto combos = detect();
    EXPECT_TRUE(combos.empty());
}

TEST_F(RuneDetectorTest, armor1_center1)
{
    SetUp();
    createRuneTarget(600, 370, deg2rad(90.f), false);
    createRuneCenter(590, 610);
    // imshow("test", src);
    // waitKey(0);
    auto combos = detect();
    EXPECT_EQ(combos.size(), 1);

    rune_ptr p_rune = Rune::cast(combos.front());
    EXPECT_FALSE(p_rune->isActive());
}

TEST_F(RuneDetectorTest, armor1_centerTilt)
{
    SetUp();
    createRuneTarget(800, 200, deg2rad(90.f), false);
    createRuneCenter(590, 610);
    // imshow("test", src);
    // waitKey(0);
    auto combos = detect();

    EXPECT_EQ(combos.size(), 0);
}

TEST_F(RuneDetectorTest, armor1_center2)
{
    SetUp();
    createRuneTarget(600, 370, deg2rad(90.f), false);
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
    createRuneTarget(600, 370, deg2rad(90.f), true);
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
    createRuneTarget(600, 370, deg2rad(90.f), false);
    createRuneTarget(600 + 230 * cosf(deg2rad(18.f)), 600 - 230 * sinf(deg2rad(18.f)), deg2rad(18.f), true);
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

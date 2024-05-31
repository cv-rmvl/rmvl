/**
 * @file test_paint.cpp
 * @author RoboMaster Vision Community
 * @brief
 * @version 1.0
 * @date 2023-01-18
 *
 * @copyright Copyright 2023 (c), RoboMaster Vision Community
 *
 */

#include "rmvl/imgproc/paint.hpp"

#include <gtest/gtest.h>

namespace rm_test
{

using rm::ArrayTool;

TEST(ArrayToolTest, linear2D)
{
    std::vector<cv::Point> pts = {{1, 1}, {1, 2}, {1, 3}};

    auto ptss = ArrayTool::linear2D(pts, {3, 1}, 3);
    EXPECT_EQ(ptss.size(), 3);
    for (size_t i = 0; i < ptss.size(); ++i)
        for (size_t j = 0; j < pts.size(); ++j)
            EXPECT_EQ(ptss[i][j], (pts[j] + cv::Point(3 * i, 1 * i)));
}

TEST(ArrayToolTest, circular2D)
{
    std::vector<cv::Point2f> pts = {{20.f, 20.f}, {-10.f, -10.f}};
    auto ptss = ArrayTool::circular2D(pts, {0, 0}, 90.0, 2);
    EXPECT_EQ(ptss.size(), 2);
    EXPECT_EQ(ptss[1][0], cv::Point2f(-20, 20));
    EXPECT_EQ(ptss[1][1], cv::Point2f(10, -10));
}

TEST(ArrayToolTest, linear3D)
{
    std::vector<cv::Point3i> pts = {{1, 1, 2},
                                    {1, 2, 4},
                                    {1, 3, 6}};

    auto ptss = ArrayTool::linear3D(pts, {3, 1, 2}, 3);
    EXPECT_EQ(ptss.size(), 3);
    for (size_t i = 0; i < ptss.size(); ++i)
        for (size_t j = 0; j < pts.size(); ++j)
            EXPECT_EQ(ptss[i][j], (pts[j] + cv::Point3i(3 * i, 1 * i, 2 * i)));
}

TEST(ArrayToolTest, circular3D)
{
    std::vector pt = {cv::Point3f(10, 20, 30)};
    auto pts1 = ArrayTool::circular3D(pt, {50, 50, 50}, {0, 0, 1}, 90.0, 2);
    EXPECT_EQ(pts1.size(), 2);
    EXPECT_EQ(pts1[1].front(), cv::Point3f(80, 10, 30));

    auto pts2 = ArrayTool::circular3D(pt, {50, 50, 50}, {0, 1, 0}, 90.0, 2);
    EXPECT_EQ(pts2[1].front(), cv::Point3f(30, 20, 90));
}

} // namespace rm_test

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

using namespace std;
using namespace cv;

namespace rm_test
{

using rm::ArrayTool;

TEST(ArrayToolTest, linear2D)
{
    vector<Point> pts = {Point(1, 1),
                         Point(1, 2),
                         Point(1, 3)};

    auto ptss = ArrayTool::linear2D(pts, Point(3, 1), 3);
    EXPECT_EQ(ptss.size(), 3);
    for (size_t i = 0; i < ptss.size(); ++i)
        for (size_t j = 0; j < pts.size(); ++j)
            EXPECT_EQ(ptss[i][j], (pts[j] + Point(3 * i, 1 * i)));
}

TEST(ArrayToolTest, circular2D)
{
    vector<Point2f> pts = {Point2f(20, 20),
                           Point2f(-10, -10)};
    auto ptss = ArrayTool::circular2D(pts, Point2f(0, 0), 90.0, 2);
    EXPECT_EQ(ptss.size(), 2);
    EXPECT_EQ(ptss[1][0], Point2f(-20, 20));
    EXPECT_EQ(ptss[1][1], Point2f(10, -10));
}

TEST(ArrayToolTest, linear3D)
{
    vector<Point3i> pts = {Point3i(1, 1, 2),
                           Point3i(1, 2, 4),
                           Point3i(1, 3, 6)};

    auto ptss = ArrayTool::linear3D(pts, Point3i(3, 1, 2), 3);
    EXPECT_EQ(ptss.size(), 3);
    for (size_t i = 0; i < ptss.size(); ++i)
        for (size_t j = 0; j < pts.size(); ++j)
            EXPECT_EQ(ptss[i][j], (pts[j] + Point3i(3 * i, 1 * i, 2 * i)));
}

TEST(ArrayToolTest, circular3D)
{
    vector<Point3f> pt = {Point3f(10, 20, 30)};
    auto pts1 = ArrayTool::circular3D(pt, Point3f(50, 50, 50), Vec3d(0, 0, 1), 90.0, 2);
    EXPECT_EQ(pts1.size(), 2);
    EXPECT_EQ(pts1[1].front(), Point3f(80, 10, 30));

    auto pts2 = ArrayTool::circular3D(pt, Point3f(50, 50, 50), Vec3d(0, 1, 0), 90.0, 2);
    EXPECT_EQ(pts2[1].front(), Point3f(30, 20, 90));
}

} // namespace rm_test

/**
 * @file test_dataio.cpp
 * @author RoboMaster Vision Community
 * @brief
 * @version 1.0
 * @date 2023-07-11
 *
 * @copyright Copyright 2023 (c), RoboMaster Vision Community
 *
 */

#include <gtest/gtest.h>

#include "rmvl/core/dataio.hpp"

namespace rm_test
{

TEST(DataIO, CornersIO)
{
    std::vector<std::vector<cv::Point2f>> corners = {{{0.0, 1.1}, {2.2, 3.3}},
                                                     {{4.4, 5.5}, {6.6, 7.7}, {8.8, 9.9}}};
    EXPECT_TRUE(rm::writeCorners("ts_dataio.yml", 2, corners));

    decltype(corners) ret;
    EXPECT_TRUE(rm::readCorners("ts_dataio.yml", 2, ret));
    EXPECT_EQ(ret[0][0], cv::Point2f(0.0, 1.1));
    EXPECT_EQ(ret[1][2], cv::Point2f(8.8, 9.9));
}

} // namespace rm_test

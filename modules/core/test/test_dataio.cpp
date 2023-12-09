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

#include <fstream>

#include <gtest/gtest.h>

#include "rmvl/core/dataio.hpp"

namespace rm_test
{

TEST(DataIO, CornersIO)
{
    std::vector<std::vector<std::array<float, 2>>> corners = {{{0.0f, 1.1f}, {2.2f, 3.3f}},
                                                              {{4.4f, 5.5f}, {6.6f, 7.7f}, {8.8f, 9.9f}}};
    std::ofstream ofs("ts_dataio.csv", std::ios::out | std::ios::trunc);
    rm::writeCorners(ofs, corners);

    decltype(corners) ret;
    std::ifstream ifs("ts_dataio.csv", std::ios::in);
    rm::readCorners(ifs, ret);
    EXPECT_EQ(ret[0][0][0], 0.0f);
    EXPECT_EQ(ret[0][0][1], 1.1f);
    EXPECT_EQ(ret[1][2][0], 8.8f);
    EXPECT_EQ(ret[1][2][1], 9.9f);
}

} // namespace rm_test

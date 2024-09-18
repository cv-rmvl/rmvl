/**
 * @file test_io.cpp
 * @author zhaoxi (535394140@qq.com)
 * @brief 数据 IO 与通信模块单元测试
 * @version 1.0
 * @date 2024-09-18
 * 
 * @copyright Copyright 2024 (c), zhaoxi
 * 
 */

#include <fstream>

#include <gtest/gtest.h>

#include "rmvl/core/io.hpp"

namespace rm_test
{

TEST(IOTest, corners_io)
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

TEST(IOTest, pipe_io)
{
    rm::PipeServer server("test_pipe");
    rm::PipeClient client("test_pipe");
    EXPECT_TRUE(server.write("hello"));
    std::string data{};
    EXPECT_TRUE(client.read(data));
    EXPECT_EQ(data, "hello");
    EXPECT_TRUE(client.write("world"));
    EXPECT_TRUE(server.read(data));
    EXPECT_EQ(data, "world");
}


} // namespace rm_test

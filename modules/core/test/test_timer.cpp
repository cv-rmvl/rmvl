/**
 * @file test_timer.cpp
 * @author RoboMaster Vision Community
 * @brief
 * @version 1.0
 * @date 2023-01-30
 *
 * @copyright Copyright 2023 (c), RoboMaster Vision Community
 *
 */

#include <gtest/gtest.h>

#include "rmvl/core/timer.hpp"

namespace rm_test
{

void asyncFoc(int *a)
{
    (*a)++;
}

TEST(TimerTest, time_tick_get)
{
    // 创建定时器对象
    rm::Timer timer;
    // 开始计时
    timer.start();
    // 记录当前时间
    int befor_ms = rm::getSystemTick<int64_t>();
    // 定时1s
    sleep(1);
    // 记录当前时间
    int after_ms = rm::getSystemTick<int64_t>();
    // 停止计时
    timer.stop();
    // 重置计时器
    timer.reset();
    EXPECT_NEAR(after_ms - befor_ms, 1000, 1);
}

TEST(TimerTest, sync_timing)
{
    // 当前的时间毫秒数
    int64_t befor_ms = rm::getSystemTick<int64_t>();
    // 定时63ms
    rm::syncWait(63000);
    // 当前的时间毫秒数
    int64_t after_ms = rm::getSystemTick<int64_t>();
    EXPECT_NEAR(after_ms - befor_ms, 63, 1);
}

TEST(TimerTest, async_timing)
{
    int i = 0;
    asyncFoc(&i);
    EXPECT_EQ(i, 1);
    rm::asyncWait(800, asyncFoc, &i);
    sleep(1);
    EXPECT_EQ(i, 2);
}

} // namespace rm_test

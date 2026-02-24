/**
 * @file test_async.cpp
 * @author zhaoxi (535394140@qq.com)
 * @brief 异步 IO 的协程框架测试
 * @version 1.0
 * @date 2025-09-13
 *
 * @copyright Copyright 2025 (c), zhaoxi
 *
 */

#include <gtest/gtest.h>

#include "rmvl/core/timer.hpp"

#include "rmvl/io/async.hpp"

#if __cplusplus >= 202002L

using namespace rm;

namespace rm_test {

async::Task<> modify(int &v) {
    v = 42;
    co_return;
}

// 异步任务测试
TEST(IO_async, single_task) {
    int val{};
    auto t = modify(val);
    auto h = t.handle();
    EXPECT_EQ(val, 0);
    EXPECT_FALSE(h.done());
    h.resume();
    EXPECT_EQ(val, 42);
    EXPECT_TRUE(h.done());
}

async::Task<int> get(int val) { co_return val; }

async::Task<> task(int &v) {
    v = co_await get(1);
    v += co_await get(41);
    co_return;
}

// 嵌套异步任务无调度测试
TEST(IO_async, nested_task) {
    int val{};
    auto t = task(val);
    auto h = t.handle();
    EXPECT_EQ(val, 0);
    EXPECT_FALSE(h.done());
    h.resume();
    EXPECT_EQ(val, 42);
    EXPECT_TRUE(h.done());
}

using namespace std::chrono_literals;

// 嵌套异步任务调度测试
TEST(IO_async, io_context) {
    async::IOContext io_context;

    int v{};
    auto task = [&](int &v) -> async::Task<> {
        v += co_await get(1);
        v += co_await get(41);
        
        io_context.stop();
    };
    co_spawn(io_context, task, std::ref(v));

    EXPECT_EQ(v, 0);
    io_context.run();
    EXPECT_EQ(v, 42);
}

TEST(IO_async, timer_sleep) {
    async::IOContext io_context;

    io_context.spawn([&]() -> async::Task<> {
        async::Timer t(io_context);
        auto now = Time::now();
        co_await t.sleep_for(200ms);
        EXPECT_NEAR(Time::now() - now, 200, 20);
        io_context.stop();
    });

    io_context.run();
}

} // namespace rm_test

#endif

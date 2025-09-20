/**
 * @file test_ipc.cpp
 * @author zhaoxi (535394140@qq.com)
 * @brief 进程间通信测试
 * @version 1.0
 * @date 2025-09-13
 *
 * @copyright Copyright 2025 (c), zhaoxi
 *
 */

#include <chrono>
#include <thread>

#include <gtest/gtest.h>

#include "rmvl/io/ipc.hpp"

using namespace rm;
using namespace std::chrono_literals;

namespace rm_test {

TEST(IO_ipc, sync_pipe) {
    std::unique_ptr<PipeServer> srv{};
    std::unique_ptr<PipeClient> cli{};

    auto srv_thrd = std::thread([&]() {
        srv = std::make_unique<PipeServer>("test_pipe");
    });
    // 由于 Windows 下命名管道的创建会阻塞直到有客户端连接，此处统一操作，稍作等待
    std::this_thread::sleep_for(5ms);
    auto cli_thrd = std::thread([&]() {
        cli = std::make_unique<PipeClient>("test_pipe");
    });
    srv_thrd.join();
    cli_thrd.join();

    std::string msg = "Hello, IPC!";
    EXPECT_TRUE(cli->write(msg));
    std::string received = srv->read();
    EXPECT_EQ(received, msg);

    msg = "Goodbye, IPC!";
    EXPECT_TRUE(srv->write(msg));
    received = cli->read();
    EXPECT_EQ(received, msg);
}

#if __cplusplus >= 202002L

TEST(IO_ipc, async_pipe) {
    async::IOContext io_context;
    std::unique_ptr<async::PipeServer> server{};
    std::unique_ptr<async::PipeClient> client{};
    auto srv_thrd = std::thread([&]() {
        server = std::make_unique<async::PipeServer>(io_context, "test_async_pipe");
    });
    // 由于 Windows 下命名管道的创建会阻塞直到有客户端连接，此处统一操作，稍作等待
    std::this_thread::sleep_for(5ms);
    auto cli_thrd = std::thread([&]() {
        client = std::make_unique<async::PipeClient>(io_context, "test_async_pipe");
    });

    srv_thrd.join();
    cli_thrd.join();

    using PipeClientPtr = std::unique_ptr<async::PipeClient>;
    using PipeServerPtr = std::unique_ptr<async::PipeServer>;

    co_spawn(
        io_context, [](PipeServerPtr srv, PipeClientPtr cli) -> async::Task<> {
            std::string msg = "Hello, Async IPC!";
            bool success = co_await cli->write(msg);
            EXPECT_TRUE(success);
            std::string received = co_await srv->read();
            EXPECT_EQ(received, msg);
            msg = "Goodbye, Async IPC!";
            success = co_await srv->write(msg);
            EXPECT_TRUE(success);
            received = co_await cli->read();
            EXPECT_EQ(received, msg);
        },
        std::move(server), std::move(client));

    std::jthread stop_thrd([&io_context]() {
        std::this_thread::sleep_for(10ms);
        io_context.stop();
    });

    io_context.run();
}

#endif

#ifndef _WIN32

TEST(IO_ipc, sync_mq) {
    MqServer ms("/test_mq");
    MqClient mc("/test_mq");

    std::string msg = "Hello, MQ!";
    EXPECT_TRUE(mc.write(msg));
    std::string received = ms.read();
    EXPECT_EQ(received, msg);

    msg = "Goodbye, MQ!";
    EXPECT_TRUE(ms.write(msg));
    received = mc.read();
    EXPECT_EQ(received, msg);
}

#endif

} // namespace rm_test

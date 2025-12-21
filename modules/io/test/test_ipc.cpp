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

    std::string str1 = "Hello, IPC!";
    EXPECT_TRUE(cli->write(str1));
    std::string received = srv->read();
    EXPECT_EQ(received, str1);

    str1 = "Goodbye, IPC!";
    EXPECT_TRUE(srv->write(str1));
    received = cli->read();
    EXPECT_EQ(received, str1);
}

TEST(IO_ipc, shm) {
    constexpr std::size_t SHM_SIZE = 32;

    SharedMemory shm1("test_shm", SHM_SIZE);
    SharedMemory shm2("test_shm", SHM_SIZE);

    const char str1[] = "SHM Test";
    memcpy(shm1.data(), str1, std::size(str1));
    char buffer[SHM_SIZE]{};
    memcpy(buffer, shm2.data(), std::size(str1));
    EXPECT_STREQ(buffer, str1);
    const char str2[] = "Hello";
    memcpy(shm2.data(), str2, std::size(str2));
    memset(buffer, 0, sizeof(buffer));
    memcpy(buffer, shm1.data(), std::size(str2));
    EXPECT_STREQ(buffer, str2);
}

TEST(IO_ipc, mpmc_atomic_shm) {
    MPMCSharedMemory<uint32_t> writer("test_atomic_shm");
    MPMCSharedMemory<uint32_t> reader("test_atomic_shm");

    writer.write(42);
    uint32_t value = reader.read();
    EXPECT_EQ(value, 42);

    writer.write(100);
    value = reader.read();
    EXPECT_EQ(value, 100);

    struct TestData {
        int a;
        float b;
        char c;
    };

    MPMCSharedMemory<TestData> shm("test_atomic_shm2");
    TestData data1{10, 3.14f, 'x'};
    shm.write(data1);
    TestData data2 = shm.read();
    EXPECT_EQ(data2.a, data1.a);
    EXPECT_FLOAT_EQ(data2.b, data1.b);
    EXPECT_EQ(data2.c, data1.c);
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
            std::string str1 = "Hello, Async IPC!";
            bool success = co_await cli->write(str1);
            EXPECT_TRUE(success);
            std::string received = co_await srv->read();
            EXPECT_EQ(received, str1);
            str1 = "Goodbye, Async IPC!";
            success = co_await srv->write(str1);
            EXPECT_TRUE(success);
            received = co_await cli->read();
            EXPECT_EQ(received, str1);
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

    std::string str1 = "Hello, MQ!";
    EXPECT_TRUE(mc.write(str1));
    std::string received = ms.read();
    EXPECT_EQ(received, str1);

    str1 = "Goodbye, MQ!";
    EXPECT_TRUE(ms.write(str1));
    received = mc.read();
    EXPECT_EQ(received, str1);
}

#endif

} // namespace rm_test

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

#include <fmt/format.h>
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

TEST(IO_ipc, shm_base) {
    constexpr std::size_t SHM_SIZE = 32;

    SHMBase shm1("test_shm", SHM_SIZE);
    SHMBase shm2("test_shm", SHM_SIZE);

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

// 定义一个简单的测试数据结构
struct TestData {
    int id{};
    double value{};
    char msg[16]{};

    bool operator==(const TestData &other) const {
        return id == other.id && std::abs(value - other.value) < 1e-6 && std::strncmp(msg, other.msg, 16) == 0;
    }
};

TEST(IO_ipc, atomic_shm_construction_and_empty) {
    rm::AtomicSHM<TestData> shm("test_atomic_shm");

    EXPECT_TRUE(shm.isCreator());
    EXPECT_TRUE(shm.empty());

    TestData data{};
    EXPECT_FALSE(shm.read(data));
}

TEST(IO_ipc, atomic_shm_basic_io) {
    rm::AtomicSHM<TestData> shm("test_atomic_shm");

    TestData input{1, 3.14159, "Hello"};
    shm.write(input);

    EXPECT_FALSE(shm.empty());

    TestData output{};
    EXPECT_TRUE(shm.read(output));
    EXPECT_EQ(input, output);

    // 更新数据
    TestData input2{2, 2.71828, "World"};
    shm.write(input2);

    EXPECT_TRUE(shm.read(output));
    EXPECT_EQ(input2, output);
}

TEST(IO_ipc, atomic_shm_multiple_connection) {
    rm::AtomicSHM<TestData> server("test_atomic_shm");
    TestData input{100, 123.456, "ServerMsg"};
    server.write(input);

    // 连接者
    rm::AtomicSHM<TestData> client("test_atomic_shm");
    EXPECT_FALSE(client.isCreator());
    EXPECT_FALSE(client.empty());

    TestData output{};
    EXPECT_TRUE(client.read(output));

    EXPECT_EQ(output.id, 100);
    EXPECT_DOUBLE_EQ(output.value, 123.456);
    EXPECT_STREQ(output.msg, "ServerMsg");
}

TEST(IO_ipc, atomic_shm_concurrency_io_spmc) {
    rm::AtomicSHM<TestData> shm("test_atomic_shm");
    std::atomic<bool> running{true};
    const int kWriteCount = 10000;

    // 写线程
    std::thread writer([&]() {
        for (int i = 1; i <= kWriteCount; ++i) {
            TestData data;
            data.id = i;
            data.value = i * 0.1;
            snprintf(data.msg, sizeof(data.msg), "Msg-%d", i);
            shm.write(data);

            // 稍微让出一点 CPU，模拟真实负载，也增加读写冲突概率
            if (i % 100 == 0)
                std::this_thread::yield();
        }
        running = false;
    });

    // 读线程
    auto reader_func = [&]() {
        TestData last_data{0, 0.0, ""};
        while (running) {
            TestData current_data;
            if (shm.read(current_data)) {
                char expected_msg[16]{};
                fmt::format_to_n(expected_msg, sizeof(expected_msg), "Msg-{}", current_data.id);
                EXPECT_STREQ(current_data.msg, expected_msg) << "Data torn detected at id " << current_data.id;
                // 验证单调性
                EXPECT_GE(current_data.id, last_data.id);
                last_data = current_data;
            }
        }
    };

    std::thread r1(reader_func);
    std::thread r2(reader_func);
    std::thread r3(reader_func);

    writer.join();
    r1.join();
    r2.join();
    r3.join();

    // 最终一致性检查
    TestData final_data;
    EXPECT_TRUE(shm.read(final_data));
    EXPECT_EQ(final_data.id, kWriteCount);
}

TEST(IO_ipc, atomic_shm_concurrency_io_mpmc) {
    rm::AtomicSHM<TestData> shm("test_atomic_shm_mpmc");
    std::atomic<bool> running{true};
    const int kProducers = 4;
    const int kConsumers = 4;
    const int kWritesPerProducer = 1000;

    // 生产者线程
    std::vector<std::thread> producers;
    for (int i = 0; i < kProducers; ++i) {
        producers.emplace_back([&, id = i]() {
            for (int j = 1; j <= kWritesPerProducer; ++j) {
                TestData data;
                data.id = id * kWritesPerProducer + j; // 生成唯一 ID
                data.value = data.id * 0.1;
                fmt::format_to_n(data.msg, sizeof(data.msg), "P{}-{}", id, j);
                shm.write(data);

                if (j % 50 == 0)
                    std::this_thread::yield();
            }
        });
    }

    // 消费者线程
    std::vector<std::thread> consumers;
    std::atomic<int> total_reads{0};
    for (int i = 0; i < kConsumers; ++i) {
        consumers.emplace_back([&]() {
            while (running) {
                TestData current_data;
                if (shm.read(current_data)) {
                    // 简单的完整性检查
                    char expected_msg_prefix[4]{};
                    int p_id = (current_data.id - 1) / kWritesPerProducer;
                    int seq_id = (current_data.id - 1) % kWritesPerProducer + 1;
                    fmt::format_to_n(expected_msg_prefix, sizeof(expected_msg_prefix), "P{}", p_id);

                    // 验证数据内部一致性 (msg 与 id 是否匹配)
                    char expected_msg[16]{};
                    fmt::format_to_n(expected_msg, sizeof(expected_msg), "P{}-{}", p_id, seq_id);

                    if (std::strncmp(current_data.msg, expected_msg, 16) != 0) {
                        ADD_FAILURE() << "Data torn detected! ID: " << current_data.id
                                      << ", Msg: " << current_data.msg
                                      << ", Expected: " << expected_msg;
                    }
                    total_reads++;
                } else
                    std::this_thread::yield();
            }
        });
    }

    // 等待所有生产者完成
    for (auto &t : producers)
        t.join();

    // 给消费者一点时间读取最后的数据
    std::this_thread::sleep_for(1ms);
    running = false;

    for (auto &t : consumers)
        t.join();

    // 至少应该读到过数据
    EXPECT_GT(total_reads.load(), 0);

    // 最终一致性检查
    TestData final_data;
    if (shm.read(final_data)) {
        int p_id = (final_data.id - 1) / kWritesPerProducer;
        int seq_id = (final_data.id - 1) % kWritesPerProducer + 1;
        char expected_msg[16]{};
        fmt::format_to_n(expected_msg, sizeof(expected_msg), "P{}-{}", p_id, seq_id);
        EXPECT_STREQ(final_data.msg, expected_msg);
    }
}

TEST(IO_ipc, ring_buffer_shm_basic_operations) {
    auto now = std::chrono::system_clock::now().time_since_epoch().count();
    auto shm_name = "/rmvl_test_shm_" + std::to_string(now);
    // 容量为 4 的缓冲区
    RingBufferSlotSHM4<int> buffer(shm_name);

    EXPECT_TRUE(buffer.isCreator());

    // 写入数据
    EXPECT_TRUE(buffer.write(10));
    EXPECT_TRUE(buffer.write(20));

    int val = 0;
    // 读取数据
    EXPECT_TRUE(buffer.read(val));
    EXPECT_EQ(val, 10);
    EXPECT_TRUE(buffer.read(val));
    EXPECT_EQ(val, 20);

    // 缓冲区空，读取应失败
    EXPECT_FALSE(buffer.read(val));
}

TEST(IO_ipc, ring_buffer_shm_overwrite_strategy) {
    auto now = std::chrono::system_clock::now().time_since_epoch().count();
    auto shm_name = "/rmvl_test_shm_" + std::to_string(now);

    // 填满缓冲区: 0, 1, 2, 3
    RingBufferSlotSHM4<int> buffer(shm_name);
    for (int i = 0; i < 4; ++i)
        EXPECT_TRUE(buffer.write(i));

    // 缓冲区已满
    EXPECT_TRUE(buffer.write(100, true));

    int val = 0;
    // 读取数据 1, 2, 3, 100
    EXPECT_TRUE(buffer.read(val));
    EXPECT_EQ(val, 1);
    EXPECT_TRUE(buffer.read(val));
    EXPECT_EQ(val, 2);
    EXPECT_TRUE(buffer.read(val));
    EXPECT_EQ(val, 3);
    EXPECT_TRUE(buffer.read(val));
    EXPECT_EQ(val, 100);
    // 再次读取应为空
    EXPECT_FALSE(buffer.read(val));
}

TEST(IO_ipc, ring_buffer_shm_complex_object_transfer) {
    auto now = std::chrono::system_clock::now().time_since_epoch().count();
    auto shm_name = "/rmvl_test_shm_" + std::to_string(now);

    RingBufferSlotSHM8<TestData> buffer(shm_name);

    TestData in_data{42, 3.14159, "Hello"};
    EXPECT_TRUE(buffer.write(in_data));

    TestData out_data{};
    EXPECT_TRUE(buffer.read(out_data));

    EXPECT_EQ(out_data.id, 42);
    EXPECT_DOUBLE_EQ(out_data.value, 3.14159);
    EXPECT_STREQ(out_data.msg, "Hello");
}

// 并发压力测试 (多生产者单消费者)
TEST(IO_ipc, ring_buffer_shm_concurrency_stress) {
    auto now = std::chrono::system_clock::now().time_since_epoch().count();
    auto shm_name = "/rmvl_test_shm_" + std::to_string(now);
    RingBufferSlotSHM<int, 1024> buffer(shm_name);

    constexpr int num_producers = 4;
    constexpr int items_per_producer = 1000;
    std::atomic<int> read_count{0};

    // 消费者线程
    std::thread consumer([&]() {
        int val{};
        constexpr int total_expected = num_producers * items_per_producer;
        int received = 0;
        while (received < total_expected) {
            if (buffer.read(val)) {
                received++;
                read_count++;
            } else
                std::this_thread::yield();
        }
    });

    // 生产者线程组
    std::vector<std::thread> producers;
    for (int i = 0; i < num_producers; ++i) {
        producers.emplace_back([&]() {
            for (int j = 0; j < items_per_producer; ++j) // 忙等待写入，模拟高负载
                while (!buffer.write(j))
                    std::this_thread::yield();
        });
    }

    for (auto &t : producers)
        t.join();
    consumer.join();

    EXPECT_EQ(read_count.load(), num_producers * items_per_producer);
}

TEST(IO_ipc, ring_buffer_shm_multi_instance) {
    auto now = std::chrono::system_clock::now().time_since_epoch().count();
    auto shm_name = "/rmvl_test_shm_multi_" + std::to_string(now);

    // 启动消费者线程 (模拟进程 B)
    std::vector<int> received_data;
    std::thread consumer_thread([&]() {
        std::this_thread::sleep_for(5ms);
        RingBufferSlotSHM8<int> consumer_buffer(shm_name);
        EXPECT_FALSE(consumer_buffer.isCreator());

        int val{};
        for (int i = 0; i < 5; ++i) {
            while (!consumer_buffer.read(val))
                std::this_thread::yield();
            received_data.push_back(val);
        }
    });

    // 主线程作为生产者 (模拟进程 A)
    RingBufferSlotSHM8<int> producer_buffer(shm_name);
    EXPECT_TRUE(producer_buffer.isCreator());

    for (int i = 0; i < 5; ++i) {
        while (!producer_buffer.write(i * 10))
            std::this_thread::yield();
        std::this_thread::sleep_for(2ms);
    }

    consumer_thread.join();

    // 验证数据
    ASSERT_EQ(received_data.size(), 5);
    EXPECT_EQ(received_data[0], 0);
    EXPECT_EQ(received_data[1], 10);
    EXPECT_EQ(received_data[2], 20);
    EXPECT_EQ(received_data[3], 30);
    EXPECT_EQ(received_data[4], 40);
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

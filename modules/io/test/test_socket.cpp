/**
 * @file test_socket.cpp
 * @author zhaoxi (535394140@qq.com)
 * @brief Socket 测试
 * @version 1.0
 * @date 2025-09-13
 *
 * @copyright Copyright 2025 (c), zhaoxi
 *
 */

#include <chrono>
#include <thread>

#include <gtest/gtest.h>

#include "rmvl/io/async.hpp"
#include "rmvl/io/socket.hpp"

using namespace rm;

namespace rm_test {

using namespace std::chrono_literals;

TEST(IO_socket, sync_net_socket) {
    Acceptor acceptor(Endpoint(ip::tcp::v4(), 10800));
    Connector connector(Endpoint(ip::tcp::v4(), 10800), "127.0.0.1");

    auto accept_thrd = std::thread([&]() {
        auto socket = acceptor.accept();
        std::string msg = socket.read();
        EXPECT_EQ(msg, "Hello, Socket!");
        EXPECT_TRUE(socket.write("Hello, Client!"));
    });

    auto connect_thrd = std::thread([&]() {
        auto socket = connector.connect();
        EXPECT_TRUE(socket.write("Hello, Socket!"));
        std::string response = socket.read();
        EXPECT_EQ(response, "Hello, Client!");
    });

    accept_thrd.join();
    connect_thrd.join();
}

TEST(IO_socket, sync_uds_socket) {
    Acceptor acceptor(Endpoint(ipc::stream(), "rmvl_test.sock"));
    Connector connector(Endpoint(ipc::stream(), "rmvl_test.sock"));

    auto accept_thrd = std::thread([&]() {
        auto socket = acceptor.accept();
        std::string msg = socket.read();
        EXPECT_EQ(msg, "Hello, Socket!");
        EXPECT_TRUE(socket.write("Hello, Client!"));
    });

    auto connect_thrd = std::thread([&]() {
        auto socket = connector.connect();
        EXPECT_TRUE(socket.write("Hello, Socket!"));
        std::string response = socket.read();
        EXPECT_EQ(response, "Hello, Client!");
    });

    accept_thrd.join();
    connect_thrd.join();
}

#if __cplusplus >= 202002L

TEST(IO_socket, async_net_socket) {
    auto io_context = async::IOContext{};
    auto acceptor = async::Acceptor(io_context, Endpoint(ip::tcp::v4(), 10801));
    auto connector = async::Connector(io_context, Endpoint(ip::tcp::v4(), 10801), "127.0.0.1");

    auto accept = [&]() -> async::Task<> {
        auto socket = co_await acceptor.accept();
        std::string msg = co_await socket.read();
        EXPECT_EQ(msg, "Hello, Socket!");
        EXPECT_TRUE(co_await socket.write("Hello, Client!"));
    };

    auto connect = [&]() -> async::Task<> {
        auto socket = co_await connector.connect();
        EXPECT_TRUE(co_await socket.write("Hello, Socket!"));
        std::string response = co_await socket.read();
        EXPECT_EQ(response, "Hello, Client!");

        io_context.stop();
    };

    co_spawn(io_context, accept);
    co_spawn(io_context, connect);

    io_context.run();
}

TEST(IO_socket, async_uds_socket) {
    auto io_context = async::IOContext{};
    auto acceptor = async::Acceptor(io_context, Endpoint(ipc::stream(), "rmvl_async_test.sock"));
    auto connector = async::Connector(io_context, Endpoint(ipc::stream(), "rmvl_async_test.sock"));

    auto accept = [&]() -> async::Task<> {
        auto socket = co_await acceptor.accept();
        std::string msg = co_await socket.read();
        EXPECT_EQ(msg, "Hello, Socket!");
        EXPECT_TRUE(co_await socket.write("Hello, Client!"));
    };

    auto connect = [&]() -> async::Task<> {
        auto socket = co_await connector.connect();
        EXPECT_TRUE(co_await socket.write("Hello, Socket!"));
        std::string response = co_await socket.read();
        EXPECT_EQ(response, "Hello, Client!");

        io_context.stop();
    };

    co_spawn(io_context, accept);
    co_spawn(io_context, connect);

    io_context.run();
}

#endif

} // namespace rm_test
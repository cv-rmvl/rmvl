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

#include <thread>

#include <gtest/gtest.h>

#include "rmvl/io/async.hpp"
#include "rmvl/io/socket.hpp"

using namespace rm;

namespace rm_test {

using namespace std::chrono_literals;

TEST(IO_socket, sync_tcp_socket) {
    Acceptor acceptor(Endpoint(ip::tcp::v4(), 10800));
    Connector connector(Endpoint(ip::tcp::v4(), 10800), "127.0.0.1");

    auto accept_thrd = std::thread([&]() {
        auto socket = acceptor.accept();
        auto endpoint = socket.endpoint();
        EXPECT_EQ(endpoint.port(), 10800);

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

TEST(IO_socket, sync_tcp_socket_nonblocking) {
    Acceptor acceptor(Endpoint(ip::tcp::v4(), 10801), false);
    Connector connector(Endpoint(ip::tcp::v4(), 10801), "127.0.0.1");

    auto accept_thrd = std::thread([&]() {
        auto socket = acceptor.accept();
        while (socket.invalid())
            socket = acceptor.accept();
        auto endpoint = socket.endpoint();
        EXPECT_EQ(endpoint.port(), 10801);

        std::string msg = socket.read();
        while (msg.empty())
            msg = socket.read();
        EXPECT_EQ(msg, "Hello, Socket!");
        EXPECT_TRUE(socket.write("Hello, Client!"));
    });

    auto connect_thrd = std::thread([&]() {
        auto socket = connector.connect();
        while (socket.invalid())
            socket = connector.connect();
        EXPECT_TRUE(socket.write("Hello, Socket!"));
        std::string response = socket.read();
        while (response.empty())
            response = socket.read();
        EXPECT_EQ(response, "Hello, Client!");
    });

    accept_thrd.join();
    connect_thrd.join();
}

TEST(IO_socket, sync_udp_socket) {
    auto server_ep = Endpoint(ip::udp::v4(), 10900);
    auto listener = Listener(server_ep);
    auto sender = Sender(ip::udp::v4());

    auto server_thrd = std::thread([&]() {
        auto socket = listener.create();
        EXPECT_EQ(socket.endpoint().port(), 10900);

        auto [msg, sender_ip, sender_port] = socket.read();
        EXPECT_EQ(msg, "Hello, UDP Socket!");
        EXPECT_EQ(sender_ip, "127.0.0.1");
        EXPECT_NE(sender_port, 0);
        EXPECT_NE(sender_port, 10900);
    });

    auto client_thrd = std::thread([&]() {
        auto socket = sender.create();
        EXPECT_TRUE(socket.write("127.0.0.1", server_ep, "Hello, UDP Socket!"));
    });

    server_thrd.join();
    client_thrd.join();
}

TEST(IO_socket, sync_udp_socket_nonblocking) {
    auto server_ep = Endpoint(ip::udp::v4(), 10905);
    auto listener = Listener(server_ep, false);
    auto sender = Sender(ip::udp::v4(), false);

    auto server_thrd = std::thread([&]() {
        auto socket = listener.create();
        EXPECT_EQ(socket.endpoint().port(), 10905);

        auto [msg, sender_ip, sender_port] = socket.read();
        while (msg.empty())
            std::tie(msg, sender_ip, sender_port) = socket.read();
        EXPECT_EQ(msg, "Hello, UDP Socket!");
        EXPECT_EQ(sender_ip, "127.0.0.1");
        EXPECT_NE(sender_port, 0);
        EXPECT_NE(sender_port, 10905);
    });

    auto client_thrd = std::thread([&]() {
        auto socket = sender.create();
        EXPECT_TRUE(socket.write("127.0.0.1", server_ep, "Hello, UDP Socket!"));
    });

    server_thrd.join();
    client_thrd.join();
}

TEST(IO_socket, sync_udp_socket_auto_port) {
    auto listener = Listener(Endpoint(ip::udp::v4(), 0));
    auto read_sock = listener.create();
    auto port = read_sock.endpoint().port();
    EXPECT_NE(port, 0);

    auto sender = Sender(ip::udp::v4());
    auto write_sock = sender.create();
    EXPECT_NE(write_sock.endpoint().port(), 0);

    auto write_thrd = std::thread([&]() {
        EXPECT_TRUE(write_sock.write("127.0.0.1", Endpoint(ip::udp::v4(), port), "Hello, UDP Socket!"));
    });

    auto read_thrd = std::thread([&]() {
        auto [msg, sender_ip, sender_port] = read_sock.read();
        EXPECT_EQ(msg, "Hello, UDP Socket!");
        EXPECT_EQ(sender_ip, "127.0.0.1");
        EXPECT_NE(sender_port, 0);
        EXPECT_NE(sender_port, port);
    });

    write_thrd.join();
    read_thrd.join();
}

TEST(IO_socket, sync_udp_socket_specify_port) {
    auto server_ep = Endpoint(ip::udp::v4(), 10901);
    auto client_ep = Endpoint(ip::udp::v4(), 10911);
    auto listener = Listener(server_ep);
    auto sender = Listener(client_ep);

    auto server_thrd = std::thread([&]() {
        auto socket = listener.create();
        auto [msg, sender_ip, sender_port] = socket.read();
        EXPECT_EQ(msg, "Hello, UDP Socket!");
        EXPECT_EQ(sender_ip, "127.0.0.1");
        EXPECT_EQ(sender_port, 10911);
        bool stat = socket.write("127.0.0.1", client_ep, "I hear you!");
        EXPECT_TRUE(stat);
    });

    auto client_thrd = std::thread([&]() {
        auto socket = sender.create();
        EXPECT_TRUE(socket.write("127.0.0.1", server_ep, "Hello, UDP Socket!"));
        auto [msg, sender_ip, sender_port] = socket.read();
        EXPECT_EQ(msg, "I hear you!");
        EXPECT_EQ(sender_ip, "127.0.0.1");
        EXPECT_EQ(sender_port, 10901);
    });

    server_thrd.join();
    client_thrd.join();
}

#if __cplusplus >= 202002L

TEST(IO_socket, async_tcp_socket) {
    auto io_context = async::IOContext{};
    auto acceptor = async::Acceptor(io_context, Endpoint(ip::tcp::v4(), 10810));
    auto connector = async::Connector(io_context, Endpoint(ip::tcp::v4(), 10810), "127.0.0.1");

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

TEST(IO_socket, async_udp_socket) {
    auto io_context = async::IOContext{};
    auto server_ep = Endpoint(ip::udp::v4(), 10902);
    auto listener = async::Listener(io_context, server_ep);
    auto sender = async::Sender(io_context, ip::udp::v4());

    auto server = [&]() -> async::Task<> {
        auto socket = listener.create();
        auto [msg, sender_ip, sender_port] = co_await socket.read();
        EXPECT_EQ(msg, "Hello, UDP Socket!");
        EXPECT_EQ(sender_ip, "127.0.0.1");
        EXPECT_NE(sender_port, 0);
        EXPECT_NE(sender_port, 10902);
    };

    auto client = [&]() -> async::Task<> {
        auto socket = sender.create();
        EXPECT_TRUE(co_await socket.write("127.0.0.1", server_ep, "Hello, UDP Socket!"));

        io_context.stop();
    };

    co_spawn(io_context, server);
    co_spawn(io_context, client);

    io_context.run();
}

TEST(IO_socket, async_udp_socket_specify_port) {
    auto io_context = async::IOContext{};
    auto server_ep = Endpoint(ip::udp::v4(), 10903);
    auto client_ep = Endpoint(ip::udp::v4(), 10913);
    auto listener = async::Listener(io_context, server_ep);
    auto sender = async::Listener(io_context, client_ep);

    auto server = [&]() -> async::Task<> {
        auto socket = listener.create();
        auto [msg, sender_ip, sender_port] = co_await socket.read();
        EXPECT_EQ(msg, "Hello, UDP Socket!");
        EXPECT_EQ(sender_ip, "127.0.0.1");
        EXPECT_NE(sender_port, 0);
        EXPECT_NE(sender_port, 10903);
    };

    auto client = [&]() -> async::Task<> {
        auto socket = sender.create();
        EXPECT_TRUE(co_await socket.write("127.0.0.1", server_ep, "Hello, UDP Socket!"));

        io_context.stop();
    };

    co_spawn(io_context, server);
    co_spawn(io_context, client);

    io_context.run();
}

#endif

} // namespace rm_test
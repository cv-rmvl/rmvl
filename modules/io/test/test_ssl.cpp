/**
 * @file test_ssl.cpp
 * @author zhaoxi (535394140@qq.com)
 * @brief SSL/TLS 测试
 * @version 1.0
 * @date 2026-06-21
 *
 * @copyright Copyright 2026 (c), zhaoxi
 *
 */

#include <thread>

#include <gtest/gtest.h>

#include "rmvl/io/ssl.hpp"

using namespace rm;

namespace rm_test {

TEST(IO_ssl, loopback) {
    if (!SSLContext::available())
        GTEST_SKIP() << "OpenSSL support is disabled";

    constexpr const char cert[] = RMVL_IO_TEST_DATA_PATH "/lo.crt";
    constexpr const char key[] = RMVL_IO_TEST_DATA_PATH "/lo.key";

    SSLContext server_context = SSLContext::server();
    ASSERT_TRUE(server_context.load_cert(cert, key)) << server_context.lasterr();

    SSLContext client_context = SSLContext::client();
    client_context.set_verify_mode(SSLVerifyMode::Peer);
    ASSERT_TRUE(client_context.load_ca(cert)) << client_context.lasterr();

    Acceptor acceptor(Endpoint(ip::tcp::v4(), 11443));
    Connector connector(Endpoint(ip::tcp::v4(), 11443), "127.0.0.1");
    auto client_socket = connector.connect();

    bool server_handshake{}, server_write{};
    std::string server_received;
    std::thread server([&] {
        SSLStream stream(acceptor.accept(), server_context);
        server_handshake = stream.handshake();
        if (server_handshake) {
            server_received = stream.read();
            server_write = stream.write("pong");
        }
    });

    SSLStream client(std::move(client_socket), client_context);
    bool client_handshake = client.handshake("127.0.0.1");
    bool client_write = client_handshake && client.write("ping");
    std::string client_received = client_write ? client.read() : std::string{};

    client.close();
    server.join();

    EXPECT_TRUE(server_handshake);
    EXPECT_TRUE(client_handshake) << client.lasterr();
    EXPECT_EQ(server_received, "ping");
    EXPECT_TRUE(server_write);
    EXPECT_TRUE(client_write);
    EXPECT_EQ(client_received, "pong");
}

} // namespace rm_test

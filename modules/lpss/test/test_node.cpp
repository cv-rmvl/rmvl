/**
 * @file test_node.cpp
 * @author zhaoxi (535394140@qq.com)
 * @brief
 * @version 1.0
 * @date 2025-11-05
 *
 * @copyright Copyright 2025 (c), zhaoxi
 *
 */

#include <gtest/gtest.h>

#include "rmvl/core/timer.hpp"
#include "rmvl/io/socket.hpp"
#include "rmvl/lpss.hpp"

namespace rm_test {

using namespace rm;

TEST(LPSS_node, guid_create) {
    lpss::Node nd1, nd2{1};
    // 相同主机、相同进程，应具有相同 GUID，不同域 ID 不影响 GUID
    EXPECT_EQ(nd1.guid(), nd2.guid());
}

TEST(LPSS_node, same_domain_discover) {
    lpss::Node nd;
    DgramSocket sock = Listener(Endpoint(ip::udp::v4(), 7500), false).create();
    sock.setOption(ip::multicast::JoinGroup(lpss::BROADCAST_IP));
    Timer::reset();
    auto [data, addr, port] = sock.read();
    while (data.empty() && Timer::now() < 40)
        std::tie(data, addr, port) = sock.read();
    EXPECT_FALSE(data.empty());
}

TEST(LPSS_node, diff_domain_issolate) {
    lpss::Node nd{1};
    DgramSocket sock = Listener(Endpoint(ip::udp::v4(), 7500), false).create();
    sock.setOption(ip::multicast::JoinGroup(lpss::BROADCAST_IP));
    Timer::reset();
    auto [data, addr, port] = sock.read();
    while (data.empty() && Timer::now() < 40)
        std::tie(data, addr, port) = sock.read();
    EXPECT_TRUE(data.empty());
}

TEST(LPSS_node, diff_domain_discover) {
    lpss::Node nd{1};
    DgramSocket sock = Listener(Endpoint(ip::udp::v4(), 7501), false).create();
    sock.setOption(ip::multicast::JoinGroup(lpss::BROADCAST_IP));
    Timer::reset();
    auto [data, addr, port] = sock.read();
    while (data.empty() && Timer::now() < 40)
        std::tie(data, addr, port) = sock.read();
    EXPECT_FALSE(data.empty());
}

} // namespace rm_test

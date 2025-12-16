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
#include <thread>

#include "rmvl/lpss/msgtmp.hpp"
#include "rmvl/lpss/node.hpp"

namespace rm_test {

using namespace rm;

TEST(LPSS_node, guid_create) {
    lpss::Node nd1;
    nd1.createPublisher<lpss::msg::String>("/person");
    std::this_thread::sleep_for(std::chrono::seconds(5));
}

} // namespace rm_test

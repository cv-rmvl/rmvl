/**
 * @file test_opcua_pubsub.cpp
 * @author zhaoxi (535394140@qq.com)
 * @brief OPC UA PubSub 单元测试
 * @version 1.0
 * @date 2023-12-01
 *
 * @copyright Copyright 2023 (c), zhaoxi
 *
 */

#include "rmvl/opcua/publisher.hpp"
#include "rmvl/opcua/subscriber.hpp"

#ifdef UA_ENABLE_PUBSUB

#include <gtest/gtest.h>

namespace rm_test
{

TEST(OPC_UA_Publisher, publisher_config)
{
    // 创建发布者
    rm::Publisher<rm::TransportID::UDP_UADP> publisher("Demo", "opc.udp://224.0.1.20:4840", 4840);
    uaCreateVariable(test_double, 3.1);
    auto node_id = publisher.addVariableNode(test_double);
    publisher.publish({{"Pub Test Double", node_id}}, 100);
}

} // namespace rm_test

#endif // UA_ENABLE_PUBSUB

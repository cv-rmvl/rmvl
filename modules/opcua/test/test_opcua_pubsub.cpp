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

TEST(OPC_UA_PubSub, pubsub_config)
{
    // 创建发布者
    rm::Publisher<rm::TransportID::UDP_UADP> pub("NumberPub", "opc.udp://224.0.1.22", 8000);
    uaCreateVariable(test_double, 3.1);
    auto node_id = pub.addVariableNode(test_double);
    pub.start();
    EXPECT_TRUE(pub.publish({{"DoubleDemo", node_id}}, 50));

    // 创建订阅者
    rm::Subscriber<rm::TransportID::UDP_UADP> sub("NumberSub", "opc.udp://224.0.1.22:8000", 8001);
    sub.start();
    rm::FieldMetaData meta_data("DoubleDemo", UA_TYPES_DOUBLE, -1);
    auto nodes = sub.subscribe("NumberPub", {meta_data});
    EXPECT_EQ(nodes.size(), 1);

    pub.write(node_id, 3.4);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    auto sub_val = sub.read(nodes[0]);
    EXPECT_EQ(sub_val.cast<double>(), 3.4);

    sub.stop();
    pub.stop();
    sub.join();
    pub.join();
}

} // namespace rm_test

#endif // UA_ENABLE_PUBSUB

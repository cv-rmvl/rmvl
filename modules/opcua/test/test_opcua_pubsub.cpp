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

#include <thread>
#include <chrono>

#include "rmvl/opcua/publisher.hpp"
#include "rmvl/opcua/subscriber.hpp"

#ifdef UA_ENABLE_PUBSUB

#include <gtest/gtest.h>

namespace rm_test
{

using namespace std::chrono_literals;

TEST(OPC_UA_PubSub, pubsub_config)
{
    // 创建发布者
    rm::Publisher pub("NumberPub", "opc.udp://224.0.1.22", 8000);
    uaCreateVariable(test_double, 3.1);
    auto node_id = pub.addVariableNode(test_double);
    std::thread t1(&rm::Publisher::spin, &pub);
    EXPECT_TRUE(pub.publish({{"DoubleDemo", node_id}}, 50));

    // 创建订阅者
    rm::Subscriber sub("NumberSub", "opc.udp://224.0.1.22:8000", 8001);
    std::thread t2(&rm::Subscriber::spin, &sub);
    rm::Variable double_demo_var = 0.0;
    double_demo_var.browse_name = "DoubleDemo";
    auto meta_data = rm::FieldMetaData::makeFrom(double_demo_var);
    auto nodes = sub.subscribe("NumberPub", {meta_data});
    EXPECT_EQ(nodes.size(), 1);

    pub.write(node_id, 3.4);
    std::this_thread::sleep_for(100ms);
    auto sub_val = sub.read(nodes[0]);
    EXPECT_EQ(sub_val.cast<double>(), 3.4);

    pub.shutdown();
    sub.shutdown();
    t1.join();
    t2.join();
}

} // namespace rm_test

#endif // UA_ENABLE_PUBSUB

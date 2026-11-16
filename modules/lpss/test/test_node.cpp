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

#include <atomic>
#include <thread>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <arpa/inet.h>
#endif

#include "rmvl/io/socket.hpp"
#include "rmvl/lpss/node.hpp"

#include "rmvlmsg/std/string.hpp"
#include "rmvlsrv/std/set_bool.hpp"

#include "rmvlpara/lpss.hpp"

namespace rm_test {

using namespace rm;
using namespace std::chrono_literals;

namespace {

std::string mtpFragment(std::string_view topic, std::string_view type, uint16_t sequence, uint16_t fragment_id, uint32_t total_size,
                        std::string_view payload, bool set_ignored_length_bits = false) {
    std::string header{"MT02"};
    uint8_t topic_size = static_cast<uint8_t>(topic.size());
    uint8_t type_size = static_cast<uint8_t>(type.size());
    if (set_ignored_length_bits) {
        topic_size |= 0xc0;
        type_size |= 0x80;
    }
    header.append(reinterpret_cast<const char *>(&topic_size), sizeof(topic_size));
    header.append(topic);
    header.append(reinterpret_cast<const char *>(&type_size), sizeof(type_size));
    header.append(type);
    sequence = htons(sequence);
    fragment_id = htons(fragment_id);
    total_size = htonl(total_size);
    auto fragment_size = htons(static_cast<uint16_t>(payload.size()));
    header.append(reinterpret_cast<const char *>(&sequence), sizeof(sequence));
    header.append(reinterpret_cast<const char *>(&fragment_id), sizeof(fragment_id));
    header.append(reinterpret_cast<const char *>(&total_size), sizeof(total_size));
    header.append(reinterpret_cast<const char *>(&fragment_size), sizeof(fragment_size));
    header.append(payload);
    return header;
}

void sendFragment(DgramSocket &sender, uint16_t port, uint16_t sequence, uint16_t fragment_id, uint32_t total_size, std::string_view payload,
                  bool set_ignored_length_bits = false) {
    auto packet = mtpFragment("/mtp", msg::String::msg_type, sequence, fragment_id, total_size, payload, set_ignored_length_bits);
    EXPECT_TRUE(sender.write("127.0.0.1", Endpoint(ip::udp::v4(), port), packet));
}

struct LongTypeMessage {
    inline static constexpr const char msg_type[] = "1234567890123456789012345678901234567890123456789012345678901234";
};

class TestDataWriter : public lpss::DataWriterBase {
public:
    using DataWriterBase::DataWriterBase;

    void addWithMtu(lpss::Guid guid, lpss::Locator locator, uint32_t mtu) { _udpv4_targets[guid] = {locator, mtu}; }
};

#if __cplusplus >= 202002L
class TestAsyncDataWriter : public lpss::async::DataWriterBase {
public:
    using DataWriterBase::DataWriterBase;

    void addWithMtu(lpss::Guid guid, lpss::Locator locator, uint32_t mtu) { _udpv4_targets[guid] = {locator, mtu}; }
};
#endif

} // namespace

TEST(LPSS_node, guid_create) {
    lpss::Node nd1("node1", 0);
    lpss::Node nd2("node2", 1);
    // 相同主机、相同进程，应具有相同 GUID，不同域 ID 不影响 GUID
    EXPECT_EQ(nd1.guid(), nd2.guid());
}

TEST(LPSS_node, same_domain_discover) {
    lpss::Node nd("node3");
    DgramSocket sock = Listener(Endpoint(ip::udp::v4(), 7500), false).create();
    sock.setOption(ip::multicast::JoinGroup(lpss::BROADCAST_IP));
    auto recvdata = sock.read();
    auto start_time = lpss::now();
    while (recvdata.data.empty() && lpss::now() - start_time < 50ms)
        recvdata = sock.read();
    EXPECT_FALSE(recvdata.data.empty());
}

TEST(LPSS_node, diff_domain_issolate) {
    lpss::Node nd("node4", 1);
    DgramSocket sock = Listener(Endpoint(ip::udp::v4(), 7500), false).create();
    sock.setOption(ip::multicast::JoinGroup(lpss::BROADCAST_IP));
    const auto start_time = lpss::now();
    do {
        const auto recvdata = sock.read();
        const auto announcement = lpss::RNDPMessage::deserialize(recvdata.data);
        ASSERT_FALSE(announcement && announcement->guid == nd.guid());
    } while (lpss::now() - start_time < 50ms);
}

TEST(LPSS_node, diff_domain_discover) {
    lpss::Node nd("node5", 1);
    DgramSocket sock = Listener(Endpoint(ip::udp::v4(), 7501), false).create();
    sock.setOption(ip::multicast::JoinGroup(lpss::BROADCAST_IP));
    auto recvdata = sock.read();
    auto start_time = lpss::now();
    while (recvdata.data.empty() && lpss::now() - start_time < 50ms)
        recvdata = sock.read();
    EXPECT_FALSE(recvdata.data.empty());
}

TEST(LPSS_node, mtp_writer_reader_fragmented_payload) {
    lpss::DataReaderBase reader(lpss::Guid{1}, msg::String::msg_type, "/mtp");
    TestDataWriter writer(lpss::Guid{2}, msg::String::msg_type, "/mtp");
    writer.addWithMtu(lpss::Guid{3}, {reader.port(), {127, 0, 0, 1}}, 512);

    std::string payload(4096, 'x');
    std::string received{};
    std::thread reader_thread([&]() { received = reader.read(); });
    writer.write(payload);
    reader_thread.join();

    EXPECT_EQ(received, payload);
}

TEST(LPSS_node, mtp_writer_reader_shm_payload_on_same_host) {
    lpss::Guid writer_guid{0x12345678, 1, 1};
    lpss::Guid reader_guid{0x12345678, 2, 1};
    lpss::DataReaderBase reader(reader_guid, msg::String::msg_type, "/mtp");
    TestDataWriter writer(writer_guid, msg::String::msg_type, "/mtp");
    reader.add(writer.guid());
    writer.add(reader.guid(), {reader.port(), {127, 0, 0, 1}});

    std::string payload(4096, 's');
    std::string received{};
    std::thread reader_thread([&]() { received = reader.read(); });
    writer.write(payload);
    reader_thread.join();

    EXPECT_EQ(received, payload);
}

TEST(LPSS_node, mtp_reassembles_out_of_order_and_ignores_duplicates) {
    lpss::DataReaderBase reader(lpss::Guid{1}, msg::String::msg_type, "/mtp");
    auto sender = Sender(ip::udp::v4()).create();
    std::string received{};
    std::thread reader_thread([&]() { received = reader.read(); });

    sendFragment(sender, reader.port(), 10, 1, 10, "World", true);
    sendFragment(sender, reader.port(), 10, 1, 10, "World");
    sendFragment(sender, reader.port(), 10, 0, 10, "Hello");
    reader_thread.join();

    EXPECT_EQ(received, "HelloWorld");
}

TEST(LPSS_node, mtp_discards_conflicting_duplicate_fragment) {
    lpss::DataReaderBase reader(lpss::Guid{1}, msg::String::msg_type, "/mtp");
    auto sender = Sender(ip::udp::v4()).create();
    std::string received{};
    std::thread reader_thread([&]() { received = reader.read(); });

    sendFragment(sender, reader.port(), 11, 0, 10, "Hello");
    sendFragment(sender, reader.port(), 11, 0, 10, "XXXXX");
    sendFragment(sender, reader.port(), 12, 0, 4, "Done");
    reader_thread.join();

    EXPECT_EQ(received, "Done");
}

TEST(LPSS_node, mtp_expires_incomplete_sequence) {
    auto previous_timeout = para::lpss_param.MTP_FRAGMENT_TIMEOUT;
    para::lpss_param.MTP_FRAGMENT_TIMEOUT = 1;

    lpss::DataReaderBase reader(lpss::Guid{1}, msg::String::msg_type, "/mtp");
    auto sender = Sender(ip::udp::v4()).create();
    std::string received{};
    std::thread reader_thread([&]() { received = reader.read(); });

    sendFragment(sender, reader.port(), 13, 0, 10, "Hello");
    std::this_thread::sleep_for(5ms);
    sendFragment(sender, reader.port(), 13, 1, 10, "World");
    sendFragment(sender, reader.port(), 14, 0, 4, "Done");
    reader_thread.join();

    para::lpss_param.MTP_FRAGMENT_TIMEOUT = previous_timeout;
    EXPECT_EQ(received, "Done");
}

TEST(LPSS_node, mtp_evicts_old_incomplete_sequence_at_budget) {
    auto previous_budget = para::lpss_param.MTP_REASSEMBLY_MAX_BYTES;
    para::lpss_param.MTP_REASSEMBLY_MAX_BYTES = 10;

    lpss::DataReaderBase reader(lpss::Guid{1}, msg::String::msg_type, "/mtp");
    auto sender = Sender(ip::udp::v4()).create();
    std::string received{};
    std::thread reader_thread([&]() { received = reader.read(); });

    sendFragment(sender, reader.port(), 15, 0, 10, "Hello");
    sendFragment(sender, reader.port(), 16, 0, 10, "0123456789");
    reader_thread.join();

    para::lpss_param.MTP_REASSEMBLY_MAX_BYTES = previous_budget;
    EXPECT_EQ(received, "0123456789");
}

TEST(LPSS_node, mtp_rejects_oversized_topic_or_type) {
    lpss::Node node("mtp_limits", 12);
    auto long_topic = std::string(64, 't');

    auto publisher = node.createPublisher<msg::String>(long_topic);
    auto subscriber = node.createSubscriber<msg::String>(long_topic, [](const msg::String &) {});
    auto typed_publisher = node.createPublisher<LongTypeMessage>("/valid");

    EXPECT_TRUE(publisher.invalid());
    EXPECT_TRUE(subscriber.invalid());
    EXPECT_TRUE(typed_publisher.invalid());
}

#if __cplusplus >= 202002L

TEST(LPSS_node, async_mtp_writer_reader_fragmented_payload) {
    rm::async::IOContext io_context{};
    lpss::async::DataReaderBase reader(io_context, lpss::Guid{1}, msg::String::msg_type, "/mtp");
    TestAsyncDataWriter writer(io_context, lpss::Guid{2}, msg::String::msg_type, "/mtp");
    writer.addWithMtu(lpss::Guid{3}, {reader.port(), {127, 0, 0, 1}}, 512);

    std::string payload(4096, 'x');
    std::string received{};
    auto read = [&]() -> rm::async::Task<> {
        received = co_await reader.read();
        io_context.stop();
    };
    auto write = [&]() -> rm::async::Task<> { co_await writer.write(payload); };

    co_spawn(io_context, read);
    co_spawn(io_context, write);
    io_context.run();
    EXPECT_EQ(received, payload);
}

TEST(LPSS_node, async_mtp_writer_reader_shm_payload_on_same_host) {
    rm::async::IOContext io_context{};
    lpss::Guid writer_guid{0x12345678, 3, 1};
    lpss::Guid reader_guid{0x12345678, 4, 1};
    lpss::async::DataReaderBase reader(io_context, reader_guid, msg::String::msg_type, "/mtp");
    TestAsyncDataWriter writer(io_context, writer_guid, msg::String::msg_type, "/mtp");
    reader.add(writer.guid());
    writer.add(reader.guid(), {reader.port(), {127, 0, 0, 1}});

    std::string payload(4096, 's');
    std::string received{};
    auto read = [&]() -> rm::async::Task<> {
        received = co_await reader.read();
        io_context.stop();
    };
    auto write = [&]() -> rm::async::Task<> { co_await writer.write(payload); };

    co_spawn(io_context, read);
    co_spawn(io_context, write);
    io_context.run();
    EXPECT_EQ(received, payload);
}

TEST(LPSS_node, async_mtp_keeps_latest_pending_write) {
    rm::async::IOContext io_context{};
    lpss::async::DataReaderBase reader(io_context, lpss::Guid{1}, msg::String::msg_type, "/mtp");
    TestAsyncDataWriter writer(io_context, lpss::Guid{2}, msg::String::msg_type, "/mtp");
    writer.addWithMtu(lpss::Guid{3}, {reader.port(), {127, 0, 0, 1}}, 512);

    std::vector<std::string> payloads{};
    for (int i = 0; i < 4; ++i)
        payloads.emplace_back(8192, static_cast<char>('a' + i));
    std::vector<std::string> received{};
    auto read = [&]() -> rm::async::Task<> {
        for (std::size_t i = 0; i < 2; ++i)
            received.push_back(co_await reader.read());
        io_context.stop();
    };
    auto timeout = [&]() -> rm::async::Task<> {
        rm::async::Timer timer(io_context);
        co_await timer.sleep_for(1s);
        io_context.stop();
    };

    co_spawn(io_context, read);
    co_spawn(io_context, timeout);
    for (const auto &payload : payloads)
        co_spawn(io_context, &lpss::async::DataWriterBase::write, &writer, payload);
    io_context.run();

    ASSERT_EQ(received.size(), 2);
    EXPECT_EQ(received[0], payloads.front());
    EXPECT_EQ(received[1], payloads.back());
}

TEST(LPSS_node, async_mtp_rejects_oversized_topic) {
    lpss::async::Node node("async_mtp_limits", 12);
    auto publisher = node.createPublisher<msg::String>(std::string(64, 't'));
    EXPECT_EQ(publisher, nullptr);
}

TEST(LPSS_node, async_subscriber_shared_lifetime) {
    lpss::async::Node node("async_subscriber_lifetime", 44);
    auto subscriber = node.createSubscriber<msg::String>("/lifetime", [](const msg::String &) {});
    ASSERT_NE(subscriber, nullptr);
    EXPECT_FALSE(subscriber->invalid());

    node.destroySubscriber(subscriber);
    EXPECT_TRUE(subscriber->invalid());
}

TEST(LPSS_node, async_service_client_creation) {
    lpss::async::Node node("service_creation", 43);
    auto service = node.createService<srv::SetBool>("/set_enabled", [](const srv::SetBool::Request &request, srv::SetBool::Response &response) {
        response.success = true;
        response.message = request.data ? "enabled" : "disabled";
    });
    auto client = node.createClient<srv::SetBool>("/set_enabled");

    ASSERT_NE(service, nullptr);
    ASSERT_NE(client, nullptr);
    EXPECT_FALSE(service->invalid());
    EXPECT_FALSE(client->invalid());
    EXPECT_EQ(node.createClient<srv::SetBool>("/set_enabled"), nullptr);
}

TEST(LPSS_node, stp_round_trip) {
    srv::SetBool::Request request{};
    request.data = true;
    lpss::stp::Header header{lpss::Guid{0x12345678, 7, 9}, 42};
    auto data = lpss::stp::pack(header, request);
    auto decoded = lpss::stp::unpack<srv::SetBool::Request>(data);

    ASSERT_TRUE(decoded.has_value());
    EXPECT_EQ(decoded->header.client_guid, header.client_guid);
    EXPECT_EQ(decoded->header.sequence, 42u);
    EXPECT_TRUE(decoded->message.data);
    EXPECT_FALSE(lpss::stp::unpack<srv::SetBool::Request>(std::string(lpss::stp::HEADER_SIZE - 1, '\0')).has_value());
}

TEST(LPSS_node, async_service_client_local_call) {
    rm::async::IOContext io_context{};
    const auto request_topic = lpss::srv2topic::request("/set_enabled");
    const auto response_topic = lpss::srv2topic::response("/set_enabled");
    auto request_reader = std::make_shared<lpss::async::DataReaderBase>(io_context, lpss::Guid{0x12345678, 1, 1}, srv::SetBool::Request::msg_type, request_topic);
    auto response_writer = std::make_shared<lpss::async::DataWriterBase>(io_context, lpss::Guid{0x12345678, 1, 2}, srv::SetBool::Response::msg_type, response_topic);
    auto request_writer = std::make_shared<lpss::async::DataWriterBase>(io_context, lpss::Guid{0x12345678, 2, 1}, srv::SetBool::Request::msg_type, request_topic);
    auto response_reader = std::make_shared<lpss::async::DataReaderBase>(io_context, lpss::Guid{0x12345678, 2, 2}, srv::SetBool::Response::msg_type, response_topic);
    request_reader->add(request_writer->guid());
    request_writer->add(request_reader->guid(), {request_reader->port(), {127, 0, 0, 1}});
    response_reader->add(response_writer->guid());
    response_writer->add(response_reader->guid(), {response_reader->port(), {127, 0, 0, 1}});

    auto service = std::make_shared<lpss::async::Service<srv::SetBool>>(
        io_context, "/set_enabled", request_reader, response_writer, [](const srv::SetBool::Request &request, srv::SetBool::Response &response) {
            response.success = request.data;
            response.message = request.data ? "enabled" : "disabled";
        });
    auto client = std::make_shared<lpss::async::Client<srv::SetBool>>(io_context, "/set_enabled", request_writer, response_reader);
    service->_delay_start();
    client->_delay_start();

    srv::SetBool::Request request{};
    request.data = true;
    bool service_ready{};
    std::optional<srv::SetBool::Response> result{};
    auto call = [](lpss::async::Client<srv::SetBool>::ptr client, srv::SetBool::Request request, bool *service_ready,
                   std::optional<srv::SetBool::Response> *result, rm::async::IOContext *ctx) -> rm::async::Task<> {
        *service_ready = co_await client->wait(100ms);
        if (!*service_ready) {
            ctx->stop();
            co_return;
        }
        *result = co_await client->call(request, 500ms);
        ctx->stop();
    };
    co_spawn(io_context, call, client, request, &service_ready, &result, &io_context);
    io_context.run();

    EXPECT_TRUE(service_ready);
    ASSERT_TRUE(result.has_value());
    EXPECT_TRUE(result->success);
    EXPECT_EQ(result->message, "enabled");
}

TEST(LPSS_node, async_timer_cancel) {
    lpss::async::Node node("timer_cancel", 44);
    std::atomic_uint32_t invoke_count{};
    auto timer = node.createTimer(1ms, [&invoke_count] { ++invoke_count; });
    std::thread spin_thread([&node] { node.spin(); });

    const auto deadline = std::chrono::steady_clock::now() + 100ms;
    while (invoke_count.load() == 0 && std::chrono::steady_clock::now() < deadline)
        std::this_thread::sleep_for(1ms);

    timer->cancel();
    const auto count_after_cancel = invoke_count.load();
    std::this_thread::sleep_for(10ms);
    node.shutdown();
    spin_thread.join();

    EXPECT_GT(count_after_cancel, 0u);
    EXPECT_EQ(invoke_count.load(), count_after_cancel);
    EXPECT_TRUE(timer->cancelled());
}

#endif

TEST(LPSS_rsd2, rndp_version_field_and_bounds) {
    lpss::RNDPMessage packet{};
    packet.guid = lpss::Guid{0x1234567800010000ULL};
    packet.heartbeat_timeout = 5;
    packet.locators = {{7850, {127, 0, 0, 1}}};
    packet.name = "rndp-version";
    auto data = packet.serialize();
    ASSERT_GE(data.size(), lpss::RNDP_HEADER_SIZE + 1);
    EXPECT_EQ(data.substr(0, 4), "ND02");
    EXPECT_EQ(data.substr(12, 4), std::string({'v', 2, 2, 2}));
    auto decoded = lpss::RNDPMessage::deserialize(data);
    ASSERT_TRUE(decoded.has_value());
    EXPECT_EQ(decoded->guid, packet.guid);
    ASSERT_EQ(decoded->locators.size(), 1u);
    EXPECT_EQ(decoded->locators.front().port, 7850);
    EXPECT_EQ(decoded->name, packet.name);

    auto obsolete = data;
    obsolete.replace(0, 4, "ND01");
    EXPECT_FALSE(lpss::RNDPMessage::deserialize(obsolete));

    auto future_versions = data;
    future_versions[13] = 3; // 允许未来版本继续使用兼容的 ND02 消息布局
    future_versions[14] = 3;
    future_versions[15] = 4;
    const auto future = lpss::RNDPMessage::deserialize(future_versions);
    ASSERT_TRUE(future.has_value());
    EXPECT_EQ(future->ndp_version, 3);
    EXPECT_EQ(future->edp_version, 3);
    EXPECT_EQ(future->mtp_version, 4);
    EXPECT_EQ(future->serialize(), future_versions);

    auto invalid_ndp = data;
    invalid_ndp[13] = 1;
    EXPECT_FALSE(lpss::RNDPMessage::deserialize(invalid_ndp));
    auto wrong_marker = data;
    wrong_marker[12] = 'x';
    EXPECT_FALSE(lpss::RNDPMessage::deserialize(wrong_marker));
    EXPECT_FALSE(lpss::RNDPMessage::deserialize(data.substr(0, data.size() - 1)));

    auto oversized = packet;
    oversized.locators.resize(256);
    EXPECT_TRUE(oversized.serialize().empty());
    oversized = packet;
    oversized.name.assign(256, 'x');
    EXPECT_TRUE(oversized.serialize().empty());
}

TEST(LPSS_rsd2, negotiate_compatible_protocol_versions) {
    const auto same = lpss::negotiate(2, 2, 2, 2);
    EXPECT_EQ(same.edp, 2);
    EXPECT_EQ(same.mtp, 2);

    const auto newer_peer = lpss::negotiate(2, 2, 3, 4);
    EXPECT_EQ(newer_peer.edp, 2);
    EXPECT_EQ(newer_peer.mtp, 2);

    const auto newer_local = lpss::negotiate(4, 3, 5, 4);
    EXPECT_EQ(newer_local.edp, 4);
    EXPECT_EQ(newer_local.mtp, 3);

    const auto mixed = lpss::negotiate(3, 4, 2, 5);
    EXPECT_EQ(mixed.edp, 2);
    EXPECT_EQ(mixed.mtp, 4);

    const auto legacy = lpss::negotiate(2, 2, 1, 0);
    EXPECT_EQ(legacy.edp, 2);
    EXPECT_EQ(legacy.mtp, 2);
}

TEST(LPSS_rsd2, redp_wire_and_ack_sequence_rollover) {
    const lpss::Guid source{0x1234567800010000ULL};
    lpss::REDPMessage snapshot{};
    snapshot.source = source;
    snapshot.syn = 65535;
    snapshot.snapshot = {{lpss::Guid{source.full | 1}, 0, "/writer", "std/String"},
                         {lpss::Guid{source.full | 2}, 7881, "/reader", "std/String"}};
    const auto wire = snapshot.serialize();
    ASSERT_GE(wire.size(), 17u);
    EXPECT_EQ(wire.substr(0, 4), "ED02");
    EXPECT_EQ(static_cast<uint8_t>(wire[14]), 0u);
    const auto decoded = lpss::REDPMessage::deserialize(wire);
    ASSERT_TRUE(decoded.has_value());
    ASSERT_EQ(decoded->snapshot.size(), 2u);
    EXPECT_EQ(decoded->snapshot[1].port, 7881u);
    EXPECT_EQ(decoded->snapshot[0].topic, "/writer");

    lpss::REDPReceiver receiver(source);
    const auto result = receiver.accept(*decoded, lpss::Guid{0x1234567800020000ULL});
    ASSERT_TRUE(result.has_value());
    EXPECT_TRUE(result->applied);
    EXPECT_TRUE(result->ack.ack);
    EXPECT_EQ(result->ack.syn, 0u);
    const auto duplicate = receiver.accept(*decoded, lpss::Guid{0x1234567800020000ULL});
    ASSERT_TRUE(duplicate.has_value());
    EXPECT_FALSE(duplicate->applied);
    auto malformed = wire;
    malformed[14] = 2;
    EXPECT_FALSE(lpss::REDPMessage::deserialize(malformed));
    EXPECT_FALSE(lpss::REDPMessage::deserialize(wire.substr(0, wire.size() - 1)));
    auto duplicate_guid = snapshot;
    duplicate_guid.snapshot.push_back(snapshot.snapshot.front());
    EXPECT_TRUE(duplicate_guid.serialize().empty());
}

TEST(LPSS_rsd2, sender_uses_supplied_snapshot) {
    const lpss::Guid a{0x1234567800010000ULL}, b{0x1234567800020000ULL};
    lpss::REDPSender sender(a, b);
    const lpss::REDPEndpoint reader{lpss::Guid{a.full | 2}, 7881, "/lifecycle", "std/String"};
    const std::vector<lpss::REDPEndpoint> current{reader};

    EXPECT_FALSE(sender.request(current));
    sender.markDirty();
    sender.markDirty();
    const auto packet = sender.request(current);
    ASSERT_TRUE(packet);
    ASSERT_EQ(packet->snapshot.size(), 1u);
    EXPECT_EQ(packet->snapshot.front().guid, reader.guid);
    EXPECT_TRUE(sender.confirm({b, static_cast<uint16_t>(packet->syn + 1), true, {}}, current));
    EXPECT_FALSE(sender.dirty());
    EXPECT_FALSE(sender.request(current));
}

TEST(LPSS_rsd2, sender_preserves_pending_and_sends_latest_after_ack) {
    const lpss::Guid a{0x1234567800010000ULL}, b{0x1234567800020000ULL};
    lpss::REDPSender sender(a, b);
    lpss::REDPReceiver receiver(a);
    const lpss::REDPEndpoint writer{lpss::Guid{a.full | 1}, 0, "/lifecycle", "std/String"};
    const lpss::REDPEndpoint reader{lpss::Guid{a.full | 2}, 7881, "/lifecycle", "std/String"};
    std::vector<lpss::REDPEndpoint> current{writer};
    sender.markDirty();
    const auto first = sender.request(current);
    ASSERT_TRUE(first);
    const auto received = receiver.accept(*first, b);
    ASSERT_TRUE(received);

    current.push_back(reader);
    sender.markDirty();
    current.erase(current.begin());
    sender.markDirty();
    const auto retry = sender.request(current);
    ASSERT_TRUE(retry);
    EXPECT_EQ(retry->serialize(), first->serialize());
    const auto duplicate = receiver.accept(*retry, b);
    ASSERT_TRUE(duplicate);
    EXPECT_FALSE(duplicate->applied);
    auto wrong_ack = received->ack;
    ++wrong_ack.syn;
    EXPECT_FALSE(sender.confirm(wrong_ack, current));
    EXPECT_TRUE(sender.waiting());
    ASSERT_TRUE(sender.confirm(received->ack, current));

    const auto latest = sender.request(current);
    ASSERT_TRUE(latest);
    EXPECT_EQ(latest->syn, static_cast<uint16_t>(first->syn + 1));
    ASSERT_EQ(latest->snapshot.size(), 1u);
    EXPECT_EQ(latest->snapshot.front().guid, reader.guid);
    const auto applied = receiver.accept(*latest, b);
    ASSERT_TRUE(applied);
    EXPECT_TRUE(applied->applied);
    ASSERT_TRUE(sender.confirm(applied->ack, current));
    EXPECT_FALSE(sender.request(current));

    current.clear();
    sender.markDirty();
    const auto empty = sender.request(current);
    ASSERT_TRUE(empty);
    EXPECT_TRUE(empty->snapshot.empty());
    const auto removed = receiver.accept(*empty, b);
    ASSERT_TRUE(removed);
    EXPECT_TRUE(receiver.snapshot().empty());
    EXPECT_TRUE(sender.confirm(removed->ack, current));
    EXPECT_FALSE(sender.request(current));
}

TEST(LPSS_rsd2, sender_discards_cancelled_changes_while_waiting) {
    const lpss::Guid a{0x1234567800010000ULL}, b{0x1234567800020000ULL};
    lpss::REDPSender sender(a, b);
    const lpss::REDPEndpoint writer{lpss::Guid{a.full | 1}, 0, "/lifecycle", "std/String"};
    const lpss::REDPEndpoint reader{lpss::Guid{a.full | 2}, 7881, "/lifecycle", "std/String"};
    std::vector<lpss::REDPEndpoint> current{writer};
    sender.markDirty();
    const auto pending = sender.request(current);
    ASSERT_TRUE(pending);
    current.push_back(reader);
    sender.markDirty();
    current.pop_back();
    sender.markDirty();
    ASSERT_TRUE(sender.confirm({b, static_cast<uint16_t>(pending->syn + 1), true, {}}, current));
    EXPECT_FALSE(sender.dirty());
    EXPECT_FALSE(sender.request(current));
}

TEST(LPSS_rsd2, receiver_rejects_out_of_order_and_conflicting_duplicate) {
    const lpss::Guid a{0x1234567800010000ULL}, b{0x1234567800020000ULL};
    lpss::REDPReceiver receiver(a);
    lpss::REDPMessage first{a, 100, false, {{lpss::Guid{a.full | 1}, 0, "/x", "std/String"}}};
    ASSERT_TRUE(receiver.accept(first, b).has_value());
    auto future = first;
    future.syn = 102;
    EXPECT_FALSE(receiver.accept(future, b).has_value());
    auto conflicting = first;
    conflicting.snapshot[0].topic = "/different";
    EXPECT_FALSE(receiver.accept(conflicting, b).has_value());
    auto next = first;
    next.syn = 101;
    next.snapshot.clear();
    ASSERT_TRUE(receiver.accept(next, b)->applied);
    EXPECT_TRUE(receiver.snapshot().empty());
}

TEST(LPSS_rsd2, sync_node_heartbeat_expiry) {
    class Probe : public lpss::Node {
    public:
        using Node::Node;

        bool hasPeer(lpss::Guid peer) {
            std::shared_lock lock(_nodes_mtx);
            return _discovered_nodes.find(peer) != _discovered_nodes.end();
        }
        bool hasWriter(std::string_view topic, lpss::Guid writer) {
            std::shared_lock lock(_discovered_mtx);
            auto it = _discovered_writers.find(std::string(topic));
            return it != _discovered_writers.end() && it->second.writers.count(writer);
        }
        void expire(lpss::Guid peer) {
            std::lock_guard lock(_nodes_mtx);
            auto it = _discovered_nodes.find(peer);
            if (it != _discovered_nodes.end())
                it->second.last_alive = std::chrono::steady_clock::now() - 5s;
        }
    };

    constexpr uint8_t domain = 117;
    auto observer = Listener(Endpoint(ip::udp::v4(), 7500 + domain), false).create();
    observer.setOption(ip::multicast::JoinGroup(lpss::BROADCAST_IP));
    Probe node("rsd2_timeout", domain);

    uint16_t control_port{};
    std::string local_addr{};
    std::array<uint8_t, 4> local_interface{};
    const auto deadline = std::chrono::steady_clock::now() + 2s;
    while (!control_port && std::chrono::steady_clock::now() < deadline) {
        auto [wire, addr, port] = observer.read();
        auto announcement = lpss::RNDPMessage::deserialize(wire);
        if (announcement && announcement->guid == node.guid() && !announcement->locators.empty()) {
            control_port = announcement->locators.front().port;
            local_addr = addr;
            ASSERT_EQ(::inet_pton(AF_INET, addr.c_str(), local_interface.data()), 1);
        }
        std::this_thread::sleep_for(5ms);
    }
    ASSERT_NE(control_port, 0);

    const lpss::Guid peer{0x12345678, 0x9876, 0};
    const lpss::Guid writer{0x12345678, 0x9876, 1};
    const std::string topic = "/rsd2/timeout";
    auto control = Listener(Endpoint(ip::udp::v4(), Endpoint::ANY_PORT), false).create();
    auto receiveREDP = [&]() -> std::optional<lpss::REDPMessage> {
        const auto until = std::chrono::steady_clock::now() + 1s;
        do {
            auto [wire, addr, port] = control.read();
            if (auto packet = lpss::REDPMessage::deserialize(wire))
                return packet;
            std::this_thread::sleep_for(5ms);
        } while (std::chrono::steady_clock::now() < until);
        return std::nullopt;
    };
    lpss::RNDPMessage announcement{};
    announcement.guid = peer;
    announcement.heartbeat_timeout = 1;
    announcement.locators = {{control.endpoint().port(), local_interface}};
    announcement.name = "synthetic-peer";
    lpss::REDPMessage snapshot{peer, 100, false, {{writer, 0, topic, msg::String::msg_type}}};
    auto sender = Sender(ip::udp::v4()).create();
    sender.setOption(ip::multicast::Loopback(true));
    sender.setOption(ip::multicast::Interface(local_interface));
    auto sendRNDP = [&] {
        sender.write(lpss::BROADCAST_IP, Endpoint(ip::udp::v4(), 7500 + domain), announcement.serialize());
    };
    auto sendREDP = [&] {
        control.write(local_addr, Endpoint(ip::udp::v4(), control_port), snapshot.serialize());
    };
    auto sendREDPAck = [&](uint16_t syn) {
        lpss::REDPMessage ack{peer, syn, true, {}};
        control.write(local_addr, Endpoint(ip::udp::v4(), control_port), ack.serialize());
    };
    auto waitFor = [&](auto predicate, auto action) {
        const auto until = std::chrono::steady_clock::now() + 2s;
        do {
            action();
            if (predicate())
                return true;
            std::this_thread::sleep_for(5ms);
        } while (std::chrono::steady_clock::now() < until);
        return predicate();
    };

    ASSERT_TRUE(waitFor([&] { return node.hasPeer(peer); }, sendRNDP));

    // The initial baseline establishes the peer's expected outbound SYN.
    auto first_baseline = receiveREDP();
    ASSERT_TRUE(first_baseline.has_value());
    ASSERT_FALSE(first_baseline->ack);
    EXPECT_TRUE(first_baseline->snapshot.empty());
    sendREDPAck(static_cast<uint16_t>(first_baseline->syn + 1));

    sendREDP();
    ASSERT_TRUE(waitFor([&] { return node.hasWriter(topic, writer); }, [] {}));
    auto first_ack = receiveREDP();
    ASSERT_TRUE(first_ack.has_value());
    ASSERT_TRUE(first_ack->ack);
    EXPECT_EQ(first_ack->syn, 101u);

    node.expire(peer);
    ASSERT_TRUE(waitFor([&] { return !node.hasPeer(peer) && !node.hasWriter(topic, writer); }, [] {}));

    ASSERT_TRUE(waitFor([&] { return node.hasPeer(peer); }, sendRNDP));
    // Rediscovery must resume the previous session instead of choosing a random SYN.
    auto resumed_baseline = receiveREDP();
    ASSERT_TRUE(resumed_baseline.has_value());
    ASSERT_FALSE(resumed_baseline->ack);
    EXPECT_EQ(resumed_baseline->syn, static_cast<uint16_t>(first_baseline->syn + 1));
    sendREDPAck(static_cast<uint16_t>(resumed_baseline->syn + 1));

    // The last accepted remote snapshot is restored without waiting for a new update.
    EXPECT_TRUE(node.hasWriter(topic, writer));
}

TEST(LPSS_rsd2, sync_node_clamps_lower_mtp_version_to_two) {
    class Probe : public lpss::Node {
    public:
        using Node::Node;

        bool hasPeer(lpss::Guid peer) {
            std::shared_lock lock(_nodes_mtx);
            return _discovered_nodes.find(peer) != _discovered_nodes.end();
        }
        bool hasCachedWriter(std::string_view topic, lpss::Guid writer) {
            std::shared_lock lock(_discovered_mtx);
            auto it = _discovered_writers.find(std::string(topic));
            return it != _discovered_writers.end() && it->second.writers.count(writer);
        }
    };

    constexpr uint8_t domain = 118;
    auto observer = Listener(Endpoint(ip::udp::v4(), 7500 + domain), false).create();
    observer.setOption(ip::multicast::JoinGroup(lpss::BROADCAST_IP));
    Probe node("rsd2_mtp_incompatible", domain);

    uint16_t control_port{};
    std::string local_addr{};
    std::array<uint8_t, 4> local_interface{};
    const auto deadline = std::chrono::steady_clock::now() + 2s;
    while (!control_port && std::chrono::steady_clock::now() < deadline) {
        auto [wire, addr, port] = observer.read();
        auto local = lpss::RNDPMessage::deserialize(wire);
        if (local && local->guid == node.guid() && !local->locators.empty()) {
            control_port = local->locators.front().port;
            local_addr = addr;
            ASSERT_EQ(::inet_pton(AF_INET, addr.c_str(), local_interface.data()), 1);
        }
        std::this_thread::sleep_for(5ms);
    }
    ASSERT_NE(control_port, 0);

    const lpss::Guid peer{0x12345678, 0x9877, 0};
    const lpss::Guid writer{0x12345678, 0x9877, 1};
    const std::string topic = "/rsd2/mtp_incompatible";
    auto control = Listener(Endpoint(ip::udp::v4(), Endpoint::ANY_PORT), false).create();
    auto receiveREDP = [&]() -> std::optional<lpss::REDPMessage> {
        const auto until = std::chrono::steady_clock::now() + 1s;
        do {
            auto [wire, addr, port] = control.read();
            if (auto packet = lpss::REDPMessage::deserialize(wire))
                return packet;
            std::this_thread::sleep_for(5ms);
        } while (std::chrono::steady_clock::now() < until);
        return std::nullopt;
    };
    lpss::RNDPMessage announcement{};
    announcement.guid = peer;
    announcement.mtp_version = 1;
    announcement.heartbeat_timeout = 1;
    announcement.locators = {{control.endpoint().port(), local_interface}};
    announcement.name = "mtp-incompatible-peer";
    const lpss::REDPMessage snapshot{peer, 100, false, {{writer, 0, topic, msg::String::msg_type}}};
    auto sender = Sender(ip::udp::v4()).create();
    sender.setOption(ip::multicast::Loopback(true));
    sender.setOption(ip::multicast::Interface(local_interface));
    auto sendRNDP = [&] {
        sender.write(lpss::BROADCAST_IP, Endpoint(ip::udp::v4(), 7500 + domain), announcement.serialize());
    };
    const auto discovery_deadline = std::chrono::steady_clock::now() + 2s;
    while (!node.hasPeer(peer) && std::chrono::steady_clock::now() < discovery_deadline) {
        sendRNDP();
        std::this_thread::sleep_for(5ms);
    }
    ASSERT_TRUE(node.hasPeer(peer));
    std::this_thread::sleep_for(10ms);

    const auto local_baseline = receiveREDP();
    ASSERT_TRUE(local_baseline.has_value());
    ASSERT_FALSE(local_baseline->ack);
    control.write(local_addr, Endpoint(ip::udp::v4(), control_port),
                  lpss::REDPMessage{peer, static_cast<uint16_t>(local_baseline->syn + 1), true, {}}.serialize());

    control.write(local_addr, Endpoint(ip::udp::v4(), control_port), snapshot.serialize());
    std::optional<lpss::REDPMessage> ack{};
    const auto ack_deadline = std::chrono::steady_clock::now() + 1s;
    while (!ack && std::chrono::steady_clock::now() < ack_deadline) {
        auto packet = receiveREDP();
        if (packet && packet->ack)
            ack = std::move(packet);
    }
    ASSERT_TRUE(ack.has_value());
    EXPECT_EQ(ack->syn, 101u);

    // MTP negotiation falls back to version 2, so the remote endpoint is usable.
    EXPECT_TRUE(node.hasCachedWriter(topic, writer));
    auto subscriber = node.createSubscriber<msg::String>(topic, [](const msg::String &) {});
    ASSERT_FALSE(subscriber.invalid());
    EXPECT_TRUE(node.hasCachedWriter(topic, writer));
    node.destroySubscriber(subscriber);
}

} // namespace rm_test

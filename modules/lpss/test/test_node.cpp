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

#ifdef _WIN32
#include <winsock2.h>
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
    auto recvdata = sock.read();
    auto start_time = lpss::now();
    while (recvdata.data.empty() && lpss::now() - start_time < 50ms)
        recvdata = sock.read();
    EXPECT_TRUE(recvdata.data.empty());
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

#endif

} // namespace rm_test

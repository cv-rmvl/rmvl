/**
 * @file node_rmtp.cpp
 * @author zhaoxi (535394140@qq.com)
 * @brief 轻量级发布订阅服务：消息传输协议 MTP 节点实现
 * @version 1.0
 * @date 2025-12-05
 *
 * @copyright Copyright 2025 (c), zhaoxi
 *
 */

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#else
#include <arpa/inet.h>
#endif

#include "rmvl/lpss/details/node_rsd.hpp"

namespace rm::lpss {

static std::string mtp_pack(std::string_view data, std::string_view type, std::string_view topic) {
    std::string res{};
    res.reserve(6 + type.size() + topic.size() + data.size());
    res.append("MT01");
    uint8_t topic_size = static_cast<uint8_t>(topic.size());
    res.append(reinterpret_cast<const char *>(&topic_size), sizeof(uint8_t));
    res.append(topic);
    uint8_t type_size = static_cast<uint8_t>(type.size());
    res.append(reinterpret_cast<const char *>(&type_size), sizeof(uint8_t));
    res.append(type);
    res.append(data);
    return res;
}

static std::string mtp_unpack(std::string_view data, std::string_view type, std::string_view topic) {
    const char *ptr = data.data();
    if (data.size() < 6 || std::string_view(ptr, 4) != "MT01")
        return {};
    ptr += 4;
    uint8_t topic_size = *reinterpret_cast<const uint8_t *>(ptr);
    ptr += sizeof(uint8_t);
    std::string_view read_topic(ptr, topic_size);
    if (read_topic != topic)
        return {};
    ptr += topic_size;
    uint8_t type_size = *reinterpret_cast<const uint8_t *>(ptr);
    ptr += sizeof(uint8_t);
    std::string_view read_type(ptr, type_size);
    if (read_type != type)
        return {};
    ptr += type_size;
    return std::string(ptr, data.size() - 6 - topic_size - type_size);
}

DataWriterBase::DataWriterBase(const Guid &guid, std::string_view type, std::string_view topic)
    : _guid(guid), _socket(Sender(ip::udp::v4()).create()), _type(type), _topic(topic) {}

void DataWriterBase::add(const Guid &guid, Locator loc) noexcept {
    std::lock_guard lk(_mtx);
    // UDPv4 通道
    _udpv4_targets[guid] = loc;
}

void DataWriterBase::remove(const Guid &guid) noexcept {
    std::lock_guard lk(_mtx);
    // UDPv4 通道
    _udpv4_targets.erase(guid);
}

using namespace std::chrono_literals;

#ifdef _WIN32
#undef min
#endif

void DataWriterBase::write(std::string data) noexcept {
    // 构造 RMTP 数据包
    std::string rmtp_data = mtp_pack(data, _type, _topic);
    // 数据分片
    std::string_view rmtp_view = rmtp_data;
    std::size_t offset = 0;
    constexpr std::size_t MAX_UDP_PAYLOAD = 65507;
    while (offset < rmtp_data.size()) {
        std::size_t len = std::min(MAX_UDP_PAYLOAD, rmtp_data.size() - offset);
        auto payload = rmtp_view.substr(offset, len);
        // 发送到 UDPv4 目标
        {
            std::shared_lock lk(_mtx);
            for (const auto &[guid, loc] : _udpv4_targets)
                _socket.write(loc.addr, Endpoint(ip::udp::v4(), loc.port), payload);
        }

        std::this_thread::sleep_for(100us);
        offset += len;
    }
    if (rmtp_view.size() > 0 && rmtp_view.size() % MAX_UDP_PAYLOAD == 0) {
        std::shared_lock lk(_mtx);
        for (const auto &[guid, loc] : _udpv4_targets)
            _socket.write(loc.addr, Endpoint(ip::udp::v4(), loc.port), std::string_view{});
    }
}

DataReaderBase::DataReaderBase(const Guid &guid, std::string_view type, std::string_view topic)
    : _guid(guid), _udpv4(Listener(Endpoint(ip::udp::v4(), Endpoint::ANY_PORT)).create()), _type(type), _topic(topic) {
    auto ep = _udpv4.endpoint();
    _port = ep.port();
}

std::string DataReaderBase::read() noexcept {
    std::string rmtp_data;
    rmtp_data.reserve(65536);

    constexpr std::size_t MAX_UDP_PAYLOAD = 65507;

    while (true) {
        // 从 UDPv4 通道读取数据
        auto [part, addr, port] = _udpv4.read();
        rmtp_data.append(part);

        if (part.size() < MAX_UDP_PAYLOAD)
            break;
    }

    // 提取数据
    return mtp_unpack(rmtp_data, _type, _topic);
}

#if __cplusplus >= 202002L

namespace async {

DataWriterBase::DataWriterBase(rm::async::IOContext &io_context, const Guid &guid, std::string_view type, std::string_view topic)
    : _guid(guid), _socket(rm::async::Sender(io_context, ip::udp::v4()).create()), _type(type), _topic(topic) {}

void DataWriterBase::add(const Guid &guid, Locator loc) noexcept {
    // UDPv4 通道
    _udpv4_targets[guid] = loc;
}

void DataWriterBase::remove(const Guid &guid) noexcept {
    // UDPv4 通道
    _udpv4_targets.erase(guid);
}

rm::async::Task<> DataWriterBase::write(std::string data) noexcept {
    // 构造 RMTP 数据包
    std::string rmtp_data = mtp_pack(data, _type, _topic);
    // 数据分片
    std::string_view rmtp_view = rmtp_data;
    std::size_t offset = 0;
    constexpr std::size_t MAX_UDP_PAYLOAD = 65507;
    while (offset < rmtp_data.size()) {
        std::size_t len = std::min(MAX_UDP_PAYLOAD, rmtp_data.size() - offset);
        auto payload = rmtp_view.substr(offset, len);
        // 发送到 UDPv4 目标
        for (const auto &[guid, loc] : _udpv4_targets)
            co_await _socket.write(loc.addr, Endpoint(ip::udp::v4(), loc.port), payload);
        std::this_thread::sleep_for(100us);
        offset += len;
    }
    if (rmtp_view.size() > 0 && rmtp_view.size() % MAX_UDP_PAYLOAD == 0)
        for (const auto &[guid, loc] : _udpv4_targets)
            co_await _socket.write(loc.addr, Endpoint(ip::udp::v4(), loc.port), std::string_view{});
}

DataReaderBase::DataReaderBase(rm::async::IOContext &io_context, const Guid &guid, std::string_view type, std::string_view topic)
    : _guid(guid), _udpv4(rm::async::Listener(io_context, Endpoint(ip::udp::v4(), Endpoint::ANY_PORT)).create()), _type(type), _topic(topic) {
    auto ep = _udpv4.endpoint();
    _port = ep.port();
}

rm::async::Task<std::string> DataReaderBase::read() noexcept {
    std::string data;
    data.reserve(65536);
    constexpr std::size_t MAX_UDP_PAYLOAD = 65507;
    while (true) {
        // 从 UDPv4 通道读取数据
        auto [part, addr, port] = co_await _udpv4.read();
        data.append(part);

        if (part.size() < MAX_UDP_PAYLOAD)
            break;
    }
    // 提取数据
    co_return mtp_unpack(data, _type, _topic);
}

} // namespace async

#endif

} // namespace rm::lpss

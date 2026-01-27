/**
 * @file node_rmtp.cpp
 * @author zhaoxi (535394140@qq.com)
 * @brief 轻量级发布订阅服务：消息传输协议 RMTP 节点实现
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
    // SHM 通道

    // UDPv4 通道
    _udpv4_targets[guid] = loc;
}

void DataWriterBase::remove(const Guid &guid) noexcept {
    std::lock_guard lk(_mtx);
    // SHM 通道

    // UDPv4 通道
    _udpv4_targets.erase(guid);
}

void DataWriterBase::write(std::string data) noexcept {
    // 构造 RMTP 数据包
    std::string rmtp_data = mtp_pack(data, _type, _topic);
    // 发送到 SHM 目标

    // 发送到 UDPv4 目标
    std::shared_lock lk(_mtx);
    for (const auto &[guid, loc] : _udpv4_targets)
        _socket.write(loc.addr, Endpoint(ip::udp::v4(), loc.port), rmtp_data);
}

DataReaderBase::DataReaderBase(const Guid &guid, std::string_view type, std::string_view topic)
    : _guid(guid), _udpv4(Listener(Endpoint(ip::udp::v4(), Endpoint::ANY_PORT)).create()), _type(type), _topic(topic) {
    auto ep = _udpv4.endpoint();
    _port = ep.port();
}

std::string DataReaderBase::read() noexcept {
    // 从 UDPv4 通道读取数据
    auto [data, addr, port] = _udpv4.read();

    // 从 SHM 通道读取数据

    // 提取数据
    return mtp_unpack(data, _type, _topic);
}

#if __cplusplus >= 202002L

namespace async {

DataWriterBase::DataWriterBase(rm::async::IOContext &io_context, const Guid &guid, std::string_view type, std::string_view topic)
    : _guid(guid), _socket(rm::async::Sender(io_context, ip::udp::v4()).create()), _type(type), _topic(topic) {}

void DataWriterBase::add(const Guid &guid, Locator loc) noexcept {
    // SHM 通道

    // UDPv4 通道
    _udpv4_targets[guid] = loc;
}

void DataWriterBase::remove(const Guid &guid) noexcept {
    // SHM 通道

    // UDPv4 通道
    _udpv4_targets.erase(guid);
}

rm::async::Task<> DataWriterBase::write(std::string data) noexcept {
    // 构造 RMTP 数据包
    std::string rmtp_data = mtp_pack(data, _type, _topic);
    // 发送到 SHM 目标

    // 发送到 UDPv4 目标
    for (const auto &[guid, loc] : _udpv4_targets)
        co_await _socket.write(loc.addr, Endpoint(ip::udp::v4(), loc.port), rmtp_data);
}

DataReaderBase::DataReaderBase(rm::async::IOContext &io_context, const Guid &guid, std::string_view type, std::string_view topic)
    : _guid(guid), _udpv4(rm::async::Listener(io_context, Endpoint(ip::udp::v4(), Endpoint::ANY_PORT)).create()), _type(type), _topic(topic) {
    auto ep = _udpv4.endpoint();
    _port = ep.port();
}

rm::async::Task<std::string> DataReaderBase::read() noexcept {
    // 从 UDPv4 通道读取数据
    auto [data, addr, port] = co_await _udpv4.read();

    // 从 SHM 通道读取数据

    // 提取数据
    co_return mtp_unpack(data, _type, _topic);
}

} // namespace async

#endif

} // namespace rm::lpss

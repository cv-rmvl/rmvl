/**
 * @file node_rtio.cpp
 * @author zhaoxi (535394140@qq.com)
 * @brief 轻量级发布订阅服务：话题 IO 实现
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

#include "rmvl/lpss/node_rsd.hpp"

namespace rm::lpss {

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

void DataWriterBase::write(std::string_view data) noexcept {
    // 构造 RTIO 数据包
    std::string rtio_data;
    rtio_data.reserve(6 + _type.size() + _topic.size() + data.size());
    rtio_data.append("RTIO");
    uint8_t topic_size = static_cast<uint8_t>(_topic.size());
    rtio_data.append(reinterpret_cast<const char *>(&topic_size), sizeof(uint8_t));
    rtio_data.append(_topic);
    uint8_t type_size = static_cast<uint8_t>(_type.size());
    rtio_data.append(reinterpret_cast<const char *>(&type_size), sizeof(uint8_t));
    rtio_data.append(_type);
    rtio_data.append(data);
    // 发送到 SHM 目标

    // 发送到 UDPv4 目标
    std::shared_lock lk(_mtx);
    for (const auto &[guid, loc] : _udpv4_targets)
        _socket.write(loc.addr, Endpoint(ip::udp::v4(), loc.port), rtio_data);
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
    const char *ptr = data.data();
    if (data.size() < 6 || std::string_view(ptr, 4) != "RTIO")
        return {};
    ptr += 4;
    uint8_t topic_size = *reinterpret_cast<const uint8_t *>(ptr);
    ptr += sizeof(uint8_t);
    std::string_view topic(ptr, topic_size);
    if (topic != _topic)
        return {};
    ptr += topic_size;
    uint8_t type_size = *reinterpret_cast<const uint8_t *>(ptr);
    ptr += sizeof(uint8_t);
    std::string_view type(ptr, type_size);
    if (type != _type)
        return {};
    ptr += type_size;
    return std::string(ptr, data.size() - 6 - topic_size - type_size);
}

} // namespace rm::lpss

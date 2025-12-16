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

DataWriterBase::DataWriterBase(const Guid &guid, std::string_view topic)
    : _guid(guid), _socket(Sender(ip::udp::v4()).create()), _topic(topic) {}

void DataWriterBase::add(const Guid &guid, Locator loc) noexcept {
    std::lock_guard lk(_mtx);
    // SHM 通道
    if (_guid.mac == guid.mac) {
        _udpv4_targets[guid] = loc; // 临时处理
    } else                          // UDPv4 通道
        _udpv4_targets[guid] = loc;
}

void DataWriterBase::remove(const Guid &guid) noexcept {
    std::lock_guard lk(_mtx);
    // SHM 通道
    if (_guid.mac == guid.mac) {
        _udpv4_targets.erase(guid); // 临时处理
    } else                          // UDPv4 通道
        _udpv4_targets.erase(guid);
}

void DataWriterBase::write(std::string_view data) noexcept {
    // 发送到 SHM 目标

    // 发送到 UDPv4 目标
    std::shared_lock lk(_mtx);
    for (const auto &[guid, loc] : _udpv4_targets) {
        char addr_str[INET_ADDRSTRLEN]{};
        inet_ntop(AF_INET, &loc.addr, addr_str, INET_ADDRSTRLEN);
        _socket.write(addr_str, Endpoint(ip::udp::v4(), loc.port), data);
    }
}

DataReaderBase::DataReaderBase(const Guid &guid, std::string_view /* topic */)
    : _guid(guid), _port(0), _udpv4(Listener(Endpoint(ip::udp::v4(), 0)).create()) {
    auto ep = _udpv4.endpoint();
    _port = ep.port();
}

std::string DataReaderBase::read() noexcept {
    // 从 UDPv4 通道读取数据
    auto [data, addr, port] = _udpv4.read();

    // 从 SHM 通道读取数据

    return data;
}

} // namespace rm::lpss

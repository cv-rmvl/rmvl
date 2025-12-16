/**
 * @file node_rsd.cpp
 * @author zhaoxi (535394140@qq.com)
 * @brief RMVL 服务发现协议实现
 * @version 1.0
 * @date 2025-11-24
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

#include <cstring>

#include "rmvl/lpss/node_rsd.hpp"

namespace rm::lpss {

size_t GuidHash::operator()(const rm::lpss::Guid &guid) const noexcept {
    size_t seed{};
    constexpr auto MAGIC = 0x9e3779b9;
    for (uint8_t byte : guid.mac)
        seed ^= std::hash<uint8_t>{}(byte) + MAGIC + (seed << 6) + (seed >> 2);
    seed ^= std::hash<uint16_t>{}(guid.pid) + MAGIC + (seed << 6) + (seed >> 2);
    seed ^= std::hash<uint16_t>{}(guid.entity) + MAGIC + (seed << 6) + (seed >> 2);
    return seed;
}

std::string RNDPMessage::serialize() const noexcept {
    const uint8_t num = static_cast<uint8_t>(locators.size());
    const size_t msg_size = RNDP_HEADER_SIZE + num * sizeof(Locator);
    std::string rndp_msg(msg_size, '\0');

    // RNDP 头部
    strcpy(rndp_msg.data(), "RNDP");
    ::memcpy(rndp_msg.data() + 4, guid.mac.data(), guid.mac.size());
    new (rndp_msg.data() + 4 + sizeof(guid.mac)) uint16_t{::htons(guid.pid)};
    new (rndp_msg.data() + 4 + sizeof(guid.mac) + sizeof(guid.pid)) uint16_t{::htons(guid.entity)};
    ::memcpy(rndp_msg.data() + 4 + sizeof(guid), &num, sizeof(num));
    ::memcpy(rndp_msg.data() + 4 + sizeof(guid) + 1, &heartbeat_timeout, sizeof(heartbeat_timeout));

    // RNDP 负载
    auto p_locator = reinterpret_cast<Locator *>(rndp_msg.data() + RNDP_HEADER_SIZE);
    for (const auto &loc : locators) {
        p_locator = new (p_locator) Locator{static_cast<uint16_t>(::htons(loc.port)), loc.addr};
        ++p_locator;
    }
    return rndp_msg;
}

RNDPMessage RNDPMessage::deserialize(const char *data) noexcept {
    RNDPMessage res;
    if (std::string_view(data, 4) != "RNDP")
        return res;
    ::memcpy(res.guid.mac.data(), data + 4, res.guid.mac.size());
    res.guid.pid = ::ntohs(*reinterpret_cast<const uint16_t *>(data + 4 + sizeof(guid.mac)));
    res.guid.entity = ::ntohs(*reinterpret_cast<const uint16_t *>(data + 4 + sizeof(guid.mac) + sizeof(guid.pid)));
    auto num = *reinterpret_cast<const uint8_t *>(data + 4 + sizeof(guid));
    res.heartbeat_timeout = *reinterpret_cast<const uint8_t *>(data + 4 + sizeof(guid) + sizeof(num));
    res.locators.reserve(num);

    for (size_t i = 0; i < num; ++i) {
        Locator locator = *reinterpret_cast<const Locator *>(data + RNDP_HEADER_SIZE + i * sizeof(Locator));
        locator.port = ::ntohs(locator.port);
        res.locators.emplace_back(locator);
    }
    return res;
}

std::string REDPMessage::serialize() const noexcept {
    // 数据准备
    const uint8_t topic_size = static_cast<uint8_t>(topic.length());
    const size_t msg_size = REDP_HEADER_SIZE + 6 + topic_size;
    std::string redp_msg(msg_size, '\0');

    // REDP 头部
    ::strcpy(redp_msg.data(), "REDP");
    ::memcpy(redp_msg.data() + 4, &endpoint_guid.mac, sizeof(endpoint_guid.mac));
    new (redp_msg.data() + 4 + sizeof(endpoint_guid.mac)) uint16_t{::htons(endpoint_guid.pid)};
    new (redp_msg.data() + 4 + sizeof(endpoint_guid.mac) + sizeof(endpoint_guid.pid)) uint16_t{::htons(endpoint_guid.entity)};
    new (redp_msg.data() + 4 + sizeof(endpoint_guid)) uint8_t{static_cast<uint8_t>(static_cast<uint8_t>(action) | static_cast<uint8_t>(type))};
    ::memcpy(redp_msg.data() + 4 + sizeof(endpoint_guid) + 1, &topic_size, sizeof(topic_size));

    // REDP 负载
    uint16_t net_port = htons(port);
    ::memcpy(redp_msg.data() + REDP_HEADER_SIZE, &net_port, sizeof(net_port));
    ::memcpy(redp_msg.data() + REDP_HEADER_SIZE + 2, topic.data(), topic_size);
    return redp_msg;
}

REDPMessage REDPMessage::deserialize(const char *data) noexcept {
    REDPMessage res;
    // REDP 头部
    if (std::string_view(data, 4) != "REDP")
        return res;
    ::memcpy(res.endpoint_guid.mac.data(), data + 4, res.endpoint_guid.mac.size());
    res.endpoint_guid.pid = ntohs(*reinterpret_cast<const uint16_t *>(data + 4 + sizeof(endpoint_guid.mac)));
    res.endpoint_guid.entity = ntohs(*reinterpret_cast<const uint16_t *>(data + 4 + sizeof(endpoint_guid.mac) + sizeof(endpoint_guid.pid)));
    res.action = static_cast<Action>(*reinterpret_cast<const uint8_t *>(data + 4 + sizeof(endpoint_guid)) & 0b01);
    res.type = static_cast<Type>(*reinterpret_cast<const uint8_t *>(data + 4 + sizeof(endpoint_guid)) & 0b10);
    uint8_t topic_size = *reinterpret_cast<const uint8_t *>(data + 4 + sizeof(endpoint_guid) + 1);

    // REDP 负载
    res.port = ntohs(*reinterpret_cast<const uint16_t *>(data + REDP_HEADER_SIZE));
    res.topic = std::string(data + REDP_HEADER_SIZE + 2, topic_size);
    return res;
}

void sendREDPMessage(Locator loc, const REDPMessage &msg) {
    static DgramSocket redp_sender = Sender(ip::udp::v4()).create();
    char addr_str[INET_ADDRSTRLEN]{};
    ::inet_ntop(AF_INET, &loc.addr, addr_str, INET_ADDRSTRLEN);
    redp_sender.write(addr_str, Endpoint(ip::udp::v4(), loc.port), msg.serialize());
}

} // namespace rm::lpss
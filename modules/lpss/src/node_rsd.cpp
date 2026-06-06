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

#include "rmvl/lpss/details/node_rsd.hpp"

namespace rm::lpss {

namespace {

constexpr std::size_t LOCATOR_WIRE_SIZE = sizeof(uint16_t) + 4;

void write_locator(char *data, const Locator &locator) noexcept {
    const uint16_t net_port = htons(locator.port);
    ::memcpy(data, &net_port, sizeof(net_port));
    ::memcpy(data + sizeof(net_port), locator.addr.data(), locator.addr.size());
}

Locator read_locator(const char *data) noexcept {
    Locator locator{};
    uint16_t net_port{};
    ::memcpy(&net_port, data, sizeof(net_port));
    locator.port = ntohs(net_port);
    ::memcpy(locator.addr.data(), data + sizeof(net_port), locator.addr.size());
    return locator;
}

} // namespace

std::string RNDPMessage::serialize() const noexcept {
    const uint8_t num = static_cast<uint8_t>(locators.size());
    const size_t msg_size = RNDP_HEADER_SIZE + num * LOCATOR_WIRE_SIZE + 1 + name.length();
    std::string rndp_msg(msg_size, '\0');

    // RNDP 头部
    strcpy(rndp_msg.data(), "ND01");
    ::memcpy(rndp_msg.data() + 4, &guid.full, sizeof(guid.full));
    ::memcpy(rndp_msg.data() + 4 + sizeof(guid.full), &num, sizeof(num));
    ::memcpy(rndp_msg.data() + 4 + sizeof(guid.full) + 1, &heartbeat_timeout, sizeof(heartbeat_timeout));

    // RNDP 数据
    auto p_locator = rndp_msg.data() + RNDP_HEADER_SIZE;
    for (const auto &loc : locators) {
        write_locator(p_locator, loc);
        p_locator += LOCATOR_WIRE_SIZE;
    }

    uint8_t name_size = static_cast<uint8_t>(name.length());
    ::memcpy(rndp_msg.data() + RNDP_HEADER_SIZE + num * LOCATOR_WIRE_SIZE, &name_size, sizeof(name_size));
    ::memcpy(rndp_msg.data() + RNDP_HEADER_SIZE + num * LOCATOR_WIRE_SIZE + 1, name.data(), name_size);

    return rndp_msg;
}

RNDPMessage RNDPMessage::deserialize(const char *data) noexcept {
    RNDPMessage res;
    if (std::string_view(data, 4) != "ND01")
        return res;
    ::memcpy(&res.guid.full, data + 4, sizeof(res.guid.full));
    uint8_t num{};
    ::memcpy(&num, data + 4 + sizeof(res.guid.full), sizeof(num));
    ::memcpy(&res.heartbeat_timeout, data + 4 + sizeof(res.guid.full) + sizeof(num), sizeof(res.heartbeat_timeout));
    res.locators.reserve(num);

    for (size_t i = 0; i < num; ++i)
        res.locators.emplace_back(read_locator(data + RNDP_HEADER_SIZE + i * LOCATOR_WIRE_SIZE));

    uint8_t name_size{};
    ::memcpy(&name_size, data + RNDP_HEADER_SIZE + num * LOCATOR_WIRE_SIZE, sizeof(name_size));
    res.name = std::string(data + RNDP_HEADER_SIZE + num * LOCATOR_WIRE_SIZE + 1, name_size);
    return res;
}

std::string REDPMessage::serialize() const noexcept {
    // 数据准备
    const uint8_t topic_size = static_cast<uint8_t>(topic.length());
    const uint8_t msgtype_size = static_cast<uint8_t>(msgtype.length());
    const size_t msg_size = REDP_HEADER_SIZE + 3 + topic_size + 1 + msgtype_size;
    std::string redp_msg(msg_size, '\0');

    // REDP 头部
    ::strcpy(redp_msg.data(), "ED01");
    ::memcpy(redp_msg.data() + 4, &endpoint_guid.full, sizeof(endpoint_guid.full));
    uint8_t action_type = static_cast<uint8_t>(static_cast<uint8_t>(action) | static_cast<uint8_t>(type));
    ::memcpy(redp_msg.data() + 4 + sizeof(endpoint_guid.full), &action_type, sizeof(action_type));

    // REDP 负载
    uint16_t net_port = htons(port);
    ::memcpy(redp_msg.data() + REDP_HEADER_SIZE, &net_port, sizeof(net_port));
    ::memcpy(redp_msg.data() + REDP_HEADER_SIZE + 2, &topic_size, sizeof(topic_size));
    ::memcpy(redp_msg.data() + REDP_HEADER_SIZE + 3, topic.data(), topic_size);
    ::memcpy(redp_msg.data() + REDP_HEADER_SIZE + 3 + topic_size, &msgtype_size, sizeof(msgtype_size));
    ::memcpy(redp_msg.data() + REDP_HEADER_SIZE + 3 + topic_size + 1, msgtype.data(), msgtype_size);
    return redp_msg;
}

REDPMessage REDPMessage::deserialize(const char *data) noexcept {
    REDPMessage res;
    // REDP 头部
    if (std::string_view(data, 4) != "ED01")
        return res;
    ::memcpy(&res.endpoint_guid.full, data + 4, sizeof(res.endpoint_guid.full));
    uint8_t action_type{};
    ::memcpy(&action_type, data + 4 + sizeof(res.endpoint_guid.full), sizeof(action_type));
    res.action = static_cast<Action>(action_type & 0b01);
    res.type = static_cast<Type>(action_type & 0b10);

    // REDP 负载
    uint16_t net_port{};
    ::memcpy(&net_port, data + REDP_HEADER_SIZE, sizeof(net_port));
    res.port = ntohs(net_port);
    uint8_t topic_size{};
    ::memcpy(&topic_size, data + REDP_HEADER_SIZE + 2, sizeof(topic_size));
    res.topic = std::string(data + REDP_HEADER_SIZE + 3, topic_size);
    uint8_t msgtype_size{};
    ::memcpy(&msgtype_size, data + REDP_HEADER_SIZE + 3 + topic_size, sizeof(msgtype_size));
    res.msgtype = std::string(data + REDP_HEADER_SIZE + 3 + topic_size + 1, msgtype_size);
    return res;
}

////////////////////////////////////////////////////////////////////////////////

void sendREDPMessage(Locator loc, const REDPMessage &msg) {
    static DgramSocket redp_sender = Sender(ip::udp::v4()).create();
    char addr_str[INET_ADDRSTRLEN]{};
    ::inet_ntop(AF_INET, &loc.addr, addr_str, INET_ADDRSTRLEN);
    redp_sender.write(addr_str, Endpoint(ip::udp::v4(), loc.port), msg.serialize());
}

Locator selectBestLocator(std::array<uint8_t, 4> target_ip, const RNDPMessage &rndp_msg) {
    for (const auto &loc : rndp_msg.locators)
        if (loc.addr == target_ip)
            return loc;
    return {};
}

} // namespace rm::lpss

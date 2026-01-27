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

#include <algorithm>
#include <cstring>

#include "rmvl/core/util.hpp"
#include "rmvl/lpss/details/node_rsd.hpp"

namespace rm::lpss {

Guid::Guid(uint32_t h, uint16_t p, uint16_t e) {
    fields.host = h;
    fields.pid = p;
    fields.entity = e;
}

std::string RNDPMessage::serialize() const noexcept {
    const uint8_t num = static_cast<uint8_t>(locators.size());
    const size_t msg_size = RNDP_HEADER_SIZE + num * sizeof(Locator) + 1 + name.length();
    std::string rndp_msg(msg_size, '\0');

    // RNDP 头部
    strcpy(rndp_msg.data(), "ND01");
    ::memcpy(rndp_msg.data() + 4, &guid.full, sizeof(guid.full));
    ::memcpy(rndp_msg.data() + 4 + sizeof(guid.full), &num, sizeof(num));
    ::memcpy(rndp_msg.data() + 4 + sizeof(guid.full) + 1, &heartbeat_timeout, sizeof(heartbeat_timeout));

    // RNDP 数据
    auto p_locator = reinterpret_cast<Locator *>(rndp_msg.data() + RNDP_HEADER_SIZE);
    for (const auto &loc : locators) {
        p_locator = new (p_locator) Locator{static_cast<uint16_t>(::htons(loc.port)), loc.addr};
        ++p_locator;
    }

    uint8_t name_size = static_cast<uint8_t>(name.length());
    ::memcpy(rndp_msg.data() + RNDP_HEADER_SIZE + num * sizeof(Locator), &name_size, sizeof(name_size));
    ::memcpy(rndp_msg.data() + RNDP_HEADER_SIZE + num * sizeof(Locator) + 1, name.data(), name_size);

    return rndp_msg;
}

RNDPMessage RNDPMessage::deserialize(const char *data) noexcept {
    RNDPMessage res;
    if (std::string_view(data, 4) != "ND01")
        return res;
    ::memcpy(&res.guid.full, data + 4, sizeof(res.guid.full));
    auto num = *reinterpret_cast<const uint8_t *>(data + 4 + sizeof(res.guid.full));
    res.heartbeat_timeout = *reinterpret_cast<const uint8_t *>(data + 4 + sizeof(res.guid.full) + sizeof(num));
    res.locators.reserve(num);

    for (size_t i = 0; i < num; ++i) {
        Locator locator = *reinterpret_cast<const Locator *>(data + RNDP_HEADER_SIZE + i * sizeof(Locator));
        locator.port = ::ntohs(locator.port);
        res.locators.emplace_back(locator);
    }

    uint8_t name_size = *reinterpret_cast<const uint8_t *>(data + RNDP_HEADER_SIZE + num * sizeof(Locator));
    res.name = std::string(data + RNDP_HEADER_SIZE + num * sizeof(Locator) + 1, name_size);
    return res;
}

std::string REDPMessage::serialize() const noexcept {
    // 数据准备
    const uint8_t topic_size = static_cast<uint8_t>(topic.length());
    const size_t msg_size = REDP_HEADER_SIZE + 6 + topic_size;
    std::string redp_msg(msg_size, '\0');

    // REDP 头部
    ::strcpy(redp_msg.data(), "ED01");
    ::memcpy(redp_msg.data() + 4, &endpoint_guid.full, sizeof(endpoint_guid.full));
    new (redp_msg.data() + 4 + sizeof(endpoint_guid.full)) uint8_t{static_cast<uint8_t>(static_cast<uint8_t>(action) | static_cast<uint8_t>(type))};
    ::memcpy(redp_msg.data() + 4 + sizeof(endpoint_guid.full) + 1, &topic_size, sizeof(topic_size));
    // REDP 负载
    uint16_t net_port = htons(port);
    ::memcpy(redp_msg.data() + REDP_HEADER_SIZE, &net_port, sizeof(net_port));
    ::memcpy(redp_msg.data() + REDP_HEADER_SIZE + 2, topic.data(), topic_size);
    return redp_msg;
}

REDPMessage REDPMessage::deserialize(const char *data) noexcept {
    REDPMessage res;
    // REDP 头部
    if (std::string_view(data, 4) != "ED01")
        return res;
    ::memcpy(&res.endpoint_guid.full, data + 4, sizeof(res.endpoint_guid.full));
    res.action = static_cast<Action>(*reinterpret_cast<const uint8_t *>(data + 4 + sizeof(res.endpoint_guid.full)) & 0b01);
    res.type = static_cast<Type>(*reinterpret_cast<const uint8_t *>(data + 4 + sizeof(res.endpoint_guid.full)) & 0b10);
    uint8_t topic_size = *reinterpret_cast<const uint8_t *>(data + 4 + sizeof(res.endpoint_guid.full) + 1);

    // REDP 负载
    res.port = ntohs(*reinterpret_cast<const uint16_t *>(data + REDP_HEADER_SIZE));
    res.topic = std::string(data + REDP_HEADER_SIZE + 2, topic_size);
    return res;
}

////////////////////////////////////////////////////////////////////////////////

std::pair<Guid, std::vector<ip::Networkv4>> generateNodeGuid() {
    Guid res{};
    auto interfaces = NetworkInterface::list();
    std::vector<NetworkInterface> candidate_interfaces;

    for (const auto &iface : interfaces)
        if (iface.up() && iface.running() && !iface.loopback() && iface.multicast())
            candidate_interfaces.push_back(iface);

    // 按类型优先级和名称排序
    std::sort(candidate_interfaces.begin(), candidate_interfaces.end(), [](const NetworkInterface &a, const NetworkInterface &b) {
        return a.type() != b.type() ? a.type() < b.type() : a.name() < b.name();
    });

    std::vector<ip::Networkv4> networks{};
    networks.reserve(candidate_interfaces.size());
    if (!candidate_interfaces.empty()) {
        // 选取最优接口为 GUID MAC 和 basic_mac
        const auto &primary_iface = candidate_interfaces.front();
        auto basic_mac = primary_iface.address();
        res.fields.host = (static_cast<uint32_t>(basic_mac[2]) << 24) |
                          (static_cast<uint32_t>(basic_mac[3]) << 16) |
                          (static_cast<uint32_t>(basic_mac[4]) << 8) |
                          static_cast<uint32_t>(basic_mac[5]);
        res.fields.pid = static_cast<uint16_t>(processId());
        res.fields.entity = 0;
        // 收集 IP
        for (const auto &iface : candidate_interfaces) {
            auto nets = iface.ipv4();
            networks.insert(networks.end(), std::make_move_iterator(nets.begin()), std::make_move_iterator(nets.end()));
        }
    } else {
        WARNING_("[LPSS Node] No valid network interface found, using 00:00:00:00:00:00 as MAC address");
        res.fields.pid = static_cast<uint16_t>(processId());
    }
    return {res, networks};
}

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
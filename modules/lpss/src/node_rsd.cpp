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
#include <random>
#include <unordered_set>

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

CompatibleProtocol negotiate(uint8_t cur_edp, uint8_t cur_mtp,
                             uint8_t peer_edp, uint8_t peer_mtp) noexcept {
    constexpr uint8_t min_version = 2;
    return {std::max(min_version, std::min(cur_edp, peer_edp)),
            std::max(min_version, std::min(cur_mtp, peer_mtp))};
}

bool sameREDPEndpoint(const REDPEndpoint &a, const REDPEndpoint &b) noexcept {
    return a.guid == b.guid && a.port == b.port &&
           a.topic == b.topic && a.msgtype == b.msgtype;
}

bool snapshotBelongsToPeer(Guid peer, const std::vector<REDPEndpoint> &snapshot) noexcept {
    for (const auto &endpoint : snapshot)
        if (!endpoint.guid.full || endpoint.guid.host() != peer.host() ||
            endpoint.guid.pid() != peer.pid())
            return false;
    return true;
}

std::string RNDPMessage::serialize() const noexcept {
    if (locators.size() > UINT8_MAX || name.size() > UINT8_MAX)
        return {};
    constexpr size_t header_size = RNDP_HEADER_SIZE;
    const auto num = static_cast<uint8_t>(locators.size());
    const auto name_size = static_cast<uint8_t>(name.size());
    std::string rndp_msg(header_size + num * LOCATOR_WIRE_SIZE + 1 + name_size, '\0');
    // RNDP 头部
    ::memcpy(rndp_msg.data(), "ND02", 4);
    ::memcpy(rndp_msg.data() + 4, &guid.full, sizeof(guid.full));
    rndp_msg[12] = 'v';
    rndp_msg[13] = static_cast<char>(ndp_version);
    rndp_msg[14] = static_cast<char>(edp_version);
    rndp_msg[15] = static_cast<char>(mtp_version);
    rndp_msg[16] = static_cast<char>(num);
    rndp_msg[17] = static_cast<char>(heartbeat_timeout);

    // RNDP 数据
    auto p_locator = rndp_msg.data() + header_size;
    for (const auto &loc : locators) {
        write_locator(p_locator, loc);
        p_locator += LOCATOR_WIRE_SIZE;
    }
    *p_locator++ = static_cast<char>(name_size);
    ::memcpy(p_locator, name.data(), name_size);
    return rndp_msg;
}

std::optional<RNDPMessage> RNDPMessage::deserialize(std::string_view data) noexcept {
    constexpr size_t header_size = RNDP_HEADER_SIZE;
    if (data.size() < header_size + 1 || data.substr(0, 4) != "ND02" ||
        data[12] != 'v' || static_cast<uint8_t>(data[13]) < 2)
        return std::nullopt;
    const auto num = static_cast<uint8_t>(data[16]);
    const size_t name_pos = header_size + size_t(num) * LOCATOR_WIRE_SIZE;
    if (name_pos >= data.size())
        return std::nullopt;
    const auto name_size = static_cast<uint8_t>(data[name_pos]);
    if (name_pos + 1 + name_size != data.size())
        return std::nullopt;

    RNDPMessage res{};
    ::memcpy(&res.guid.full, data.data() + 4, 8);
    if (!res.guid.full)
        return std::nullopt;
    res.ndp_version = static_cast<uint8_t>(data[13]);
    res.edp_version = static_cast<uint8_t>(data[14]);
    res.mtp_version = static_cast<uint8_t>(data[15]);
    res.heartbeat_timeout = static_cast<uint8_t>(data[17]);
    if (!res.heartbeat_timeout)
        return std::nullopt;
    res.locators.reserve(num);
    for (size_t i = 0; i < num; ++i)
        res.locators.emplace_back(read_locator(data.data() + header_size + i * LOCATOR_WIRE_SIZE));
    res.name.assign(data.data() + name_pos + 1, name_size);
    return res;
}

////////////////////////////////////////////////////////////////////////////////
// REDP 报文与可靠同步

uint16_t initialREDPSyn() {
    std::mt19937 rng(std::random_device{}());
    return std::uniform_int_distribution<uint16_t>{0, UINT16_MAX}(rng);
}

bool countREDPEndpointSize(size_t &total, std::string_view topic,
                           std::string_view type) noexcept {
    constexpr size_t max_payload = 65507;
    if (topic.empty() || type.empty() || topic.size() > 63 || type.size() > 63)
        return false;
    const size_t entry_size = 12 + topic.size() + type.size();
    if (total > max_payload || entry_size > max_payload - total)
        return false;
    total += entry_size;
    return true;
}

std::string REDPMessage::serialize() const {
    if (snapshot.size() > UINT16_MAX || (ack && !snapshot.empty()))
        return {};
    size_t msg_size = 17;
    std::unordered_set<uint64_t> ids{};
    for (const auto &ep : snapshot)
        if (!ep.guid.full || !ids.insert(ep.guid.full).second ||
            !countREDPEndpointSize(msg_size, ep.topic, ep.msgtype))
            return {};

    std::string redp_msg(msg_size, '\0');
    ::memcpy(redp_msg.data(), "ED02", 4);
    ::memcpy(redp_msg.data() + 4, &source.full, 8);
    const auto net_syn = htons(syn);
    const auto net_count = htons(static_cast<uint16_t>(snapshot.size()));
    ::memcpy(redp_msg.data() + 12, &net_syn, 2);
    redp_msg[14] = ack ? 1 : 0;
    ::memcpy(redp_msg.data() + 15, &net_count, 2);
    size_t off = 17;
    for (const auto &ep : snapshot) {
        ::memcpy(redp_msg.data() + off, &ep.guid.full, 8);
        off += 8;
        const auto net_port = htons(ep.port);
        ::memcpy(redp_msg.data() + off, &net_port, 2);
        off += 2;
        redp_msg[off++] = static_cast<char>(ep.topic.size());
        ::memcpy(redp_msg.data() + off, ep.topic.data(), ep.topic.size());
        off += ep.topic.size();
        redp_msg[off++] = static_cast<char>(ep.msgtype.size());
        ::memcpy(redp_msg.data() + off, ep.msgtype.data(), ep.msgtype.size());
        off += ep.msgtype.size();
    }
    return redp_msg;
}

std::optional<REDPMessage> REDPMessage::deserialize(std::string_view data) noexcept {
    if (data.size() < 17 || data.size() > 65507 || data.substr(0, 4) != "ED02" ||
        static_cast<uint8_t>(data[14]) > 1)
        return std::nullopt;
    REDPMessage res{};
    ::memcpy(&res.source.full, data.data() + 4, 8);
    if (!res.source.full)
        return std::nullopt;
    uint16_t net_syn{}, net_count{};
    ::memcpy(&net_syn, data.data() + 12, 2);
    ::memcpy(&net_count, data.data() + 15, 2);
    res.syn = ntohs(net_syn);
    res.ack = data[14] == 1;
    const auto count = ntohs(net_count);
    if ((res.ack && count != 0) || count > (data.size() - 17) / 14)
        return std::nullopt;
    size_t off = 17;
    res.snapshot.reserve(count);
    std::unordered_set<uint64_t> ids{};
    for (size_t i = 0; i < count; ++i) {
        if (data.size() - off < 12)
            return std::nullopt;
        REDPEndpoint ep{};
        ::memcpy(&ep.guid.full, data.data() + off, 8);
        off += 8;
        uint16_t net_port{};
        ::memcpy(&net_port, data.data() + off, 2);
        ep.port = ntohs(net_port);
        off += 2;
        const size_t topic_size = static_cast<uint8_t>(data[off++]);
        if (topic_size == 0 || topic_size > 63 || topic_size >= data.size() - off)
            return std::nullopt;
        ep.topic.assign(data.data() + off, topic_size);
        off += topic_size;
        const size_t msgtype_size = static_cast<uint8_t>(data[off++]);
        if (msgtype_size == 0 || msgtype_size > 63 || msgtype_size > data.size() - off ||
            !ep.guid.full || !ids.insert(ep.guid.full).second)
            return std::nullopt;
        ep.msgtype.assign(data.data() + off, msgtype_size);
        off += msgtype_size;
        res.snapshot.push_back(std::move(ep));
    }
    if (off != data.size())
        return std::nullopt;
    return res;
}

std::optional<REDPMessage> REDPSender::request(
    const std::vector<REDPEndpoint> &snapshot) {
    if (_pending)
        return _pending;
    if (!_dirty)
        return std::nullopt;
    _pending = REDPMessage{_local, _next++, false, snapshot};
    _dirty = false;
    return _pending;
}

bool REDPSender::confirm(const REDPMessage &reply,
                         const std::vector<REDPEndpoint> &snapshot) {
    if (!_pending || !reply.ack || !reply.snapshot.empty() ||
        !(reply.source == _peer) ||
        reply.syn != static_cast<uint16_t>(_pending->syn + 1))
        return false;
    // Node 快照遍历顺序固定，等待确认期间相互抵消的变化无需再次发送。
    _dirty = snapshot.size() != _pending->snapshot.size() ||
             !std::equal(snapshot.begin(), snapshot.end(), _pending->snapshot.begin(),
                         sameREDPEndpoint);
    _pending.reset();
    return true;
}

std::optional<REDPReceiver::Result> REDPReceiver::accept(
    const REDPMessage &request, Guid local) {
    if (request.ack || !(request.source == _peer))
        return std::nullopt;
    bool applied = !_initialized;
    if (_initialized) {
        if (request.syn == _last_syn) {
            if (request.snapshot.size() != _snapshot.size())
                return std::nullopt;
            for (size_t i = 0; i < _snapshot.size(); ++i) {
                const auto &a = request.snapshot[i], &b = _snapshot[i];
                if (a.guid.full != b.guid.full || a.port != b.port ||
                    a.topic != b.topic || a.msgtype != b.msgtype)
                    return std::nullopt;
            }
        } else if (request.syn == static_cast<uint16_t>(_last_syn + 1)) {
            applied = true;
        } else {
            return std::nullopt;
        }
    }
    if (applied) {
        _snapshot = request.snapshot;
        _last_syn = request.syn;
        _initialized = true;
    }
    return Result{applied, REDPMessage{local,
        static_cast<uint16_t>(request.syn + 1), true, {}}};
}

bool sendREDPMessage(DgramSocket &socket, Locator locator, const REDPMessage &message) {
    if (locator.invalid())
        return false;
    auto data = message.serialize();
    if (data.empty())
        return false;
    // 使用 RNDP 公告的控制端口发送
    return socket.write(locator.addr, Endpoint(ip::udp::v4(), locator.port), data);
}

////////////////////////////////////////////////////////////////////////////////

Locator selectBestLocator(std::array<uint8_t, 4> target_ip, const RNDPMessage &rndp_msg) {
    for (const auto &loc : rndp_msg.locators)
        if (loc.addr == target_ip)
            return loc;
    return {};
}

} // namespace rm::lpss

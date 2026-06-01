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

#include <algorithm>
#include <cstring>
#include <limits>
#include <mutex>
#include <optional>
#include <vector>

#include "rmvl/core/util.hpp"
#include "rmvl/lpss/details/node_rmtp.hpp"
#include "rmvlpara/lpss.hpp"

namespace rm::lpss {

namespace {

constexpr std::size_t MTP_FIXED_HEADER_SIZE = 16;
constexpr std::size_t MAX_UDP_PAYLOAD = 65507;
constexpr std::size_t IPV4_UDP_HEADER_SIZE = 20 + 8;
constexpr uint32_t DEFAULT_MTU = 1500;
constexpr uint8_t NAME_SIZE_MASK = 0x3f;
constexpr std::string_view MTP_SHM_NOTIFY = "MSHM";

using AssemblyMap = std::unordered_map<MTPAsmKey, MTPAsm, MTPAsmKeyHash>;

//! 计算 MTP 消息头部大小
std::size_t mtp_header_size(std::string_view type, std::string_view topic) noexcept { return MTP_FIXED_HEADER_SIZE + topic.size() + type.size(); }
//! 判断两个 GUID 是否属于同一主机
bool same_host(const Guid &lhs, const Guid &rhs) noexcept { return lhs.host() == rhs.host(); }
//! 判断两个 GUID 是否属于同一节点
bool same_node(const Guid &lhs, const Guid &rhs) noexcept { return lhs.host() == rhs.host() && lhs.pid() == rhs.pid(); }

template <typename Map>
void erase_endpoint_or_node(Map &map, const Guid &guid) {
    if (map.erase(guid) != 0)
        return;
    for (auto it = map.begin(); it != map.end();)
        same_node(it->first, guid) ? it = map.erase(it) : ++it;
}

std::string shm_channel_name(const Guid &writer, const Guid &reader) {
    return "lpss_mtp_" + std::to_string(writer.full) + "_" + std::to_string(reader.full);
}

std::shared_ptr<LatestBytesSHM> create_shm_channel(std::string_view name) {
    return std::make_shared<LatestBytesSHM>(name, static_cast<std::size_t>(para::lpss_param.MTP_REASSEMBLY_MAX_BYTES));
}

uint8_t prefix_length(const std::array<uint8_t, 4> &mask) noexcept {
    uint8_t count{};
    for (uint8_t byte : mask)
        while (byte != 0) {
            count += static_cast<uint8_t>(byte & 1U);
            byte >>= 1;
        }
    return count;
}

bool same_subnet(const std::array<uint8_t, 4> &target, const ip::Networkv4 &network) noexcept {
    auto local = network.address();
    auto mask = network.netmask();
    for (std::size_t i = 0; i < target.size(); ++i)
        if ((target[i] & mask[i]) != (local[i] & mask[i]))
            return false;
    return true;
}

/**
 * @brief 获取出站数据报的 MTU
 *
 * @param[in] locator 目标定位器
 * @return 将向目标定位器发送数据的本地接口 MTU
 */
uint32_t outbound_mtu(const Locator &locator) noexcept {
    uint32_t fallback{};
    uint32_t selected{};
    uint8_t best_prefix{};
    bool matched{};

    for (const auto &iface : NetworkInterface::list()) {
        if (!iface.up() || !iface.running() || iface.mtu() == 0)
            continue;
        auto networks = iface.ipv4();
        if (networks.empty())
            continue;
        fallback = fallback == 0 ? iface.mtu() : std::min(fallback, iface.mtu());
        for (const auto &network : networks) {
            if (!same_subnet(locator.addr, network))
                continue;
            auto current_prefix = prefix_length(network.netmask());
            if (!matched || current_prefix > best_prefix || (current_prefix == best_prefix && iface.mtu() < selected)) {
                matched = true;
                best_prefix = current_prefix;
                selected = iface.mtu();
            }
        }
    }

    if (matched)
        return selected;
    if (fallback != 0) {
        WARNING_("[LPSS MTP] No outbound subnet matches a reader locator; falling back to MTU %u", fallback);
        return fallback;
    }
    WARNING_("[LPSS MTP] No valid local IPv4 interface MTU found; falling back to MTU %u", DEFAULT_MTU);
    return DEFAULT_MTU;
}

/**
 * @brief 计算单个数据报能够承载的最大有效载荷大小
 *
 * @param[in] mtu 数据报发送接口的 MTU
 * @param[in] header_size MTP 头部大小
 * @return 最大 MTP 载荷大小，如果无法发送任何数据，则返回 0
 */
std::size_t fragment_capacity(uint32_t mtu, std::size_t header_size) noexcept {
    if (mtu <= IPV4_UDP_HEADER_SIZE)
        return 0;
    auto datagram_capacity = std::min<std::size_t>(mtu - IPV4_UDP_HEADER_SIZE, MAX_UDP_PAYLOAD);
    return datagram_capacity > header_size ? datagram_capacity - header_size : 0;
}

void append_net_u16(std::string &data, uint16_t value) {
    value = htons(value);
    data.append(reinterpret_cast<const char *>(&value), sizeof(value));
}

void append_net_u32(std::string &data, uint32_t value) {
    value = htonl(value);
    data.append(reinterpret_cast<const char *>(&value), sizeof(value));
}

//! 可复用的 MTP 消息头部
struct MTPHeader {
    std::string data;
    std::size_t fragment_id_offset{};
    std::size_t payload_size_offset{};

    static MTPHeader create(std::string_view type, std::string_view topic, uint16_t sequence, uint32_t all_payload_size) {
        MTPHeader header{};
        auto &result = header.data;
        result.reserve(mtp_header_size(type, topic));
        result.append("MT02");
        uint8_t topic_size = static_cast<uint8_t>(topic.size()) & NAME_SIZE_MASK;
        result.append(reinterpret_cast<const char *>(&topic_size), sizeof(topic_size));
        result.append(topic);
        uint8_t type_size = static_cast<uint8_t>(type.size()) & NAME_SIZE_MASK;
        result.append(reinterpret_cast<const char *>(&type_size), sizeof(type_size));
        result.append(type);
        append_net_u16(result, sequence);
        header.fragment_id_offset = result.size();
        append_net_u16(result, 0);
        append_net_u32(result, all_payload_size);
        header.payload_size_offset = result.size();
        append_net_u16(result, 0);
        return header;
    }

    void set(uint16_t fragment_id, uint16_t payload_size) noexcept {
        set_net_u16(data, fragment_id_offset, fragment_id);
        set_net_u16(data, payload_size_offset, payload_size);
    }

private:
    void set_net_u16(std::string &data, std::size_t offset, uint16_t value) noexcept {
        value = htons(value);
        std::memcpy(data.data() + offset, &value, sizeof(value));
    }
};

//! 解析的 MTP 分片信息
struct ParsedFragment {
    uint16_t sequence{};    //!< 消息序列号
    uint16_t fragment_id{}; //!< 分片 ID
    uint32_t total_size{};  //!< 载荷总大小
};

/**
 * @brief 解析 MTP 消息头部并验证有效性
 *
 * @param[in] header MTP 消息头部字符串
 * @param[in] payload MTP 消息载荷字符串
 * @param[in] type 消息类型字符串
 * @param[in] topic 消息话题字符串
 * @return 解析结果，包含序列号、分片 ID 和载荷总大小，如果解析失败或验证不通过，则返回 std::nullopt
 */
std::optional<ParsedFragment> parse_header(std::string_view header, std::string_view payload, std::string_view type, std::string_view topic) noexcept {
    if (header.size() != mtp_header_size(type, topic) || header.size() < MTP_FIXED_HEADER_SIZE || header.substr(0, 4) != "MT02")
        return std::nullopt;

    std::size_t offset = 4;
    auto topic_size = static_cast<uint8_t>(header[offset++]) & NAME_SIZE_MASK;
    if (static_cast<std::size_t>(topic_size) != topic.size() || header.substr(offset, topic_size) != topic)
        return std::nullopt;
    offset += topic_size;
    auto type_size = static_cast<uint8_t>(header[offset++]) & NAME_SIZE_MASK;
    if (static_cast<std::size_t>(type_size) != type.size() || header.substr(offset, type_size) != type)
        return std::nullopt;
    offset += type_size;
    if (offset + sizeof(uint16_t) * 3 + sizeof(uint32_t) != header.size())
        return std::nullopt;

    uint16_t net_sequence{}, net_fragment{}, net_fragment_size{};
    uint32_t net_total_size{};
    std::memcpy(&net_sequence, header.data() + offset, sizeof(net_sequence));
    offset += sizeof(net_sequence);
    std::memcpy(&net_fragment, header.data() + offset, sizeof(net_fragment));
    offset += sizeof(net_fragment);
    std::memcpy(&net_total_size, header.data() + offset, sizeof(net_total_size));
    offset += sizeof(net_total_size);
    std::memcpy(&net_fragment_size, header.data() + offset, sizeof(net_fragment_size));

    ParsedFragment result{ntohs(net_sequence), ntohs(net_fragment), ntohl(net_total_size)};
    auto fragment_size = ntohs(net_fragment_size);
    if (payload.size() != fragment_size || fragment_size > result.total_size)
        return std::nullopt;
    if ((result.total_size == 0 && (result.fragment_id != 0 || fragment_size != 0)) ||
        (result.total_size != 0 && fragment_size == 0))
        return std::nullopt;
    return result;
}

/**
 * @brief 移除指定的分片记录
 *
 * @param[in] asms 当前的分片记录集合
 * @param[in] asm_bytes 当前分片组装记录占用的总字节数
 * @param[in] it 指向要移除的分片记录的迭代器
 * @return 移除后指向的迭代器，适合用于 `for` 循环中继续迭代
 */
AssemblyMap::iterator removeAsm(AssemblyMap &asms, std::size_t &asm_bytes, AssemblyMap::iterator it) noexcept {
    asm_bytes -= it->second.bytes;
    return asms.erase(it);
}

/**
 * @brief 从集合中移除已过期的分片记录
 *
 * @param[in] asms 当前的分片记录集合
 * @param[in] asm_bytes 当前分片组装记录占用的总字节数
 * @param[in] now 当前时间
 */
void removeExpired(AssemblyMap &asms, std::size_t &asm_bytes, std::chrono::steady_clock::time_point now) noexcept {
    auto timeout = std::chrono::milliseconds(para::lpss_param.MTP_FRAGMENT_TIMEOUT);
    for (auto it = asms.begin(); it != asms.end();)
        if (now - it->second.updated > timeout)
            it = removeAsm(asms, asm_bytes, it);
        else
            ++it;
}

/**
 * @brief 为新增分片预留内存，必要时通过移除旧的分片记录来释放空间
 *
 * @param[in,out] asms 当前的分片记录集合
 * @param[in,out] asm_bytes 当前分片组装记录占用的总字节数
 * @param[in] kept 在预留内存过程中需要保留的分片记录的键，通常是当前正在处理的分片所属的分片组装记录的键
 * @param[in] amount 需要预留的字节数
 * @return 是否成功预留到足够的内存
 */
bool reserve4Memory(AssemblyMap &asms, std::size_t &asm_bytes, const MTPAsmKey &kept, std::size_t amount) noexcept {
    auto budget = static_cast<std::size_t>(para::lpss_param.MTP_REASSEMBLY_MAX_BYTES);
    while (asm_bytes + amount > budget) {
        auto oldest = asms.end();
        for (auto it = asms.begin(); it != asms.end(); ++it) {
            if (it->first == kept)
                continue;
            if (oldest == asms.end() || it->second.updated < oldest->second.updated)
                oldest = it;
        }
        if (oldest == asms.end())
            return false;
        removeAsm(asms, asm_bytes, oldest);
    }
    return true;
}

std::optional<std::string> accept_fragment(std::string_view header, std::string_view payload, std::string_view addr, uint16_t port,
                                           std::string_view type, std::string_view topic, AssemblyMap &asms, std::size_t &asm_bytes) {
    auto now = std::chrono::steady_clock::now();
    removeExpired(asms, asm_bytes, now);

    auto parsed = parse_header(header, payload, type, topic);
    if (!parsed || parsed->total_size > para::lpss_param.MTP_REASSEMBLY_MAX_BYTES)
        return std::nullopt;

    MTPAsmKey key{std::string(addr), port, parsed->sequence};
    auto [it, inserted] = asms.try_emplace(key, MTPAsm{parsed->total_size, {}, 0, now});
    auto &assembly = it->second;
    if (!inserted && assembly.total_size != parsed->total_size) {
        removeAsm(asms, asm_bytes, it);
        return std::nullopt;
    }
    assembly.updated = now;

    auto fragment = assembly.fragments.find(parsed->fragment_id);
    if (fragment != assembly.fragments.end()) {
        if (fragment->second != payload) {
            removeAsm(asms, asm_bytes, it);
            return std::nullopt;
        }
    } else {
        if (assembly.bytes + payload.size() > assembly.total_size ||
            !reserve4Memory(asms, asm_bytes, key, payload.size())) {
            removeAsm(asms, asm_bytes, it);
            return std::nullopt;
        }
        assembly.fragments.emplace(parsed->fragment_id, payload);
        assembly.bytes += payload.size();
        asm_bytes += payload.size();
    }

    std::size_t expected_fragment{};
    std::size_t total{};
    for (const auto &[fragment_id, part] : assembly.fragments) {
        if (fragment_id != expected_fragment++)
            return std::nullopt;
        total += part.size();
        if (total > assembly.total_size) {
            removeAsm(asms, asm_bytes, it);
            return std::nullopt;
        }
    }
    if (total != assembly.total_size)
        return std::nullopt;

    std::string result{};
    result.reserve(total);
    for (const auto &[fragment_id, part] : assembly.fragments)
        result.append(part);
    removeAsm(asms, asm_bytes, it);
    return result;
}

std::optional<std::string> read_shm_sources(std::unordered_map<Guid, MTPShmSource, GuidHash> &sources) {
    for (auto &[guid, source] : sources) {
        std::string data{};
        if (source.shm && source.shm->read(data, source.sequence))
            return data;
    }
    return std::nullopt;
}

template <typename SendCallback>
void sendMTPMessage(std::string_view data, std::string_view type, std::string_view topic, uint16_t sequence,
                    const std::vector<MTPWriterTarget> &targets, SendCallback &&send) {
    if (topic.size() > NAME_SIZE_MASK || type.size() > NAME_SIZE_MASK || data.size() > std::numeric_limits<uint32_t>::max()) {
        WARNING_("[LPSS MTP] Message or endpoint metadata exceeds MTP limits");
        return;
    }
    const auto header_size = mtp_header_size(type, topic);
    // 检验分片次数有无超过 uint16_t 上限
    for (const auto &target : targets) {
        auto capacity = fragment_capacity(target.mtu, header_size);
        if (capacity == 0) {
            WARNING_("[LPSS MTP] Message cannot be fragmented within target MTU and MTP limits");
            return;
        }
        auto fragment_count = data.empty() ? 1 : 1 + (data.size() - 1) / capacity;
        if (fragment_count > static_cast<std::size_t>(std::numeric_limits<uint16_t>::max()) + 1) {
            WARNING_("[LPSS MTP] Message cannot be fragmented within target MTU and MTP limits");
            return;
        }
    }

    auto total_size = static_cast<uint32_t>(data.size());
    auto header = MTPHeader::create(type, topic, sequence, total_size);
    for (const auto &target : targets) {
        std::size_t capacity = fragment_capacity(target.mtu, header_size);
        std::size_t fragment_count = data.empty() ? 1 : 1 + (data.size() - 1) / capacity;
        std::size_t offset{};
        for (std::size_t id = 0; id < fragment_count; ++id) {
            if (id > 0 && id % 10 == 0)
                std::this_thread::sleep_for(std::chrono::microseconds(2));
            auto size = std::min(capacity, data.size() - offset);
            header.set(static_cast<uint16_t>(id), static_cast<uint16_t>(size));
            send(target.locator, header.data, data.substr(offset, size));
            offset += size;
        }
    }
}

} // namespace

DataWriterBase::DataWriterBase(const Guid &guid, std::string_view type, std::string_view topic)
    : _guid(guid), _socket(Sender(ip::udp::v4()).create()), _type(type), _topic(topic) {}

void DataWriterBase::add(const Guid &guid, Locator loc) noexcept {
    std::lock_guard lk(_mtx);
    if (same_host(_guid, guid)) {
        auto name = shm_channel_name(_guid, guid);
        _shm_targets[guid] = {name, loc, create_shm_channel(name)};
        _udpv4_targets.erase(guid);
    } else {
        _udpv4_targets[guid] = {loc, outbound_mtu(loc)};
        _shm_targets.erase(guid);
    }
}

void DataWriterBase::remove(const Guid &guid) noexcept {
    std::lock_guard lk(_mtx);
    erase_endpoint_or_node(_udpv4_targets, guid);
    erase_endpoint_or_node(_shm_targets, guid);
}

void DataWriterBase::write(std::string data) noexcept {
    std::vector<MTPWriterTarget> targets{};
    std::vector<MTPShmTarget> shm_targets{};
    {
        std::shared_lock lk(_mtx);
        targets.reserve(_udpv4_targets.size());
        for (const auto &[guid, target] : _udpv4_targets)
            targets.push_back(target);
        shm_targets.reserve(_shm_targets.size());
        for (const auto &[guid, target] : _shm_targets)
            shm_targets.push_back(target);
    }

    for (const auto &target : shm_targets) {
        if (!target.shm || !target.shm->write(data)) {
            WARNING_("[LPSS MTP] Failed to write an MTP SHM message");
            continue;
        }
        _socket.write(target.locator.addr, Endpoint(ip::udp::v4(), target.locator.port), MTP_SHM_NOTIFY);
    }

    auto sequence = _sequence.fetch_add(1, std::memory_order_relaxed);
    sendMTPMessage(data, _type, _topic, sequence, targets, [this](const Locator &loc, std::string_view header, std::string_view payload) {
        if (!_socket.multiwrite(loc.addr, Endpoint(ip::udp::v4(), loc.port), header, payload))
            WARNING_("[LPSS MTP] Failed to send an MTP UDP fragment");
    });
}

DataReaderBase::DataReaderBase(const Guid &guid, std::string_view type, std::string_view topic)
    : _guid(guid), _udpv4(Listener(Endpoint(ip::udp::v4(), Endpoint::ANY_PORT)).create()), _type(type), _topic(topic) {
    auto ep = _udpv4.endpoint();
    _port = ep.port();
}

void DataReaderBase::add(const Guid &guid) noexcept {
    if (!same_host(_guid, guid))
        return;
    std::lock_guard lk(_shm_mtx);
    auto name = shm_channel_name(guid, _guid);
    _shm_sources[guid] = {name, create_shm_channel(name), 0};
}

void DataReaderBase::remove(const Guid &guid) noexcept {
    std::lock_guard lk(_shm_mtx);
    erase_endpoint_or_node(_shm_sources, guid);
}

std::string DataReaderBase::read() noexcept {
    while (true) {
        {
            std::lock_guard lk(_shm_mtx);
            auto message = read_shm_sources(_shm_sources);
            if (message)
                return std::move(*message);
        }

        auto header_size = mtp_header_size(_type, _topic);
        auto [parts, addr, port] = _udpv4.multiread(header_size, MAX_UDP_PAYLOAD - header_size);
        {
            std::lock_guard lk(_shm_mtx);
            auto message = read_shm_sources(_shm_sources);
            if (message)
                return std::move(*message);
        }
        if (parts.size() != 2)
            continue;
        auto message = accept_fragment(parts[0], parts[1], addr, port, _type, _topic, _asms, _asm_bytes);
        if (message)
            return std::move(*message);
    }
}

#if __cplusplus >= 202002L

namespace async {

DataWriterBase::DataWriterBase(rm::async::IOContext &io_context, const Guid &guid, std::string_view type, std::string_view topic)
    : _guid(guid), _socket(rm::async::Sender(io_context, ip::udp::v4()).create()), _type(type), _topic(topic) {}

void DataWriterBase::add(const Guid &guid, Locator loc) noexcept {
    if (same_host(_guid, guid)) {
        auto name = shm_channel_name(_guid, guid);
        _shm_targets[guid] = {name, loc, create_shm_channel(name)};
        _udpv4_targets.erase(guid);
    } else {
        _udpv4_targets[guid] = {loc, outbound_mtu(loc)};
        _shm_targets.erase(guid);
    }
}

void DataWriterBase::remove(const Guid &guid) noexcept {
    erase_endpoint_or_node(_udpv4_targets, guid);
    erase_endpoint_or_node(_shm_targets, guid);
}

rm::async::Task<> DataWriterBase::write(std::string data) noexcept {
    if (_sending) {
        _pending = std::move(data);
        co_return;
    }
    _sending = true;

    std::string current = std::move(data);
    while (true) {
        if (_topic.size() > NAME_SIZE_MASK || _type.size() > NAME_SIZE_MASK || current.size() > std::numeric_limits<uint32_t>::max()) {
            WARNING_("[LPSS MTP] Message or endpoint metadata exceeds MTP limits");
        } else {
            std::vector<MTPWriterTarget> targets{};
            targets.reserve(_udpv4_targets.size());
            for (const auto &[guid, target] : _udpv4_targets)
                targets.push_back(target);
            std::vector<MTPShmTarget> shm_targets{};
            shm_targets.reserve(_shm_targets.size());
            for (const auto &[guid, target] : _shm_targets)
                shm_targets.push_back(target);

            auto sequence = _sequence.fetch_add(1, std::memory_order_relaxed);
            auto header_size = mtp_header_size(_type, _topic);
            std::string_view data_view = current;
            bool can_send = true;
            for (const auto &target : targets) {
                auto capacity = fragment_capacity(target.mtu, header_size);
                if (capacity == 0) {
                    WARNING_("[LPSS MTP] Message cannot be fragmented within target MTU and MTP limits");
                    can_send = false;
                    break;
                }
                std::size_t fragment_count = current.empty() ? 1 : 1 + (current.size() - 1) / capacity;
                if (fragment_count > static_cast<std::size_t>(std::numeric_limits<uint16_t>::max()) + 1) {
                    WARNING_("[LPSS MTP] Message cannot be fragmented within target MTU and MTP limits");
                    can_send = false;
                    break;
                }
            }

            if (can_send) {
                for (const auto &target : shm_targets) {
                    if (!target.shm || !target.shm->write(current)) {
                        WARNING_("[LPSS MTP] Failed to write an MTP SHM message");
                        continue;
                    }
                    co_await _socket.write(target.locator.addr, Endpoint(ip::udp::v4(), target.locator.port), MTP_SHM_NOTIFY);
                }

                auto total_size = static_cast<uint32_t>(current.size());
                auto header = MTPHeader::create(_type, _topic, sequence, total_size);
                for (const auto &target : targets) {
                    auto capacity = fragment_capacity(target.mtu, header_size);
                    std::size_t fragment_count = current.empty() ? 1 : 1 + (current.size() - 1) / capacity;
                    std::size_t offset{};
                    for (std::size_t id = 0; id < fragment_count; ++id) {
                        if (id > 0 && id % 10 == 0)
                            std::this_thread::sleep_for(std::chrono::microseconds(2));
                        auto size = std::min(capacity, current.size() - offset);
                        header.set(static_cast<uint16_t>(id), static_cast<uint16_t>(size));
                        if (!(co_await _socket.multiwrite(target.locator.addr, Endpoint(ip::udp::v4(), target.locator.port), header.data, data_view.substr(offset, size))))
                            WARNING_("[LPSS MTP] Failed to send an MTP UDP fragment");
                        offset += size;
                    }
                }
            }
        }

        if (!_pending) {
            _sending = false;
            co_return;
        }
        current = std::move(*_pending);
        _pending.reset();
    }
}

DataReaderBase::DataReaderBase(rm::async::IOContext &io_context, const Guid &guid, std::string_view type, std::string_view topic)
    : _guid(guid), _udpv4(rm::async::Listener(io_context, Endpoint(ip::udp::v4(), Endpoint::ANY_PORT)).create()), _type(type), _topic(topic) {
    auto ep = _udpv4.endpoint();
    _port = ep.port();
}

void DataReaderBase::add(const Guid &guid) noexcept {
    if (!same_host(_guid, guid))
        return;
    auto name = shm_channel_name(guid, _guid);
    _shm_sources[guid] = {name, create_shm_channel(name), 0};
}

void DataReaderBase::remove(const Guid &guid) noexcept { erase_endpoint_or_node(_shm_sources, guid); }

rm::async::Task<std::string> DataReaderBase::read() noexcept {
    while (true) {
        auto shm_message = read_shm_sources(_shm_sources);
        if (shm_message) {
            std::string result = std::move(*shm_message);
            co_return result;
        }

        auto header_size = mtp_header_size(_type, _topic);
        auto [parts, addr, port] = co_await _udpv4.multiread(header_size, MAX_UDP_PAYLOAD - header_size);
        shm_message = read_shm_sources(_shm_sources);
        if (shm_message) {
            std::string result = std::move(*shm_message);
            co_return result;
        }
        if (parts.size() != 2)
            continue;
        auto message = accept_fragment(parts[0], parts[1], addr, port, _type, _topic, _asms, _asm_bytes);
        if (message) {
            std::string result = std::move(*message);
            co_return result;
        }
    }
}

} // namespace async

#endif

} // namespace rm::lpss

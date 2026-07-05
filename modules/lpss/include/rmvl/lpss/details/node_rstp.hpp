/**
 * @file node_rstp.hpp
 * @author zhaoxi (535394140@qq.com)
 * @brief LPSS 服务传输协议（Service Transport Protocol）
 * @version 1.0
 * @date 2026-07-05
 *
 * @copyright Copyright 2026 (c), zhaoxi
 */

#pragma once

#include <cstring>
#include <optional>

#include "node_util.hpp"

//! @cond

namespace rm::lpss {

namespace stp {

//! STP 请求标识，由客户端端点 GUID 与单调递增序列号共同标识一次调用
struct Header {
    Guid client_guid{};
    uint16_t sequence{};
};

constexpr std::size_t HEADER_SIZE = sizeof(Guid) + sizeof(uint16_t);

//! STP 解包结果
template <typename MsgType>
struct Datapack {
    Header header{};
    MsgType message{};
};

//! 将 STP 头和业务消息封装为 MTP 载荷
template <typename MsgType>
std::string pack(const stp::Header &header, const MsgType &message) {
    auto payload = message.serialize();
    std::string result(HEADER_SIZE, '\0');
    std::memcpy(result.data(), &header.client_guid.full, sizeof(header.client_guid.full));
    std::memcpy(result.data() + sizeof(header.client_guid.full), &header.sequence, sizeof(header.sequence));
    result.append(payload);
    return result;
}

//! 从 MTP 载荷中解析 STP 头和业务消息
template <typename MsgType>
std::optional<stp::Datapack<MsgType>> unpack(std::string_view data) {
    if (data.size() < HEADER_SIZE)
        return std::nullopt;
    stp::Datapack<MsgType> result{};
    std::memcpy(&result.header.client_guid.full, data.data(), sizeof(result.header.client_guid.full));
    std::memcpy(&result.header.sequence, data.data() + sizeof(result.header.client_guid.full), sizeof(result.header.sequence));
    result.message = MsgType::deserialize(data.data() + HEADER_SIZE);
    return result;
}

} // namespace stp

namespace srv2topic {

//! 获取服务请求对应的内部话题名称
inline std::string request(std::string_view service) { return service.empty() || service.front() != '/' ? "lq/" + std::string(service) : "lq" + std::string(service); }

//! 获取服务响应对应的内部话题名称
inline std::string response(std::string_view service) { return service.empty() || service.front() != '/' ? "lr/" + std::string(service) : "lr" + std::string(service); }

} // namespace srv2topic

} // namespace rm::lpss

//! @endcond

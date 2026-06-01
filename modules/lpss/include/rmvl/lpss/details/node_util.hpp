/**
 * @file node_util.hpp
 * @author zhaoxi (535394140@qq.com)
 * @brief 
 * @version 1.0
 * @date 2026-05-27
 * 
 * @copyright Copyright 2026 (c), zhaoxi
 * 
 */

#pragma once

#include <cstdint>

#include "rmvl/io/socket.hpp"

namespace rm::lpss {

//! @cond

//! GUID
class Guid {
public:
    uint64_t full{};

    //! 获取 Host LowMAC（高 32 位）
    uint32_t host() const noexcept { return full >> 32; }
    //! 获取 Process ID（中 16 位）
    uint16_t pid() const noexcept { return (full >> 16) & 0xffff; }
    //! 获取 Entity ID（低 16 位）
    uint16_t entity() const noexcept { return full & 0xffff; }

    /**
     * @brief 设置 Host LowMAC（高 32 位）
     * 
     * @param[in] h LowMAC 地址，通常取自网络接口的 MAC 地址的低 4 字节
     */
    void set_host(uint32_t h) { full = (static_cast<uint64_t>(h) << 32) | (full & 0xffffffff); }
    
    /**
     * @brief 设置 Process ID（中 16 位）
     * 
     * @param[in] p 进程 ID
     */
    void set_pid(uint16_t p) { full = (full & 0xffffffff0000ffffULL) | (static_cast<uint64_t>(p) << 16); }

    /**
     * @brief 设置 Entity ID（低 16 位）
     * 
     * @param[in] e Entity ID
     */
    void set_entity(uint16_t e) { full = (full & 0xffffffffffff0000ULL) | e; }

    // 构造函数
    Guid() = default;
    Guid(uint64_t val) : full(val) {}
    Guid(uint32_t h, uint16_t p, uint16_t e) { full = (static_cast<uint64_t>(h) << 32) | (static_cast<uint64_t>(p) << 16) | e; }

    // 运算符重载
    inline bool operator==(const Guid &oth) const { return full == oth.full; }
};

//! GUID 哈希函数对象
struct GuidHash {
    inline std::size_t operator()(const rm::lpss::Guid &guid) const noexcept { return static_cast<std::size_t>(guid.full); }
};

constexpr std::size_t RNDP_HEADER_SIZE = 4 + sizeof(Guid) + 2;
constexpr std::size_t REDP_HEADER_SIZE = 4 + sizeof(Guid) + 1;
constexpr auto BROADCAST_IP = "239.255.0.5";

//! 定位器
struct Locator {
    uint16_t port{};               //!< 端口号
    std::array<uint8_t, 4> addr{}; //!< 地址

    bool invalid() const noexcept { return port == 0 && addr == std::array<uint8_t, 4>{}; }
};

//! 生成 GUID 和本地 IPv4 地址列表
std::pair<Guid, std::vector<ip::Networkv4>> generateNodeGuid();

//! @endcond

} // namespace rm::lpss
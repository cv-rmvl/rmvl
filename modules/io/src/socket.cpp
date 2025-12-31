/**
 * @file socket.cpp
 * @author zhaoxi (535394140@qq.com)
 * @brief 基于 socket 的异步 IO 传输层、会话层通信框架实现
 * @version 1.0
 * @date 2025-07-31
 *
 * @copyright Copyright 2025 (c), zhaoxi
 *
 */

#include "rmvl/io/socket.hpp"
#include "rmvl/core/util.hpp"

#ifdef _WIN32
#include <MSWSock.h>
#include <WS2tcpip.h>
#include <afunix.h>
#include <iphlpapi.h>
#pragma comment(lib, "iphlpapi.lib")
#pragma comment(lib, "Ws2_32.lib")
#else
#include <arpa/inet.h>
#include <fcntl.h>
#include <ifaddrs.h>
#include <net/if.h>
#include <netpacket/packet.h>
#include <sys/un.h>
#include <unistd.h>
#endif

#if __cplusplus >= 202002L

#ifndef _WIN32
#include <sys/epoll.h>
#endif

#endif

namespace rm {

inline static int error_code() {
#ifdef _WIN32
    return WSAGetLastError();
#else
    return errno;
#endif
}

#ifdef __MSVC__
#pragma region Network Interface
#endif

#ifdef _WIN32

std::vector<NetworkInterface> NetworkInterface::list() noexcept {
    SocketEnv::ensure_init();

    std::vector<NetworkInterface> res{};
    ULONG family = AF_UNSPEC;
    ULONG flags = GAA_FLAG_INCLUDE_PREFIX;
    ULONG size{};
    if (GetAdaptersAddresses(family, flags, nullptr, nullptr, &size) != ERROR_BUFFER_OVERFLOW)
        return res;

    std::vector<char> buffer(size);
    auto *adapters = reinterpret_cast<IP_ADAPTER_ADDRESSES *>(buffer.data());

    if (GetAdaptersAddresses(family, flags, nullptr, adapters, &size) != NO_ERROR)
        return res;

    for (PIP_ADAPTER_ADDRESSES adapter = adapters; adapter; adapter = adapter->Next) {
        // 物理地址长度不为 6 (MAC地址) 则跳过
        if (adapter->PhysicalAddressLength != 6)
            continue;

        NetworkInterface iface{};
        iface._name = adapter->AdapterName;
        std::copy(adapter->PhysicalAddress, adapter->PhysicalAddress + 6, iface._addr.begin());

        // 设置 flag
        if (adapter->OperStatus == IfOperStatusUp)
            iface._flag |= NetworkInterfaceFlag::Up | NetworkInterfaceFlag::Running;
        if (adapter->IfType == IF_TYPE_SOFTWARE_LOOPBACK)
            iface._flag |= NetworkInterfaceFlag::Loopback;
        if (!(adapter->Flags & IP_ADAPTER_NO_MULTICAST))
            iface._flag |= NetworkInterfaceFlag::Multicast | NetworkInterfaceFlag::Broadcast;
        if (adapter->IfType == IF_TYPE_PPP || adapter->IfType == IF_TYPE_TUNNEL)
            iface._flag |= NetworkInterfaceFlag::P2P;

        // 设置 type
        switch (adapter->IfType) {
        case IF_TYPE_ETHERNET_CSMACD:
            iface._type = NetworkInterfaceType::Ethernet;
            break;
        case IF_TYPE_SOFTWARE_LOOPBACK:
            iface._type = NetworkInterfaceType::Loopback;
            break;
        case IF_TYPE_IEEE80211:
            iface._type = NetworkInterfaceType::Wireless;
            break;
        case IF_TYPE_PPP:
            iface._type = NetworkInterfaceType::PPP;
            break;
        case IF_TYPE_TUNNEL:
            iface._type = NetworkInterfaceType::Tunnel;
            break;
        default:
            iface._type = NetworkInterfaceType::Other;
            break;
        }
        res.push_back(std::move(iface));
    }

    return res;
}

template <int Family>
static auto getIp(std::string_view if_name) noexcept {
    using NetworkType = std::conditional_t<Family == AF_INET, ip::Networkv4, ip::Networkv6>;
    std::vector<NetworkType> res{};

    ULONG family = AF_UNSPEC;
    ULONG flags = GAA_FLAG_INCLUDE_PREFIX;
    ULONG size{};
    if (GetAdaptersAddresses(family, flags, nullptr, nullptr, &size) != ERROR_BUFFER_OVERFLOW)
        return res;

    std::vector<char> buffer(size);
    auto *adapters = reinterpret_cast<IP_ADAPTER_ADDRESSES *>(buffer.data());

    if (GetAdaptersAddresses(family, flags, nullptr, adapters, &size) != NO_ERROR)
        return res;

    for (PIP_ADAPTER_ADDRESSES adapter = adapters; adapter; adapter = adapter->Next) {
        if (if_name != adapter->AdapterName)
            continue;
        for (auto *addr = adapter->FirstUnicastAddress; addr; addr = addr->Next) {
            if constexpr (Family == AF_INET) {
                if (addr->Address.lpSockaddr->sa_family == AF_INET) {
                    auto *addr_v4 = reinterpret_cast<sockaddr_in *>(addr->Address.lpSockaddr);
                    std::array<uint8_t, 4> ip_bytes{};
                    memcpy(ip_bytes.data(), &addr_v4->sin_addr.s_addr, 4);

                    std::array<uint8_t, 4> mask_bytes{};
                    ULONG prefix_length = addr->OnLinkPrefixLength;
                    for (ULONG i = 0; i < 4; ++i) {
                        if (prefix_length >= 8) {
                            mask_bytes[i] = 0xFF;
                            prefix_length -= 8;
                        } else {
                            mask_bytes[i] = static_cast<uint8_t>(0xFF << (8 - prefix_length));
                            prefix_length = 0;
                        }
                    }
                    res.emplace_back(ip_bytes, mask_bytes);
                }
            } else if constexpr (Family == AF_INET6) {
                if (addr->Address.lpSockaddr->sa_family == AF_INET6) {
                    auto *addr_v6 = reinterpret_cast<sockaddr_in6 *>(addr->Address.lpSockaddr);
                    std::array<uint8_t, 16> ip_bytes{};
                    memcpy(ip_bytes.data(), &addr_v6->sin6_addr.s6_addr, 16);
                    res.emplace_back(ip_bytes);
                }
            }
        }
        break;
    }
    return res;
}

#else

std::vector<NetworkInterface> NetworkInterface::list() noexcept {
    std::unordered_map<std::string, NetworkInterface> interfaces;
    ifaddrs *ifaddr_list{};
    if (getifaddrs(&ifaddr_list) == -1)
        return {};

    for (auto *ifa = ifaddr_list; ifa != nullptr; ifa = ifa->ifa_next) {
        if (ifa->ifa_addr == nullptr)
            continue;

        // 使用接口名称作为 key，确保每个接口只处理一次
        auto &iface = interfaces[ifa->ifa_name];
        if (iface._name.empty()) {
            iface._name = ifa->ifa_name;
            // 设置 flag
            if (ifa->ifa_flags & IFF_UP)
                iface._flag |= NetworkInterfaceFlag::Up;
            if (ifa->ifa_flags & IFF_BROADCAST)
                iface._flag |= NetworkInterfaceFlag::Broadcast;
            if (ifa->ifa_flags & IFF_LOOPBACK)
                iface._flag |= NetworkInterfaceFlag::Loopback;
            if (ifa->ifa_flags & IFF_POINTOPOINT)
                iface._flag |= NetworkInterfaceFlag::P2P;
            if (ifa->ifa_flags & IFF_MULTICAST)
                iface._flag |= NetworkInterfaceFlag::Multicast;
            if (ifa->ifa_flags & IFF_RUNNING)
                iface._flag |= NetworkInterfaceFlag::Running;
            // 设置 type
            if (ifa->ifa_flags & IFF_LOOPBACK)
                iface._type = NetworkInterfaceType::Loopback;
            else if (ifa->ifa_flags & IFF_POINTOPOINT)
                iface._type = NetworkInterfaceType::PPP; // 或 Tunnel
            else if (std::string_view(ifa->ifa_name).rfind("eth", 0) == 0 ||
                     std::string_view(ifa->ifa_name).rfind("en", 0) == 0)
                iface._type = NetworkInterfaceType::Ethernet;
            else if (std::string_view(ifa->ifa_name).rfind("wl", 0) == 0)
                iface._type = NetworkInterfaceType::Wireless;
            else
                iface._type = NetworkInterfaceType::Other;
        }
        // 填充 MAC 地址
        if (ifa->ifa_addr->sa_family == AF_PACKET) {
            auto *sll = reinterpret_cast<sockaddr_ll *>(ifa->ifa_addr);
            if (sll->sll_halen == 6)
                std::copy(sll->sll_addr, sll->sll_addr + 6, iface._addr.begin());
        }
    }

    freeifaddrs(ifaddr_list);
    std::vector<NetworkInterface> res{};
    res.reserve(interfaces.size());
    for (auto const &[name, iface] : interfaces)
        res.push_back(iface);

    return res;
}

template <int Family>
static auto getIp(const std::string &if_name) noexcept {
    using NetworkType = std::conditional_t<Family == AF_INET, ip::Networkv4, ip::Networkv6>;
    std::vector<NetworkType> res{};
    ifaddrs *ifaddr_list{};
    if (getifaddrs(&ifaddr_list) == -1)
        return res;

    for (auto *ifa = ifaddr_list; ifa != nullptr; ifa = ifa->ifa_next) {
        if (ifa->ifa_addr != nullptr && if_name == ifa->ifa_name && ifa->ifa_addr->sa_family == Family) {
            if constexpr (Family == AF_INET) {
                auto *addr_v4 = reinterpret_cast<sockaddr_in *>(ifa->ifa_addr);
                std::array<uint8_t, 4> ip_bytes{};
                memcpy(ip_bytes.data(), &addr_v4->sin_addr.s_addr, 4);
                std::array<uint8_t, 4> mask_bytes{};
                if (ifa->ifa_netmask != nullptr) {
                    auto *mask_v4 = reinterpret_cast<sockaddr_in *>(ifa->ifa_netmask);
                    memcpy(mask_bytes.data(), &mask_v4->sin_addr.s_addr, 4);
                }
                res.emplace_back(ip_bytes, mask_bytes);
            } else if constexpr (Family == AF_INET6) {
                auto *addr_v6 = reinterpret_cast<sockaddr_in6 *>(ifa->ifa_addr);
                std::array<uint8_t, 16> ip_bytes{};
                memcpy(ip_bytes.data(), &addr_v6->sin6_addr.s6_addr, 16);
                res.emplace_back(ip_bytes);
            }
        }
    }
    freeifaddrs(ifaddr_list);
    return res;
}

#endif

std::string NetworkInterface::to_string() const {
    char buf[18]{};
    std::snprintf(buf, sizeof(buf), "%02x:%02x:%02x:%02x:%02x:%02x",
                  _addr[0], _addr[1], _addr[2], _addr[3], _addr[4], _addr[5]);
    return buf;
}

NetworkInterface NetworkInterface::findByName(std::string_view name) noexcept {
    auto interfaces = list();
    for (const auto &iface : interfaces)
        if (iface._name == name)
            return iface;
    return {};
}

NetworkInterface NetworkInterface::findByAddress(const std::array<uint8_t, 6> &addr) noexcept {
    auto interfaces = list();
    for (const auto &iface : interfaces)
        if (iface._addr == addr)
            return iface;
    return {};
}

std::vector<ip::Networkv4> NetworkInterface::ipv4() const noexcept { return getIp<AF_INET>(_name); }
std::vector<ip::Networkv6> NetworkInterface::ipv6() const noexcept { return getIp<AF_INET6>(_name); }

#ifdef __MSVC__
#pragma endregion
#pragma region IP / Socket Options
#endif

namespace ip {

namespace multicast {

// Interface Implementation

Interface::Interface(std::array<uint8_t, 4> addr) { std::copy(addr.begin(), addr.end(), _data); }
int Interface::name() const { return IP_MULTICAST_IF; }
sockopt_data_t Interface::data() const { return reinterpret_cast<sockopt_data_t>(&_data); }
unsigned int Interface::size() const { return sizeof(_data); }

// Loopback Implementation

Loopback::Loopback(bool enabled) : _data(enabled ? 1 : 0) {}
int Loopback::name() const { return IP_MULTICAST_LOOP; }
sockopt_data_t Loopback::data() const { return reinterpret_cast<sockopt_data_t>(&_data); }
unsigned int Loopback::size() const { return sizeof(_data); }

// JoinGroup Implementation

JoinGroup::JoinGroup(std::string_view multicast_addr) {
    ip_mreq mreq{};
    inet_pton(AF_INET, multicast_addr.data(), &mreq.imr_multiaddr);
    mreq.imr_interface.s_addr = htonl(INADDR_ANY);
    memcpy(&_data, &mreq, sizeof(mreq));
}

int JoinGroup::name() const { return IP_ADD_MEMBERSHIP; }
sockopt_data_t JoinGroup::data() const { return reinterpret_cast<sockopt_data_t>(&_data); }
unsigned int JoinGroup::size() const { return sizeof(_data); }

} // namespace multicast

namespace tcp {

Protocol v4() { return {AF_INET, SOCK_STREAM}; }
Protocol v6() { return {AF_INET6, SOCK_STREAM}; }

} // namespace tcp

namespace udp {

Protocol v4() { return {AF_INET, SOCK_DGRAM}; }
Protocol v6() { return {AF_INET6, SOCK_DGRAM}; }

} // namespace udp

} // namespace ip

template <typename SockOpt>
static void set_fd_option(SocketFd fd, const SockOpt &opt) {
    RMVL_DbgAssert(fd != INVALID_SOCKET_FD);
    if (setsockopt(fd, IPPROTO_IP, opt.name(), opt.data(), static_cast<socklen_t>(opt.size())) < 0)
        RMVL_Error_(RMVL_StsError, "setsockopt failed, error code: %d", error_code());
}

template <>
void DgramSocket::setOption(const ip::multicast::Loopback &opt) { set_fd_option(_fd, opt); }
template <>
void DgramSocket::setOption(const ip::multicast::JoinGroup &opt) { set_fd_option(_fd, opt); }
template <>
void DgramSocket::setOption(const ip::multicast::Interface &opt) { set_fd_option(_fd, opt); }

#ifdef __MSVC__
#pragma endregion
#pragma region Socket Implements
#endif

static void setNonblock(SocketFd fd) {
    RMVL_Assert(fd != INVALID_SOCKET_FD);
#ifdef _WIN32
    u_long mode = 1;
    ioctlsocket(fd, FIONBIO, &mode);
#else
    int flags = fcntl(fd, F_GETFL, 0);
    fcntl(fd, F_SETFL, flags | O_NONBLOCK);
#endif
}

// 绑定 Socket 到指定端口，0 表示随机端口
static void fdbind(SocketFd fd, int family, uint16_t port) {
    RMVL_Assert(fd != INVALID_SOCKET_FD);
#ifdef _WIN32
    char opt = 1;
#else
    int opt = 1;
#endif
    // 设置允许重用处于 TIME_WAIT 状态的地址
    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
        WARNING_("setsockopt SO_REUSEADDR failed, error code: %d", error_code());
    // IPv4 Socket
    if (family == AF_INET) {
        sockaddr_in addr{};
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = INADDR_ANY;
        addr.sin_port = htons(port);
        if (bind(fd, reinterpret_cast<sockaddr *>(&addr), sizeof(addr)) < 0)
            RMVL_Error_(RMVL_StsError, "bind failed, error code: %d", error_code());
    }
    // IPv6 Socket
    else if (family == AF_INET6) {
        sockaddr_in6 addr{};
        addr.sin6_family = AF_INET6;
        addr.sin6_addr = in6addr_any;
        addr.sin6_port = htons(port);
        if (bind(fd, reinterpret_cast<sockaddr *>(&addr), sizeof(addr)) < 0)
            RMVL_Error_(RMVL_StsError, "bind failed, error code: %d", error_code());
    } else
        RMVL_Error(RMVL_StsError, "Unsupported socket family");
}

struct _dgram_recvfrom_res {
    char ip_str[INET6_ADDRSTRLEN]{};
    uint16_t port{};
};

static _dgram_recvfrom_res parse_recvfrom(const sockaddr_storage &sender_addr) {
    _dgram_recvfrom_res res{};
    if (sender_addr.ss_family == AF_INET) {
        const auto *addr_v4 = reinterpret_cast<const sockaddr_in *>(&sender_addr);
        inet_ntop(AF_INET, &addr_v4->sin_addr, res.ip_str, sizeof(res.ip_str));
        res.port = ntohs(addr_v4->sin_port);
    } else if (sender_addr.ss_family == AF_INET6) {
        const auto *addr_v6 = reinterpret_cast<const sockaddr_in6 *>(&sender_addr);
        inet_ntop(AF_INET6, &addr_v6->sin6_addr, res.ip_str, sizeof(res.ip_str));
        res.port = ntohs(addr_v6->sin6_port);
    }
    return res;
}

struct _dgram_sendto_res {
    sockaddr_storage dst_storage{};
    socklen_t addr_len{};
};

static _dgram_sendto_res parse_sendto(const std::string_view &addr, const Endpoint &endpoint) noexcept {
    _dgram_sendto_res res{};
    if (endpoint.family() == AF_INET) {
        auto *dst_addr = reinterpret_cast<sockaddr_in *>(&res.dst_storage);
        dst_addr->sin_family = AF_INET;
        dst_addr->sin_port = htons(endpoint.port());
        inet_pton(AF_INET, addr.data(), &dst_addr->sin_addr);
        res.addr_len = sizeof(sockaddr_in);
    } else if (endpoint.family() == AF_INET6) {
        auto *dst_addr = reinterpret_cast<sockaddr_in6 *>(&res.dst_storage);
        dst_addr->sin6_family = AF_INET6;
        dst_addr->sin6_port = htons(endpoint.port());
        inet_pton(AF_INET6, addr.data(), &dst_addr->sin6_addr);
        res.addr_len = sizeof(sockaddr_in6);
    }
    return res;
}

static std::tuple<std::string, std::string, uint16_t> fdrecvfrom(SocketFd fd) {
    char buf[2048]{};
    sockaddr_storage sender_addr{};
    socklen_t addr_len = sizeof(sender_addr);
    auto n = ::recvfrom(fd, buf, sizeof(buf), 0, reinterpret_cast<sockaddr *>(&sender_addr), &addr_len);
    if (n > 0) {
        std::string data(buf, n);
        auto [sender_ip_str, sender_port] = parse_recvfrom(sender_addr);
        return {data, sender_ip_str, sender_port};
    }
    return {};
}

static bool fdsendto(SocketFd fd, std::string_view addr, const Endpoint &endpoint, std::string_view data) {
    auto [dst_storage, addr_len] = parse_sendto(addr, endpoint);
#ifdef _WIN32
    auto n = ::sendto(fd, data.data(), static_cast<int>(data.size()), 0, reinterpret_cast<sockaddr *>(&dst_storage), addr_len);
#else
    auto n = ::sendto(fd, data.data(), data.size(), 0, reinterpret_cast<sockaddr *>(&dst_storage), addr_len);
#endif
    return n == static_cast<decltype(n)>(data.size());
}

Endpoint _endpoint(SocketFd fd) {
    RMVL_DbgAssert(fd != INVALID_SOCKET_FD);
    sockaddr_storage addr{};
    socklen_t addr_len = sizeof(addr);
    if (getsockname(fd, reinterpret_cast<sockaddr *>(&addr), &addr_len) < 0)
        RMVL_Error_(RMVL_StsError, "getsockname failed, error code: %d", error_code());

    if (addr.ss_family == AF_INET6) {
        auto *addr_v6 = reinterpret_cast<sockaddr_in6 *>(&addr);
        return Endpoint(ip::tcp::v6(), ntohs(addr_v6->sin6_port));
    } else {
        auto *addr_v4 = reinterpret_cast<sockaddr_in *>(&addr);
        return Endpoint(ip::tcp::v4(), ntohs(addr_v4->sin_port));
    }
}

std::tuple<std::string, std::string, uint16_t> DgramSocket::read() noexcept {
    RMVL_DbgAssert(_fd != INVALID_SOCKET_FD);
    return fdrecvfrom(_fd);
}

bool DgramSocket::write(std::string_view addr, const Endpoint &endpoint, std::string_view data) noexcept {
    RMVL_DbgAssert(_fd != INVALID_SOCKET_FD);
    return fdsendto(_fd, addr, endpoint, data);
}

bool DgramSocket::write(std::array<uint8_t, 4> addr, const Endpoint &endpoint, std::string_view data) noexcept {
    RMVL_DbgAssert(_fd != INVALID_SOCKET_FD);
    sockaddr_in dst{};
    socklen_t addr_len{};
    dst.sin_family = AF_INET;
    dst.sin_port = htons(endpoint.port());
    memcpy(&dst.sin_addr.s_addr, addr.data(), 4);
    addr_len = sizeof(sockaddr_in);
#ifdef _WIN32
    auto n = ::sendto(_fd, data.data(), static_cast<int>(data.size()), 0, reinterpret_cast<sockaddr *>(&dst), addr_len);
#else
    auto n = ::sendto(_fd, data.data(), data.size(), 0, reinterpret_cast<sockaddr *>(&dst), addr_len);
#endif
    return n == static_cast<decltype(n)>(data.size());
}

Endpoint DgramSocket::endpoint() const { return _endpoint(_fd); }

std::string StreamSocket::read() noexcept {
    char buf[2048]{};
#if _WIN32
    auto n = ::recv(_fd, buf, static_cast<int>(sizeof(buf)), 0);
#else
    auto n = ::recv(_fd, buf, sizeof(buf), 0);
#endif
    return n > 0 ? std::string(buf, n) : std::string{};
}

bool StreamSocket::write(std::string_view data) noexcept {
    auto n = ::send(_fd, data.data(), data.size(), 0);
    return n == static_cast<decltype(n)>(data.size());
}

Endpoint StreamSocket::endpoint() const { return _endpoint(_fd); }

#ifdef _WIN32
Listener::Listener(const Endpoint &ep, bool blocking, bool ov) : _endpoint(ep), _fd(INVALID_SOCKET_FD) {
    SocketEnv::ensure_init();
    _fd = ov ? WSASocket(ep.family(), ep.type(), 0, NULL, 0, WSA_FLAG_OVERLAPPED) : socket(ep.family(), ep.type(), 0);
#else
Listener::Listener(const Endpoint &ep, bool blocking, bool) : _endpoint(ep), _fd(socket(ep.family(), ep.type(), 0)) {
#endif
    if (!blocking)
        setNonblock(_fd);
    fdbind(_fd, ep.family(), ep.port());
}

DgramSocket Listener::create() {
    RMVL_Assert(_fd != INVALID_SOCKET_FD);
    SocketFd fd = std::exchange(_fd, INVALID_SOCKET_FD); // 转移所有权
    return DgramSocket(fd);
}

#ifdef _WIN32
Sender::Sender(const ip::Protocol &proto, bool blocking, bool ov) : _protocol(proto), _fd(INVALID_SOCKET_FD) {
    SocketEnv::ensure_init();
    _fd = ov ? WSASocket(proto.family, proto.type, 0, NULL, 0, WSA_FLAG_OVERLAPPED) : socket(proto.family, proto.type, 0);
#else
Sender::Sender(const ip::Protocol &proto, bool blocking, bool) : _protocol(proto), _fd(socket(proto.family, proto.type, 0)) {
#endif
    if (!blocking)
        setNonblock(_fd);
    fdbind(_fd, proto.family, 0);
}

DgramSocket Sender::create() {
    RMVL_Assert(_fd != INVALID_SOCKET_FD);
    SocketFd fd = std::exchange(_fd, INVALID_SOCKET_FD); // 转移所有权
    return DgramSocket(fd);
}

StreamSocket &StreamSocket::operator=(StreamSocket &&other) noexcept {
    _fd = std::exchange(other._fd, INVALID_SOCKET_FD);
    return *this;
}

#ifdef _WIN32
Acceptor::Acceptor(const Endpoint &ep, bool blocking, bool ov) : _endpoint(ep) {
    SocketEnv::ensure_init();
    _fd = ov ? WSASocket(ep.family(), ep.type(), 0, NULL, 0, WSA_FLAG_OVERLAPPED) : socket(ep.family(), ep.type(), 0);
#else
Acceptor::Acceptor(const Endpoint &ep, bool blocking, bool) : _endpoint(ep), _fd(socket(ep.family(), ep.type(), 0)) {
#endif
    if (!blocking)
        setNonblock(_fd);
    fdbind(_fd, ep.family(), ep.port());
    if (listen(_fd, SOMAXCONN) < 0)
        RMVL_Error_(RMVL_StsError, "listen failed, error code: %d", error_code());
}

StreamSocket Acceptor::accept() { return StreamSocket(::accept(_fd, nullptr, nullptr)); }

static int fdconnect(int fd, const Endpoint &ep, std::string_view url) {
    // IPv4 Socket
    if (ep.family() == AF_INET) {
        sockaddr_in addr{};
        addr.sin_family = ep.family();
        addr.sin_port = htons(ep.port());
        if (inet_pton(AF_INET, url.data(), &addr.sin_addr) != 1)
            RMVL_Error(RMVL_StsError, "Invalid IPv4 address");
        if (ep.type() == SOCK_STREAM)
            if (connect(fd, reinterpret_cast<sockaddr *>(&addr), sizeof(addr)) < 0)
                RMVL_Error_(RMVL_StsError, "Connection failed, error code: %d", error_code());
    }
    // IPv6 Socket
    else if (ep.family() == AF_INET6) {
        sockaddr_in6 addr{};
        addr.sin6_family = ep.family();
        addr.sin6_port = htons(ep.port());
        if (inet_pton(AF_INET6, url.data(), &addr.sin6_addr) != 1)
            RMVL_Error(RMVL_StsError, "Invalid IPv6 address");
        if (ep.type() == SOCK_STREAM)
            if (connect(fd, reinterpret_cast<sockaddr *>(&addr), sizeof(addr)) < 0)
                RMVL_Error_(RMVL_StsError, "Connection failed, error code: %d", error_code());
    } else
        RMVL_Error(RMVL_StsError, "Unsupported socket family");
    return fd;
}

#ifdef _WIN32
Connector::Connector(const Endpoint &ep, std::string_view url, bool blocking, bool ov) : _url(url), _endpoint(ep) {
    SocketEnv::ensure_init();
    _fd = ov ? WSASocket(ep.family(), ep.type(), 0, NULL, 0, WSA_FLAG_OVERLAPPED) : socket(ep.family(), ep.type(), 0);
#else
Connector::Connector(const Endpoint &ep, std::string_view url, bool blocking, bool) : _url(url), _endpoint(ep), _fd(socket(ep.family(), ep.type(), 0)) {
#endif
    if (!blocking)
        setNonblock(_fd);
    RMVL_Assert(_fd != INVALID_SOCKET_FD);
}

StreamSocket Connector::connect() {
    int sfd = fdconnect(static_cast<int>(_fd), _endpoint, _url);
    return StreamSocket(sfd);
}

static bool valid(SocketFd fd) {
#ifdef _WIN32
    if (fd == INVALID_SOCKET)
        return false;
    int type{};
    int len = sizeof(type);
    return getsockopt(fd, SOL_SOCKET, SO_TYPE, (char *)&type, &len) == 0;
#else
    if (fd < 0)
        return false;
    return fcntl(fd, F_GETFD) != -1;
#endif
}

#ifdef _WIN32

SocketEnv::SocketEnv() {
    WSADATA wsa_data{};
    if (WSAStartup(MAKEWORD(2, 2), &wsa_data) != 0)
        RMVL_Error(RMVL_StsBadFunc, "WSAStartup failed");
}

SocketEnv::~SocketEnv() { WSACleanup(); }

DgramSocket::~DgramSocket() { valid(_fd) ? ::closesocket(_fd) : 0; }
Sender::~Sender() { valid(_fd) ? ::closesocket(_fd) : 0; }
Listener::~Listener() { valid(_fd) ? ::closesocket(_fd) : 0; }
StreamSocket::~StreamSocket() { valid(_fd) ? ::closesocket(_fd) : 0; }
Acceptor::~Acceptor() { valid(_fd) ? ::closesocket(_fd) : 0; }
Connector::~Connector() { valid(_fd) ? ::closesocket(_fd) : 0; }

#else

DgramSocket::~DgramSocket() { valid(_fd) ? ::close(_fd) : 0; }
Sender::~Sender() { valid(_fd) ? ::close(_fd) : 0; }
Listener::~Listener() { valid(_fd) ? ::close(_fd) : 0; }
StreamSocket::~StreamSocket() { valid(_fd) ? ::close(_fd) : 0; }
Acceptor::~Acceptor() { valid(_fd) ? ::close(_fd) : 0; }
Connector::~Connector() { valid(_fd) ? ::close(_fd) : 0; }

#endif

#ifdef __MSVC__
#pragma endregion
#pragma region Async Socket
#endif

#if __cplusplus >= 202002L

namespace async {

DgramSocket Listener::create() {
    RMVL_Assert(_fd != INVALID_SOCKET_FD);
    return DgramSocket(_ctx, _fd);
}

DgramSocket Sender::create() {
    RMVL_Assert(_fd != INVALID_SOCKET_FD);
    return DgramSocket(_ctx, _fd);
}

#ifdef _WIN32

DgramSocket::DgramSocket(IOContext &io_context, SocketFd fd) : ::rm::DgramSocket(fd), _ctx(io_context) {
    if (CreateIoCompletionPort((HANDLE)_fd, _ctx.get().handle(), 0, 0) == nullptr) {
        auto err = GetLastError();
        if (err != ERROR_INVALID_PARAMETER)
            RMVL_Error_(RMVL_StsError, "Associate fd with IOCP failed: %lu", err);
    }
}

void DgramSocket::SocketReadAwaiter::await_suspend(std::coroutine_handle<> handle) {
    RMVL_DbgAssert(_fd != INVALID_FD);
    _ovl = std::make_unique<IocpOverlapped>(handle);

    auto *p_addr = new (_ovl->info) sockaddr_storage{};
    auto *p_addr_len = new (p_addr + 1) socklen_t{sizeof(sockaddr_storage)};

    WSABUF buf{};
    buf.buf = _ovl->buf;
    buf.len = static_cast<ULONG>(sizeof(_ovl->buf));
    DWORD flags = 0;

    if (WSARecvFrom(reinterpret_cast<SOCKET>(_fd), &buf, 1, nullptr, &flags, reinterpret_cast<sockaddr *>(p_addr),
                    p_addr_len, &_ovl->ov, nullptr) == SOCKET_ERROR) {
        DWORD error = WSAGetLastError();
        if (error != ERROR_IO_PENDING)
            ERROR_("WSARecvFrom failed with error: %lu", error);
    }
}

std::tuple<std::string, std::string, uint16_t> DgramSocket::SocketReadAwaiter::await_resume() noexcept {
    RMVL_DbgAssert(_fd != INVALID_FD);
    DWORD bytes_transferred = 0;
    if (!GetOverlappedResult((HANDLE)_fd, &_ovl->ov, &bytes_transferred, FALSE)) {
        WARNING_("GetOverlappedResult failed with error: %lu", GetLastError());
        return {};
    }
    if (bytes_transferred > 0) {
        auto *p_addr = reinterpret_cast<sockaddr_storage *>(_ovl->info);
        std::string data(_ovl->buf, bytes_transferred);
        auto [sender_ip_str, sender_port] = parse_recvfrom(*p_addr);
        return {data, sender_ip_str, sender_port};
    }
    return {};
}

void DgramSocket::SocketWriteAwaiter::await_suspend(std::coroutine_handle<> handle) {
    RMVL_DbgAssert(_fd != INVALID_FD);
    _ovl = std::make_unique<IocpOverlapped>(handle);

    WSABUF buf{};
    buf.buf = const_cast<char *>(_data.data());
    buf.len = static_cast<ULONG>(_data.size());

    // 准备目标地址
    auto [dst_storage, addr_len] = parse_sendto(_addr, _endpoint);
    if (WSASendTo(reinterpret_cast<SOCKET>(_fd), &buf, 1, nullptr, 0, reinterpret_cast<const sockaddr *>(&dst_storage),
                  addr_len, &_ovl->ov, nullptr) == SOCKET_ERROR) {
        DWORD error = WSAGetLastError();
        if (error != ERROR_IO_PENDING)
            RMVL_Error_(RMVL_StsBadArg, "WSASendTo failed with error: %lu", error);
    }
}

Listener::Listener(IOContext &io_context, const Endpoint &endpoint) : ::rm::Listener(endpoint, true), _ctx(io_context) {
    if (CreateIoCompletionPort((HANDLE)_fd, _ctx.get().handle(), 0, 0) == nullptr) {
        auto err = GetLastError();
        if (err != ERROR_INVALID_PARAMETER)
            RMVL_Error_(RMVL_StsError, "Associate fd with IOCP failed: %lu", err);
    }
}

Sender::Sender(IOContext &io_context, const ip::Protocol &protocol) : ::rm::Sender(protocol, true), _ctx(io_context) {
    if (CreateIoCompletionPort((HANDLE)_fd, _ctx.get().handle(), 0, 0) == nullptr) {
        auto err = GetLastError();
        if (err != ERROR_INVALID_PARAMETER)
            RMVL_Error_(RMVL_StsError, "Associate fd with IOCP failed: %lu", err);
    }
}

StreamSocket::StreamSocket(IOContext &io_context, SocketFd fd) : ::rm::StreamSocket(fd), _ctx(io_context) {
    if (CreateIoCompletionPort((HANDLE)_fd, _ctx.get().handle(), 0, 0) == nullptr) {
        auto err = GetLastError();
        if (err != ERROR_INVALID_PARAMETER)
            RMVL_Error_(RMVL_StsError, "Associate fd with IOCP failed: %lu", err);
    }
}

void StreamSocket::SocketReadAwaiter::await_suspend(std::coroutine_handle<> handle) {
    RMVL_DbgAssert(_fd != INVALID_FD);
    _ovl = std::make_unique<IocpOverlapped>(handle);

    WSABUF buf{};
    buf.buf = _ovl->buf;
    buf.len = sizeof(_ovl->buf);
    DWORD flags = 0, len = 0;

    if (WSARecv((SOCKET)_fd, &buf, 1, &len, &flags, &_ovl->ov, nullptr) == SOCKET_ERROR) {
        DWORD error = WSAGetLastError();
        if (error != ERROR_IO_PENDING)
            ERROR_("ReadFile failed with error: %lu", error);
    }
}

void StreamSocket::SocketWriteAwaiter::await_suspend(std::coroutine_handle<> handle) {
    RMVL_DbgAssert(_fd != INVALID_FD);
    _ovl = std::make_unique<IocpOverlapped>(handle);

    WSABUF buf{};
    buf.buf = const_cast<char *>(_data.data());
    buf.len = static_cast<ULONG>(_data.size());
    DWORD len = 0;

    if (WSASend((SOCKET)_fd, &buf, 1, &len, 0, &_ovl->ov, nullptr) == SOCKET_ERROR) {
        DWORD error = WSAGetLastError();
        if (error != ERROR_IO_PENDING)
            RMVL_Error_(RMVL_StsBadArg, "WriteFile failed with error: %lu", error);
    }
}

Acceptor::Acceptor(IOContext &io_context, const Endpoint &endpoint) : ::rm::Acceptor(endpoint, true), _ctx(io_context) {
    if (CreateIoCompletionPort((HANDLE)_fd, _ctx.get().handle(), 0, 0) == nullptr) {
        auto err = GetLastError();
        if (err != ERROR_INVALID_PARAMETER)
            RMVL_Error_(RMVL_StsError, "Associate fd with IOCP failed: %lu", err);
    }
}

void Acceptor::AcceptAwaiter::await_suspend(std::coroutine_handle<> handle) {
    RMVL_DbgAssert(_fd != INVALID_FD);
    _ovl = std::make_unique<IocpOverlapped>(handle);

    // 创建新 socket 并存储至重叠 I/O 的 info 中
    auto sfd = WSASocket(_endpoint.family(), _endpoint.type(), 0, NULL, 0, WSA_FLAG_OVERLAPPED);
    if (sfd == INVALID_SOCKET)
        RMVL_Error_(RMVL_StsBadArg, "Create accept socket failed: %d", WSAGetLastError());
    new (_ovl->info) SOCKET{sfd};

    // 获取 AcceptEx
    LPFN_ACCEPTEX acceptEx{nullptr};
    GUID guid = WSAID_ACCEPTEX;
    DWORD bytes{0};
    if (WSAIoctl((SOCKET)_fd, SIO_GET_EXTENSION_FUNCTION_POINTER, &guid, sizeof(guid),
                 &acceptEx, sizeof(acceptEx), &bytes, NULL, NULL) == SOCKET_ERROR)
        RMVL_Error_(RMVL_StsBadArg, "Get AcceptEx function failed: %d", WSAGetLastError());

    // 异步 accept 提交至 IOCP
    DWORD received{};
    if (!acceptEx((SOCKET)_fd, sfd, _ovl->buf, 0, sizeof(sockaddr_storage) + 16,
                  sizeof(sockaddr_storage) + 16, &received, &_ovl->ov)) {
        DWORD error = WSAGetLastError();
        if (error != ERROR_IO_PENDING) {
            closesocket(sfd);
            RMVL_Error_(RMVL_StsBadArg, "AcceptEx failed with error: %lu", error);
        }
    }
}

StreamSocket Acceptor::AcceptAwaiter::await_resume() noexcept {
    RMVL_DbgAssert(_fd != INVALID_FD);
    SOCKET sfd = *reinterpret_cast<SOCKET *>(_ovl->info);

    // 更新 socket 上下文
    if (setsockopt(sfd, SOL_SOCKET, SO_UPDATE_ACCEPT_CONTEXT, (char *)&_fd, sizeof(_fd)) == SOCKET_ERROR)
        WARNING_("setsockopt SO_UPDATE_ACCEPT_CONTEXT failed: %d", WSAGetLastError());
    return StreamSocket(_ctx, sfd);
}

Connector::Connector(IOContext &io_context, const Endpoint &endpoint, std::string_view url) : ::rm::Connector(endpoint, url, true), _ctx(io_context) {
    if (CreateIoCompletionPort((HANDLE)_fd, _ctx.get().handle(), 0, 0) == nullptr) {
        auto err = GetLastError();
        if (err != ERROR_INVALID_PARAMETER)
            RMVL_Error_(RMVL_StsError, "Associate fd with IOCP failed: %lu", err);
    }
}

void Connector::ConnectAwaiter::await_suspend(std::coroutine_handle<> handle) {
    RMVL_DbgAssert(_fd != INVALID_FD);
    _ovl = std::make_unique<IocpOverlapped>(handle);

    // 获取 ConnectEx
    LPFN_CONNECTEX connectEx{nullptr};
    GUID guid = WSAID_CONNECTEX;
    DWORD bytes{0};
    if (WSAIoctl((SOCKET)_fd, SIO_GET_EXTENSION_FUNCTION_POINTER, &guid, sizeof(guid),
                 &connectEx, sizeof(connectEx), &bytes, NULL, NULL) == SOCKET_ERROR)
        RMVL_Error_(RMVL_StsBadArg, "Get ConnectEx function failed: %d", WSAGetLastError());

    // 为 ConnectEx 绑定一个本地地址，并准备远程地址
    sockaddr_storage remote{};
    int addr_len{};
    if (_endpoint.family() == AF_INET) {
        sockaddr_in local{}, *addr{reinterpret_cast<sockaddr_in *>(&remote)};
        local.sin_family = AF_INET;
        local.sin_port = 0;
        if (bind((SOCKET)_fd, reinterpret_cast<sockaddr *>(&local), sizeof(local)) == SOCKET_ERROR)
            RMVL_Error_(RMVL_StsBadArg, "Bind for ConnectEx failed: %d", WSAGetLastError());

        addr->sin_family = AF_INET;
        addr->sin_port = htons(_endpoint.port());
        inet_pton(AF_INET, _data.data(), &addr->sin_addr);
        addr_len = sizeof(sockaddr_in);
    } else if (_endpoint.family() == AF_INET6) {
        sockaddr_in6 local{}, *addr{reinterpret_cast<sockaddr_in6 *>(&remote)};
        local.sin6_family = AF_INET6;
        local.sin6_addr = in6addr_any;
        local.sin6_port = 0;
        if (bind((SOCKET)_fd, reinterpret_cast<sockaddr *>(&local), sizeof(local)) == SOCKET_ERROR)
            RMVL_Error_(RMVL_StsBadArg, "Bind for ConnectEx failed: %d", WSAGetLastError());

        addr->sin6_family = AF_INET6;
        addr->sin6_port = htons(_endpoint.port());
        if (inet_pton(AF_INET6, _data.data(), &addr->sin6_addr) != 1)
            RMVL_Error(RMVL_StsBadArg, "Invalid IPv6 address");
        addr_len = sizeof(sockaddr_in6);
    }

    // 异步 connect 提交至 IOCP
    DWORD sent{};
    if (!connectEx((SOCKET)_fd, reinterpret_cast<sockaddr *>(&remote), addr_len,
                   nullptr, 0, &sent, &_ovl->ov)) {
        DWORD error = WSAGetLastError();
        if (error != ERROR_IO_PENDING)
            RMVL_Error_(RMVL_StsBadArg, "ConnectEx failed with error: %lu", error);
    }
}

StreamSocket Connector::ConnectAwaiter::await_resume() noexcept {
    RMVL_DbgAssert(_fd != INVALID_FD);
    SOCKET sfd = (SOCKET)_fd;
    // 更新 socket 上下文
    if (setsockopt(sfd, SOL_SOCKET, SO_UPDATE_CONNECT_CONTEXT, NULL, 0) == SOCKET_ERROR)
        WARNING_("setsockopt SO_UPDATE_CONNECT_CONTEXT failed: %d", WSAGetLastError());
    return StreamSocket(_ctx, sfd);
}

#else

DgramSocket::DgramSocket(IOContext &io_context, SocketFd fd) : ::rm::DgramSocket(fd), _ctx(io_context) {}

std::tuple<std::string, std::string, uint16_t> DgramSocket::SocketReadAwaiter::await_resume() noexcept {
    RMVL_DbgAssert(_fd != INVALID_FD);
    epoll_ctl(_aioh, EPOLL_CTL_DEL, _fd, nullptr);
    return fdrecvfrom(_fd);
}

bool DgramSocket::SocketWriteAwaiter::await_resume() {
    RMVL_DbgAssert(_fd != INVALID_FD);
    epoll_ctl(_aioh, EPOLL_CTL_DEL, _fd, nullptr);
    return fdsendto(_fd, _addr, _endpoint, _data);
}

Sender::Sender(IOContext &io_context, const ip::Protocol &protocol) : ::rm::Sender(protocol, true), _ctx(io_context) {}

Listener::Listener(IOContext &io_context, const Endpoint &endpoint) : ::rm::Listener(endpoint, true), _ctx(io_context) {}

StreamSocket::StreamSocket(IOContext &io_context, SocketFd fd) : ::rm::StreamSocket(fd), _ctx(io_context) {}

std::string StreamSocket::SocketReadAwaiter::await_resume() {
    RMVL_DbgAssert(_fd != INVALID_FD);
    epoll_ctl(_aioh, EPOLL_CTL_DEL, _fd, nullptr);
    char buf[2048]{};
    ssize_t n = ::recv(_fd, buf, sizeof(buf), 0);
    return n > 0 ? std::string(buf, n) : std::string{};
}

bool StreamSocket::SocketWriteAwaiter::await_resume() {
    RMVL_DbgAssert(_fd != INVALID_FD);
    epoll_ctl(_aioh, EPOLL_CTL_DEL, _fd, nullptr);
    return ::send(_fd, _data.data(), _data.size(), 0) == static_cast<ssize_t>(_data.size());
}

Acceptor::Acceptor(IOContext &io_context, const Endpoint &endpoint) : ::rm::Acceptor(endpoint, true), _ctx(io_context) {}

StreamSocket Acceptor::AcceptAwaiter::await_resume() noexcept {
    RMVL_DbgAssert(_fd != INVALID_FD);
    epoll_ctl(_aioh, EPOLL_CTL_DEL, _fd, nullptr);
    int sfd = _endpoint.type() == SOCK_STREAM ? ::accept(_fd, nullptr, nullptr) : _fd;
    return StreamSocket(_ctx, sfd);
}

Connector::Connector(IOContext &io_context, const Endpoint &endpoint, std::string_view url) : ::rm::Connector(endpoint, url, true), _ctx(io_context) {}

StreamSocket Connector::ConnectAwaiter::await_resume() noexcept {
    RMVL_DbgAssert(_fd != INVALID_FD);
    epoll_ctl(_aioh, EPOLL_CTL_DEL, _fd, nullptr);
    int sfd = fdconnect(_fd, _endpoint, _data);
    return StreamSocket(_ctx, sfd);
}

#endif

} //  namespace async

#endif // __cplusplus >= 202002L

#ifdef __MSVC__
#pragma endregion
#endif

} // namespace rm

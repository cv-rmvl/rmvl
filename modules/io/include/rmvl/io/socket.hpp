/**
 * @file socket.hpp
 * @author zhaoxi (535394140@qq.com)
 * @brief 以及基于 socket 的同步/异步 IPC 通信、传输层与会话层通信框架
 * @version 1.0
 * @date 2025-07-31
 *
 * @copyright Copyright 2025 (c), zhaoxi
 *
 */

#pragma once

#include <array>
#include <cstdint>
#include <utility>

#include "async.hpp"

namespace rm {

//! @addtogroup io_net
//! @{

#ifdef _WIN32
using SocketFd = SOCKET;
constexpr SocketFd INVALID_SOCKET_FD = INVALID_SOCKET;
#else
using SocketFd = int;
constexpr SocketFd INVALID_SOCKET_FD = -1;
#endif

/**
 * @brief IP 协议族，包含 IPv4 和 IPv6 以及多播选项的相关定义
 *
 * @details
 * - 在网络层，提供 ip::Networkv4 和 ip::Networkv6 类来表示 IPv4 和 IPv6 地址
 * - 在传输层，提供 ip::tcp 和 ip::udp 命名空间，分别表示 TCP 和 UDP 协议
 * - 提供多播选项的封装类，便于在 Socket 上设置多播相关选项
 */
namespace ip {

//! 网络协议
struct Protocol {
    //! 协议族
    int family{};
    //! Socket 类型
    int type{};
};

//! IPv4 网络协议
class Networkv4 {
public:
    Networkv4(std::array<uint8_t, 4> addr, std::array<uint8_t, 4> netmask) : _addr(addr), _netmask(netmask) {}

    /**
     * @brief 获取 IPv4 地址
     *
     * @return IPv4 地址数组
     */
    std::array<uint8_t, 4> address() const noexcept { return _addr; }

    /**
     * @brief 获取广播地址
     *
     * @return 广播地址数组
     */
    std::array<uint8_t, 4> broadcast() const noexcept;

    /**
     * @brief 获取子网掩码
     *
     * @return 子网掩码数组
     */
    std::array<uint8_t, 4> netmask() const noexcept { return _netmask; }

private:
    std::array<uint8_t, 4> _addr{};    //!< IPv4 地址
    std::array<uint8_t, 4> _netmask{}; //!< 子网掩码
};

//! IPv6 网络协议
class Networkv6 {
public:
    explicit Networkv6(const std::array<uint8_t, 16> &addr) : _addr(addr) {}

    /**
     * @brief 获取 IPv6 地址
     *
     * @return IPv6 地址数组
     */
    const std::array<uint8_t, 16> &address() const noexcept { return _addr; }

private:
    std::array<uint8_t, 16> _addr{}; //!< IPv6 地址
};

//! 多播
namespace multicast {

#ifdef _WIN32
#define sockopt_data_t const char *
#else
#define sockopt_data_t const void *
#endif

//! Outbound 多播接口设置选项
class Interface {
public:
    /**
     * @brief 构造 Outbound 多播接口设置选项
     *
     * @param[in] addr 多播接口的 IPv4 地址
     */
    explicit Interface(std::array<uint8_t, 4> addr);

    //! @cond
    int name() const;
    sockopt_data_t data() const;
    unsigned int size() const;
    //! @endcond

private:
    uint8_t _data[4]{}; //!< 多播接口的 IPv4 地址
};

//! 多播环回控制选项
class Loopback {
public:
    explicit Loopback(bool enabled = true);

    //! @cond
    int name() const;
    sockopt_data_t data() const;
    unsigned int size() const;
    //! @endcond

private:
    uint8_t _data{};
};

//! 加入多播组控制选项
class JoinGroup {
public:
    explicit JoinGroup(std::string_view group);

    //! @cond
    int name() const;
    sockopt_data_t data() const;
    unsigned int size() const;
    //! @endcond

private:
    uint8_t _data[8]{}; //!< 多播组数据结构
};

} // namespace multicast

//! TCP 协议
namespace tcp {

//! 构造端点，以表示 IPv4 TCP 协议
Protocol v4();
//! 构造端点，以表示 IPv6 TCP 协议
Protocol v6();

} // namespace tcp

//! UDP 协议
namespace udp {

//! 构造端点，以表示 IPv4 UDP 协议
Protocol v4();
//! 构造端点，以表示 IPv6 UDP 协议
Protocol v6();

} // namespace udp

} // namespace ip

//! 接口驱动类型
enum class NetworkInterfaceType : uint8_t {
    Ethernet, //!< 以太网
    Wireless, //!< 无线接口
    PPP,      //!< 点对点协议
    Tunnel,   //!< 隧道接口
    Loopback, //!< `lo` 回环设备
    Other,    //!< 其他
    Unknown   //!< 未知类型
};

//! 接口功能与状态标志
struct NetworkInterfaceFlag {
    static constexpr uint8_t Up = 1 << 0;        //!< 接口已启用
    static constexpr uint8_t Broadcast = 1 << 1; //!< 支持广播
    static constexpr uint8_t Loopback = 1 << 2;  //!< 回环接口
    static constexpr uint8_t P2P = 1 << 3;       //!< 点对点链接，如 PPP、TUN 等
    static constexpr uint8_t Multicast = 1 << 4; //!< 支持多播
    static constexpr uint8_t Running = 1 << 5;   //!< 接口正在运行
};

//! 网络接口
class NetworkInterface {
public:
    //! 获取所有网络接口的 MAC 地址列表
    static std::vector<NetworkInterface> list() noexcept;

    /**
     * @brief 根据名称查找网络接口
     *
     * @param[in] name 名称字符串，如 "eth0"
     * @return 找到的网络接口，未找到时返回一个无效的接口
     */
    static NetworkInterface findByName(std::string_view name) noexcept;

    /**
     * @brief 根据 MAC 地址查找网络接口
     *
     * @param[in] addr MAC 地址字节数组
     * @return 找到的网络接口，未找到时返回一个无效的接口
     */
    static NetworkInterface findByAddress(const std::array<uint8_t, 6> &addr) noexcept;

    //! 获取网络接口的 MAC 地址
    const std::array<uint8_t, 6> &address() const noexcept { return _addr; }

    //! 获取接口名称
    std::string name() const noexcept { return _name; }

    //! 获取接口类型
    NetworkInterfaceType type() const noexcept { return _type; }

    //! 获取接口功能与状态标志
    uint8_t flag() const noexcept { return _flag; }

    //! 接口是否启用
    bool up() const noexcept { return (_flag & NetworkInterfaceFlag::Up) != 0; }

    //! 是否为回环接口
    bool loopback() const noexcept { return (_flag & NetworkInterfaceFlag::Loopback) != 0; }

    //! 是否支持广播
    bool broadcast() const noexcept { return (_flag & NetworkInterfaceFlag::Broadcast) != 0; }

    //! 是否为点对点接口
    bool p2p() const noexcept { return (_flag & NetworkInterfaceFlag::P2P) != 0; }

    //! 是否支持多播
    bool multicast() const noexcept { return (_flag & NetworkInterfaceFlag::Multicast) != 0; }

    //! 接口是否正在运行
    bool running() const noexcept { return (_flag & NetworkInterfaceFlag::Running) != 0; }

    //! 获取 MAC 地址的字符串表示形式
    std::string to_string() const;

    //! 获取 IPv4 地址列表
    std::vector<ip::Networkv4> ipv4() const noexcept;

    //! 获取 IPv6 地址列表
    std::vector<ip::Networkv6> ipv6() const noexcept;

private:
    std::string _name{};            //!< 接口名称
    NetworkInterfaceType _type{};   //!< 驱动类型
    uint8_t _flag{};                //!< 功能与状态标志
    std::array<uint8_t, 6> _addr{}; //!< MAC 地址
};

#if __cplusplus >= 202002L

#endif

//! 端点
class Endpoint {
public:
    static constexpr uint16_t ANY_PORT = 0; //!< 任意可用或自动分配端口标识

    /**
     * @brief 构造 Socket 端点
     *
     * @param[in] ip 网络协议
     * @param[in] port 端口号，为 `0` 时表示任意可用或自动分配
     * @code {.cpp}
     * Endpoint ep(ip::tcp::v4(), 8080);
     * @endcode
     */
    Endpoint(const ip::Protocol &ip, uint16_t port) : _family(ip.family), _type(ip.type), _port(port) {}

    //! 获取协议族
    int family() const { return _family; }
    //! 获取 Socket 类型
    int type() const { return _type; }
    //! 获取端口号
    uint16_t port() const { return _port; }

private:
    int _family{};    //!< 协议族
    int _type{};      //!< Socket 类型
    uint16_t _port{}; //!< 端口号，`0` 表示任意可用或自动分配
};

#ifdef _WIN32

/**
 * @brief Socket 环境管理
 * @note
 * - Windows 环境专属
 * - 一般无需手动管理，在第一次创建 `rm::Acceptor`、`rm::Connector`、`rm::Listener`、`rm::Sender` 及其派生类时自动初始化
 */
class SocketEnv {
public:
    //! 线程安全的确保 Socket 环境已初始化，仅 Windows 环境有效
    static void ensure_init() { static SocketEnv se; }

private:
    SocketEnv();
    ~SocketEnv();

    SocketEnv(const SocketEnv &) = delete;
    SocketEnv &operator=(const SocketEnv &) = delete;
};

#endif

//! 由 rm::Listener 或 rm::Sender 建立的数据报式 Socket 会话
class DgramSocket {
public:
    //! @cond
    explicit DgramSocket(SocketFd fd) : _fd(fd) {}
    DgramSocket(const DgramSocket &) = delete;
    DgramSocket(DgramSocket &&other) noexcept : _fd(std::exchange(other._fd, INVALID_SOCKET_FD)) {}

    DgramSocket &operator=(const DgramSocket &) = delete;
    DgramSocket &operator=(DgramSocket &&other) noexcept;

    ~DgramSocket();
    //! @endcond

    //! 会话是否失效
    [[nodiscard]] bool invalid() const noexcept { return _fd == INVALID_SOCKET_FD; }

    /**
     * @brief 设置 Socket 选项
     *
     * @param[in] opt Socket 选项
     */
    template <typename SockOpt>
    void setOption(const SockOpt &opt);

    //! 获取绑定的端点
    Endpoint endpoint() const;

    /**
     * @brief 同步读取 Socket 中的数据（阻塞）
     * @code {.cpp}
     * // 使用示例
     * auto [str, addr, port] = socket.read();
     * @endcode
     *
     * @return 读取到的数据
     * @retval data 读取的数据
     * @retval addr 发送方地址
     * @retval port 发送方端口
     */
    std::tuple<std::string, std::string, uint16_t> read() noexcept;

    /**
     * @brief 同步写入数据到的 Socket 中（阻塞）
     * @code {.cpp}
     * // 使用示例
     * bool success = socket.write("192.168.1.100", rm::Endpoint(rm::ip::udp::v4(), 12345), "Hello, World!");
     * @endcode
     *
     * @param[in] addr 目标地址
     * @param[in] endpoint 目标端点
     * @param[in] data 待写入的数据
     * @return 是否写入成功
     */
    bool write(std::string_view addr, const Endpoint &endpoint, std::string_view data) noexcept;

    /**
     * @brief 同步写入数据到的 IPv4 的 Socket 中（阻塞）
     * @code {.cpp}
     * // 使用示例
     * bool success = socket.write({192, 168, 1, 100}, rm::Endpoint(rm::ip::udp::v4(), 12345), "Hello, World!");
     * @endcode
     *
     * @param[in] addr 使用数组形式表示的 IPv4 目标地址
     * @param[in] endpoint 目标端点
     * @param[in] data 待写入的数据
     * @return 是否写入成功
     */
    bool write(std::array<uint8_t, 4> addr, const Endpoint &endpoint, std::string_view data) noexcept;

protected:
    SocketFd _fd{INVALID_SOCKET_FD}; //!< 会话文件描述符
};

/**
 * @brief 同步数据报式 Socket 发送器
 * @details 用于将数据报 Socket 绑定到指定端点，并返回新的 Socket 会话
 */
class Sender {
public:
    /**
     * @brief 创建数据报式 Socket 发送器
     * @code {.cpp}
     * // 使用示例
     * auto sender = rm::Sender(rm::ip::udp::v4());
     * @endcode
     *
     * @param[in] protocol 协议，如 rm::ip::udp::v4()
     * @param[in] blocking 是否为阻塞模式，默认 `true` 阻塞
     */
    explicit Sender(const ip::Protocol &protocol, bool blocking = true) : Sender(protocol, blocking, false) {}

    ~Sender();

    /**
     * @brief 同步绑定 Socket（阻塞）
     * @code {.cpp}
     * // 使用示例
     * auto socket = sender.create();
     * @endcode
     *
     * @return 数据报式 Socket 会话对象
     */
    DgramSocket create();

protected:
    Sender(const ip::Protocol &protocol, bool blocking, bool ov);

    ip::Protocol _protocol;          //!< 协议
    SocketFd _fd{INVALID_SOCKET_FD}; //!< 未建立会话的 Socket 描述符
};

/**
 * @brief 同步数据报式 Socket 监听器
 * @details
 * - 用于将数据报 Socket 绑定到指定端点，并返回新的 Socket 会话
 * - Listener 拥有 Sender 的全部功能，在仅需要发送数据时可使用 Sender
 */
class Listener {
public:
    /**
     * @brief 创建数据报式 Socket 监听器
     * @code {.cpp}
     * // 使用示例
     * auto listener = rm::Listener(rm::Endpoint(rm::ip::udp::v4(), 12345));
     * @endcode
     *
     * @param[in] endpoint 端点
     * @param[in] blocking 是否为阻塞模式，默认 `true` 阻塞
     */
    explicit Listener(const Endpoint &endpoint, bool blocking = true) : Listener(endpoint, blocking, false) {}

    ~Listener();

    /**
     * @brief 同步绑定 Socket（阻塞）
     * @code {.cpp}
     * // 使用示例
     * auto socket = listener.create();
     * @endcode
     *
     * @return 数据报式 Socket 会话对象
     */
    DgramSocket create();

protected:
    Listener(const Endpoint &endpoint, bool blocking, bool ov);

    Endpoint _endpoint;              //!< 端点
    SocketFd _fd{INVALID_SOCKET_FD}; //!< 未建立会话的 Socket 描述符
};

//! 由 rm::Acceptor 或 rm::Connector 建立的流式 Socket 会话
class StreamSocket {
public:
    //! @cond
    explicit StreamSocket(SocketFd fd) : _fd(fd) {}
    StreamSocket(const StreamSocket &) = delete;
    StreamSocket(StreamSocket &&other) noexcept : _fd(std::exchange(other._fd, INVALID_SOCKET_FD)) {}

    StreamSocket &operator=(const StreamSocket &) = delete;
    StreamSocket &operator=(StreamSocket &&other) noexcept;

    ~StreamSocket();
    //! @endcond

    //! 会话是否失效
    [[nodiscard]] bool invalid() const noexcept { return _fd == INVALID_SOCKET_FD; }

    /**
     * @brief 设置 Socket 选项
     *
     * @param[in] opt Socket 选项
     */
    template <typename SockOpt>
    void setOption(const SockOpt &opt);

    //! 获取绑定的端点
    Endpoint endpoint() const;

    /**
     * @brief 同步读取已连接的 Socket 中的数据（阻塞）
     * @code {.cpp}
     * // 使用示例
     * auto str = socket.read();
     * @endcode
     *
     * @return 使用 `std::string` 存储的读取到的数据
     */
    std::string read() noexcept;

    /**
     * @brief 同步写入数据到已连接的 Socket 中（阻塞）
     * @code {.cpp}
     * // 使用示例
     * bool success = socket.write("Hello, World!");
     * @endcode
     *
     * @param[in] data 待写入的数据
     * @return 是否写入成功
     */
    bool write(std::string_view data) noexcept;

protected:
    SocketFd _fd{INVALID_SOCKET_FD}; //!< 会话文件描述符
};

/**
 * @brief Socket 接受器
 * @details 用于监听端口并接受连接请求，并返回新的 Socket 会话，常用于服务器端
 * @code {.cpp}
 * rm::Acceptor acceptor(rm::Endpoint(rm::ip::tcp::v4(), 8080));
 * @endcode
 */
class Acceptor {
public:
    /**
     * @brief 创建 Socket 接受器
     *
     * @param[in] endpoint 端点
     * @param[in] blocking 是否为阻塞模式，默认 `true` 阻塞
     */
    explicit Acceptor(const Endpoint &endpoint, bool blocking = true) : Acceptor(endpoint, blocking, false) {}

    ~Acceptor();

    /**
     * @brief 同步接受连接（阻塞）
     * @code {.cpp}
     * // 使用示例
     * auto socket = acceptor.accept();
     * @endcode
     *
     * @return Socket 会话对象
     */
    StreamSocket accept();

protected:
    Acceptor(const Endpoint &endpoint, bool blocking, bool ov);

    Endpoint _endpoint;              //!< 端点
    SocketFd _fd{INVALID_SOCKET_FD}; //!< 未建立会话的 Socket 描述符
};

/**
 * @brief Socket 连接器
 * @details 用于连接到远程或本地服务器，建立会话，常用于客户端
 * @code {.cpp}
 * // 连接远程服务器
 * rm::Connector c1(rm::Endpoint(rm::ip::tcp::v4(), 8080), "127.0.0.1");
 * // 连接本地服务器
 * rm::Connector c2(rm::Endpoint(rm::ipc::stream(), "/tmp/socket"));
 * @endcode
 */
class Connector {
public:
    /**
     * @brief 创建 Socket 连接器
     *
     * @param[in] endpoint 指向连接的服务器的端点
     * @param[in] url 目标地址，建立网络连接时有效，默认为 `127.0.0.1`
     * @param[in] blocking 是否为阻塞模式，默认 `true` 阻塞
     */
    explicit Connector(const Endpoint &endpoint, std::string_view url = "127.0.0.1", bool blocking = true) : Connector(endpoint, url, blocking, false) {}
    ~Connector();

    /**
     * @brief 同步连接（阻塞）
     * @code {.cpp}
     * // 使用示例
     * auto socket = connector.connect();
     * @endcode
     *
     * @return Socket 会话对象
     */
    StreamSocket connect();

protected:
    Connector(const Endpoint &endpoint, std::string_view url, bool blocking, bool ov);

    std::string _url;                //!< 目标地址
    Endpoint _endpoint;              //!< 端点
    SocketFd _fd{INVALID_SOCKET_FD}; //!< 未建立会话的 Socket 描述符
};

//! @} io_net

#if __cplusplus >= 202002L

namespace async {

//! @addtogroup io_net
//! @{

//! 由 rm::async::Listener 建立的数据报式 Socket 异步会话
class DgramSocket : public ::rm::DgramSocket {
public:
    //! @cond
    DgramSocket(IOContext &io_context, SocketFd fd);
    DgramSocket(const DgramSocket &) = delete;
    DgramSocket(DgramSocket &&other) noexcept = default;

    DgramSocket &operator=(const DgramSocket &) = delete;
    DgramSocket &operator=(DgramSocket &&other) = default;

    ~DgramSocket() = default;
    //! @endcond

    //! 数据报式 Socket 异步读等待器
    class SocketReadAwaiter final : public AsyncReadAwaiter {
    public:
        /**
         * @brief 创建 Socket 异步读等待器
         *
         * @param[in] ctx 异步 I/O 执行上下文
         * @param[in] fd 需要监听的文件描述符
         */
        SocketReadAwaiter(IOContext &ctx, SocketFd fd) : AsyncReadAwaiter(ctx, FileDescriptor(fd)) {}

        //! @cond
#ifdef _WIN32
        void await_suspend(std::coroutine_handle<> handle);
#endif
        std::tuple<std::string, std::string, uint16_t> await_resume() noexcept;
        //! @endcond
    };

    /**
     * @brief 异步读取已连接的 Socket 中的数据
     * @code {.cpp}
     * // 使用示例
     * auto [data, addr, port] = co_await socket.read();
     * @endcode
     *
     * @return 读取到的数据
     * @retval data 读取的数据
     * @retval addr 发送方地址
     * @retval port 发送方端口
     */
    SocketReadAwaiter read() { return {_ctx, _fd}; }

    //! 数据报式 Socket 异步写等待器
    class SocketWriteAwaiter final : public AsyncWriteAwaiter {
    public:
        /**
         * @brief 创建流式 Socket 异步写等待器
         *
         * @param[in] ctx 异步 I/O 执行上下文
         * @param[in] fd 需要监听的文件描述符
         * @param[in] addr 目标地址
         * @param[in] ep 目标端点
         * @param[in] data 待写入的数据
         */
        SocketWriteAwaiter(IOContext &ctx, SocketFd fd, std::string_view addr, const Endpoint &ep, std::string_view data) : AsyncWriteAwaiter(ctx, FileDescriptor(fd), data), _addr(addr), _endpoint(ep) {}

        /**
         * @brief 创建流式 Socket 异步写等待器
         *
         * @param[in] ctx 异步 I/O 执行上下文
         * @param[in] fd 需要监听的文件描述符
         * @param[in] addr 目标地址
         * @param[in] ep 目标端点
         * @param[in] data 待写入的数据
         */
        SocketWriteAwaiter(IOContext &ctx, SocketFd fd, std::array<uint8_t, 4> addr, const Endpoint &ep, std::string_view data);

        //! @cond
#ifdef _WIN32
        void await_suspend(std::coroutine_handle<> handle);
#else
        bool await_resume();
#endif
        //! @endcond

    private:
        std::string_view _addr;
        Endpoint _endpoint;
    };

    /**
     * @brief 异步写入数据到已连接的 Socket 中
     * @code {.cpp}
     * // 使用示例
     * bool success = co_await socket.write("192.168.1.100", rm::Endpoint(rm::ip::udp::v4(), 12345), "Hello, World!");
     * @endcode
     *
     * @param[in] addr 目标地址
     * @param[in] endpoint 目标端点
     * @param[in] data 待写入的数据
     * @return 是否写入成功
     */
    SocketWriteAwaiter write(std::string_view addr, const Endpoint &endpoint, std::string_view data) { return {_ctx, _fd, addr, endpoint, data}; }

    /**
     * @brief 异步写入数据到的 IPv4 的 Socket 中
     * @code {.cpp}
     * // 使用示例
     * bool success = co_await socket.write({192, 168, 1, 100}, rm::Endpoint(rm::ip::udp::v4(), 12345), "Hello, World!");
     * @endcode
     *
     * @param[in] addr 使用数组形式表示的 IPv4 目标地址
     * @param[in] endpoint 目标端点
     * @param[in] data 待写入的数据
     * @return 是否写入成功
     */
    SocketWriteAwaiter write(std::array<uint8_t, 4> addr, const Endpoint &endpoint, std::string_view data) { return {_ctx, _fd, addr, endpoint, data}; }

private:
    IOContextRef _ctx; //!< 异步 I/O 执行上下文
};

//! 异步数据报式 Socket 发送器
class Sender : public ::rm::Sender {
public:
    /**
     * @brief 创建异步数据报式 Socket 发送器
     * @code {.cpp}
     * // 使用示例
     * auto listener = rm::async::Sender(io_context, rm::ip::udp::v4());
     * @endcode
     *
     * @param[in] io_context 异步 I/O 执行上下文
     * @param[in] protocol 协议，如 rm::ip::udp::v4()
     */
    Sender(IOContext &io_context, const ip::Protocol &protocol);
    ~Sender() = default;

    /**
     * @brief 异步绑定 Socket
     * @code {.cpp}
     * // 使用示例
     * auto socket = listener.create(); // 注意：此处不是异步操作，不得使用 co_await !!!
     * @endcode
     *
     * @return 数据报式 Socket 会话对象
     */
    DgramSocket create();

private:
    IOContextRef _ctx; //!< 异步 I/O 执行上下文
};

/**
 * @brief 异步数据报式 Socket 监听器
 * - async::Listener 拥有 async::Sender 的全部功能，在仅需要发送数据时可使用 async::Sender
 */
class Listener : public ::rm::Listener {
public:
    /**
     * @brief 创建异步数据报式 Socket 监听器
     * @code {.cpp}
     * // 使用示例
     * auto listener = rm::async::Listener(io_context, rm::Endpoint(rm::ip::udp::v4(), 12345));
     * @endcode
     *
     * @param[in] io_context 异步 I/O 执行上下文
     * @param[in] endpoint 端点
     */
    Listener(IOContext &io_context, const Endpoint &endpoint);
    ~Listener() = default;

    /**
     * @brief 异步绑定 Socket
     * @code {.cpp}
     * // 使用示例
     * auto socket = listener.create(); // 注意：此处不是异步操作，不得使用 co_await !!!
     * @endcode
     *
     * @return 数据报式 Socket 会话对象
     */
    DgramSocket create();

private:
    IOContextRef _ctx; //!< 异步 I/O 执行上下文
};

//! 由 rm::async::Acceptor 建立的流式 Socket 异步会话
class StreamSocket : public ::rm::StreamSocket {
public:
    //! @cond
    StreamSocket(IOContext &io_context, SocketFd fd);
    StreamSocket(const StreamSocket &) = delete;
    StreamSocket(StreamSocket &&other) noexcept = default;

    StreamSocket &operator=(const StreamSocket &) = delete;
    StreamSocket &operator=(StreamSocket &&other) = default;

    ~StreamSocket() = default;
    //! @endcond

    //! 流式 Socket 异步读等待器
    class SocketReadAwaiter final : public AsyncReadAwaiter {
    public:
        /**
         * @brief 创建 Socket 异步读等待器
         *
         * @param[in] ctx 异步 I/O 执行上下文
         * @param[in] fd 需要监听的文件描述符
         */
        SocketReadAwaiter(IOContext &ctx, SocketFd fd) : AsyncReadAwaiter(ctx, FileDescriptor(fd)) {}

        //! @cond
#ifdef _WIN32
        void await_suspend(std::coroutine_handle<> handle);
#else
        std::string await_resume();
#endif
        //! @endcond
    };

    /**
     * @brief 异步读取已连接的 Socket 中的数据
     * @code {.cpp}
     * // 使用示例
     * auto str = co_await socket.read();
     * @endcode
     *
     * @return 使用 `std::string` 存储的读取到的数据
     */
    SocketReadAwaiter read() { return {_ctx, _fd}; }

    //! 流式 Socket 异步写等待器
    class SocketWriteAwaiter final : public AsyncWriteAwaiter {
    public:
        /**
         * @brief 创建流式 Socket 异步写等待器
         *
         * @param[in] ctx 异步 I/O 执行上下文
         * @param[in] fd 需要监听的文件描述符
         * @param[in] data 待写入的数据
         */
        SocketWriteAwaiter(IOContext &ctx, SocketFd fd, std::string_view data) : AsyncWriteAwaiter(ctx, FileDescriptor(fd), data) {}

        //! @cond
#ifdef _WIN32
        void await_suspend(std::coroutine_handle<> handle);
#else
        bool await_resume();
#endif
        //! @endcond
    };

    /**
     * @brief 异步写入数据到已连接的 Socket 中
     * @code {.cpp}
     * // 使用示例
     * bool success = co_await socket.write("Hello, World!");
     * @endcode
     *
     * @param[in] data 待写入的数据
     * @return 是否写入成功
     */
    SocketWriteAwaiter write(std::string_view data) { return {_ctx, _fd, data}; }

private:
    IOContextRef _ctx; //!< 异步 I/O 执行上下文
};

/**
 * @brief 异步流式 Socket 接受器
 * @details 用于监听端口并接受连接请求，常用于服务器端
 * @code {.cpp}
 * auto io_context = rm::IOContext();
 * rm::async::Acceptor acceptor(io_context, rm::Endpoint(rm::ip::tcp::v4(), 8080));
 * @endcode
 * @note 异步流式 Socket 接受器需要在异步 I/O 执行上下文中使用协程来处理
 */
class Acceptor : public ::rm::Acceptor {
public:
    /**
     * @brief 创建异步流式 Socket 接受器
     *
     * @param[in] io_context 异步 I/O 执行上下文
     * @param[in] endpoint 端点
     */
    Acceptor(IOContext &io_context, const Endpoint &endpoint);
    ~Acceptor() = default;

    //! 接受等待器
    class AcceptAwaiter : public AsyncReadAwaiter {
    public:
        AcceptAwaiter(IOContext &ctx, SocketFd fd, const Endpoint &ep) : AsyncReadAwaiter(ctx, FileDescriptor(fd)), _ctx(ctx), _endpoint(ep) {}

        //! @cond
#ifdef _WIN32
        void await_suspend(std::coroutine_handle<> handle);
#endif
        StreamSocket await_resume() noexcept;
        //! @endcond
    private:
        IOContextRef _ctx;  //!< 异步 I/O 执行上下文
        Endpoint _endpoint; //!< 端点
    };

    /**
     * @brief 异步接受连接
     * @code {.cpp}
     * // 使用示例
     * auto socket = co_await acceptor.accept();
     * @endcode
     *
     * @return Socket 会话对象
     */
    AcceptAwaiter accept() { return AcceptAwaiter(_ctx, _fd, _endpoint); }

private:
    IOContextRef _ctx; //!< 异步 I/O 执行上下文
};

/**
 * @brief 异步流式 Socket 连接器
 * @details 用于连接到远程服务器，建立会话，常用于客户端
 * @code {.cpp}
 * auto io_context = rm::IOContext();
 * rm::Connector connector(io_context, "localhost", rm::Endpoint(rm::ip::tcp::v4(), 8080));
 * @endcode
 */
class Connector : public ::rm::Connector {
public:
    /**
     * @brief 创建异步 Socket 连接器
     *
     * @param[in] io_context 异步 I/O 执行上下文
     * @param[in] endpoint 端点
     * @param[in] url 目标地址
     */
    Connector(IOContext &io_context, const Endpoint &endpoint, std::string_view url = "");
    ~Connector() = default;

    //! 连接等待器
    class ConnectAwaiter : public AsyncWriteAwaiter {
    public:
        ConnectAwaiter(IOContext &ctx, SocketFd fd, const Endpoint &ep, std::string_view url)
            : AsyncWriteAwaiter(ctx, FileDescriptor(fd), url), _ctx(ctx), _endpoint(ep) {}

        //! @cond
#ifdef _WIN32
        void await_suspend(std::coroutine_handle<> handle);
#endif
        StreamSocket await_resume() noexcept;
        //! @endcond

    private:
        IOContextRef _ctx;  //!< 异步 I/O 执行上下文
        Endpoint _endpoint; //!< 端点
    };

    /**
     * @brief 异步连接
     * @code {.cpp}
     * // 使用示例
     * auto socket = co_await connector.connect();
     * @endcode
     *
     * @return Socket 会话对象
     */
    ConnectAwaiter connect() { return ConnectAwaiter(_ctx, _fd, _endpoint, _url); }

private:
    IOContextRef _ctx; //!< 异步 I/O 执行上下文
};

//! @} io_net

} // namespace async

#endif // __cplusplus >= 202002L

} // namespace rm

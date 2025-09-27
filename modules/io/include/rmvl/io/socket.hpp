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

#include <cstdint>
#include <string_view>
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

//! 进程间通信协议族
struct ipc {
    //! 构造端点，以表示本地流式协议
    static ipc stream();
    //! 构造端点，以表示本地数据报协议
    static ipc packet();
    //! 协议族
    int family{};
    //! Socket 类型
    int type{};
};

//! IP 协议族
struct ip {
    //! TCP 协议
    struct tcp {
        //! 构造端点，以表示 IPv4 TCP 协议
        static tcp v4();
        //! 构造端点，以表示 IPv6 TCP 协议
        static tcp v6();
        //! 协议族
        int family{};
        //! Socket 类型
        int type{};
    };

    //! UDP 协议
    struct udp {
        //! 构造端点，以表示 IPv4 UDP 协议
        static udp v4();
        //! 构造端点，以表示 IPv6 UDP 协议
        static udp v6();
        //! 获取协议族
        int family{};
        //! 获取 Socket 类型
        int type{};
    };
};

#if __cplusplus >= 202002L

/**
 * @brief IP 协议
 * @details 必须为 rm::ip::tcp 或 rm::ip::udp
 */
template <typename Tp>
concept IpProtocol = std::same_as<Tp, ip::tcp> || std::same_as<Tp, ip::udp>;

/**
 * @brief 本地协议
 * @details 必须为 rm::ipc
 */
template <typename Tp>
concept LocalProtocol = std::same_as<Tp, ipc>;

#endif

//! 端点
class Endpoint {
public:
    /**
     * @brief 构造网络 Socket 端点
     *
     * @param[in] ip 网络协议
     * @param[in] port 端口号
     * @code {.cpp}
     * Endpoint ep(ip::tcp::v4(), 8080);
     * @endcode
     */
#if __cplusplus >= 202002L
    Endpoint(IpProtocol auto ip, uint16_t port) : _family(ip.family), _type(ip.type), _port(port) {}
#else
    template <typename Tp>
    Endpoint(const Tp &ip, uint16_t port) : _family(ip.family), _type(ip.type), _port(port) {}
#endif

    /**
     * @brief 构造本地 Socket 端点
     *
     * @param[in] lp 本地协议
     * @param[in] path 路径
     * @code {.cpp}
     * Endpoint ep(ipc::stream(), "/tmp/socket");
     * @endcode
     */
#if __cplusplus >= 202002L
    Endpoint(LocalProtocol auto lp, std::string_view path) : _family(lp.family), _type(lp.type), _path(path) {}
#else
    template <typename Tp>
    Endpoint(const Tp &lp, std::string_view path) : _family(lp.family), _type(lp.type), _path(path) {}
#endif

    //! 获取协议族
    int family() const { return _family; }
    //! 获取 Socket 类型
    int type() const { return _type; }
    //! 获取端口号
    uint16_t port() const { return _port; }
    //! 获取路径，仅适用于本地 Socket
    std::string path() const { return _path; }

private:
    int _family{};       //!< 协议族
    int _type{};         //!< Socket 类型
    uint16_t _port{};    //!< 端口号，仅适用于网络 Socket
    std::string _path{}; //!< 路径，仅适用于本地 Socket
};

#ifdef _WIN32

/**
 * @brief Socket 环境管理
 * @note
 * - Windows 环境专属
 * - 一般无需手动管理，在第一次创建 `rm::Socket`、`rm::Acceptor`、`rm::Connector` 及其派生类时自动初始化
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

//! Socket 会话层
class Socket {
public:
    /**
     * @brief 建立 Socket 会话
     *
     * @param[in] fd 会话文件描述符，一般是已建立会话的 Socket 描述符
     */
    Socket(SocketFd fd) : _fd(fd) {}
    Socket(const Socket &) = delete;
    Socket(Socket &&other) noexcept : _fd(std::exchange(other._fd, INVALID_SOCKET_FD)) {}

    Socket &operator=(const Socket &) = delete;
    Socket &operator=(Socket &&other) noexcept;

    //! 会话是否失效
    [[nodiscard]] bool invalid() const noexcept { return _fd == INVALID_SOCKET_FD; }

    ~Socket();

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
     */
    explicit Acceptor(const Endpoint &endpoint) : Acceptor(endpoint, false) {}

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
    Socket accept();

protected:
    Acceptor(const Endpoint &endpoint, bool ov);

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
 * rm::Connector c2(rm::Endpoint(rm::ipc::packet(), "/tmp/socket"));
 * @endcode
 */
class Connector {
public:
    /**
     * @brief 创建 Socket 连接器
     *
     * @param[in] endpoint 端点
     * @param[in] url 目标地址，建立网络连接时有效，默认为 `127.0.0.1`
     */
    explicit Connector(const Endpoint &endpoint, std::string_view url = "127.0.0.1") : Connector(endpoint, url, false) {}
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
    Socket connect();

protected:
    Connector(const Endpoint &endpoint, std::string_view url, bool ov);

    std::string _url;                //!< 目标地址
    Endpoint _endpoint;              //!< 端点
    SocketFd _fd{INVALID_SOCKET_FD}; //!< 未建立会话的 Socket 描述符
};

//! @} io_net

#if __cplusplus >= 202002L

namespace async {

//! @addtogroup io_net
//! @{

//! Socket 异步会话层
class Socket : public ::rm::Socket {
public:
    /**
     * @brief 建立 Socket 异步会话
     *
     * @param[in] io_context 异步 I/O 执行上下文
     * @param[in] fd 会话文件描述符，一般是已建立会话的 Socket 描述符
     */
    Socket(IOContext &io_context, SocketFd fd);
    Socket(const Socket &) = delete;
    Socket(Socket &&other) noexcept = default;

    Socket &operator=(const Socket &) = delete;
    Socket &operator=(Socket &&other) = default;

    ~Socket() = default;

    //! Socket 异步读等待器
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

    //! Socket 异步写等待器
    class SocketWriteAwaiter final : public AsyncWriteAwaiter {
    public:
        /**
         * @brief 创建 Socket 异步写等待器
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
 * @brief 异步 Socket 接受器
 * @details 用于监听端口并接受连接请求，常用于服务器端
 * @code {.cpp}
 * auto io_context = rm::IOContext();
 * rm::async::Acceptor acceptor(io_context, rm::Endpoint(rm::ip::tcp::v4(), 8080));
 * @endcode
 * @note 异步 Socket 接受器需要在异步 I/O 执行上下文中使用协程来处理
 */
class Acceptor : public ::rm::Acceptor {
public:
    /**
     * @brief 创建异步 Socket 接受器
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
        Socket await_resume() noexcept;
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
 * @brief 异步 Socket 连接器
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
        Socket await_resume() noexcept;
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

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

#pragma comment(lib, "Ws2_32.lib")
#else
#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/socket.h>
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

ipc ipc::stream() { return {AF_UNIX, SOCK_STREAM}; }
ipc ipc::packet() { return {AF_UNIX, SOCK_DGRAM}; }
ip::tcp ip::tcp::v4() { return {AF_INET, SOCK_STREAM}; }
ip::tcp ip::tcp::v6() { return {AF_INET6, SOCK_STREAM}; }
ip::udp ip::udp::v4() { return {AF_INET, SOCK_DGRAM}; }
ip::udp ip::udp::v6() { return {AF_INET6, SOCK_DGRAM}; }

Socket &Socket::operator=(Socket &&other) noexcept {
    _fd = std::exchange(other._fd, INVALID_SOCKET_FD);
    return *this;
}

#ifdef _WIN32
Acceptor::Acceptor(const Endpoint &ep, bool ov) : _endpoint(ep) {
    SocketEnv::ensure_init();
    _fd = ov ? WSASocket(ep.family(), ep.type(), 0, NULL, 0, WSA_FLAG_OVERLAPPED) : socket(ep.family(), ep.type(), 0);
#else
Acceptor::Acceptor(const Endpoint &ep, bool) : _endpoint(ep), _fd(socket(ep.family(), ep.type(), 0)) {
#endif
    RMVL_Assert(_fd != INVALID_SOCKET_FD);

    // 本地 Socket
    if (ep.family() == AF_UNIX) {
        sockaddr_un addr{};
        addr.sun_family = AF_UNIX;
        strcpy(addr.sun_path, ep.path().c_str());
        unlink(addr.sun_path);
        if (bind(_fd, reinterpret_cast<sockaddr *>(&addr), sizeof(addr)) < 0)
            RMVL_Error_(RMVL_StsError, "bind failed, error code: %d", error_code());
    }
    // 网络 Socket
    else {
        // 设置允许重用处于 TIME_WAIT 状态的地址
        int opt = 1;
#ifdef _WIN32
        if (setsockopt(_fd, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<const char *>(&opt), sizeof(opt)) < 0)
#else
        if (setsockopt(_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
#endif
            WARNING_("setsockopt SO_REUSEADDR failed, error code: %d", error_code());
        // IPv4 Socket
        if (ep.family() == AF_INET) {
            sockaddr_in addr{};
            addr.sin_family = AF_INET;
            addr.sin_addr.s_addr = INADDR_ANY;
            addr.sin_port = htons(ep.port());
            if (bind(_fd, reinterpret_cast<sockaddr *>(&addr), sizeof(addr)) < 0)
                RMVL_Error_(RMVL_StsError, "bind failed, error code: %d", error_code());
        }
        // IPv6 Socket
        else if (ep.family() == AF_INET6) {
            sockaddr_in6 addr{};
            addr.sin6_family = AF_INET6;
            addr.sin6_addr = in6addr_any;
            addr.sin6_port = htons(ep.port());
            if (bind(_fd, reinterpret_cast<sockaddr *>(&addr), sizeof(addr)) < 0)
                RMVL_Error_(RMVL_StsError, "bind failed, error code: %d", error_code());
        } else
            RMVL_Error(RMVL_StsError, "Unsupported socket family");
    }

    if (_endpoint.type() == SOCK_STREAM) // 流式 Socket 需要监听
        if (listen(_fd, SOMAXCONN) < 0)
            RMVL_Error_(RMVL_StsError, "listen failed, error code: %d", error_code());
}

Socket Acceptor::accept() {
    // 流式 Socket 需要接受连接
    int sfd = _endpoint.type() == SOCK_STREAM ? ::accept(_fd, nullptr, nullptr) : _fd;
    return Socket(sfd);
}

static int fdconnect(int fd, const Endpoint &ep, std::string_view url) {
    // 本地 Socket
    if (ep.family() == AF_UNIX) {
        sockaddr_un addr{};
        addr.sun_family = ep.family();
#ifdef _WIN32
        strcpy_s(addr.sun_path, ep.path().c_str());
#else
        strcpy(addr.sun_path, ep.path().c_str());
#endif
        // 统一使用 connect 使包 Socket 可以使用 send
        if (connect(fd, reinterpret_cast<sockaddr *>(&addr), sizeof(addr)) < 0)
            RMVL_Error_(RMVL_StsError, "Connection failed, error code: %d", error_code());
    }
    // IPv4 Socket
    else if (ep.family() == AF_INET) {
        sockaddr_in addr{};
        addr.sin_family = ep.family();
        addr.sin_port = htons(ep.port());
        if (inet_pton(AF_INET, url.data(), &addr.sin_addr) != 1)
            RMVL_Error(RMVL_StsError, "Invalid IPv4 address");
        // 统一使用 connect 使 UDP 可以使用 send
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
        // 统一使用 connect 使 UDP 可以使用 send
        if (connect(fd, reinterpret_cast<sockaddr *>(&addr), sizeof(addr)) < 0)
            RMVL_Error_(RMVL_StsError, "Connection failed, error code: %d", error_code());
    } else
        RMVL_Error(RMVL_StsError, "Unsupported socket family");
    return fd;
}

#ifdef _WIN32
Connector::Connector(const Endpoint &ep, std::string_view url, bool ov) : _url(url), _endpoint(ep) {
    SocketEnv::ensure_init();
    _fd = ov ? WSASocket(ep.family(), ep.type(), 0, NULL, 0, WSA_FLAG_OVERLAPPED) : socket(ep.family(), ep.type(), 0);
#else
Connector::Connector(const Endpoint &ep, std::string_view url, bool) : _url(url), _endpoint(ep), _fd(socket(ep.family(), ep.type(), 0)) {
#endif
    RMVL_Assert(_fd != INVALID_SOCKET_FD);
}

Socket Connector::connect() {
    int sfd = fdconnect(_fd, _endpoint, _url);
    return Socket(sfd);
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

std::string Socket::read() noexcept {
    char buf[1024]{};
    int n = ::recv(_fd, buf, sizeof(buf), 0);
    if (n == SOCKET_ERROR) {
        int error = WSAGetLastError();
        if (error != WSAEWOULDBLOCK)
            WARNING_("Socket read failed, error: %d", error);
        return "";
    }
    return n > 0 ? std::string(buf, n) : std::string{};
}

bool Socket::write(std::string_view data) noexcept {
    int n = ::send(_fd, data.data(), data.size(), 0);
    if (n == SOCKET_ERROR) {
        int error = WSAGetLastError();
        if (error != WSAEWOULDBLOCK)
            WARNING_("Socket write failed, error: %d", error);
        return false;
    }
    return n == static_cast<int>(data.size());
}

Socket::~Socket() { valid(_fd) ? ::closesocket(_fd) : 0; }
Acceptor::~Acceptor() { valid(_fd) ? ::closesocket(_fd) : 0; }
Connector::~Connector() { valid(_fd) ? ::closesocket(_fd) : 0; }

#else

std::string Socket::read() noexcept {
    char buf[1024]{};
    ssize_t n = ::recv(_fd, buf, sizeof(buf), 0);
    return n > 0 ? std::string(buf, n) : std::string{};
}

bool Socket::write(std::string_view data) noexcept {
    ssize_t n = ::send(_fd, data.data(), data.size(), 0);
    return n == static_cast<ssize_t>(data.size());
}

Socket::~Socket() { valid(_fd) ? ::close(_fd) : 0; }
Acceptor::~Acceptor() { valid(_fd) ? ::close(_fd) : 0; }
Connector::~Connector() { valid(_fd) ? ::close(_fd) : 0; }

#endif

#if __cplusplus >= 202002L

namespace async {

#ifdef _WIN32

Socket::Socket(IOContext &io_context, SocketFd fd) : ::rm::Socket(fd), _ctx(io_context) {
    if (CreateIoCompletionPort((HANDLE)_fd, _ctx.get().handle(), 0, 0) == nullptr) {
        auto err = GetLastError();
        if (err != ERROR_INVALID_PARAMETER)
            RMVL_Error_(RMVL_StsError, "Associate fd with IOCP failed: %lu", err);
    }
}

void Socket::SocketReadAwaiter::await_suspend(std::coroutine_handle<> handle) {
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

void Socket::SocketWriteAwaiter::await_suspend(std::coroutine_handle<> handle) {
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

    // 创建新 socket 并存储至重叠 I/O 的 buf 中
    auto sfd = WSASocket(_endpoint.family(), _endpoint.type(), 0, NULL, 0, WSA_FLAG_OVERLAPPED);
    if (sfd == INVALID_SOCKET)
        RMVL_Error_(RMVL_StsBadArg, "Create accept socket failed: %d", WSAGetLastError());
    new (_ovl->buf) SOCKET{sfd};

    // 获取 AcceptEx
    LPFN_ACCEPTEX acceptEx{nullptr};
    GUID guid = WSAID_ACCEPTEX;
    DWORD bytes{0};
    if (WSAIoctl((SOCKET)_fd, SIO_GET_EXTENSION_FUNCTION_POINTER, &guid, sizeof(guid),
                 &acceptEx, sizeof(acceptEx), &bytes, NULL, NULL) == SOCKET_ERROR)
        RMVL_Error_(RMVL_StsBadArg, "Get AcceptEx function failed: %d", WSAGetLastError());

    // 异步 accept 提交至 IOCP
    DWORD received{};
    if (!acceptEx((SOCKET)_fd, sfd,
                  _ovl->buf + sizeof(SOCKET), 0, sizeof(sockaddr_storage) + 16,
                  sizeof(sockaddr_storage) + 16, &received, &_ovl->ov)) {
        DWORD error = WSAGetLastError();
        if (error != ERROR_IO_PENDING) {
            closesocket(sfd);
            RMVL_Error_(RMVL_StsBadArg, "AcceptEx failed with error: %lu", error);
        }
    }
}

Socket Acceptor::AcceptAwaiter::await_resume() noexcept {
    RMVL_DbgAssert(_fd != INVALID_FD);
    SOCKET sfd = *reinterpret_cast<SOCKET *>(_ovl->buf);

    // 更新 socket 上下文
    if (setsockopt(sfd, SOL_SOCKET, SO_UPDATE_ACCEPT_CONTEXT, (char *)&_fd, sizeof(_fd)) == SOCKET_ERROR)
        WARNING_("setsockopt SO_UPDATE_ACCEPT_CONTEXT failed: %d", WSAGetLastError());
    return async::Socket(_ctx, sfd);
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
    if (_endpoint.family() == AF_UNIX) {
        sockaddr_un local{}, *addr{reinterpret_cast<sockaddr_un *>(&remote)};
        local.sun_family = AF_UNIX;
        local.sun_path[0] = '\0';
        if (bind((SOCKET)_fd, reinterpret_cast<sockaddr *>(&local), sizeof(local)) == SOCKET_ERROR)
            RMVL_Error_(RMVL_StsBadArg, "Bind for ConnectEx failed: %d", WSAGetLastError());

        addr->sun_family = AF_UNIX;
        strcpy_s(addr->sun_path, _endpoint.path().c_str());
        addr_len = sizeof(sockaddr_un);
    } else if (_endpoint.family() == AF_INET) {
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

Socket Connector::ConnectAwaiter::await_resume() noexcept {
    RMVL_DbgAssert(_fd != INVALID_FD);
    SOCKET sfd = (SOCKET)_fd;
    // 更新 socket 上下文
    if (setsockopt(sfd, SOL_SOCKET, SO_UPDATE_CONNECT_CONTEXT, NULL, 0) == SOCKET_ERROR)
        WARNING_("setsockopt SO_UPDATE_CONNECT_CONTEXT failed: %d", WSAGetLastError());
    return async::Socket(_ctx, sfd);
}

#else

Socket::Socket(IOContext &io_context, SocketFd fd) : ::rm::Socket(fd), _ctx(io_context) {}

std::string Socket::SocketReadAwaiter::await_resume() {
    RMVL_DbgAssert(_fd != INVALID_FD);
    epoll_ctl(_aioh, EPOLL_CTL_DEL, _fd, nullptr);
    char buf[1024]{};
    ssize_t n = ::recv(_fd, buf, sizeof(buf), 0);
    return n > 0 ? std::string(buf, n) : std::string{};
}

bool Socket::SocketWriteAwaiter::await_resume() {
    RMVL_DbgAssert(_fd != INVALID_FD);
    epoll_ctl(_aioh, EPOLL_CTL_DEL, _fd, nullptr);
    return ::send(_fd, _data.data(), _data.size(), 0) == static_cast<ssize_t>(_data.size());
}

Acceptor::Acceptor(IOContext &io_context, const Endpoint &endpoint) : ::rm::Acceptor(endpoint, true), _ctx(io_context) {}

Socket Acceptor::AcceptAwaiter::await_resume() noexcept {
    RMVL_DbgAssert(_fd != INVALID_FD);
    epoll_ctl(_aioh, EPOLL_CTL_DEL, _fd, nullptr);
    int sfd = _endpoint.type() == SOCK_STREAM ? ::accept(_fd, nullptr, nullptr) : _fd;
    return async::Socket(_ctx, sfd);
}

Connector::Connector(IOContext &io_context, const Endpoint &endpoint, std::string_view url) : ::rm::Connector(endpoint, url, true), _ctx(io_context) {}

Socket Connector::ConnectAwaiter::await_resume() noexcept {
    RMVL_DbgAssert(_fd != INVALID_FD);
    epoll_ctl(_aioh, EPOLL_CTL_DEL, _fd, nullptr);
    int sfd = fdconnect(_fd, _endpoint, _data);
    return Socket(_ctx, sfd);
}

#endif

} //  namespace async

#endif // __cplusplus >= 202002L

} // namespace rm

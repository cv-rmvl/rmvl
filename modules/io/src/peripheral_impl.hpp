/**
 * @file peripheral_impl.hpp
 * @author zhaoxi (535394140@qq.com)
 * @brief Linux 外设文件描述符管理与 IO 就绪等待的内部实现
 * @version 1.0
 * @date 2026-10-13
 *
 * @copyright Copyright 2026 (c), zhaoxi
 *
 */

#pragma once

#ifdef __linux__

#include <poll.h>
#include <unistd.h>
#if __cplusplus >= 202002L
#include <sys/epoll.h>
#endif

#include <cerrno>
#include <system_error>

#include "rmvl/io/async.hpp"

namespace rm::io_details {

inline void fail(const char *what) { throw std::system_error(errno, std::generic_category(), what); }

struct Fd {
    int value{-1};
    explicit Fd(int fd = -1) : value(fd) {}
    ~Fd() {
        if (value >= 0)
            ::close(value);
    }
    Fd(const Fd &) = delete;
    Fd &operator=(const Fd &) = delete;
};

inline std::string path(std::string_view value) {
    if (value.empty() || value.find('\0') != std::string_view::npos)
        throw std::invalid_argument("invalid device path");
    return std::string(value);
}
inline void wait(int fd, short events) {
    pollfd p{fd, events, 0};
    int n;
    do {
        n = ::poll(&p, 1, -1);
    } while (n < 0 && errno == EINTR);
    if (n < 0)
        fail("poll peripheral");
    if (p.revents & POLLNVAL) {
        errno = EBADF;
        fail("poll peripheral");
    }
    if (!(p.revents & events)) {
        errno = EIO;
        fail("peripheral disconnected");
    }
}

} // namespace rm::io_details

#if __cplusplus >= 202002L

namespace rm::io_details {

// One registration owns its coroutine pointer, including cancellation cleanup.
class Ready {
    int _context, _fd;
    uint32_t _events;
    bool _registered{};

public:
    Ready(async::IOContext &ctx, int fd, uint32_t events) : _context(ctx.handle()), _fd(fd), _events(events) {}
    Ready(const Ready &) = delete;
    ~Ready() {
        if (_registered)
            ::epoll_ctl(_context, EPOLL_CTL_DEL, _fd, nullptr);
    }
    bool await_ready() const noexcept { return false; }
    void await_suspend(std::coroutine_handle<> h) {
        epoll_event event{};
        event.events = _events | EPOLLONESHOT;
        event.data.ptr = h.address();
        if (::epoll_ctl(_context, EPOLL_CTL_ADD, _fd, &event) < 0)
            fail("register peripheral");
        _registered = true;
    }
    void await_resume() {
        if (::epoll_ctl(_context, EPOLL_CTL_DEL, _fd, nullptr) < 0)
            fail("unregister peripheral");
        _registered = false;
    }
};
} // namespace rm::io_details
#endif
#endif

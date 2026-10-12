#ifdef __linux__

#include <fcntl.h>
#include <linux/can/raw.h>
#include <net/if.h>
#include <sys/socket.h>

#include <cstring>
#include <limits>

#include "peripheral_impl.hpp"
#include "rmvl/io/can.hpp"

namespace rm {

struct Can::Impl {
    io_details::Fd fd;
    CanReadMode mode;
    bool enable_fd;
    Impl(std::string_view name, CanReadMode m, bool enable) : mode(m), enable_fd(enable) {
        const auto interface = io_details::path(name);
        if (interface.size() >= IFNAMSIZ || (m != CanReadMode::BLOCK && m != CanReadMode::NONBLOCK))
            throw std::invalid_argument("CAN interface or mode");
        const auto index = ::if_nametoindex(interface.c_str());
        if (!index)
            io_details::fail("CAN interface index");
        fd.value = ::socket(PF_CAN, SOCK_RAW | SOCK_CLOEXEC | SOCK_NONBLOCK, CAN_RAW);
        if (fd.value < 0)
            io_details::fail("CAN socket");
        if (enable) {
            const int yes = 1;
            if (::setsockopt(fd.value, SOL_CAN_RAW, CAN_RAW_FD_FRAMES, &yes, sizeof(yes)) < 0)
                io_details::fail("CAN FD enable");
        }
        sockaddr_can address{};
        address.can_family = AF_CAN;
        address.can_ifindex = static_cast<int>(index);
        if (::bind(fd.value, reinterpret_cast<sockaddr *>(&address), sizeof(address)) < 0)
            io_details::fail("bind CAN");
    }
};
Can::Can(std::string_view name, CanReadMode mode, bool enable) : _impl(std::make_shared<Impl>(name, mode, enable)) {}
Can::~Can() = default;
Can::Can(Can &&) noexcept = default;
Can &Can::operator=(Can &&) noexcept = default;
int Can::handle() const noexcept { return _impl ? _impl->fd.value : -1; }
bool Can::isOpened() const noexcept { return bool(_impl); }
std::optional<CanFrame> Can::read() const {
    if (!_impl)
        throw std::system_error(EBADF, std::generic_category(), "CAN moved from");
    for (;;) {
        canfd_frame native{};
        const auto n = ::recv(handle(), &native, sizeof(native), 0);
        if (n == CAN_MTU || n == CANFD_MTU) {
            CanFrame frame;
            frame.fd = n == CANFD_MTU;
            frame.extended = native.can_id & CAN_EFF_FLAG;
            frame.remote = native.can_id & CAN_RTR_FLAG;
            frame.error = native.can_id & CAN_ERR_FLAG;
            frame.id = native.can_id & (frame.error ? CAN_ERR_MASK : frame.extended ? CAN_EFF_MASK
                                                                                    : CAN_SFF_MASK);
            frame.size = native.len;
            if (frame.size > (frame.fd ? 64 : 8))
                throw std::system_error(EIO, std::generic_category(), "CAN frame length");
            if (frame.fd) {
                frame.bitrate_switch = native.flags & CANFD_BRS;
                frame.error_state_indicator = native.flags & CANFD_ESI;
            }
            if (!frame.remote)
                std::memcpy(frame.data.data(), native.data, frame.size);
            return frame;
        }
        if (n < 0 && errno == EINTR)
            continue;
        if (n < 0 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
            if (_impl->mode == CanReadMode::NONBLOCK)
                return std::nullopt;
            io_details::wait(handle(), POLLIN);
            continue;
        }
        if (n >= 0)
            errno = EIO;
        io_details::fail("receive CAN frame");
    }
}
// A frame is submitted atomically. Successful submission is not a bus acknowledgement.
bool Can::write(const CanFrame &frame) const {
    if (!_impl)
        throw std::system_error(EBADF, std::generic_category(), "CAN moved from");
    if (frame.error || frame.id > (frame.extended ? CAN_EFF_MASK : CAN_SFF_MASK) ||
        frame.size > (frame.fd ? 64 : 8) || (frame.fd && (!_impl->enable_fd || frame.remote)) ||
        (!frame.fd && (frame.bitrate_switch || frame.error_state_indicator)))
        throw std::invalid_argument("invalid CAN frame");
    if (frame.fd && frame.size > 8 && frame.size != 12 && frame.size != 16 && frame.size != 20 &&
        frame.size != 24 && frame.size != 32 && frame.size != 48 && frame.size != 64)
        throw std::invalid_argument("CAN FD wire length");
    canfd_frame native{};
    native.can_id = frame.id | (frame.extended ? CAN_EFF_FLAG : 0) | (frame.remote ? CAN_RTR_FLAG : 0);
    native.len = frame.size;
    if (frame.fd)
        native.flags = (frame.bitrate_switch ? CANFD_BRS : 0) | (frame.error_state_indicator ? CANFD_ESI : 0);
    if (!frame.remote)
        std::memcpy(native.data, frame.data.data(), frame.size);
    const size_t length = frame.fd ? CANFD_MTU : CAN_MTU;
    for (;;) {
        const auto n = ::send(handle(), &native, length, 0);
        if (n == static_cast<ssize_t>(length))
            return true;
        if (n < 0 && errno == EINTR)
            continue;
        if (n < 0 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
            if (_impl->mode == CanReadMode::NONBLOCK)
                return false;
            io_details::wait(handle(), POLLOUT);
            continue;
        }
        if (n >= 0)
            errno = EIO;
        io_details::fail("send CAN frame");
    }
}
void Can::setFilters(const std::vector<CanFilter> &filters) const {
    if (filters.size() > std::numeric_limits<socklen_t>::max() / sizeof(can_filter))
        throw std::invalid_argument("CAN filter count");
    std::vector<can_filter> native;
    native.reserve(filters.size());
    for (const auto &f : filters)
        native.push_back({f.id, f.mask});
    if (::setsockopt(handle(), SOL_CAN_RAW, CAN_RAW_FILTER, native.data(), static_cast<socklen_t>(native.size() * sizeof(can_filter))) < 0)
        io_details::fail("CAN filters");
}
void Can::setErrorMask(uint32_t mask) const {
    if (mask & ~CAN_ERR_MASK)
        throw std::invalid_argument("CAN error mask");
    if (::setsockopt(handle(), SOL_CAN_RAW, CAN_RAW_ERR_FILTER, &mask, sizeof(mask)) < 0)
        io_details::fail("CAN error filter");
}
#if __cplusplus >= 202002L
namespace async {
Can::Can(IOContext &ctx, std::string_view name, bool enable) : _ctx(ctx), _can(std::make_shared<::rm::Can>(name, CanReadMode::NONBLOCK, enable)) {}
Task<CanFrame> Can::read() {
    auto can = _can;
    for (;;) {
        if (auto frame = can->read())
            co_return *frame;
        co_await io_details::Ready(_ctx, can->handle(), EPOLLIN);
    }
}
Task<> Can::write(CanFrame frame) {
    auto can = _can;
    if (can->write(frame))
        co_return;
    // A separate epoll key permits a simultaneous read waiter on the same socket.
    io_details::Fd fd(::fcntl(can->handle(), F_DUPFD_CLOEXEC, 0));
    if (fd.value < 0)
        io_details::fail("duplicate CAN write fd");
    do {
        co_await io_details::Ready(_ctx, fd.value, EPOLLOUT);
    } while (!can->write(frame));
}
} // namespace async
#endif
} // namespace rm
#endif

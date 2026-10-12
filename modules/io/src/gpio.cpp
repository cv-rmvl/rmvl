#ifdef __linux__

#include <fcntl.h>
#include <linux/gpio.h>
#include <sys/ioctl.h>

#include <algorithm>
#include <cstring>
#include <mutex>

#include "peripheral_impl.hpp"
#include "rmvl/io/gpio.hpp"

namespace rm {

struct Gpio::Impl {
    io_details::Fd fd;
    uint64_t mask{};
    GpioOptions options;
    GpioReadMode mode;
    std::mutex mutex;

    Impl(std::string_view device, std::vector<uint32_t> offsets, GpioOptions opts, GpioReadMode m) : options(opts), mode(m) {
        if (offsets.empty() || offsets.size() > 64)
            throw std::invalid_argument("GPIO line count");
        auto sorted = offsets;
        std::sort(sorted.begin(), sorted.end());
        if (std::adjacent_find(sorted.begin(), sorted.end()) != sorted.end())
            throw std::invalid_argument("duplicate GPIO offsets");
        mask = offsets.size() == 64 ? UINT64_MAX : (uint64_t{1} << offsets.size()) - 1;
        if ((opts.initial_values & ~mask) ||
            (opts.direction == GpioDirection::Input && opts.initial_values) ||
            (opts.direction == GpioDirection::Output && (opts.edge != GpioEdge::None || opts.debounce_us)))
            throw std::invalid_argument("GPIO direction or initial values");
        if (m != GpioReadMode::BLOCK && m != GpioReadMode::NONBLOCK)
            throw std::invalid_argument("GPIO mode");
#ifdef GPIO_V2_GET_LINE_IOCTL
        gpio_v2_line_request request{};
        request.num_lines = static_cast<uint32_t>(offsets.size());
        std::copy(offsets.begin(), offsets.end(), request.offsets);
        std::strcpy(request.consumer, "rmvl");
        auto &config = request.config;
        switch (opts.direction) {
        case GpioDirection::Input:
            config.flags = GPIO_V2_LINE_FLAG_INPUT;
            break;
        case GpioDirection::Output:
            config.flags = GPIO_V2_LINE_FLAG_OUTPUT;
            break;
        default:
            throw std::invalid_argument("GPIO direction");
        }
        switch (opts.edge) {
        case GpioEdge::None:
            break;
        case GpioEdge::Rising:
            config.flags |= GPIO_V2_LINE_FLAG_EDGE_RISING;
            break;
        case GpioEdge::Falling:
            config.flags |= GPIO_V2_LINE_FLAG_EDGE_FALLING;
            break;
        case GpioEdge::Both:
            config.flags |= GPIO_V2_LINE_FLAG_EDGE_RISING | GPIO_V2_LINE_FLAG_EDGE_FALLING;
            break;
        default:
            throw std::invalid_argument("GPIO edge");
        }
        switch (opts.bias) {
        case GpioBias::Default:
            break;
        case GpioBias::Disabled:
            config.flags |= GPIO_V2_LINE_FLAG_BIAS_DISABLED;
            break;
        case GpioBias::PullUp:
            config.flags |= GPIO_V2_LINE_FLAG_BIAS_PULL_UP;
            break;
        case GpioBias::PullDown:
            config.flags |= GPIO_V2_LINE_FLAG_BIAS_PULL_DOWN;
            break;
        default:
            throw std::invalid_argument("GPIO bias");
        }
        if (opts.active_low)
            config.flags |= GPIO_V2_LINE_FLAG_ACTIVE_LOW;
        if (opts.direction == GpioDirection::Output) {
            auto &a = config.attrs[config.num_attrs++];
            a.mask = mask;
            a.attr.id = GPIO_V2_LINE_ATTR_ID_OUTPUT_VALUES;
            a.attr.values = opts.initial_values;
        }
        if (opts.debounce_us) {
            auto &a = config.attrs[config.num_attrs++];
            a.mask = mask;
            a.attr.id = GPIO_V2_LINE_ATTR_ID_DEBOUNCE;
            a.attr.debounce_period_us = opts.debounce_us;
        }
        io_details::Fd chip(::open(io_details::path(device).c_str(), O_RDONLY | O_CLOEXEC));
        if (chip.value < 0)
            io_details::fail("open GPIO chip");
        if (::ioctl(chip.value, GPIO_V2_GET_LINE_IOCTL, &request) < 0)
            io_details::fail("request GPIO lines");
        fd.value = request.fd;
        const int flags = ::fcntl(fd.value, F_GETFL);
        if (flags < 0 || ::fcntl(fd.value, F_SETFL, flags | O_NONBLOCK) < 0 ||
            ::fcntl(fd.value, F_SETFD, FD_CLOEXEC) < 0)
            io_details::fail("configure GPIO fd");
#else
        (void)device;
        throw std::system_error(EOPNOTSUPP, std::generic_category(), "GPIO v2 headers required");
#endif
    }
};
Gpio::Gpio(std::string_view device, std::vector<uint32_t> offsets, GpioOptions opts, GpioReadMode mode)
    : _impl(std::make_shared<Impl>(device, std::move(offsets), opts, mode)) {}
Gpio::~Gpio() = default;
Gpio::Gpio(Gpio &&) noexcept = default;
Gpio &Gpio::operator=(Gpio &&) noexcept = default;
bool Gpio::isOpened() const noexcept { return bool(_impl); }
int Gpio::handle() const noexcept { return _impl ? _impl->fd.value : -1; }
uint64_t Gpio::read() const {
    if (!_impl)
        throw std::system_error(EBADF, std::generic_category(), "GPIO moved from");
#ifdef GPIO_V2_GET_LINE_IOCTL
    gpio_v2_line_values values{};
    values.mask = _impl->mask;
    std::lock_guard<std::mutex> lock(_impl->mutex);
    if (::ioctl(handle(), GPIO_V2_LINE_GET_VALUES_IOCTL, &values) < 0)
        io_details::fail("read GPIO values");
    return values.bits & values.mask;
#else
    throw std::system_error(EOPNOTSUPP, std::generic_category(), "GPIO v2");
#endif
}
void Gpio::write(uint64_t values) const {
    if (!_impl)
        throw std::system_error(EBADF, std::generic_category(), "GPIO moved from");
    if (_impl->options.direction != GpioDirection::Output || (values & ~_impl->mask))
        throw std::invalid_argument("GPIO output values");
#ifdef GPIO_V2_GET_LINE_IOCTL
    gpio_v2_line_values native{};
    native.mask = _impl->mask;
    native.bits = values;
    std::lock_guard<std::mutex> lock(_impl->mutex);
    if (::ioctl(handle(), GPIO_V2_LINE_SET_VALUES_IOCTL, &native) < 0)
        io_details::fail("write GPIO values");
#endif
}
// Only edge reads use fd readiness. Value ioctls do not have readiness semantics.
std::optional<GpioEvent> Gpio::wait() const {
    if (!_impl)
        throw std::system_error(EBADF, std::generic_category(), "GPIO moved from");
    if (_impl->options.edge == GpioEdge::None)
        throw std::invalid_argument("GPIO edge detection disabled");
#ifdef GPIO_V2_GET_LINE_IOCTL
    gpio_v2_line_event event{};
    for (;;) {
        const auto n = ::read(handle(), &event, sizeof(event));
        if (n == sizeof(event))
            return GpioEvent{event.id == GPIO_V2_LINE_EVENT_RISING_EDGE ? GpioEdge::Rising : GpioEdge::Falling,
                             event.offset, event.timestamp_ns, event.seqno};
        if (n < 0 && errno == EINTR)
            continue;
        if (n < 0 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
            if (_impl->mode == GpioReadMode::NONBLOCK)
                return std::nullopt;
            io_details::wait(handle(), POLLIN);
            continue;
        }
        if (n >= 0)
            errno = EIO;
        io_details::fail("read GPIO event");
    }
#else
    throw std::system_error(EOPNOTSUPP, std::generic_category(), "GPIO v2");
#endif
}
#if __cplusplus >= 202002L
namespace async {
Gpio::Gpio(IOContext &ctx, std::string_view device, std::vector<uint32_t> offsets, GpioOptions opts)
    : _ctx(ctx), _gpio(std::make_shared<::rm::Gpio>(device, std::move(offsets), opts, GpioReadMode::NONBLOCK)) {}
Task<GpioEvent> Gpio::wait() {
    auto gpio = _gpio;
    for (;;) {
        if (auto event = gpio->wait())
            co_return *event;
        co_await io_details::Ready(_ctx, gpio->handle(), EPOLLIN);
    }
}
} // namespace async
#endif
} // namespace rm
#endif

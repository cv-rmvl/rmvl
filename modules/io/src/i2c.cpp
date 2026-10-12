#ifdef __linux__

#include <fcntl.h>
#include <linux/i2c-dev.h>
#include <linux/i2c.h>
#include <sys/ioctl.h>

#include <mutex>

#include "peripheral_impl.hpp"
#include "rmvl/io/i2c.hpp"

namespace rm {

struct I2c::Impl {
    io_details::Fd fd;
    unsigned long functions{};
    std::mutex mutex;
    explicit Impl(std::string_view device) : fd(::open(io_details::path(device).c_str(), O_RDWR | O_CLOEXEC)) {
        if (fd.value < 0)
            io_details::fail("open I2C");
        if (::ioctl(fd.value, I2C_FUNCS, &functions) < 0)
            io_details::fail("I2C capabilities");
        if (!(functions & I2C_FUNC_I2C))
            throw std::system_error(EOPNOTSUPP, std::generic_category(), "I2C_RDWR");
    }
};

I2c::I2c(std::string_view device) : _impl(std::make_unique<Impl>(device)) {}
I2c::~I2c() = default;
I2c::I2c(I2c &&) noexcept = default;
I2c &I2c::operator=(I2c &&) noexcept = default;
bool I2c::isOpened() const noexcept { return bool(_impl); }

// Submit the whole repeated-START transaction once, without replaying writes.
std::vector<I2cMessage> I2c::transfer(std::vector<I2cMessage> messages) {
    if (!_impl)
        throw std::system_error(EBADF, std::generic_category(), "I2C moved from");
    if (messages.empty() || messages.size() > I2C_RDWR_IOCTL_MAX_MSGS)
        throw std::invalid_argument("I2C message count");
    std::vector<i2c_msg> native;
    native.reserve(messages.size());
    for (auto &m : messages) {
        if (m.address > (m.ten_bit ? 0x3ff : 0x7f) || m.data.size() > 8192)
            throw std::invalid_argument("I2C address or message size");
        if (m.ten_bit && !(_impl->functions & I2C_FUNC_10BIT_ADDR))
            throw std::system_error(EOPNOTSUPP, std::generic_category(), "I2C ten-bit address");
        native.push_back({m.address, static_cast<__u16>((m.reading ? I2C_M_RD : 0) | (m.ten_bit ? I2C_M_TEN : 0)),
                          static_cast<__u16>(m.data.size()), m.data.data()});
    }
    i2c_rdwr_ioctl_data transaction{native.data(), static_cast<__u32>(native.size())};
    std::lock_guard<std::mutex> lock(_impl->mutex);
    const int count = ::ioctl(_impl->fd.value, I2C_RDWR, &transaction);
    if (count < 0)
        io_details::fail("I2C_RDWR");
    if (count != static_cast<int>(native.size()))
        throw std::system_error(EIO, std::generic_category(), "partial I2C transaction");
    return messages;
}
} // namespace rm
#endif

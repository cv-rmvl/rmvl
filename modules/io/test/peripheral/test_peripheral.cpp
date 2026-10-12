#include <linux/can/raw.h>
#include <linux/gpio.h>
#include <linux/i2c-dev.h>
#include <linux/i2c.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cstdarg>
#include <cstring>
#include <thread>

#include <gtest/gtest.h>

#include "rmvl/io/can.hpp"
#include "rmvl/io/gpio.hpp"
#include "rmvl/io/i2c.hpp"

namespace {

bool mock{};
int channel[2]{-1, -1};
uint64_t levels{};

#ifdef GPIO_V2_GET_LINE_IOCTL
gpio_v2_line_request requested{};
#endif

std::atomic<int> transactions{};
std::atomic<int> transaction_error{};

class Peripheral : public testing::Test {
    void SetUp() override {
        ASSERT_EQ(::socketpair(AF_UNIX, SOCK_DGRAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0, channel), 0);
        levels = 0;
        transactions = 0;
        transaction_error = 0;
        mock = true;
    }
    void TearDown() override {
        mock = false;
        ::close(channel[0]);
        ::close(channel[1]);
    }
};

} // namespace

// Linker wrapping is restricted to this test executable, never the production library.
extern "C" int __real_ioctl(int, unsigned long, ...);
extern "C" unsigned __real_if_nametoindex(const char *);
extern "C" int __real_socket(int, int, int);
extern "C" int __real_bind(int, const sockaddr *, socklen_t);
extern "C" int __real_setsockopt(int, int, int, const void *, socklen_t);
extern "C" ssize_t __real_send(int, const void *, size_t, int);
extern "C" ssize_t __real_recv(int, void *, size_t, int);
extern "C" ssize_t __wrap_send(int fd, const void *data, size_t size, int flags) {
    return mock ? ::write(fd, data, size) : __real_send(fd, data, size, flags);
}
extern "C" ssize_t __wrap_recv(int fd, void *data, size_t size, int flags) {
    return mock ? ::read(fd, data, size) : __real_recv(fd, data, size, flags);
}
extern "C" int __wrap_ioctl(int fd, unsigned long request, ...) {
    va_list args;
    va_start(args, request);
    void *arg = va_arg(args, void *);
    va_end(args);
    if (!mock)
        return __real_ioctl(fd, request, arg);
    switch (request) {
#ifdef GPIO_V2_GET_LINE_IOCTL
    case GPIO_V2_GET_LINE_IOCTL: {
        auto &r = *static_cast<gpio_v2_line_request *>(arg);
        requested = r;
        r.fd = ::dup(channel[0]);
        return r.fd < 0 ? -1 : 0;
    }
    case GPIO_V2_LINE_GET_VALUES_IOCTL:
        static_cast<gpio_v2_line_values *>(arg)->bits = levels;
        return 0;
    case GPIO_V2_LINE_SET_VALUES_IOCTL:
        levels = static_cast<gpio_v2_line_values *>(arg)->bits;
        return 0;
#endif
    case I2C_FUNCS:
        *static_cast<unsigned long *>(arg) = I2C_FUNC_I2C | I2C_FUNC_10BIT_ADDR;
        return 0;
    case I2C_RDWR: {
        ++transactions;
        auto &t = *static_cast<i2c_rdwr_ioctl_data *>(arg);
        for (unsigned i = 0; i < t.nmsgs; ++i)
            if (t.msgs[i].flags & I2C_M_RD)
                std::memset(t.msgs[i].buf, 0x5a, t.msgs[i].len);
        if (transaction_error) {
            errno = transaction_error;
            return -1;
        }
        return static_cast<int>(t.nmsgs);
    }
    default:
        return __real_ioctl(fd, request, arg);
    }
}
extern "C" unsigned __wrap_if_nametoindex(const char *name) {
    return mock && std::strcmp(name, "rmvl-test-can") == 0 ? 1 : __real_if_nametoindex(name);
}
extern "C" int __wrap_socket(int domain, int type, int protocol) {
    return mock && domain == PF_CAN ? ::dup(channel[0]) : __real_socket(domain, type, protocol);
}
extern "C" int __wrap_bind(int fd, const sockaddr *address, socklen_t length) {
    return mock && address->sa_family == AF_CAN ? 0 : __real_bind(fd, address, length);
}
extern "C" int __wrap_setsockopt(int fd, int level, int option, const void *value, socklen_t length) {
    return mock && level == SOL_CAN_RAW ? 0 : __real_setsockopt(fd, level, option, value, length);
}

#ifdef GPIO_V2_GET_LINE_IOCTL
TEST_F(Peripheral, GpioValuesAndConfiguration) {
    rm::GpioOptions options;
    options.direction = rm::GpioDirection::Output;
    options.initial_values = 2;
    options.active_low = true;
    rm::Gpio gpio("/dev/null", {3, 7}, options);
    EXPECT_EQ(requested.offsets[1], 7u);
    EXPECT_TRUE(requested.config.flags & GPIO_V2_LINE_FLAG_ACTIVE_LOW);
    EXPECT_EQ(requested.config.attrs[0].attr.values, 2u);
    gpio.write(3);
    EXPECT_EQ(gpio.read(), 3u);
    EXPECT_THROW(gpio.write(4), std::invalid_argument);
    EXPECT_THROW(gpio.wait(), std::invalid_argument);
    auto moved = std::move(gpio);
    EXPECT_FALSE(gpio.isOpened());
    EXPECT_THROW(gpio.read(), std::system_error);
    EXPECT_TRUE(moved.isOpened());
}
TEST_F(Peripheral, GpioEdgesAndValidation) {
    EXPECT_THROW((rm::Gpio("/dev/null", {})), std::invalid_argument);
    EXPECT_THROW((rm::Gpio("/dev/null", {2, 2})), std::invalid_argument);
    rm::GpioOptions options;
    options.edge = rm::GpioEdge::Both;
    rm::Gpio gpio("/dev/null", {3}, options, rm::GpioReadMode::NONBLOCK);
    EXPECT_FALSE(gpio.wait());
    gpio_v2_line_event event{};
    event.id = GPIO_V2_LINE_EVENT_RISING_EDGE;
    event.offset = 3;
    event.timestamp_ns = 123;
    event.seqno = 4;
    ASSERT_EQ(::send(channel[1], &event, sizeof(event), 0), sizeof(event));
    const auto received = gpio.wait();
    ASSERT_TRUE(received);
    EXPECT_EQ(received->edge, rm::GpioEdge::Rising);
    EXPECT_EQ(received->timestamp_ns, 123u);
    EXPECT_EQ(received->sequence, 4u);
    EXPECT_THROW(gpio.write(1), std::invalid_argument);
}
TEST_F(Peripheral, GpioBlockingEdge) {
    rm::GpioOptions options;
    options.edge = rm::GpioEdge::Falling;
    rm::Gpio gpio("/dev/null", {5}, options);
    std::thread producer([] {
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
        gpio_v2_line_event event{};
        event.id = GPIO_V2_LINE_EVENT_FALLING_EDGE;
        event.offset = 5;
        EXPECT_EQ(::write(channel[1], &event, sizeof(event)), sizeof(event));
    });
    auto event = gpio.wait();
    producer.join();
    ASSERT_TRUE(event);
    EXPECT_EQ(event->edge, rm::GpioEdge::Falling);
}
#endif
TEST_F(Peripheral, I2cCombinedTransaction) {
    rm::I2c bus("/dev/null");
    const auto messages = bus.transfer({{0x48, false, false, {0x10}}, {0x48, true, false, {0, 0}}});
    ASSERT_EQ(messages.size(), 2u);
    EXPECT_EQ(messages[0].data[0], 0x10);
    EXPECT_EQ(messages[1].data, (std::vector<uint8_t>{0x5a, 0x5a}));
    EXPECT_EQ(transactions, 1);
    EXPECT_THROW(bus.transfer({}), std::invalid_argument);
    EXPECT_THROW(bus.transfer({{0x80, false, false, {}}}), std::invalid_argument);
    transaction_error = EINTR;
    EXPECT_THROW(bus.transfer({{0x48, false, false, {1}}}), std::system_error);
    EXPECT_EQ(transactions, 2); // Side-effecting transactions must not be retried.
}
TEST_F(Peripheral, CanFramesAndNonblocking) {
    rm::Can can("rmvl-test-can", rm::CanReadMode::NONBLOCK, true);
    EXPECT_FALSE(can.read());
    rm::CanFrame frame;
    frame.id = 0x123;
    frame.size = 2;
    frame.data[0] = 0x45;
    ASSERT_TRUE(can.write(frame));
    canfd_frame native{};
    ASSERT_EQ(::recv(channel[1], &native, sizeof(native), 0), CAN_MTU);
    EXPECT_EQ(native.can_id, frame.id);
    EXPECT_EQ(native.data[0], 0x45);
    native.can_id = CAN_EFF_FLAG | 0x12345;
    native.len = 12;
    native.flags = CANFD_BRS;
    ASSERT_EQ(::send(channel[1], &native, CANFD_MTU, 0), CANFD_MTU);
    auto received = can.read();
    ASSERT_TRUE(received);
    EXPECT_TRUE(received->fd);
    EXPECT_TRUE(received->extended);
    EXPECT_TRUE(received->bitrate_switch);
    EXPECT_EQ(received->id, 0x12345u);
    frame.size = 9;
    EXPECT_THROW(can.write(frame), std::invalid_argument);
    frame.size = 1;
    frame.error = true;
    EXPECT_THROW(can.write(frame), std::invalid_argument);
    can.setFilters({});
    can.setErrorMask(1);
    frame.error = false;
    frame.fd = true;
    frame.size = 9;
    EXPECT_THROW(can.write(frame), std::invalid_argument);
    frame.size = 12;
    frame.extended = true;
    frame.bitrate_switch = true;
    ASSERT_TRUE(can.write(frame));
    ASSERT_EQ(::recv(channel[1], &native, sizeof(native), 0), CANFD_MTU);
    EXPECT_EQ(native.flags, CANFD_BRS);
}
TEST_F(Peripheral, CanBlockingRead) {
    rm::Can can("rmvl-test-can");
    std::thread producer([] {
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
        can_frame frame{};
        frame.can_id = 0x456;
        EXPECT_EQ(::write(channel[1], &frame, sizeof(frame)), sizeof(frame));
    });
    auto frame = can.read();
    producer.join();
    ASSERT_TRUE(frame);
    EXPECT_EQ(frame->id, 0x456u);
}
TEST_F(Peripheral, RealInvalidDeviceErrors) {
    mock = false;
    EXPECT_THROW(rm::I2c("/dev/null"), std::system_error);
    EXPECT_THROW(rm::Gpio("/dev/null", {0}), std::system_error);
    EXPECT_THROW(rm::Can("rmvl-absent"), std::system_error);
    EXPECT_THROW(rm::I2c(std::string_view("a\0b", 3)), std::invalid_argument);
    mock = true;
}

#if __cplusplus >= 202002L
TEST_F(Peripheral, AsyncCanWaitsForReadiness) {
    rm::async::IOContext context;
    rm::async::Can can(context, "rmvl-test-can");
    bool received = false;
    auto reader = [&]() -> rm::async::Task<> {
        const auto frame = co_await can.read();
        received = frame.id == 0x321;
        context.stop();
    };
    auto sender = [&]() -> rm::async::Task<> {
        can_frame frame{};
        frame.can_id = 0x321;
        EXPECT_EQ(::send(channel[1], &frame, sizeof(frame), 0), sizeof(frame));
        co_return;
    };
    context.spawn(reader);
    context.spawn(sender);
    context.run();
    EXPECT_TRUE(received);
}
TEST_F(Peripheral, AsyncCanSimultaneousReadAndBlockedWrite) {
    rm::async::IOContext context;
    rm::async::Can can(context, "rmvl-test-can");
    can_frame native{};
    // Fill the mock socket's send queue to exercise EPOLLOUT rather than the fast path.
    while (::write(channel[0], &native, sizeof(native)) >= 0) {
    }
    ASSERT_EQ(errno, EAGAIN);
    int completed = 0;
    auto reader = [&]() -> rm::async::Task<> {
        const auto frame = co_await can.read();
        EXPECT_EQ(frame.id, 0x345u);
        if (++completed == 2)
            context.stop();
    };
    auto writer = [&]() -> rm::async::Task<> {
        rm::CanFrame frame;
        frame.id = 0x123;
        co_await can.write(frame);
        if (++completed == 2)
            context.stop();
    };
    auto peer = [&]() -> rm::async::Task<> {
        while (::read(channel[1], &native, sizeof(native)) >= 0) {
        }
        native.can_id = 0x345;
        EXPECT_EQ(::write(channel[1], &native, sizeof(native)), sizeof(native));
        co_return;
    };
    context.spawn(reader);
    context.spawn(writer);
    context.spawn(peer);
    context.run();
    EXPECT_EQ(completed, 2);
    ASSERT_EQ(::read(channel[1], &native, sizeof(native)), sizeof(native));
    EXPECT_EQ(native.can_id, 0x123u);
}
#ifdef GPIO_V2_GET_LINE_IOCTL
TEST_F(Peripheral, AsyncGpioCancellationUnregistersFd) {
    rm::async::IOContext context;
    rm::GpioOptions options;
    options.edge = rm::GpioEdge::Both;
    rm::async::Gpio gpio(context, "/dev/null", {3}, options);
    bool completed = false;
    auto reader = [&]() -> rm::async::Task<> {
        auto event = co_await gpio.wait();
        completed = event.offset == 3;
        context.stop();
    };
    auto stopper = [&]() -> rm::async::Task<> { context.stop(); co_return; };
    context.spawn(reader);
    context.spawn(stopper);
    context.run();
    EXPECT_FALSE(completed);
    auto producer = [&]() -> rm::async::Task<> {
        gpio_v2_line_event event{};
        event.id = GPIO_V2_LINE_EVENT_RISING_EDGE;
        event.offset = 3;
        EXPECT_EQ(::write(channel[1], &event, sizeof(event)), sizeof(event));
        co_return;
    };
    context.spawn(reader);
    context.spawn(producer);
    context.run();
    EXPECT_TRUE(completed);
}
#endif
#endif

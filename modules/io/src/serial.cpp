/**
 * @file serial.hpp
 * @author zhaoxi (535394140@qq.com)
 * @brief 同步/异步的数据链路层串口通信框架
 * @version 1.0
 * @date 2025-07-31
 *
 * @copyright Copyright 2025 (c), zhaoxi
 *
 */

#ifndef _WIN32
#include <dirent.h>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#endif

#if __cplusplus >= 202002L

#ifndef _WIN32
#include <sys/epoll.h>
#endif

#endif

#include "rmvl/core/util.hpp"

#include "rmvl/io/serial.hpp"

namespace rm {

SerialPort::~SerialPort() = default;

SerialPort::SerialPort(std::string_view device, BaudRate rate, SerialReadMode mode) : _device(device), _baud_rate(rate), _read_mode(mode) { open(); }

static unsigned int getBaudRate(BaudRate baud_rate) {
#ifdef _WIN32
    constexpr unsigned int baud_rate_map[] = {CBR_1200, CBR_2400, CBR_4800, CBR_9600, CBR_19200, CBR_38400, CBR_57600, CBR_115200};
#else
    constexpr unsigned int baud_rate_map[] = {B1200, B2400, B4800, B9600, B19200, B38400, B57600, B115200};
#endif
    return baud_rate_map[static_cast<std::size_t>(baud_rate)];
}

bool SerialPort::read(std::string &data) {
    bool retval{};
    constexpr int MAX_LENGTH{256};
    char buffer[MAX_LENGTH]{};
    auto len_result = fdread(buffer, MAX_LENGTH);
    if (len_result > 0) {
        data.assign(buffer, len_result);
        retval = true;
    }
    return retval;
}

#ifdef _WIN32

void SerialPort::open() {
    INFO_("Opening the serial port: %s", _device.c_str());
    _fd = CreateFileA(
        _device.c_str(),
        GENERIC_READ | GENERIC_WRITE,
        0,
        nullptr,
        OPEN_EXISTING,
        0,
        nullptr);
    if (_fd == INVALID_HANDLE_VALUE) {
        ERROR_("Failed to open the serial port.");
        _is_open = false;
        return;
    }

    SetupComm(_fd, 1024, 1024);

    COMMTIMEOUTS timeouts{};
    if (_read_mode == SerialReadMode::BLOCK) {
        timeouts.ReadIntervalTimeout = 0;
        timeouts.ReadTotalTimeoutConstant = 0;
        timeouts.ReadTotalTimeoutMultiplier = 0;
    } else {
        timeouts.ReadIntervalTimeout = MAXDWORD;
        timeouts.ReadTotalTimeoutConstant = 0;
        timeouts.ReadTotalTimeoutMultiplier = 0;
    }
    if (!SetCommTimeouts(_fd, &timeouts)) {
        WARNING_("Failed to set the serial port timeout.");
        _is_open = false;
        return;
    }

    DCB dcb{};
    GetCommState(_fd, &dcb);
    DWORD bps = getBaudRate(_baud_rate);
    dcb.BaudRate = bps;        // 波特率
    dcb.ByteSize = 8;          // 数据位
    dcb.Parity = NOPARITY;     // 无校验
    dcb.StopBits = ONESTOPBIT; // 1 位停止位
    if (!SetCommState(_fd, &dcb)) {
        WARNING_("Failed to set the serial port state.");
        _is_open = false;
        return;
    }

    _is_open = true;
}

void SerialPort::close() {
    if (_is_open)
        CloseHandle(_fd);
    _is_open = false;
}

long int SerialPort::fdwrite(const void *data, std::size_t len) {
    DWORD len_result{};
    if (_is_open) {
        PurgeComm(_fd, PURGE_TXCLEAR);
        if (!WriteFile(_fd, data, static_cast<DWORD>(len), &len_result, nullptr)) {
            WARNING_("Unable to write to serial port, error code: %ld", ::GetLastError());
            open();
        } else
            DEBUG_INFO_("Success to write the serial port.");
    }

    return len_result;
}

long int SerialPort::fdread(void *data, std::size_t len) {
    DWORD len_result{};

    if (_is_open) {
        if (!ReadFile(_fd, data, static_cast<DWORD>(len), &len_result, nullptr)) {
            WARNING_("The serial port cannot be read, error code: %ld, restart...", ::GetLastError());
            open();
        } else
            PurgeComm(_fd, PURGE_RXCLEAR);
    } else if (len_result == 0)
        DEBUG_WARNING_("Serial port read: null");
    else
        DEBUG_PASS_("Success to read the serial port.");
    return len_result;
}

#else

void SerialPort::open() {
    INFO_("Opening the serial port: %s", _device.c_str());
    _fd = ::open(_device.c_str(), O_RDWR | O_NOCTTY | O_NDELAY);
    if (_fd < 0) {
        ERROR_("Failed to open the serial port.");
        _is_open = false;
        return;
    }
    if (_read_mode == SerialReadMode::BLOCK) {
        // 清除 O_NDELAY 标志，设置为阻塞模式
        int flags = fcntl(_fd, F_GETFL, 0);
        flags &= ~O_NONBLOCK;
        fcntl(_fd, F_SETFL, flags);
    }

    termios option;
    tcgetattr(_fd, &option);

    // 修改所获得的参数
    option.c_iflag = 0;                 // 原始输入模式
    option.c_oflag = 0;                 // 原始输出模式
    option.c_lflag = 0;                 // 关闭终端模式
    option.c_cflag |= (CLOCAL | CREAD); // 设置控制模式状态，本地连接，接收使能
    option.c_cflag &= ~CSIZE;           // 字符长度，设置数据位之前一定要屏掉这个位
    option.c_cflag &= ~CRTSCTS;         // 无硬件流控
    option.c_cflag |= CS8;              // 8 位数据长度
    option.c_cflag &= ~CSTOPB;          // 1 位停止位
    option.c_cc[VTIME] = 0;
    option.c_cc[VMIN] = _read_mode == SerialReadMode::BLOCK ? 1 : 0;
    cfsetspeed(&option, getBaudRate(_baud_rate)); // 设置输入波特率

    // 设置新属性，TCSANOW：所有改变立即生效
    tcsetattr(_fd, TCSANOW, &option);

    _is_open = true;
}

void SerialPort::close() {
    if (_is_open)
        ::close(_fd);
    _is_open = false;
}

long int SerialPort::fdwrite(const void *data, std::size_t length) {
    ssize_t len_result = -1;
    if (_is_open) {
        tcflush(_fd, TCOFLUSH);
        len_result = ::write(_fd, data, length);
    }

    if (len_result != static_cast<ssize_t>(length)) {
        WARNING_("Unable to write to serial port, restart...");
        open();
    } else
        DEBUG_INFO_("Success to write the serial port.");

    return len_result;
}

long int SerialPort::fdread(void *data, std::size_t len) {
    ssize_t len_result = -1;

    if (_is_open) {
        len_result = ::read(_fd, data, len);
        tcflush(_fd, TCIFLUSH);
    }

    if (len_result == -1) {
        WARNING_("The serial port cannot be read, restart...");
        open();
    } else if (len_result == 0)
        DEBUG_WARNING_("Serial port read: null");
    else
        DEBUG_PASS_("Success to read the serial port.");
    return len_result;
}

#endif

#if __cplusplus >= 202002L

namespace async {

#ifdef _WIN32

SerialPort::SerialPort(IOContext &io_context, std::string_view device, BaudRate baud_rate)
    : ::rm::SerialPort(device, baud_rate, SerialReadMode::BLOCK), _ctx(io_context) {
    if (CreateIoCompletionPort(_fd, _ctx.get().handle(), 0, 0) == nullptr) {
        auto err = GetLastError();
        if (err != ERROR_INVALID_PARAMETER)
            RMVL_Error_(RMVL_StsError, "Associate fd with IOCP failed: %lu", err);
    }
}

std::string SerialPort::SerialReadAwaiter::await_resume() noexcept {
    PurgeComm(_fd, PURGE_RXCLEAR);
    return _ovl ? _ovl->buf : std::string{};
}

bool SerialPort::SerialWriteAwaiter::await_resume() noexcept {
    PurgeComm(_fd, PURGE_TXCLEAR);
    return _ovl != nullptr;
}

#else

SerialPort::SerialPort(IOContext &io_context, std::string_view device, BaudRate baud_rate)
    : ::rm::SerialPort(device, baud_rate, SerialReadMode::BLOCK), _ctx(io_context) {}

std::string SerialPort::SerialReadAwaiter::await_resume() noexcept {
    epoll_ctl(_aioh, EPOLL_CTL_DEL, _fd, nullptr);
    char buffer[256]{};
    ssize_t n = ::read(_fd, buffer, sizeof(buffer));
    tcflush(_fd, TCIFLUSH);
    return n > 0 ? std::string(buffer, n) : std::string{};
}

bool SerialPort::SerialWriteAwaiter::await_resume() noexcept {
    epoll_ctl(_aioh, EPOLL_CTL_DEL, _fd, nullptr);
    tcflush(_fd, TCOFLUSH);
    ssize_t n = ::write(_fd, _data.data(), _data.size());
    return n == static_cast<ssize_t>(_data.size());
}

#endif

} // namespace async

#endif

} // namespace rm

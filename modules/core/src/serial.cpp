/**
 * @file serial.cpp
 * @author RoboMaster Vision Community
 * @brief Unix 串口类
 * @version 2.0
 * @date 2018-12-08
 *
 * @copyright Copyright 2018 (c), RoboMaster Vision Community
 *
 */

#include "io_impl.hpp"

#include "rmvl/core/util.hpp"

#ifndef _WIN32
#include <dirent.h>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#endif

namespace rm
{

RMVL_IMPL_DEF(SerialPort)

static unsigned int getBaudRate(BaudRate baud_rate)
{
#ifdef _WIN32
    switch (baud_rate)
    {
    case BaudRate::BR_1200:
        return 1200;
    case BaudRate::BR_4800:
        return 4800;
    case BaudRate::BR_9600:
        return 9600;
    case BaudRate::BR_57600:
        return 57600;
    case BaudRate::BR_115200:
        return 115200;
    default:
        return 115200;
    }
#else
    switch (baud_rate)
    {
    case BaudRate::BR_1200:
        return B1200;
    case BaudRate::BR_4800:
        return B4800;
    case BaudRate::BR_9600:
        return B9600;
    case BaudRate::BR_57600:
        return B57600;
    case BaudRate::BR_115200:
        return B115200;
    default:
        return B115200;
    }
#endif
}

SerialPort::SerialPort(std::string_view device, SerialPortMode mode) : _impl(new Impl(device, mode)) {}
bool SerialPort::isOpened() const { return _impl->isOpened(); }
long int SerialPort::fdwrite(const void *data, size_t length) { return _impl->fdwrite(data, length); }
long int SerialPort::fdread(void *data, size_t len) { return _impl->fdread(data, len); }

#ifdef _WIN32
void SerialPort::Impl::open()
{
    INFO_("Opening the serial port: %s", _device.c_str());
    _handle = CreateFileA(
        _device.c_str(),
        GENERIC_READ | GENERIC_WRITE,
        0,
        nullptr,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        nullptr);
    if (_handle == INVALID_HANDLE_VALUE)
    {
        ERROR_("Failed to open the serial port.");
        _is_open = false;
        return;
    }

    COMMTIMEOUTS timeouts{};
    if (_mode.read_mode == SPReadMode::BLOCK)
    {
        timeouts.ReadIntervalTimeout = 0;
        timeouts.ReadTotalTimeoutConstant = 0;
        timeouts.ReadTotalTimeoutMultiplier = 0;
    }
    else
    {
        timeouts.ReadIntervalTimeout = MAXDWORD;
        timeouts.ReadTotalTimeoutConstant = 0;
        timeouts.ReadTotalTimeoutMultiplier = 0;
    }
    timeouts.WriteTotalTimeoutConstant = 1;
    timeouts.WriteTotalTimeoutMultiplier = 1;
    if (!SetCommTimeouts(_handle, &timeouts))
    {
        WARNING_("Failed to set the serial port timeout.");
        _is_open = false;
        return;
    }

    DCB dcb{};
    GetCommState(_handle, &dcb);
    DWORD bps = getBaudRate(_mode.baud_rate);
    dcb.BaudRate = bps;        // 波特率
    dcb.ByteSize = 8;          // 数据位
    dcb.Parity = NOPARITY;     // 无校验
    dcb.StopBits = ONESTOPBIT; // 1 位停止位
    if (!SetCommState(_handle, &dcb))
    {
        WARNING_("Failed to set the serial port state.");
        _is_open = false;
        return;
    }

    _is_open = true;
}

void SerialPort::Impl::close()
{
    if (_is_open)
        CloseHandle(_handle);
    _is_open = false;
}

long int SerialPort::Impl::fdwrite(const void *data, std::size_t len)
{
    DWORD len_result{};
    if (_is_open)
    {
        if (!WriteFile(_handle, data, static_cast<DWORD>(len), &len_result, nullptr))
        {
            WARNING_("Unable to write to serial port, error code: %ld", ::GetLastError());
            open();
        }
        else
            DEBUG_INFO_("Success to write the serial port.");
    }

    return len_result;
}

long int SerialPort::Impl::fdread(void *data, std::size_t len)
{
    DWORD len_result{};

    if (_is_open)
    {
        if (!ReadFile(_handle, data, static_cast<DWORD>(len), &len_result, nullptr))
        {
            WARNING_("The serial port cannot be read, error code: %ld, restart...", ::GetLastError());
            open();
        }
    }
    else if (len_result == 0)
        DEBUG_WARNING_("Serial port read: null");
    else
        DEBUG_PASS_("Success to read the serial port.");
    return len_result;
}

#else
void SerialPort::Impl::open()
{
    INFO_("Opening the serial port: %s", _device.c_str());
    _fd = ::open(_device.c_str(), O_RDWR | O_NOCTTY | O_NDELAY);
    if (_fd == -1)
    {
        ERROR_("Failed to open the serial port.");
        _is_open = false;
        return;
    }
    if (_mode.read_mode == SPReadMode::BLOCK)
    {
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
    option.c_cc[VMIN] = _mode.read_mode == SPReadMode::BLOCK ? 1 : 0;
    cfsetspeed(&option, getBaudRate(_mode.baud_rate)); // 设置输入波特率

    // 设置新属性，TCSANOW：所有改变立即生效
    tcsetattr(_fd, TCSANOW, &option);

    _is_open = true;
}

void SerialPort::Impl::close()
{
    if (_is_open)
        ::close(_fd);
    _is_open = false;
}

long int SerialPort::Impl::fdwrite(const void *data, std::size_t length)
{
    ssize_t len_result = -1;
    if (_is_open)
    {
        // 清空，防止数据累积在缓存区
        tcflush(_fd, TCOFLUSH);
        len_result = ::write(_fd, data, length);
    }

    if (len_result != static_cast<ssize_t>(length))
    {
        WARNING_("Unable to write to serial port, restart...");
        open();
    }
    else
        DEBUG_INFO_("Success to write the serial port.");

    return len_result;
}

long int SerialPort::Impl::fdread(void *data, std::size_t len)
{
    ssize_t len_result = -1;

    if (_is_open)
    {
        len_result = ::read(_fd, data, len);
        tcflush(_fd, TCIFLUSH);
    }

    if (len_result == -1)
    {
        WARNING_("The serial port cannot be read, restart...");
        open();
    }
    else if (len_result == 0)
        DEBUG_WARNING_("Serial port read: null");
    else
        DEBUG_PASS_("Success to read the serial port.");
    return len_result;
}

#endif

} // namespace rm

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
#include <unistd.h>
#endif

namespace rm
{

RMVL_IMPL_DEF(SerialPort)

int getBaudRate(BaudRate baud_rate)
{
#ifdef _WIN32
    switch (baud_rate)
    {
    case BaudRate::BR_57600:
        return 57600;
    case BaudRate::BR_115200:
        return 115200;
    case BaudRate::BR_230400:
        return 230400;
    default:
        return 115200;
    }
#else
    switch (baud_rate)
    {
    case BaudRate::BR_57600:
        return B57600;
    case BaudRate::BR_115200:
        return B115200;
    case BaudRate::BR_230400:
        return B230400;
    default:
        return B115200;
    }
#endif
}

SerialPort::SerialPort(std::string_view device, BaudRate baud_rate) : _impl(new Impl(device, baud_rate)) {}
bool SerialPort::isOpened() const { return _impl->isOpened(); }
long int SerialPort::fdwrite(void *data, size_t length) { return _impl->fdwrite(data, length); }
long int SerialPort::fdread(void *data, size_t len) { return _impl->fdread(data, len); }

#ifdef _WIN32
void SerialPort::Impl::open()
{
    _is_open = false;
    INFO_("Opening the serial port: %s", _device.c_str());
    _handle = CreateFileA(_device.c_str(), GENERIC_READ | GENERIC_WRITE, 0, nullptr, OPEN_EXISTING, 0, nullptr);

    if (_handle == INVALID_HANDLE_VALUE)
    {
        ERROR_("Failed to open the serial port.");
        return;
    }

    DCB dcb;
    GetCommState(_handle, &dcb);
    dcb.BaudRate = _baud_rate;
    dcb.ByteSize = 8;
    dcb.Parity = NOPARITY;
    dcb.StopBits = ONESTOPBIT;
    SetCommState(_handle, &dcb);

    _is_open = true;
}

void SerialPort::Impl::close()
{
    if (_is_open)
        CloseHandle(_handle);
    _is_open = false;
}

long int SerialPort::Impl::fdwrite(void *data, std::size_t len)
{
    DWORD len_result{};
    if (_is_open)
    {
        if (!WriteFile(_handle, data, static_cast<DWORD>(len), &len_result, nullptr))
        {
            WARNING_("Unable to write to serial port, error code: %d", ::GetLastError());
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
            WARNING_("The serial port cannot be read, error code: %d, restart...", ::GetLastError());
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
    _is_open = false;
    INFO_("Opening the serial port: %s", _device.c_str());
    _fd = ::open(_device.c_str(), O_RDWR | O_NOCTTY | O_NDELAY); // 非堵塞情况

    if (_fd == -1)
    {
        ERROR_("Failed to open the serial port.");
        return;
    }
    tcgetattr(_fd, &_option);

    // 修改所获得的参数
    _option.c_iflag = 0;                 // 原始输入模式
    _option.c_oflag = 0;                 // 原始输出模式
    _option.c_lflag = 0;                 // 关闭终端模式
    _option.c_cflag |= (CLOCAL | CREAD); // 设置控制模式状态，本地连接，接收使能
    _option.c_cflag &= ~CSIZE;           // 字符长度，设置数据位之前一定要屏掉这个位
    _option.c_cflag &= ~CRTSCTS;         // 无硬件流控
    _option.c_cflag |= CS8;              // 8位数据长度
    _option.c_cflag &= ~CSTOPB;          // 1位停止位
    _option.c_cc[VTIME] = 0;
    _option.c_cc[VMIN] = 0;
    cfsetospeed(&_option, _baud_rate); // 设置输入波特率
    cfsetispeed(&_option, _baud_rate); // 设置输出波特率

    // 设置新属性，TCSANOW：所有改变立即生效
    tcsetattr(_fd, TCSANOW, &_option);

    _is_open = true;
}

void SerialPort::Impl::close()
{
    if (_is_open)
        ::close(_fd);
    _is_open = false;
}

long int SerialPort::Impl::fdwrite(void *data, std::size_t length)
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

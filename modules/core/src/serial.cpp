/**
 * @file serial.cpp
 * @author RoboMaster Vision Community
 * @brief Linux 串口类
 * @version 2.0
 * @date 2018-12-08
 *
 * @copyright Copyright 2023 (c), RoboMaster Vision Community
 *
 */

#include <cstring>
#include <dirent.h>
#include <fcntl.h>
#include <string>
#include <unistd.h>

#include "rmvl/core/serial.hpp"

void rm::SerialPort::open()
{
    _is_open = false;

    DIR *dir = nullptr;
    std::string file_name = _device;
    const char *dir_path = "/dev/";
    if ((dir = opendir(dir_path)) != nullptr)
    {
        if (_device.empty())
        {
            struct dirent *dire = nullptr;
            while ((dire = readdir(dir)) != nullptr)
            {
                if ((strstr(dire->d_name, "ttyUSB") != nullptr) ||
                    (strstr(dire->d_name, "ttyACM") != nullptr))
                {
                    file_name = dire->d_name;
                    break;
                }
            }
            closedir(dir);
        }
    }
    else
    {
        SER_ERROR("Cannot find the directory: \"%s\"", dir_path);
        return;
    }

    if (file_name.empty())
    {
        SER_ERROR("Cannot find the serial port.");
        return;
    }
    else
        file_name = dir_path + file_name;

    SER_INFO("Opening the serial port: %s", file_name.c_str());
    _fd = ::open(file_name.c_str(), O_RDWR | O_NOCTTY | O_NDELAY); // 非堵塞情况

    if (_fd == -1)
    {
        SER_ERROR("Failed to open the serial port.");
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

void rm::SerialPort::close()
{
    if (_is_open)
        ::close(_fd);
    _is_open = false;
}

ssize_t rm::SerialPort::write(void *data, size_t length)
{
    ssize_t len_result = -1;
    if (_is_open)
    {
        tcflush(_fd, TCOFLUSH); // 清空，防止数据累积在缓存区
        len_result = ::write(_fd, data, length);
    }

    if (len_result != static_cast<ssize_t>(length))
    {
        SER_WARNING("Unable to write to serial port, restart...");
        open();
    }
    else
        DEBUG_SER_INFO("Success to write the serial port.");

    return len_result;
}

/**
 * @brief Read data
 *
 * @param data Start position of the data
 * @param len The length of the data to be read
 * @return Length
 */
ssize_t rm::SerialPort::read(void *data, size_t len)
{
    ssize_t len_result = -1;

    if (_is_open)
    {
        len_result = ::read(_fd, data, len);
        tcflush(_fd, TCIFLUSH);
    }

    if (len_result == -1)
    {
        SER_WARNING("The serial port cannot be read, restart...");
        open();
    }
    else if (len_result == 0)
        DEBUG_SER_WARNING("Serial port read: null");
    else
        DEBUG_SER_PASS("Success to read the serial port.");
    return len_result;
}

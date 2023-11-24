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

#include <cstring>

#ifdef __GNUC__
#include <dirent.h>
#include <fcntl.h>
#include <unistd.h>
#endif // __GNUC__

#include "rmvl/core/util.hpp"
#include "rmvl/core/serial.hpp"

#ifdef __GNUC__

void rm::SerialPort::open()
{
    _is_open = false;

    DIR *dir = nullptr;
    std::string file_name = _device;
    const char *dir_path = "/dev/";
    if ((dir = opendir(dir_path)) != nullptr)
    {
        // 未指定设备名则搜寻 ttyUSB 或 ttyACM
        if (_device.empty())
        {
            dirent *dire = nullptr;
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
        ERROR_("Cannot find the directory: \"%s\"", dir_path);
        return;
    }

    if (file_name.empty())
    {
        ERROR_("Cannot find the serial port.");
        return;
    }
    else
        file_name = dir_path + file_name;

    INFO_("Opening the serial port: %s", file_name.c_str());
    _fd = ::open(file_name.c_str(), O_RDWR | O_NOCTTY | O_NDELAY); // 非堵塞情况

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

void rm::SerialPort::close()
{
    if (_is_open)
        ::close(_fd);
    _is_open = false;
}

ssize_t rm::SerialPort::fdwrite(void *data, size_t length)
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

/**
 * @brief Read data
 *
 * @param data Start position of the data
 * @param len The length of the data to be read
 * @return Length
 */
ssize_t rm::SerialPort::fdread(void *data, size_t len)
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

#endif // __GNUC__

/**
 * @file io_impl.hpp
 * @author zhaoxi (535394140@qq.com)
 * @brief 数据 IO 与通信模块实现
 * @version 1.0
 * @date 2024-09-14
 *
 * @copyright Copyright 2024 (c), zhaoxi
 *
 */

#pragma once

#ifdef _WIN32
#include <windows.h>
#else
#include <termios.h>
#endif

#include "rmvl/core/io.hpp"

namespace rm
{

class SerialPort::Impl
{
public:
#ifdef _WIN32
    explicit Impl(const std::string &device = {}, int baud_rate = 115200) : _device(device), _baud_rate(baud_rate) { open(); }
#else
    explicit Impl(const std::string &device = {}, int baud_rate = B115200) : _device(device), _baud_rate(baud_rate) { open(); }
#endif

    ~Impl() { close(); }

    void open();
    void close();

    //! 串口是否打开
    inline bool isOpened() const { return _is_open; };
    //! 写入数据
    long int fdwrite(void *data, std::size_t len);
    //! 读取数据
    long int fdread(void *data, std::size_t len);

private:
#ifdef _WIN32
    HANDLE _handle{INVALID_HANDLE_VALUE}; //!< 文件句柄
#else
    int _fd{-1};     //!< 文件描述符
    termios _option; //!< 终端控制
#endif
    bool _is_open{};     //!< 串口打开标志位
    std::string _device; //!< 设备名
    int _baud_rate{};    //!< 波特率
};

class PipeServer::Impl
{
public:
    explicit Impl(std::string_view name);

    ~Impl();

    //! 读取数据
    bool read(std::string &data);
    //! 写入数据
    bool write(std::string_view data);

private:
    std::string _name; //!< 命名管道名称

#ifdef _WIN32
    HANDLE _handle{INVALID_HANDLE_VALUE}; //!< 文件句柄
#else
    int _fd{-1}; //!< 文件描述符
#endif
};

class PipeClient::Impl
{
public:
    explicit Impl(std::string_view name);

    ~Impl();

    //! 读取数据
    bool read(std::string &data);
    //! 写入数据
    bool write(std::string_view data);

private:
#ifdef _WIN32
    HANDLE _handle{}; //!< 文件句柄
#else
    int _fd{}; //!< 文件描述符
#endif
};

} // namespace rm

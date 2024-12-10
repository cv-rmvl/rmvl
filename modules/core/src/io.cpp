/**
 * @file io.cpp
 * @author zhaoxi (535394140@qq.com)
 * @brief
 * @version 1.0
 * @date 2024-09-14
 *
 * @copyright Copyright 2024 (c), zhaoxi
 *
 */

#ifndef _WIN32
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#endif

#include "io_impl.hpp"
#include "rmvl/core/util.hpp"

namespace rm
{

RMVL_IMPL_DEF(PipeServer)
RMVL_IMPL_DEF(PipeClient)

PipeServer::PipeServer(std::string_view name) : _impl(new Impl(name)) {}
bool PipeServer::read(std::string &data) { return _impl->read(data); }
bool PipeServer::write(std::string_view data) { return _impl->write(data); }

PipeClient::PipeClient(std::string_view name) : _impl(new Impl(name)) {}
bool PipeClient::read(std::string &data) { return _impl->read(data); }
bool PipeClient::write(std::string_view data) { return _impl->write(data); }

using namespace std::string_literals;

#ifdef _WIN32

static void closePipe(HANDLE handle)
{
    if (handle != INVALID_HANDLE_VALUE)
        CloseHandle(handle);
}

static bool readPipe(HANDLE handle, std::string &data)
{
    RMVL_DbgAssert(handle != INVALID_HANDLE_VALUE);
    char buffer[1024];
    DWORD len{};
    if (!ReadFile(handle, buffer, sizeof(buffer), &len, nullptr))
    {
        ERROR_("Failed to read from named pipe");
        return false;
    }
    data.assign(buffer, len);
    return true;
}

static bool writePipe(HANDLE handle, std::string_view data)
{
    RMVL_DbgAssert(handle != INVALID_HANDLE_VALUE);
    DWORD len{};
    if (!WriteFile(handle, data.data(), static_cast<DWORD>(data.size()), &len, nullptr))
    {
        ERROR_("Failed to write to named pipe");
        return false;
    }
    return true;
}

PipeServer::Impl::Impl(std::string_view name) : _name("\\\\.\\pipe\\"s + name.data())
{
    _handle = CreateNamedPipeA(
        _name.c_str(),                                   // 命名管道名称
        PIPE_ACCESS_DUPLEX,                              // 读写权限
        PIPE_TYPE_BYTE | PIPE_READMODE_BYTE | PIPE_WAIT, // 消息类型管道
        PIPE_UNLIMITED_INSTANCES,                        // 无限实例
        1024,                                            // 输出缓冲区大小
        1024,                                            // 输入缓冲区大小
        0,                                               // 默认超时时间
        nullptr);                                        // 默认安全属性
    if (_handle == INVALID_HANDLE_VALUE)
    {
        ERROR_("Failed to create named pipe");
        return;
    }
    if (!ConnectNamedPipe(_handle, nullptr))
    {
        ERROR_("Failed to connect named pipe");
        CloseHandle(_handle);
        return;
    }
}

PipeClient::Impl::Impl(std::string_view name)
{
    std::string pipe_name = "\\\\.\\pipe\\"s + name.data();
    _handle = CreateFileA(
        pipe_name.c_str(),            // 命名管道名称
        GENERIC_READ | GENERIC_WRITE, // 读写权限
        0,                            // 不共享
        nullptr,                      // 默认安全属性
        OPEN_EXISTING,                // 打开已存在的管道
        0,                            // 默认属性
        nullptr);                     // 无模板文件
    if (_handle == INVALID_HANDLE_VALUE)
    {
        ERROR_("Failed to open named pipe");
        return;
    }
}

PipeServer::Impl::~Impl()
{
    if (!DisconnectNamedPipe(_handle))
        ERROR_("Failed to disconnect named pipe");
    closePipe(_handle);
}

bool PipeServer::Impl::read(std::string &data) { return readPipe(_handle, data); }
bool PipeServer::Impl::write(std::string_view data) { return writePipe(_handle, data); }

PipeClient::Impl::~Impl() { closePipe(_handle); }
bool PipeClient::Impl::read(std::string &data) { return readPipe(_handle, data); }
bool PipeClient::Impl::write(std::string_view data) { return writePipe(_handle, data); }

#else

static inline int openPipe(std::string_view name)
{
    int fd = open(name.data(), O_RDWR);
    if (fd == -1)
        ERROR_("Failed to open named pipe");
    return fd;
}

static inline void closePipe(int fd)
{
    if (fd != -1)
        ::close(fd);
}

static bool readPipe(int fd, std::string &data)
{
    RMVL_DbgAssert(fd != -1);
    char buffer[1024];
    ssize_t len = ::read(fd, buffer, sizeof(buffer));
    if (len > 0)
    {
        data.assign(buffer, len);
        return true;
    }
    else
    {
        ERROR_("Failed to read from named pipe");
        return false;
    }
}

static bool writePipe(int fd, std::string_view data)
{
    RMVL_DbgAssert(fd != -1);
    ssize_t len = ::write(fd, data.data(), data.size());
    if (len == -1)
    {
        ERROR_("Failed to write to named pipe");
        return false;
    }
    return true;
}

PipeServer::Impl::Impl(std::string_view name) : _name("/tmp/"s + name.data())
{
    if (mkfifo(_name.data(), 0666))
    {
        ERROR_("Failed to create named pipe");
        return;
    }
    _fd = openPipe(_name);
}

PipeServer::Impl::~Impl()
{
    closePipe(_fd);
    unlink(_name.c_str());
}
bool PipeServer::Impl::read(std::string &data) { return readPipe(_fd, data); }
bool PipeServer::Impl::write(std::string_view data) { return writePipe(_fd, data); }

PipeClient::Impl::Impl(std::string_view name) : _fd(openPipe("/tmp/"s + name.data())) {}
PipeClient::Impl::~Impl() { closePipe(_fd); }
bool PipeClient::Impl::read(std::string &data) { return readPipe(_fd, data); }
bool PipeClient::Impl::write(std::string_view data) { return writePipe(_fd, data); }

#endif

} // namespace rm

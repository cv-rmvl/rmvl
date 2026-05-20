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
#include <mqueue.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#endif

#if __cplusplus >= 202002L
#ifndef _WIN32
#include <sys/epoll.h>
#endif
#endif

#include <cstring>
#include <utility>

#include "rmvl/core/util.hpp"
#include "rmvl/io/ipc.hpp"

#ifndef _WIN32
#include "rmvlpara/io.hpp"
#endif

namespace rm {

using namespace std::string_literals;

/////////////////////////////// Pipe ///////////////////////////////

#ifdef _WIN32

static void closePipe(HANDLE handle) {
    if (handle != INVALID_HANDLE_VALUE)
        CloseHandle(handle);
}

static std::string readPipe(HANDLE handle) {
    RMVL_DbgAssert(handle != INVALID_HANDLE_VALUE);
    char buffer[1024];
    DWORD len{};
    bool success = ReadFile(handle, buffer, sizeof(buffer), &len, nullptr);
    if (!success) {
        ERROR_("Failed to read from named pipe");
        return "";
    }
    return std::string(buffer, len);
}

static bool writePipe(HANDLE handle, std::string_view data) {
    RMVL_DbgAssert(handle != INVALID_HANDLE_VALUE);
    DWORD len{};
    if (!WriteFile(handle, data.data(), static_cast<DWORD>(data.size()), &len, nullptr)) {
        ERROR_("Failed to write to named pipe");
        return false;
    }
    return true;
}

PipeServer::PipeServer(std::string_view name, bool ov) : _name("\\\\.\\pipe\\"s + name.data()) {
    _fd = CreateNamedPipeA(
        _name.c_str(),                                                       // 命名管道名称
        ov ? PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED : PIPE_ACCESS_DUPLEX, // 读写权限 + 异步模式
        PIPE_TYPE_BYTE | PIPE_READMODE_BYTE | PIPE_WAIT,                     // 消息类型管道
        PIPE_UNLIMITED_INSTANCES,                                            // 无限实例
        1024,                                                                // 输出缓冲区大小
        1024,                                                                // 输入缓冲区大小
        0,                                                                   // 默认超时时间
        nullptr);                                                            // 默认安全属性
    if (_fd == INVALID_HANDLE_VALUE) {
        ERROR_("Failed to create named pipe");
        return;
    }
    if (!ConnectNamedPipe(_fd, nullptr)) {
        DWORD error = GetLastError();
        if (error != ERROR_PIPE_CONNECTED) {
            ERROR_("Failed to connect named pipe: %lu", error);
            CloseHandle(_fd);
            _fd = INVALID_HANDLE_VALUE;
            return;
        }
    }
}

PipeClient::PipeClient(std::string_view name, bool ov) {
    std::string pipe_name = "\\\\.\\pipe\\"s + name.data();
    _fd = CreateFileA(
        pipe_name.c_str(),             // 命名管道名称
        GENERIC_READ | GENERIC_WRITE,  // 读写权限
        0,                             // 不共享
        nullptr,                       // 默认安全属性
        OPEN_EXISTING,                 // 打开已存在的管道
        ov ? FILE_FLAG_OVERLAPPED : 0, // 默认属性
        nullptr);                      // 无模板文件
    if (_fd == INVALID_HANDLE_VALUE) {
        ERROR_("Failed to open named pipe");
        return;
    }
}

PipeServer::~PipeServer() {
    if (_fd == INVALID_HANDLE_VALUE)
        return;
    if (!DisconnectNamedPipe(_fd))
        ERROR_("Failed to disconnect named pipe");
    closePipe(_fd);
    _fd = INVALID_HANDLE_VALUE;
}

PipeClient::~PipeClient() {
    if (_fd != INVALID_HANDLE_VALUE) {
        closePipe(_fd);
        _fd = INVALID_HANDLE_VALUE;
    }
}

PipeServer::PipeServer(PipeServer &&other) noexcept
    : _name(std::exchange(other._name, {})), _fd(std::exchange(other._fd, INVALID_HANDLE_VALUE)) {}

PipeServer &PipeServer::operator=(PipeServer &&other) noexcept {
    if (this != &other) {
        if (_fd != INVALID_HANDLE_VALUE) {
            DisconnectNamedPipe(_fd);
            closePipe(_fd);
        }
        _name = std::exchange(other._name, {});
        _fd = std::exchange(other._fd, INVALID_HANDLE_VALUE);
    }
    return *this;
}

PipeClient::PipeClient(PipeClient &&other) noexcept : _fd(std::exchange(other._fd, INVALID_HANDLE_VALUE)) {}

PipeClient &PipeClient::operator=(PipeClient &&other) noexcept {
    if (this != &other) {
        if (_fd != INVALID_HANDLE_VALUE)
            closePipe(_fd);
        _fd = std::exchange(other._fd, INVALID_HANDLE_VALUE);
    }
    return *this;
}

SHMBase::SHMBase(std::string_view name, std::size_t size) : _size(size) {
    _fd = CreateFileMappingA(
        INVALID_HANDLE_VALUE,     // 使用系统分页文件
        nullptr,                  // 默认安全属性
        PAGE_READWRITE,           // 可读写
        0,                        // 最大对象大小（高32位）
        static_cast<DWORD>(size), // 最大对象大小（低32位）
        name.data());             // 名称
    if (_fd == nullptr) {
        ERROR_("Failed to create shared memory");
        return;
    }
    _is_creator = (GetLastError() != ERROR_ALREADY_EXISTS);
    _ptr = MapViewOfFile(_fd, FILE_MAP_ALL_ACCESS, 0, 0, size);
    if (_ptr == nullptr) {
        ERROR_("Failed to map view of file");
        CloseHandle(_fd);
        _fd = nullptr;
        return;
    }
}

SHMBase::~SHMBase() {
    if (_ptr != nullptr)
        UnmapViewOfFile(_ptr);
    if (_fd != nullptr)
        CloseHandle(_fd);
    _ptr = nullptr;
    _fd = nullptr;
    _is_creator = false;
}

SHMBase::SHMBase(SHMBase &&other) noexcept
    : _size(std::exchange(other._size, 0)),
      _name(std::exchange(other._name, {})),
      _ptr(std::exchange(other._ptr, nullptr)),
      _fd(std::exchange(other._fd, nullptr)),
      _is_creator(std::exchange(other._is_creator, false)) {}

SHMBase &SHMBase::operator=(SHMBase &&other) noexcept {
    if (this != &other) {
        if (_ptr != nullptr)
            UnmapViewOfFile(_ptr);
        if (_fd != nullptr)
            CloseHandle(_fd);
        _size = std::exchange(other._size, 0);
        _name = std::exchange(other._name, {});
        _ptr = std::exchange(other._ptr, nullptr);
        _fd = std::exchange(other._fd, nullptr);
        _is_creator = std::exchange(other._is_creator, false);
    }
    return *this;
}

#else

static std::string readPipe(int fd) noexcept {
    RMVL_DbgAssert(fd >= 0);
    char buffer[1024]{};
    ssize_t len = ::read(fd, buffer, sizeof(buffer));
    return len > 0 ? std::string(buffer, len) : std::string{};
}

static bool writePipe(int fd, std::string_view data) noexcept {
    RMVL_DbgAssert(fd >= 0);
    ssize_t len = ::write(fd, data.data(), data.size());
    if (len == -1) {
        ERROR_("Failed to write to named pipe");
        return false;
    }
    return true;
}

PipeServer::PipeServer(std::string_view name, bool) : _name("/tmp/"s + name.data()) {
    unlink(_name.c_str());
    if (mkfifo(_name.data(), 0666)) {
        ERROR_("Failed to create named pipe");
        return;
    }
    _fd = ::open(_name.data(), O_RDWR);
    if (_fd == -1) {
        ERROR_("Failed to open named pipe");
        return;
    }
}

PipeServer::~PipeServer() {
    if (_fd != -1) {
        ::close(_fd);
        _fd = -1;
    }
    if (!_name.empty())
        ::unlink(_name.c_str());
}

PipeClient::PipeClient(std::string_view name, bool) : _fd(::open(("/tmp/"s + name.data()).c_str(), O_RDWR)) {}
PipeClient::~PipeClient() {
    if (_fd != -1) {
        ::close(_fd);
        _fd = -1;
    }
}

PipeServer::PipeServer(PipeServer &&other) noexcept
    : _name(std::exchange(other._name, {})), _fd(std::exchange(other._fd, INVALID_FD)) {}

PipeServer &PipeServer::operator=(PipeServer &&other) noexcept {
    if (this != &other) {
        if (_fd != -1)
            ::close(_fd);
        if (!_name.empty())
            ::unlink(_name.c_str());
        _name = std::exchange(other._name, {});
        _fd = std::exchange(other._fd, INVALID_FD);
    }
    return *this;
}

PipeClient::PipeClient(PipeClient &&other) noexcept : _fd(std::exchange(other._fd, INVALID_FD)) {}

PipeClient &PipeClient::operator=(PipeClient &&other) noexcept {
    if (this != &other) {
        if (_fd != -1)
            ::close(_fd);
        _fd = std::exchange(other._fd, INVALID_FD);
    }
    return *this;
}

SHMBase::SHMBase(std::string_view name, std::size_t size) : _size(size), _name(name.find('/') == 0 ? std::string(name) : "/"s.append(name)) {
    _fd = ::shm_open(_name.c_str(), O_RDWR | O_CREAT | O_EXCL, 0666);

    if (_fd >= 0)
        _is_creator = true;
    else if (errno == EEXIST) {
        _fd = ::shm_open(_name.c_str(), O_RDWR, 0);
        if (_fd == -1) {
            ERROR_("Failed to open existing shared memory: %s", strerror(errno));
            return;
        }
        struct stat st{};
        if (::fstat(_fd, &st) == -1) {
            ERROR_("Failed to get shared memory status: %s", strerror(errno));
            ::close(_fd);
            _fd = -1;
            return;
        }
        if (static_cast<size_t>(st.st_size) != size) {
            ERROR_("Shared memory size mismatch: expected %zu, got %ld", size, st.st_size);
            ::close(_fd);
            _fd = -1;
            return;
        }
    } else {
        ERROR_("Failed to create shared memory: %s", strerror(errno));
        return;
    }

    // 设置内存大小
    if (_is_creator) {
        if (ftruncate(_fd, size) == -1) {
            ERROR_("Failed to set shared memory size: %s", strerror(errno));
            close(_fd);
            ::shm_unlink(_name.c_str());
            _fd = -1;
            return;
        }
    }
    _ptr = mmap(nullptr, size, PROT_READ | PROT_WRITE, MAP_SHARED, _fd, 0);
    if (_ptr == MAP_FAILED) {
        ERROR_("Failed to map shared memory: %s", strerror(errno));
        close(_fd);
        if (_is_creator)
            ::shm_unlink(_name.c_str());
        _fd = -1;
        _ptr = nullptr;
        return;
    }
}

SHMBase::~SHMBase() {
    if (_ptr != nullptr && _ptr != MAP_FAILED)
        munmap(_ptr, _size);
    if (_fd != -1)
        close(_fd);
    if (_is_creator)
        ::shm_unlink(_name.c_str());
    _ptr = nullptr;
    _fd = -1;
    _is_creator = false;
}

SHMBase::SHMBase(SHMBase &&other) noexcept
    : _size(std::exchange(other._size, 0)),
      _name(std::exchange(other._name, {})),
      _ptr(std::exchange(other._ptr, nullptr)),
      _fd(std::exchange(other._fd, INVALID_FD)),
      _is_creator(std::exchange(other._is_creator, false)) {}

SHMBase &SHMBase::operator=(SHMBase &&other) noexcept {
    if (this != &other) {
        if (_ptr != nullptr && _ptr != MAP_FAILED)
            munmap(_ptr, _size);
        if (_fd != -1)
            close(_fd);
        if (_is_creator)
            ::shm_unlink(_name.c_str());
        _size = std::exchange(other._size, 0);
        _name = std::exchange(other._name, {});
        _ptr = std::exchange(other._ptr, nullptr);
        _fd = std::exchange(other._fd, INVALID_FD);
        _is_creator = std::exchange(other._is_creator, false);
    }
    return *this;
}

#endif

std::string PipeServer::read() noexcept { return readPipe(_fd); }
bool PipeServer::write(std::string_view data) noexcept { return writePipe(_fd, data); }

std::string PipeClient::read() noexcept { return readPipe(_fd); }
bool PipeClient::write(std::string_view data) noexcept { return writePipe(_fd, data); }

#if __cplusplus >= 202002L

namespace async {

PipeServer::PipeServer(IOContext &io_context, std::string_view name) : ::rm::PipeServer(name, true), _ctx(io_context) {
#ifdef _WIN32
    if (CreateIoCompletionPort(_fd, _ctx.get().handle(), 0, 0) == nullptr) {
        auto err = GetLastError();
        if (err != ERROR_INVALID_PARAMETER)
            RMVL_Error_(RMVL_StsError, "Associate fd with IOCP failed: %lu", err);
    }
#endif
}

PipeClient::PipeClient(IOContext &io_context, std::string_view name) : ::rm::PipeClient(name, true), _ctx(io_context) {
#ifdef _WIN32
    if (CreateIoCompletionPort(_fd, _ctx.get().handle(), 0, 0) == nullptr) {
        auto err = GetLastError();
        if (err != ERROR_INVALID_PARAMETER)
            RMVL_Error_(RMVL_StsError, "Associate fd with IOCP failed: %lu", err);
    }
#endif
}

} // namespace async

#endif

//////////////////////////////// MQ ////////////////////////////////

#ifdef _WIN32

MqServer::MqServer(std::string_view name) {
    RMVL_Error(RMVL_StsBadFunc, "Message queue is not supported on Windows, please use 'rm::PipeServer' or 'rm::PipeClient' instead");
}

MqClient::MqClient(std::string_view name) {
    RMVL_Error(RMVL_StsBadFunc, "Message queue is not supported on Windows, please use 'rm::PipeServer' or 'rm::PipeClient' instead");
}

std::string MqServer::read() noexcept { return {}; }
bool MqServer::write(std::string_view, uint32_t) noexcept { return false; }

std::string MqClient::read() noexcept { return {}; }
bool MqClient::write(std::string_view, uint32_t) noexcept { return false; }

MqServer::~MqServer() {}
MqClient::~MqClient() {}

MqServer::MqServer(MqServer &&other) noexcept : _name(std::exchange(other._name, {})), _mq(std::exchange(other._mq, INVALID_HANDLE_VALUE)) {}
MqServer &MqServer::operator=(MqServer &&other) noexcept {
    if (this != &other) {
        _name = std::exchange(other._name, {});
        _mq = std::exchange(other._mq, INVALID_HANDLE_VALUE);
    }
    return *this;
}
MqClient::MqClient(MqClient &&other) noexcept : _mq(std::exchange(other._mq, INVALID_HANDLE_VALUE)) {}
MqClient &MqClient::operator=(MqClient &&other) noexcept {
    if (this != &other)
        _mq = std::exchange(other._mq, INVALID_HANDLE_VALUE);
    return *this;
}

#else

MqServer::MqServer(std::string_view name) : _name(name) {
    mq_attr mqstat{};
    mqstat.mq_flags = 0;
    mqstat.mq_maxmsg = para::io_param.MQ_MAX_MSG;
    mqstat.mq_msgsize = para::io_param.MQ_MSG_SIZE;
    mqstat.mq_curmsgs = 0;
    _mq = mq_open(name.data(), O_CREAT | O_RDWR, 0644, &mqstat);
    if (_mq < 0) {
        ERROR_("Failed to create message queue: %s", strerror(errno));
        return;
    }
}

MqServer::~MqServer() {
    if (_mq >= 0) {
        mq_close(_mq);
        _mq = -1;
    }
    if (!_name.empty())
        mq_unlink(_name.data());
}

MqServer::MqServer(MqServer &&other) noexcept : _name(std::exchange(other._name, {})), _mq(std::exchange(other._mq, INVALID_FD)) {}

MqServer &MqServer::operator=(MqServer &&other) noexcept {
    if (this != &other) {
        if (_mq >= 0)
            mq_close(_mq);
        if (!_name.empty())
            mq_unlink(_name.data());
        _name = std::exchange(other._name, {});
        _mq = std::exchange(other._mq, INVALID_FD);
    }
    return *this;
}

MqClient::MqClient(std::string_view name) : _mq(mq_open(name.data(), O_RDWR)) {
    if (_mq < 0) {
        ERROR_("Failed to connect to message queue: %s", strerror(errno));
        return;
    }
}

MqClient::~MqClient() {
    if (_mq >= 0) {
        mq_close(_mq);
        _mq = -1;
    }
}

MqClient::MqClient(MqClient &&other) noexcept : _mq(std::exchange(other._mq, INVALID_FD)) {}

MqClient &MqClient::operator=(MqClient &&other) noexcept {
    if (this != &other) {
        if (_mq >= 0)
            mq_close(_mq);
        _mq = std::exchange(other._mq, INVALID_FD);
    }
    return *this;
}

static std::string mqRead(int mq) noexcept {
    char buffer[8192]{};
    auto len = mq_receive(mq, buffer, para::io_param.MQ_MSG_SIZE, nullptr);
    return len > 0 ? std::string(buffer, len) : std::string{};
}

static bool mqWrite(int mq, std::string_view data, uint32_t prio) noexcept {
    if (mq_send(mq, data.data(), data.size(), prio) == -1) {
        ERROR_("Failed to send message");
        return false;
    }
    return true;
}

std::string MqServer::read() noexcept {
    RMVL_DbgAssert(_mq >= 0);
    return mqRead(_mq);
}

bool MqServer::write(std::string_view data, uint32_t prio) noexcept {
    RMVL_DbgAssert(_mq >= 0);
    return mqWrite(_mq, data, prio);
}

std::string MqClient::read() noexcept {
    RMVL_DbgAssert(_mq >= 0);
    return mqRead(_mq);
}

bool MqClient::write(std::string_view data, uint32_t prio) noexcept {
    RMVL_DbgAssert(_mq >= 0);
    return mqWrite(_mq, data, prio);
}

#endif

} // namespace rm

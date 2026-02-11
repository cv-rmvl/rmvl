/**
 * @file async.cpp
 * @author zhaoxi (535394140@qq.com)
 * @brief 异步 I/O 协程框架实现
 * @version 1.0
 * @date 2025-07-31
 *
 * @copyright Copyright 2025 (c), zhaoxi
 *
 */

#include <csignal>
#include <unordered_map>

#ifndef _WIN32
#include <cstring>
#include <sys/epoll.h>
#include <sys/eventfd.h>
#include <sys/signalfd.h>
#include <sys/timerfd.h>
#else
#include <mutex>
#include <vector>
#endif

#include "rmvl/core/util.hpp"
#include "rmvl/io/async.hpp"

#if __cplusplus >= 202002L

namespace rm::async {

#ifdef _WIN32

IOContext::IOContext() : _aioh(CreateIoCompletionPort(INVALID_HANDLE_VALUE, nullptr, 0, 0)) { RMVL_Assert(_aioh != INVALID_HANDLE_VALUE); }

IOContext::~IOContext() {
    stop();
    CloseHandle(_aioh);
}

void IOContext::run() {
    _running = true;
    std::unordered_map<void *, BasicTask::ptr> unfinish;
    while (_running) {
        // 优先处理就绪队列
        while (!_ready.empty()) {
            auto task = std::move(_ready.front());
            _ready.pop();
            task->handle().resume();
            if (!task->handle().done())
                unfinish.insert({task->handle().address(), std::move(task)});
        }
        // 处理 IO 事件
        constexpr std::size_t MAX_EVENTS = 256;
        OVERLAPPED_ENTRY events[MAX_EVENTS]{};
        ULONG n{};

        BOOL ok = GetQueuedCompletionStatusEx(_aioh, events, MAX_EVENTS, &n, unfinish.empty() ? 0 : INFINITE, FALSE);
        if (!ok) {
            // 处理错误
            DWORD error = GetLastError();
            if (error != WAIT_TIMEOUT)
                RMVL_Error_(RMVL_StsError, "GetQueuedCompletionStatusEx failed with error: %lu", error);
            continue;
        }

        for (ULONG i = 0; i < n; ++i) {
            // 处理唤醒事件
            if (events[i].lpOverlapped == nullptr)
                continue;
            // 处理其他 IO 事件
            auto handle = CONTAINING_RECORD(events[i].lpOverlapped, IocpOverlapped, ov)->handle;
            if (unfinish.contains(handle.address())) {
                // IO 事件就绪，加入就绪队列可被唤醒
                _ready.emplace(std::move(unfinish.at(handle.address())));
                // Task 已被移动，直接删除
                unfinish.erase(handle.address());
            } else {
                // 未在 unfinish 中的协程，说明是 unfinish 中协程的子协程，直接唤醒
                handle.resume();
            }
        }
    }
}

void IOContext::stop() noexcept {
    _running = false;
    // 向 IOCP 投递一个空的完成包以唤醒阻塞
    PostQueuedCompletionStatus(_aioh, 0, 0, nullptr);
}

void AsyncReadAwaiter::await_suspend(std::coroutine_handle<> handle) {
    RMVL_DbgAssert(_fd != INVALID_FD);
    _ovl = std::make_unique<IocpOverlapped>(handle);
    if (!ReadFile(_fd, _ovl->buf, sizeof(_ovl->buf), nullptr, &_ovl->ov)) {
        DWORD error = GetLastError();
        if (error != ERROR_IO_PENDING)
            RMVL_Error_(RMVL_StsError, "ReadFile failed with error: %lu", error);
    }
}

std::string AsyncReadAwaiter::await_resume() {
    RMVL_DbgAssert(_ovl != nullptr);
    DWORD bytes_transferred = 0;
    if (!GetOverlappedResult(_fd, &_ovl->ov, &bytes_transferred, false)) {
        WARNING_("AsyncRead: GetOverlappedResult failed with error: %lu", GetLastError());
        return {};
    }
    return std::string(_ovl->buf, bytes_transferred);
}

void AsyncWriteAwaiter::await_suspend(std::coroutine_handle<> handle) {
    RMVL_DbgAssert(_fd != INVALID_FD);
    _ovl = std::make_unique<IocpOverlapped>(handle);
    if (!WriteFile(_fd, _data.data(), static_cast<DWORD>(_data.size()), nullptr, &_ovl->ov)) {
        DWORD error = GetLastError();
        if (error != ERROR_IO_PENDING)
            RMVL_Error_(RMVL_StsBadArg, "WriteFile failed with error: %lu", error);
    }
}

bool AsyncWriteAwaiter::await_resume() {
    RMVL_DbgAssert(_ovl != nullptr);
    DWORD bytes_transferred = 0;
    if (!GetOverlappedResult(_fd, &_ovl->ov, &bytes_transferred, false)) {
        WARNING_("AsyncWrite: GetOverlappedResult failed with error: %lu", GetLastError());
        return false;
    }
    return bytes_transferred == _data.size();
}

Timer::Timer(IOContext &io_context) : _ctx(io_context) {}

Timer::~Timer() = default;

struct TimerContext {
    FileDescriptor aioh{};
    IocpOverlapped *ovl{};
};

void CALLBACK timer_callback(PTP_CALLBACK_INSTANCE, PVOID context, PTP_TIMER timer) {
    auto timer_context = reinterpret_cast<TimerContext *>(context);
    // 手动投递完成包
    PostQueuedCompletionStatus(timer_context->aioh, 0, 0, &timer_context->ovl->ov);
    CloseThreadpoolTimer(timer);
}

void Timer::TimerAwaiter::await_suspend(std::coroutine_handle<> handle) {
    _ovl = std::make_unique<IocpOverlapped>(handle);
    static_assert(sizeof(TimerContext) <= sizeof(_ovl->info), "TimerContext size exceeds buffer size");
    auto timer_ctx = new (_ovl->info) TimerContext{_aioh, _ovl.get()};
    _fd = CreateThreadpoolTimer(timer_callback, timer_ctx, nullptr);
    if (_fd == nullptr) {
        handle.resume();
        return;
    }

    LARGE_INTEGER due_time{};
    due_time.QuadPart = -static_cast<LONGLONG>(_duration * 10000.0); // 毫秒转换为 100 纳秒
    FILETIME ft{};
    ft.dwLowDateTime = due_time.LowPart;
    ft.dwHighDateTime = due_time.HighPart;

    SetThreadpoolTimer(reinterpret_cast<PTP_TIMER>(_fd), &ft, 0, 0);
}

void Timer::TimerAwaiter::await_resume() noexcept {}

namespace helper {

struct WinSignalState {
    int signum{};
    IOContext *ctx;
    IocpOverlapped *waiting_ovl{nullptr};
    std::atomic<int> pending_count{0};
};

static std::mutex g_sig_mtx{};
static std::vector<std::unique_ptr<WinSignalState>> g_sig_states{};
static bool g_handler_installed = false;

static BOOL WINAPI ConsoleCtrlHandler(DWORD dwCtrlType) {
    int signum = 0;
    switch (dwCtrlType) {
    case CTRL_C_EVENT:
        signum = SIGINT;
        break;
    case CTRL_BREAK_EVENT:
    case CTRL_CLOSE_EVENT:
        signum = SIGTERM;
        break;
    default:
        return FALSE;
    }

    std::lock_guard<std::mutex> lock(g_sig_mtx);
    bool handled = false;
    for (const auto &state_ptr : g_sig_states) {
        auto *state = state_ptr.get();
        if (state->signum == signum) {
            if (state->waiting_ovl) {
                // 有协程在等待，直接挂入 IOCP 唤醒
                PostQueuedCompletionStatus(state->ctx->handle(), 0, 0, &state->waiting_ovl->ov);
                state->waiting_ovl = nullptr;
            } else
                state->pending_count.fetch_add(1);
            handled = true;
        }
    }
    return handled ? TRUE : FALSE;
}

} // namespace helper

Signal::Signal(IOContext &io_context, int signum) : _ctx(io_context) {
    std::lock_guard<std::mutex> lock(helper::g_sig_mtx);
    if (!helper::g_handler_installed) {
        SetConsoleCtrlHandler(helper::ConsoleCtrlHandler, TRUE);
        helper::g_handler_installed = true;
    }

    // 创建状态对象并作为虚假的 _fd 存储
    auto state = std::make_unique<helper::WinSignalState>(signum, &io_context);
    auto raw_ptr = state.get();
    helper::g_sig_states.push_back(std::move(state));
    _fd = static_cast<HANDLE>(raw_ptr);
}

Signal::~Signal() {
    if (_fd != INVALID_HANDLE_VALUE) {
        auto *raw_ptr = static_cast<helper::WinSignalState *>(_fd);
        {
            std::lock_guard<std::mutex> lock(helper::g_sig_mtx);
            std::erase_if(helper::g_sig_states, [raw_ptr](const auto &ptr) { return ptr.get() == raw_ptr; });
        }
        _fd = INVALID_HANDLE_VALUE;
    }
}

void Signal::SignalAwaiter::await_suspend(std::coroutine_handle<> handle) {
    _ovl = std::make_unique<IocpOverlapped>(handle);
    auto state = static_cast<helper::WinSignalState *>(_fd);

    std::lock_guard<std::mutex> lock(helper::g_sig_mtx);
    if (state->pending_count.load() > 0) {
        // 有挂起的信号，消耗一个并立即投递完成包以唤醒协程
        state->pending_count.fetch_sub(1);
        PostQueuedCompletionStatus(_aioh, 0, 0, &_ovl->ov);
    } else {
        // 无挂起信号，注册等待
        state->waiting_ovl = _ovl.get();
    }
}

int Signal::SignalAwaiter::await_resume() {
    auto state = static_cast<helper::WinSignalState *>(_fd);
    return state->signum;
}

#else

IOContext::IOContext() : _aioh(epoll_create1(0)), _wakeup(eventfd(0, EFD_CLOEXEC)) {
    RMVL_Assert(_aioh >= 0 && _wakeup >= 0);

    epoll_event ev{};
    ev.events = EPOLLIN;
    ev.data.fd = _wakeup;
    epoll_ctl(_aioh, EPOLL_CTL_ADD, _wakeup, &ev);
}

IOContext::~IOContext() {
    stop();
    ::close(_aioh);
}

void IOContext::run() {
    _running.store(true, std::memory_order_release);
    std::unordered_map<void *, BasicTask::ptr> unfinish;
    while (_running.load(std::memory_order_acquire)) {
        // 优先处理就绪队列
        while (!_ready.empty()) {
            auto task = std::move(_ready.front());
            _ready.pop();
            task->handle().resume();
            if (!task->handle().done())
                unfinish.insert({task->handle().address(), std::move(task)});
        }
        // 处理 IO 事件
        constexpr std::size_t MAX_EVENTS = 256;
        epoll_event events[MAX_EVENTS]{};
        int n = epoll_wait(_aioh, events, MAX_EVENTS, unfinish.empty() ? 0 : -1);
        for (int i = 0; i < n; ++i) {
            // 处理唤醒事件
            if (events[i].data.fd == _wakeup) {
                uint64_t buf{};
                // 清除事件
                [[maybe_unused]] auto _ = read(_wakeup, &buf, sizeof(buf));
                continue;
            }
            // 处理其他 IO 事件
            auto handle = std::coroutine_handle<>::from_address(events[i].data.ptr);
            if (unfinish.contains(handle.address())) {
                // IO 事件就绪，加入就绪队列可被唤醒
                _ready.emplace(std::move(unfinish.at(handle.address())));
                // Task 已被移动，直接删除
                unfinish.erase(handle.address());
            } else {
                // 未在 unfinish 中的协程，说明是 unfinish 中协程的子协程，直接唤醒
                handle.resume();
            }
        }
    }
}

void IOContext::stop() noexcept {
    _running.store(false, std::memory_order_release);
    uint64_t one = 1;
    [[maybe_unused]] auto _ = write(_wakeup, &one, sizeof(one));
}

void AsyncReadAwaiter::await_suspend(std::coroutine_handle<> handle) {
    RMVL_DbgAssert(_fd >= 0);
    epoll_event ev{};
    ev.events = EPOLLIN;
    ev.data.ptr = handle.address();
    if (epoll_ctl(_aioh, EPOLL_CTL_ADD, _fd, &ev) == -1)
        RMVL_Error_(RMVL_StsBadArg, "Failed to add fd to epoll: %s", strerror(errno));
}

std::string AsyncReadAwaiter::await_resume() {
    RMVL_DbgAssert(_fd >= 0);
    epoll_ctl(_aioh, EPOLL_CTL_DEL, _fd, nullptr);
    std::string buf{};
    buf.resize(65536);
    ssize_t n = ::read(_fd, buf.data(), buf.size());
    if (n > 0) {
        buf.resize(n);
        return buf;
    }
    return {};
}

void AsyncWriteAwaiter::await_suspend(std::coroutine_handle<> handle) {
    RMVL_DbgAssert(_fd >= 0);
    epoll_event ev{};
    ev.events = EPOLLOUT;
    ev.data.ptr = handle.address();
    if (epoll_ctl(_aioh, EPOLL_CTL_ADD, _fd, &ev) == -1)
        RMVL_Error_(RMVL_StsBadArg, "Failed to add fd to epoll: %s", strerror(errno));
}

bool AsyncWriteAwaiter::await_resume() {
    RMVL_DbgAssert(_fd >= 0);
    epoll_ctl(_aioh, EPOLL_CTL_DEL, _fd, nullptr);
#if _WIN32
    ssize_t n = ::write(_fd, _data.data(), static_cast<int>(_data.size()));
#else
    ssize_t n = ::write(_fd, _data.data(), _data.size());
#endif
    return n == static_cast<ssize_t>(_data.size());
}

Timer::Timer(IOContext &io_context) : _ctx(io_context) {
    _fd = timerfd_create(CLOCK_MONOTONIC, TFD_CLOEXEC);
    if (_fd == -1)
        RMVL_Error_(RMVL_StsError, "Failed to create timerfd: %s", strerror(errno));
}

Timer::~Timer() {
    if (_fd != -1) {
        close(_fd);
        _fd = -1;
    }
}

void Timer::TimerAwaiter::await_suspend(std::coroutine_handle<> handle) {
    RMVL_DbgAssert(_fd >= 0);
    itimerspec tconfig{};
    std::memset(&tconfig, 0, sizeof(tconfig));
    tconfig.it_value.tv_sec = static_cast<time_t>(_duration / 1000);
    tconfig.it_value.tv_nsec = static_cast<long>((_duration / 1000 - tconfig.it_value.tv_sec) * 1e9);

    if (timerfd_settime(_fd, 0, &tconfig, nullptr) == -1)
        RMVL_Error_(RMVL_StsBadArg, "Failed to set timerfd: %s", strerror(errno));

    epoll_event ev{};
    ev.events = EPOLLIN;
    ev.data.ptr = handle.address();

    // 尝试修改已存在的文件描述符，如果不存在则添加新的
    if (epoll_ctl(_aioh, EPOLL_CTL_MOD, _fd, &ev) == -1) {
        if (errno == ENOENT) {
            // 文件描述符不在 epoll 中，添加新的
            if (epoll_ctl(_aioh, EPOLL_CTL_ADD, _fd, &ev) == -1)
                RMVL_Error_(RMVL_StsBadArg, "Failed to add timerfd to epoll: %s", strerror(errno));
        } else {
            RMVL_Error_(RMVL_StsBadArg, "Failed to modify timerfd in epoll: %s", strerror(errno));
        }
    }
}

void Timer::TimerAwaiter::await_resume() noexcept {
    RMVL_DbgAssert(_fd >= 0);
    epoll_ctl(_aioh, EPOLL_CTL_DEL, _fd, nullptr);
    uint64_t expirations;
    [[maybe_unused]] auto _ = read(_fd, &expirations, sizeof(expirations));
}

Signal::Signal(IOContext &io_context, int signum) : _ctx(io_context) {
    sigset_t mask{};
    sigemptyset(&mask);
    sigaddset(&mask, signum);
    sigprocmask(SIG_BLOCK, &mask, nullptr);
    _fd = signalfd(-1, &mask, 0);
    if (_fd == -1)
        RMVL_Error_(RMVL_StsError, "Failed to set signalfd mask: %s", strerror(errno));
}

Signal::~Signal() {
    if (_fd != -1) {
        close(_fd);
        _fd = -1;
    }
}

void Signal::SignalAwaiter::await_suspend(std::coroutine_handle<> handle) {
    RMVL_DbgAssert(_fd >= 0);
    epoll_event ev{};
    ev.events = EPOLLIN;
    ev.data.ptr = handle.address(); // 设置唤醒时的协程句柄地址
    // 将 signalfd 加入到 epoll 实例中进行监听
    if (epoll_ctl(_aioh, EPOLL_CTL_ADD, _fd, &ev) == -1)
        RMVL_Error_(RMVL_StsError, "Failed to add signalfd to epoll: %s", strerror(errno));
}

int Signal::SignalAwaiter::await_resume() {
    RMVL_DbgAssert(_fd >= 0);
    epoll_ctl(_aioh, EPOLL_CTL_DEL, _fd, nullptr);

    signalfd_siginfo fdsi{};
    auto s = read(_fd, &fdsi, sizeof(signalfd_siginfo));
    if (s != sizeof(signalfd_siginfo))
        return -1;

    return static_cast<int>(fdsi.ssi_signo);
}

#endif

} // namespace rm::async

#endif // __cplusplus >= 202002L

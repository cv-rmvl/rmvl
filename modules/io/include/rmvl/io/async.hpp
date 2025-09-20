/**
 * @file async.hpp
 * @author zhaoxi (535394140@qq.com)
 * @brief 异步 I/O 协程框架
 * @version 1.0
 * @date 2025-07-31
 *
 * @copyright Copyright 2025 (c), zhaoxi
 *
 */

#pragma once

#ifdef _WIN32
#include <winsock2.h>
#endif

#include <atomic>
#include <exception>
#include <functional>
#include <memory>
#include <queue>
#include <string>
#include <string_view>

#if __cplusplus >= 202002L

#include <chrono>
#include <coroutine>

namespace rm {

//! @addtogroup io
//! @{

//! 文件描述符类型定义
#ifdef _WIN32
using FileDescriptor = HANDLE;
inline const FileDescriptor INVALID_FD = INVALID_HANDLE_VALUE;
#else
using FileDescriptor = int;
constexpr FileDescriptor INVALID_FD = -1;
#endif

//! @} io

namespace async {

//! @addtogroup io
//! @{

template <typename Tp>
class Promise;

//! `final_suspend` 等待器
template <typename Tp>
struct FinalAwaiter {
    using handle_t = std::coroutine_handle<Promise<Tp>>;

    bool await_ready() const noexcept { return false; }
    std::coroutine_handle<> await_suspend(handle_t handle) noexcept {
        return handle.promise().previous ? handle.promise().previous : std::noop_coroutine();
    }
    void await_resume() noexcept {}
};

//! 协程承诺基类，管理协程的生命周期和异常处理
class BasicPromise {
public:
    std::suspend_always initial_suspend() noexcept { return {}; }
    void unhandled_exception() { _exception = std::current_exception(); }

    std::coroutine_handle<> previous{};

protected:
    std::exception_ptr _exception{nullptr};
};

//! 异步协程承诺
template <typename Tp>
class Promise : public BasicPromise {
public:
    auto get_return_object() noexcept { return std::coroutine_handle<Promise>::from_promise(*this); }
    FinalAwaiter<Tp> final_suspend() noexcept { return {}; }
    void return_value(Tp value) noexcept { _value = std::move(value); }

    Tp get() {
        if (_exception)
            std::rethrow_exception(_exception);
        return std::move(_value);
    }

private:
    Tp _value{};
};

//! 特化 `void` 类型的异步协程承诺
template <>
class Promise<void> : public BasicPromise {
public:
    auto get_return_object() noexcept { return std::coroutine_handle<Promise>::from_promise(*this); }
    FinalAwaiter<void> final_suspend() noexcept { return {}; }
    void return_void() {
        if (_exception)
            std::rethrow_exception(_exception);
    }
};

/**
 * @brief 协程任务等待器
 * @code {.cpp}
 * // 使用示例
 * Task<int> f();
 * Task<> g() {
 *     int val = co_await f();
 *     // ...
 *     co_return;
 * }
 * @endcode
 */
template <typename Tp>
struct TaskAwaiter {
    using handle_t = std::coroutine_handle<Promise<Tp>>;

    bool await_ready() const noexcept { return false; }
    handle_t await_suspend(std::coroutine_handle<> handle) noexcept {
        self.promise().previous = handle;
        return self;
    }
    Tp await_resume() noexcept {
        if constexpr (!std::is_void_v<Tp>)
            return self.promise().get();
        else
            return;
    }

    handle_t self{};
};

//! 保有 `rm::async::Promise` 的异步协程任务
template <typename Tp = void>
class Task final {
public:
    using value_type = Tp;
    using promise_type = Promise<Tp>;
    using handle_t = std::coroutine_handle<promise_type>;

    Task(handle_t h) : _handle(h) {}
    Task(const Task &) = delete;
    Task &operator=(const Task &) = delete;

    Task(Task &&other) noexcept : _handle(other._handle) { other._handle = nullptr; }
    Task &operator=(Task &&other) noexcept = default;

    TaskAwaiter<Tp> operator co_await() noexcept { return TaskAwaiter<Tp>{_handle}; }

    ~Task() { _handle ? _handle.destroy() : void(0); }

    std::coroutine_handle<> handle() const noexcept { return _handle; }

private:
    handle_t _handle{};
};

//! 可调用的协程任务概念
template <typename Callable, typename... Args>
concept InvokableTask = requires(Callable &&c, Args &&...args) {
    typename std::invoke_result_t<Callable, Args...>::promise_type;
    { std::invoke(std::forward<Callable>(c), std::forward<Args>(args)...) };
};

//! 异步 I/O 执行上下文，负责管理 IO 事件循环和协程任务的调度
class IOContext {
    friend class AsyncIOAwaiter;

public:
    //! 创建异步 I/O 执行上下文，初始化以异步 I/O 为核心的调度器
    IOContext();

    //! @cond
    IOContext(const IOContext &) = delete;
    IOContext &operator=(const IOContext &) = delete;
    IOContext(IOContext &&) noexcept = delete;
    IOContext &operator=(IOContext &&) noexcept = delete;
    //! @endcond

    ~IOContext();

    /**
     * @brief 生成 `Task<>` 协程任务，并添加到调度器
     *
     * @param[in] fn 协程函数
     * @param[in] args 协程函数参数
     *
     * @note
     * - `fn` 必须是一个返回 `Task<Tp>` 的协程函数
     * - 带捕获列表的 lambda 也可以作为 `fn`，但极易引起悬垂引用的未定义行为
     */
    template <typename Callable, typename... Args>
        requires InvokableTask<Callable, Args...>
    void spawn(Callable &&fn, Args &&...args) {
        using Tp = typename std::invoke_result_t<Callable, Args...>::value_type;
        _ready.emplace(std::make_unique<TaskWrapper<Tp>>(std::invoke(std::forward<Callable>(fn), std::forward<Args>(args)...)));
    }

    //! 获取异步 I/O 句柄
    FileDescriptor handle() const noexcept { return _aioh; }

    //! 启用事件循环
    void run();

    //! 停止事件循环
    void stop() noexcept;

    //! 获取运行状态
    bool running() const noexcept { return _running; }

private:
    struct BasicTask {
        using ptr = std::unique_ptr<BasicTask>;

        virtual ~BasicTask() = default;
        virtual std::coroutine_handle<> handle() const noexcept = 0;
    };

    template <typename Tp>
    struct TaskWrapper : public BasicTask {
        using ptr = std::unique_ptr<TaskWrapper>;

        explicit TaskWrapper(Task<Tp> &&t) : task(std::move(t)) {}
        std::coroutine_handle<> handle() const noexcept override { return task.handle(); }

        Task<Tp> task{};
    };

    std::atomic_bool _running{};      //!< 运行状态
    FileDescriptor _aioh{INVALID_FD}; //!< 异步 I/O 句柄

#ifndef _WIN32
    int _wakeup{-1}; //!< 用于唤醒 `epoll_wait` 的 eventfd
#endif

    std::queue<BasicTask::ptr> _ready{}; //!< 就绪队列
};

//! 异步 I/O 执行上下文左值引用包装器
using IOContextRef = std::reference_wrapper<IOContext>;

/**
 * @brief 在指定的异步 I/O 执行上下文中生成并调度一个协程任务
 *
 * @param[in] ctx 异步 I/O 执行上下文
 * @param[in] fn 协程函数
 * @param[in] args 协程函数参数
 *
 * @see IOContext::spawn
 */
template <typename Callable, typename... Args>
    requires InvokableTask<Callable, Args...>
inline void co_spawn(IOContext &ctx, Callable &&fn, Args &&...args) {
    ctx.spawn(std::forward<Callable>(fn), std::forward<Args>(args)...);
}

#ifdef _WIN32

//! IOCP 重叠结构体
struct IocpOverlapped {
    OVERLAPPED ov{};
    std::coroutine_handle<> handle{};
    char buf[1024]{};

    IocpOverlapped(std::coroutine_handle<> h) : handle(h) {}
};

#endif

//! IO 事件异步等待器
class AsyncIOAwaiter {
public:
    /**
     * @brief 创建异步 IO 等待器
     *
     * @param[in] context 异步 I/O 执行上下文
     * @param[in] fd 需要监听的文件描述符（文件句柄）
     *
     * @note Windows 会将 `fd` 关联到 `context` 的 IOCP 上，而 Linux 的关联操作将延迟到具体的 `await_suspend` 中完成
     */
    AsyncIOAwaiter(IOContext &context, FileDescriptor fd) : _aioh(context._aioh), _fd(fd) {}

    //! @cond
    bool await_ready() const noexcept { return false; }
    //! @endcond

protected:
    FileDescriptor _aioh{INVALID_FD}; //!< 异步 I/O 文件描述符（Windows 下为 IOCP，Linux 下为 epoll）
    FileDescriptor _fd{INVALID_FD};   //!< 文件句柄
#ifdef _WIN32
    std::unique_ptr<IocpOverlapped> _ovl{}; //!< 重叠结构体
#endif
};

/**
 * @brief 通用异步读等待器，核心操作使用文件 I/O 系统调用的 `read`、`ReadFile`，使用者可以通过
 * - 继承该类并选择性的实现 `await_suspend` 以及 `await_resume` 方法
 * - 直接使用该类
 * 来实现自己的异步读等待器
 *
 * @code {.cpp}
 * // 使用示例
 * class Socket {
 * public:
 *     Socket(IOContext &io_context, FileDescriptor fd) : _ctx(io_context), _fd(fd) {}
 *     AsyncReadAwaiter read() { return {_ctx, _fd}; }
 *     // 其他成员函数...
 *
 * private:
 *     IOContext &_ctx;
 *     FileDescriptor _fd{INVALID_FD};
 * };
 *
 * // 在协程中使用异步读等待器
 * Task<> session(Socket &s) {
 *    auto data = co_await s.read();
 *    // 处理读取到的数据...
 * }
 * @endcode
 */
class AsyncReadAwaiter : public AsyncIOAwaiter {
public:
    /**
     * @brief 创建异步读等待器
     *
     * @param[in] ctx 异步 I/O 执行上下文
     * @param[in] fd 需要监听的文件描述符（文件句柄）
     */
    AsyncReadAwaiter(IOContext &ctx, FileDescriptor fd) : AsyncIOAwaiter(ctx, fd) {}

    //! @cond
    void await_suspend(std::coroutine_handle<> handle);
    std::string await_resume();
    //! @endcond
};

/**
 * @brief 通用异步写等待器，核心操作使用文件 I/O 系统调用的 `write`、`WriteFile`，使用者可以通过
 * - 继承该类并选择性的实现 `await_suspend` 以及 `await_resume` 方法
 * - 直接使用该类
 * 来实现自己的异步写等待器
 *
 * @code {.cpp}
 * // 使用示例
 * class Socket {
 * public:
 *     Socket(IOContext &io_context, FileDescriptor fd) : _ctx(io_context), _fd(fd) {}
 *     AsyncWriteAwaiter write(std::string_view data) { return {_ctx, _fd, data}; }
 *     // 其他成员函数...
 *
 * private:
 *     IOContext &_ctx;
 *     FileDescriptor _fd{INVALID_FD};
 * };
 *
 * // 在协程中使用异步写等待器
 * Task<> session(Socket &s) {
 *     bool success = co_await s.write("Hello, World!");
 *     if (success) {
 *         // 处理写入成功的逻辑...
 *     } else {
 *         // 处理写入失败的逻辑...
 *     }
 * }
 * @endcode
 */
class AsyncWriteAwaiter : public AsyncIOAwaiter {
public:
    /**
     * @brief 创建异步写等待器
     *
     * @param[in] ctx 异步 I/O 执行上下文
     * @param[in] fd 需要监听的文件描述符（文件句柄）
     * @param[in] data 待写入的数据
     */
    AsyncWriteAwaiter(IOContext &ctx, FileDescriptor fd, std::string_view data) : AsyncIOAwaiter(ctx, fd), _data(data) {}

    //! @cond
    void await_suspend(std::coroutine_handle<> handle);
    bool await_resume();
    //! @endcond

protected:
    std::string_view _data{}; //!< 待写入的数据
};

//! 异步定时器
class Timer {
public:
    Timer(IOContext &io_context);
    ~Timer();

    //! 定时等待器
    class TimerAwaiter : public AsyncIOAwaiter {
    public:
        TimerAwaiter(IOContext &ctx, FileDescriptor fd, double duration) : AsyncIOAwaiter(ctx, fd), _duration(duration) {}

        //! @cond
        void await_suspend(std::coroutine_handle<> handle);
        void await_resume() noexcept;
        //! @endcond

    private:
        double _duration{}; //!< 定时器持续时间
    };

    /**
     * @brief 创建一个持续指定时间的定时等待器
     *
     * @param[in] duration 定时器持续时间
     * @return 定时等待器
     */
    template <typename Rep, typename Period>
    TimerAwaiter sleep_for(const std::chrono::duration<Rep, Period> &duration) {
        auto duration_ms = std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(duration).count();
        return {_ctx, _fd, duration_ms > 0 ? duration_ms : 0};
    }

    /**
     * @brief 创建一个在指定时间点触发的定时等待器
     *
     * @param[in] time_point 定时器触发的时间点，通常使用 `rm::Timer::now()` 获取当前时间
     * @return 定时等待器
     */
    template <typename Clock, typename Duration>
    TimerAwaiter sleep_until(const std::chrono::time_point<Clock, Duration> &time_point) {
        auto duration = time_point - Clock::now();
        auto duration_ms = std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(duration).count();
        return {_ctx, _fd, duration_ms > 0 ? duration_ms : 0};
    }

private:
    IOContextRef _ctx;              //!< 异步 I/O 执行上下文
    FileDescriptor _fd{INVALID_FD}; //!< 定时器文件句柄
};

//! @} io

} // namespace async

} // namespace rm

#endif // __cplusplus >= 202002L

/**
 * @file ipc.hpp
 * @author zhaoxi (535394140@qq.com)
 * @brief 同步/异步的进程间通信框架
 * @version 1.0
 * @date 2025-07-31
 *
 * @copyright Copyright 2025 (c), zhaoxi
 *
 */

#pragma once

#include <thread>

#include "rmvl/core/rmvldef.hpp"

#include "async.hpp"

namespace rm {

//! @addtogroup io_ipc
//! @{

//! 命名管道服务端
class RMVL_EXPORTS_W PipeServer {
public:
    PipeServer(const PipeServer &) = delete;
    PipeServer(PipeServer &&) = default;
    PipeServer &operator=(const PipeServer &) = delete;
    PipeServer &operator=(PipeServer &&) = default;
    ~PipeServer();

    /**
     * @brief 在文件系统中创建新的命名管道并打开，销毁时自动移除该管道
     * @note
     * - Windows 命名管道在构造时会等待客户端连接并阻塞，除非已有客户端连接
     *
     * @param[in] name 命名管道名称，Windows 下的命名管道名称为 `\\.\pipe\` +`name`, Linux
     *                 下的命名管道名称为 `/tmp/` + `name`，长度不超过 256 个字符
     */
    RMVL_W PipeServer(std::string_view name) : PipeServer(name, false) {}

    /**
     * @brief 从管道读取数据
     *
     * @return 读取到的数据，成功时返回非空字符串，失败时返回空字符串
     */
    RMVL_W std::string read() noexcept;

    /**
     * @brief 从管道读取数据
     *
     * @param[out] data 读取到的数据
     */
    PipeServer &operator>>(std::string &data) noexcept { return (data = read(), *this); }

    /**
     * @brief 向管道写入数据
     *
     * @param[in] data 待写入的数据
     * @return 是否写入成功
     */
    RMVL_W bool write(std::string_view data) noexcept;

    /**
     * @brief 向管道写入数据
     *
     * @param[in] data 待写入的数据
     */
    PipeServer &operator<<(std::string_view data) noexcept { return (write(data), *this); }

protected:
    PipeServer(std::string_view name, bool ov);

    std::string _name;    //!< 命名管道名称
    FileDescriptor _fd{}; //!< 文件句柄
};

//! 命名管道客户端
class RMVL_EXPORTS_W PipeClient {
public:
    PipeClient(const PipeClient &) = delete;
    PipeClient(PipeClient &&) = default;
    PipeClient &operator=(const PipeClient &) = delete;
    PipeClient &operator=(PipeClient &&) = default;
    ~PipeClient();

    /**
     * @brief 打开存在的命名管道
     *
     * @param[in] name 命名管道名称
     */
    RMVL_W PipeClient(std::string_view name) : PipeClient(name, false) {}

    /**
     * @brief 从管道读取数据
     *
     * @return 读取到的数据，成功时返回非空字符串，失败时返回空字符串
     */
    RMVL_W std::string read() noexcept;

    /**
     * @brief 从管道读取数据
     *
     * @param[out] data 读取到的数据
     */
    PipeClient &operator>>(std::string &data) noexcept { return (data = read(), *this); }

    /**
     * @brief 向管道写入数据
     *
     * @param[in] data 待写入的数据
     * @return 是否写入成功
     */
    RMVL_W bool write(std::string_view data) noexcept;

    /**
     * @brief 向管道写入数据
     *
     * @param[in] data 待写入的数据
     */
    PipeClient &operator<<(std::string_view data) noexcept { return (write(data), *this); }

protected:
    PipeClient(std::string_view name, bool ov);

    FileDescriptor _fd{}; //!< 文件句柄
};

//! 消息队列服务端
class RMVL_EXPORTS_W MqServer {
public:
    /**
     * @brief 构造消息队列服务端
     *
     * @param[in] name 消息队列名称，必须以 `'/'` 开头，例如 `"/test_mq"`
     */
    RMVL_W MqServer(std::string_view name);

    MqServer(const MqServer &) = delete;
    MqServer(MqServer &&) = default;
    MqServer &operator=(const MqServer &) = delete;
    MqServer &operator=(MqServer &&) = default;
    ~MqServer();

    /**
     * @brief 从消息队列读取数据
     *
     * @return 读取到的数据，成功时返回非空字符串，失败时返回空字符串
     */
    RMVL_W std::string read() noexcept;

    /**
     * @brief 从消息队列读取数据
     *
     * @param[out] data 读取到的数据
     */
    MqServer &operator>>(std::string &data) noexcept { return (data = read(), *this); }

    /**
     * @brief 向消息队列写入数据
     *
     * @param[in] data 待写入的数据
     * @param[in] prio 消息优先级，数值越大优先级越高，相同优先级的消息按照 FIFO 处理
     * @return 是否写入成功
     */
    RMVL_W bool write(std::string_view data, uint32_t prio = 0) noexcept;

    /**
     * @brief 向消息队列写入数据
     *
     * @param[in] data 待写入的数据
     */
    MqServer &operator<<(std::string_view data) noexcept { return (write(data), *this); }

protected:
    std::string _name{};  //!< 消息队列名称
    FileDescriptor _mq{}; //!< 消息队列描述符句柄
};

//! 消息队列客户端
class RMVL_EXPORTS_W MqClient {
public:
    /**
     * @brief 构造消息队列客户端
     *
     * @param[in] name 消息队列名称
     */
    RMVL_W MqClient(std::string_view name);

    MqClient(const MqClient &) = delete;
    MqClient(MqClient &&) = default;
    MqClient &operator=(const MqClient &) = delete;
    MqClient &operator=(MqClient &&) = default;
    ~MqClient();

    /**
     * @brief 从消息队列读取数据
     *
     * @return 读取到的数据，成功时返回非空字符串，失败时返回空字符串
     */
    RMVL_W std::string read() noexcept;

    /**
     * @brief 从消息队列读取数据
     *
     * @param[out] data 读取到的数据
     */
    MqClient &operator>>(std::string &data) noexcept { return (data = read(), *this); }

    /**
     * @brief 向消息队列写入数据
     *
     * @param[in] data 待写入的数据
     * @param[in] prio 消息优先级，数值越大优先级越高，相同优先级的消息按照 FIFO 处理
     * @return 是否写入成功
     */
    RMVL_W bool write(std::string_view data, uint32_t prio = 0) noexcept;

    /**
     * @brief 向消息队列写入数据
     *
     * @param[in] data 待写入的数据
     */
    MqClient &operator<<(std::string_view data) noexcept { return (write(data), *this); }

protected:
#ifdef _WIN32
    HANDLE _mq{}; //!< 消息队列句柄
#else
    int _mq{}; //!< 消息队列描述符
#endif
};

//! 共享内存对象
class SharedMemory {
public:
    /**
     * @brief 创建或打开共享内存对象并映射到当前进程地址空间
     *
     * @param[in] name 共享内存名称
     * @param[in] size 共享内存大小
     */
    SharedMemory(std::string_view name, std::size_t size);
    ~SharedMemory();

    SharedMemory(const SharedMemory &) = delete;
    SharedMemory(SharedMemory &&) = default;
    SharedMemory &operator=(const SharedMemory &) = delete;
    SharedMemory &operator=(SharedMemory &&) = default;

    /**
     * @brief 获取共享内存映射指针
     *
     * @return 共享内存映射指针
     */
    void *data() noexcept { return _ptr; }

private:
    std::size_t _size{};  //!< 共享内存大小
    void *_ptr{nullptr};  //!< 共享内存映射指针
    FileDescriptor _fd{}; //!< 共享内存文件描述符
};

/**
 * @brief MPMC 共享内存对象
 *
 * @tparam T 共享内存数据类型
 */
template <typename T, typename = std::enable_if_t<std::is_trivially_copyable_v<T>>>
class MPMCSharedMemory : public SharedMemory {
    struct SHMWrapper {
        std::atomic_bool lock{false};
        std::atomic_bool empty{true};
        T data;
    };

public:
    /**
     * @brief 构造 MPMC 共享内存对象
     *
     * @param[in] name 共享内存名称
     */
    MPMCSharedMemory(std::string_view name) : SharedMemory(name, sizeof(SHMWrapper)) {}

    MPMCSharedMemory(const MPMCSharedMemory &) = delete;
    MPMCSharedMemory(MPMCSharedMemory &&) = delete;
    MPMCSharedMemory &operator=(const MPMCSharedMemory &) = delete;
    MPMCSharedMemory &operator=(MPMCSharedMemory &&) = delete;

    /**
     * @brief 获取共享内存中的原子数据，当 `empty()` 为真时，行为未定义
     *
     * @return 共享内存中的原子数据
     */
    T read() noexcept {
        T value;
        auto ptr = static_cast<SHMWrapper *>(this->data());
        while (ptr->lock.exchange(true, std::memory_order_acquire))
            std::this_thread::yield();
        value = ptr->data;
        ptr->lock.store(false, std::memory_order_release);
        return value;
    }

    //! 清空共享内存中的数据
    void clear() noexcept {
        auto ptr = static_cast<SHMWrapper *>(this->data());
        while (ptr->lock.exchange(true, std::memory_order_acquire))
            std::this_thread::yield();
        ptr->empty.store(true, std::memory_order_relaxed);
        ptr->lock.store(false, std::memory_order_release);
    }

    //! 判断共享内存是否为空
    bool empty() noexcept {
        auto ptr = static_cast<SHMWrapper *>(this->data());
        bool is_empty{};
        while (ptr->lock.exchange(true, std::memory_order_acquire))
            std::this_thread::yield();
        is_empty = ptr->empty.load(std::memory_order_relaxed);
        ptr->lock.store(false, std::memory_order_release);
        return is_empty;
    }

    /**
     * @brief 向共享内存中写入原子数据
     *
     * @param[in] value 待写入的原子数据
     */
    void write(const T &value) noexcept {
        auto ptr = static_cast<SHMWrapper *>(this->data());
        while (ptr->lock.exchange(true, std::memory_order_acquire))
            std::this_thread::yield();
        ptr->empty.store(false, std::memory_order_relaxed);
        ptr->data = value;
        ptr->lock.store(false, std::memory_order_release);
    }
};

/**
 * @brief SPMC 共享内存对象
 * @note 允许多个消费者并发读取，性能优于 MPMCSharedMemory，但仅限单生产者场景。
 * @tparam T 共享内存数据类型
 */
template <typename T, typename = std::enable_if_t<std::is_trivially_copyable_v<T>>>
class SPMCSharedMemory : public SharedMemory {
    struct SHMWrapper {
        std::atomic<size_t> seq{0}; // 序列号，偶数表示稳定，奇数表示正在写入
        std::atomic_bool empty{true};
        T data;
    };

public:
    /**
     * @brief 构造 SPMC 共享内存对象
     *
     * @param[in] name 共享内存名称
     */
    SPMCSharedMemory(std::string_view name) : SharedMemory(name, sizeof(SHMWrapper)) {}

    SPMCSharedMemory(const SPMCSharedMemory &) = delete;
    SPMCSharedMemory(SPMCSharedMemory &&) = delete;
    SPMCSharedMemory &operator=(const SPMCSharedMemory &) = delete;
    SPMCSharedMemory &operator=(SPMCSharedMemory &&) = delete;

    /**
     * @brief 从共享内存中读取数据，当 `empty()` 为真时，行为未定义
     * @note 多个消费者可以同时调用此函数而不会互相阻塞
     * @return 共享内存中的数据
     */
    T read() noexcept {
        auto ptr = static_cast<SHMWrapper *>(this->data());
        T value{};
        size_t seq1{}, seq2{};
        do {
            seq1 = ptr->seq.load(std::memory_order_acquire);
            if (seq1 & 1) {
                std::this_thread::yield();
                continue;
            }
            value = ptr->data;
            seq2 = ptr->seq.load(std::memory_order_acquire);
        } while (seq1 != seq2);
        return value;
    }

    /**
     * @brief 清空共享内存中的数据
     */
    void clear() noexcept {
        auto ptr = static_cast<SHMWrapper *>(this->data());
        ptr->seq.fetch_add(1, std::memory_order_release);
        ptr->empty.store(true, std::memory_order_relaxed);
        ptr->seq.fetch_add(1, std::memory_order_release);
    }

    /**
     * @brief 判断共享内存是否为空
     * @return 是否为空
     */
    bool empty() noexcept {
        auto ptr = static_cast<SHMWrapper *>(this->data());
        bool is_empty{};
        size_t seq1{}, seq2{};
        do {
            seq1 = ptr->seq.load(std::memory_order_acquire);
            if (seq1 & 1) {
                std::this_thread::yield();
                continue;
            }
            is_empty = ptr->empty.load(std::memory_order_relaxed);
            seq2 = ptr->seq.load(std::memory_order_acquire);
        } while (seq1 != seq2);
        return is_empty;
    }

    /**
     * @brief 向共享内存中写入数据
     *
     * @param[in] value 待写入的数据
     */
    void write(const T &value) noexcept {
        auto ptr = static_cast<SHMWrapper *>(this->data());
        ptr->seq.fetch_add(1, std::memory_order_release);
        ptr->data = value;
        ptr->empty.store(false, std::memory_order_relaxed);
        ptr->seq.fetch_add(1, std::memory_order_release);
    }
};

//! @} io_ipc

#if __cplusplus >= 202002L

namespace async {

//! @addtogroup io_ipc
//! @{

//! 异步命名管道服务端
class PipeServer final : public ::rm::PipeServer {
public:
    /**
     * @brief 在文件系统中创建新的命名管道并打开，销毁时自动移除该管道
     * @note
     * - Windows 命名管道在构造时会等待客户端连接并阻塞，除非已有客户端连接
     *
     * @param[in] io_context 异步 I/O 执行上下文
     * @param[in] name 命名管道名称，Windows 下的命名管道名称为 `\\.\pipe\` + `name`, Linux
     *                 下的命名管道名称为 `/tmp/` + `name`，长度不超过 256 个字符
     */
    PipeServer(IOContext &io_context, std::string_view name);

    /**
     * @brief 从管道读取数据
     *
     * @return 读取到的数据，成功时返回非空字符串，失败时返回空字符串
     * @code {.cpp}
     * // 使用示例
     * std::string str = co_await server.read();
     * @endcode
     */
    AsyncReadAwaiter read() { return {_ctx, _fd}; }

    PipeServer &operator>>(std::string &) = delete;

    /**
     * @brief 向管道写入数据
     *
     * @param[in] data 待写入的数据
     * @return 是否写入成功
     * @code {.cpp}
     * // 使用示例
     * bool success = co_await server.write("Hello World!");
     * @endcode
     */
    AsyncWriteAwaiter write(std::string_view data) { return {_ctx, _fd, data}; }

    PipeServer &operator<<(std::string_view) = delete;

private:
    IOContextRef _ctx; //!< 异步 I/O 执行上下文
};

//! 异步命名管道客户端
class PipeClient final : public ::rm::PipeClient {
public:
    PipeClient(IOContext &io_context, std::string_view name);

    /**
     * @brief 从管道读取数据
     *
     * @return 读取到的数据，成功时返回非空字符串，失败时返回空字符串
     * @code {.cpp}
     * // 使用示例
     * std::string str = co_await client.read();
     * @endcode
     */
    AsyncReadAwaiter read() { return {_ctx, _fd}; }

    PipeClient &operator>>(std::string &) = delete;

    /**
     * @brief 向管道写入数据
     *
     * @param[in] data 待写入的数据
     * @return 是否写入成功
     * @code {.cpp}
     * // 使用示例
     * bool success = co_await client.write("Hello World!");
     * @endcode
     */
    AsyncWriteAwaiter write(std::string_view data) { return {_ctx, _fd, data}; }

    PipeClient &operator<<(std::string_view) = delete;

private:
    IOContextRef _ctx; //!< 异步 I/O 执行上下文
};

//! @} io_ipc

} // namespace async

#endif // __cplusplus >= 202002L

} // namespace rm

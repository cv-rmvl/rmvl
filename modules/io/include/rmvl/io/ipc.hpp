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

//! 共享内存对象基类
class SHMBase {
public:
    /**
     * @brief 创建或打开共享内存对象并映射到当前进程地址空间
     *
     * @param[in] name 共享内存名称
     * @param[in] size 共享内存大小
     */
    SHMBase(std::string_view name, std::size_t size);
    ~SHMBase();

    SHMBase(const SHMBase &) = delete;
    SHMBase(SHMBase &&) = default;
    SHMBase &operator=(const SHMBase &) = delete;
    SHMBase &operator=(SHMBase &&) = default;

    /**
     * @brief 获取共享内存映射指针
     *
     * @return 共享内存映射指针
     */
    void *data() noexcept { return _ptr; }

    /**
     * @brief 获取共享内存映射指针
     *
     * @return 共享内存映射指针
     */
    const void *data() const noexcept { return _ptr; }

    //! 是否为创建者
    bool isCreator() const noexcept { return _is_creator; }

    /**
     * @brief 显式移除指定名称的共享内存对象，Windows 平台下调用该函数无效果
     * @details 析构中不提供清理操作，这是数据持久化优于自动清理的设计选择
     * @param[in] name 共享内存名称
     */
    static void destroy(std::string_view name);

private:
    std::size_t _size{};     //!< 共享内存大小
    std::string _name{};     //!< 共享内存名称
    void *_ptr{nullptr};     //!< 共享内存映射指针
    FileDescriptor _fd{};    //!< 共享内存文件描述符
    bool _is_creator{false}; //!< 是否为创建者
};

/**
 * @brief MPMC 原子共享内存对象
 *
 * @tparam T 共享内存数据类型
 */
template <typename T, typename = std::enable_if_t<std::is_trivially_copyable_v<T>>>
class AtomicSHM : public SHMBase {
    struct Layout {
        alignas(64) std::atomic_flag writer_mtx = ATOMIC_FLAG_INIT; // 写者互斥锁
        alignas(64) std::atomic_uint32_t seq{0};                    // 序列号：偶数=有效，奇数=正在写
        T data{};                                                   // 真实数据
    };

public:
    /**
     * @brief 构造 MPMC 共享内存对象
     *
     * @param[in] name 共享内存名称
     */
    AtomicSHM(std::string_view name);

    AtomicSHM(const AtomicSHM &) = delete;
    AtomicSHM(AtomicSHM &&) = delete;
    AtomicSHM &operator=(const AtomicSHM &) = delete;
    AtomicSHM &operator=(AtomicSHM &&) = delete;

    /**
     * @brief 原子的获取共享内存中数据，当 `empty()` 为真时，行为未定义
     *
     * @return 共享内存中的数据
     */
    bool read(T &value) noexcept;

    /**
     * @brief 向共享内存中原子的写入数据
     *
     * @param[in] value 要写入的数据
     */
    void write(const T &value) noexcept;

    //! 判断是否为空 (从未写入过)
    bool empty() const noexcept;
};

/**
 * @brief 基于共享内存的无锁 MPMC（多生产者多消费者）环形缓冲区
 *
 * @tparam T 共享数据类型，必须是可平凡复制类型
 * @tparam Capacity 环形缓冲区容量，必须是 2 的幂
 */
template <typename T, size_t Capacity, typename = std::enable_if_t<std::is_trivially_copyable_v<T>>>
class RingBufferSlotSHM : public SHMBase {
    static_assert((Capacity > 0) && ((Capacity & (Capacity - 1)) == 0), "Capacity must be a power of two");

    struct Slot {
        alignas(64) std::atomic_size_t sequence{}; // 避免伪共享
        T data{};
    };

    struct Buffer {
        alignas(64) std::atomic_size_t head{};
        alignas(64) std::atomic_size_t tail{};
        Slot slots[Capacity]{};
    };

public:
    /**
     * @brief 构造或连接到一个 MPMC 环形缓冲区
     *
     * @param[in] name 共享内存的唯一名称
     */
    RingBufferSlotSHM(std::string_view name);

    /**
     * @brief 向缓冲区中写入一个值（非阻塞）
     *
     * @param[in] value 要写入的值
     * @return 如果成功写入则返回 `true`，如果缓冲区已满则返回 `false`
     */
    bool write(const T &value) noexcept;

    /**
     * @brief 从缓冲区读取一个值（非阻塞）
     *
     * @param[out] value 读取到的值
     * @return 如果成功读取则返回 `true`，如果缓冲区为空则返回 `false`
     */
    bool read(T &value) noexcept;
};

//! 2 槽位共享内存无锁环形缓冲区 @tparam T 共享数据类型，必须是可平凡复制类型
template <typename T>
using RingBufferSlotSHM2 = RingBufferSlotSHM<T, 2>;

//! 4 槽位共享内存无锁环形缓冲区 @tparam T 共享数据类型，必须是可平凡复制类型
template <typename T>
using RingBufferSlotSHM4 = RingBufferSlotSHM<T, 4>;

//! 8 槽位共享内存无锁环形缓冲区 @tparam T 共享数据类型，必须是可平凡复制类型
template <typename T>
using RingBufferSlotSHM8 = RingBufferSlotSHM<T, 8>;

//! 16 槽位共享内存无锁环形缓冲区 @tparam T 共享数据类型，必须是可平凡复制类型
template <typename T>
using RingBufferSlotSHM16 = RingBufferSlotSHM<T, 16>;

//! 32 槽位共享内存无锁环形缓冲区 @tparam T 共享数据类型，必须是可平凡复制类型
template <typename T>
using RingBufferSlotSHM32 = RingBufferSlotSHM<T, 32>;

//! 64 槽位共享内存无锁环形缓冲区 @tparam T 共享数据类型，必须是可平凡复制类型
template <typename T>
using RingBufferSlotSHM64 = RingBufferSlotSHM<T, 64>;

//! @} io_ipc

#include "details/shm.hpp"

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

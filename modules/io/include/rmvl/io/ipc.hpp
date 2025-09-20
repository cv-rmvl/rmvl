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

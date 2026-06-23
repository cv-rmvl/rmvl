/**
 * @file ssl.hpp
 * @author zhaoxi (535394140@qq.com)
 * @brief 基于 OpenSSL 的 TLS/SSL 安全流封装
 * @version 1.0
 * @date 2026-06-21
 *
 * @copyright Copyright 2026 (c), zhaoxi
 *
 */

#pragma once

#include <functional>

#include "socket.hpp"

namespace rm {

//! @addtogroup io_net
//! @{

/**
 * @brief TLS 工作模式
 *
 * @note TLS/SSL 位于 TCP 之上、HTTP/WebSocket 等应用层协议之下。客户端与服务端在 TCP
 *       连接建立后完成 TLS 握手，握手成功后再通过加密流传输应用层数据。
 */
enum class SSLMode {
    Client, //!< 客户端模式
    Server  //!< 服务端模式
};

//! TLS 证书验证方式
enum class SSLVerifyMode {
    None, //!< 不验证对端证书
    Peer  //!< 验证对端证书链
};

/**
 * @brief TLS 上下文
 *
 * @details 保存证书、私钥、CA 等可被多个连接复用的 TLS 配置。该类不拥有 Socket，仅负责创建
 *          SSLStream 所需的 OpenSSL 上下文。
 */
class SSLContext {
public:
    //! 创建 TLS 上下文
    explicit SSLContext(SSLMode mode = SSLMode::Client);
    SSLContext(const SSLContext &) = delete;
    SSLContext(SSLContext &&) noexcept = default;

    SSLContext &operator=(const SSLContext &) = delete;
    SSLContext &operator=(SSLContext &&) noexcept = default;

    ~SSLContext() = default;

    //! 当前构建是否启用了 OpenSSL
    static bool available() noexcept;

    //! 创建客户端 TLS 上下文
    static SSLContext client() { return SSLContext(SSLMode::Client); }

    //! 创建服务端 TLS 上下文
    static SSLContext server() { return SSLContext(SSLMode::Server); }

    //! 上下文是否有效
    [[nodiscard]] bool valid() const noexcept { return _ctx != nullptr; }

    //! 获取 TLS 工作模式
    [[nodiscard]] SSLMode mode() const noexcept { return _mode; }

    //! 获取最近一次错误信息
    [[nodiscard]] std::string lasterr() const { return _lasterr; }

    /**
     * @brief 加载证书与私钥文件
     *
     * @param[in] cert_file 证书文件路径，PEM 格式
     * @param[in] key_file 私钥文件路径，PEM 格式
     * @return 是否加载成功
     */
    bool load_cert(std::string_view cert_file, std::string_view key_file);

    /**
     * @brief 加载用于验证对端证书的受信任 CA
     *
     * @note 仅在通过 set_verify_mode() 启用 SSLVerifyMode::Peer 时参与对端证书认证
     *
     * @param[in] ca_file CA 文件路径，可为空
     * @param[in] ca_path CA 目录路径，可为空
     * @return 是否加载成功
     */
    bool load_ca(std::string_view ca_file, std::string_view ca_path = {});

    //! 设置对端证书验证方式
    void set_verify_mode(SSLVerifyMode mode);

    //! @cond
    void *native_handle() const noexcept { return _ctx.get(); }
    //! @endcond

private:
    static void free_ctx(void *ctx) noexcept;

    SSLMode _mode{SSLMode::Client};                                  //!< TLS 工作模式
    std::unique_ptr<void, void (*)(void *)> _ctx{nullptr, free_ctx}; //!< OpenSSL SSL_CTX
    std::string _lasterr{};                                          //!< 最近一次错误
};

//! SSL 上下文左值引用包装器
using SSLContextRef = std::reference_wrapper<SSLContext>;

/**
 * @brief TLS 安全流
 *
 * @details SSLStream 包装一个已经建立 TCP 连接的 StreamSocket，在其上执行 TLS 握手、
 *          加密读取与加密写入。未启用 OpenSSL 时，该类仍可被包含，但实例不可用。
 */
class SSLStream {
public:
    //! 由已有 TCP 流式 Socket 创建 TLS 流
    SSLStream(StreamSocket socket, SSLContext &ctx);
    SSLStream(const SSLStream &) = delete;
    SSLStream(SSLStream &&) noexcept = default;

    SSLStream &operator=(const SSLStream &) = delete;
    SSLStream &operator=(SSLStream &&) noexcept = default;

    ~SSLStream();

    //! TLS 流是否失效
    [[nodiscard]] bool invalid() const noexcept { return _ssl == nullptr || _socket.invalid(); }

    //! 获取底层 Socket
    [[nodiscard]] const StreamSocket &socket() const noexcept { return _socket; }

    //! 获取底层 Socket
    [[nodiscard]] StreamSocket &socket() noexcept { return _socket; }

    //! 获取最近一次错误信息
    [[nodiscard]] std::string lasterr() const { return _lasterr; }

    /**
     * @brief 执行客户端 TLS 握手
     *
     * @param[in] server_name 服务器名称，用于 SNI，可为空
     * @return 握手是否成功
     */
    bool connect(std::string_view server_name = {});

    //! 执行服务端 TLS 握手
    bool accept();

    /**
     * @brief 根据上下文模式执行 TLS 握手
     *
     * @param[in] server_name 客户端模式下的 SNI，可为空
     * @return 握手是否成功
     */
    bool handshake(std::string_view server_name = {});

    //! 加密读取数据
    std::string read(size_t max_size = 65536) noexcept;

    //! 加密读取数据到指定内存
    size_t read_to(char *buf, size_t size) noexcept;

    //! 加密写入数据
    bool write(std::string_view data) noexcept;

    //! 关闭 TLS 流与底层 Socket
    void close() noexcept;

    //! @cond
    void *native_handle() const noexcept { return _ssl.get(); }
    //! @endcond

protected:
    //! 获取 TLS 上下文
    [[nodiscard]] SSLContext &context() noexcept { return _ctx; }

    //! 获取 TLS 上下文
    [[nodiscard]] const SSLContext &context() const noexcept { return _ctx; }

private:
    using SSLConnPtr = std::unique_ptr<void, void (*)(void *)>;

    static void free_ssl(void *ssl) noexcept;

    SSLContextRef _ctx;                 //!< TLS/SSL 上下文
    StreamSocket _socket;               //!< TCP Socket
    SSLConnPtr _ssl{nullptr, free_ssl}; //!< TLS/SSL 连接
    std::string _lasterr{};             //!< 最近一次错误
};

//! @} io_net

#if __cplusplus >= 202002L

namespace async {

//! @addtogroup io_net
//! @{

/**
 * @brief 异步 TLS 安全流
 *
 * @details 该类包装 rm::async::StreamSocket，并在 OpenSSL 返回 WANT_READ/WANT_WRITE
 *          时挂起协程等待对应 I/O 事件，避免阻塞 IOContext 事件循环。
 */
class SSLStream : public ::rm::SSLStream {
public:
    //! 由异步 TCP 流式 Socket 创建异步 TLS 流
    SSLStream(StreamSocket socket, SSLContext &ctx);
    SSLStream(const SSLStream &) = delete;
    SSLStream(SSLStream &&) noexcept = default;

    SSLStream &operator=(const SSLStream &) = delete;
    SSLStream &operator=(SSLStream &&) noexcept = default;

    ~SSLStream() = default;

    //! 获取最近一次错误信息
    [[nodiscard]] std::string lasterr() const { return _lasterr.empty() ? ::rm::SSLStream::lasterr() : _lasterr; }

    /**
     * @brief 执行客户端 TLS 握手
     *
     * @param[in] server_name 服务器名称，用于 SNI，可为空
     * @return 握手是否成功
     */
    Task<bool> connect(std::string_view server_name = {});

    //! 执行服务端 TLS 握手
    Task<bool> accept();

    /**
     * @brief 根据上下文模式执行 TLS 握手
     *
     * @param[in] server_name 客户端模式下的 SNI，可为空
     * @return 握手是否成功
     */
    Task<bool> handshake(std::string_view server_name = {});

    //! 异步加密读取数据
    Task<std::string> read(size_t max_size = 65536);

    //! 异步加密读取数据到指定内存
    Task<size_t> read_to(char *buf, size_t size);

    //! 异步加密写入数据
    Task<bool> write(std::string_view data);

private:
    //! TLS I/O 等待器，只等待 fd 就绪，不消耗 Socket 数据
    class SSLIOAwaiter final : public AsyncIOAwaiter {
    public:
        SSLIOAwaiter(IOContext &ctx, SocketFd fd, bool wait_write) : AsyncIOAwaiter(ctx, FileDescriptor(fd)), _wait_write(wait_write) {}

        //! @cond
        void await_suspend(std::coroutine_handle<> handle);
        void await_resume() noexcept;
        //! @endcond

    private:
        bool _wait_write{};
    };

    Task<bool> do_handshake(std::string_view server_name, bool client_mode);

    IOContextRef _ctx; //!< 异步 I/O 执行上下文
    std::string _lasterr{};
};

//! @} io_net

} // namespace async

#endif // __cplusplus >= 202002L

} // namespace rm

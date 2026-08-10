/**
 * @file logger.hpp
 * @author zhaoxi (535394140@qq.com)
 * @brief 异步线程安全日志库
 * @details 提供基于 fmt 格式化的多级别日志记录功能。日志消息由调用线程写入内存队列，随后由内部工作线程异步输出，支持以下功能：
 * - 运行时日志级别过滤；
 * - stdout、轮转文件、串口和 UDP 输出后端；
 * - 按缓存阈值、时间间隔或用户请求持久化日志；
 * - 有界异步队列及阻塞、丢弃最新、丢弃最早三种溢出策略；
 * - 后端错误回调，以及丢弃日志数和后端错误数统计。
 * @version 1.0
 * @date 2026-08-08
 *
 * @copyright Copyright 2026 (c), zhaoxi
 *
 */

#pragma once

#include <chrono>

#include <fmt/format.h>

#include "rmvl/io/serial.hpp"

namespace rm {

//! @defgroup logger 日志模块
//! @{
//! @brief 提供异步、线程安全的多后端日志记录功能
//! @details
//! - rm::Logger 负责管理独立的日志级别、异步队列、工作线程和输出后端；LoggerOptions 只保存各后端共用的配置。
//! - rm::FileSink、rm::SerialSink 和 rm::UDPSink 分别封装对应输出后端的资源与写入行为。
//! - rm::std_logger 是进程内默认 stdout 日志器的懒加载代理，适合无需单独配置后端的通用日志场景。

//! 日志级别
enum class LogLevel : uint8_t {
    Debug,    //!< 调试信息
    Info,     //!< 普通信息
    Warn,     //!< 警告信息
    Error,    //!< 错误信息
    Critical, //!< 严重错误
    Off       //!< 关闭日志
};

/**
 * @brief 日志后端写入定制点
 *
 * @tparam SinkType 日志后端类型
 * @details 自定义日志后端需要通过 ADL 提供返回 bool 的 `sink_write_invoke(SinkType &, LogLevel,
 *          std::string_view)` 重载，即自定义的 `sink_write_invoke` 需要定义在和 SinkType 相同的命名空间中。
 */
struct sink_write_t {
    template <typename SinkType>
    bool operator()(SinkType &sink, LogLevel level, std::string_view record) const noexcept(noexcept(sink_write_invoke(sink, level, record))) {
        return sink_write_invoke(sink, level, record);
    }
};

/**
 * @brief 日志后端持久化定制点
 *
 * @tparam SinkType 日志后端类型
 * @details 自定义日志后端需要通过 ADL 提供返回 bool 的 `sink_flush_invoke(SinkType &)`
 *          重载，即自定义的 `sink_flush_invoke` 需要定义在和 SinkType 相同的命名空间中。
 */
struct sink_flush_t {
    template <typename SinkType>
    bool operator()(SinkType &sink) const noexcept(noexcept(sink_flush_invoke(sink))) {
        return sink_flush_invoke(sink);
    }
};

//! 【定制点对象】日志后端写入器
constexpr sink_write_t sink_write{};
//! 【定制点对象】日志后端持久化器
constexpr sink_flush_t sink_flush{};

//! stdout 日志后端
struct StdoutSink {
    friend bool sink_write_invoke(StdoutSink &, LogLevel, std::string_view) noexcept;
    friend bool sink_flush_invoke(StdoutSink &) noexcept;
};

//! 文件日志后端
class FileSink {
public:
    FileSink(std::string path = ".", std::string name = "rmvl", std::size_t max_file_size = 10U * 1024U * 1024U, std::size_t max_files = 128);
    FileSink(const FileSink &) = delete;
    FileSink(FileSink &&) noexcept;
    FileSink &operator=(const FileSink &) = delete;
    FileSink &operator=(FileSink &&) noexcept;
    ~FileSink();

private:
    class Impl;
    std::unique_ptr<Impl> _impl;

    friend bool sink_write_invoke(FileSink &, LogLevel, std::string_view) noexcept;
    friend bool sink_flush_invoke(FileSink &) noexcept;
};

//! 串口日志后端
class SerialSink {
public:
    SerialSink(std::string device, BaudRate baud_rate = BaudRate::BR_115200);
    SerialSink(const SerialSink &) = delete;
    SerialSink(SerialSink &&) noexcept;
    SerialSink &operator=(const SerialSink &) = delete;
    SerialSink &operator=(SerialSink &&) noexcept;
    ~SerialSink();

private:
    class Impl;
    std::unique_ptr<Impl> _impl;

    friend bool sink_write_invoke(SerialSink &, LogLevel, std::string_view) noexcept;
    friend bool sink_flush_invoke(SerialSink &) noexcept;
};

//! UDP 日志后端
class UDPSink {
public:
    UDPSink(std::string address, uint16_t port, std::size_t mtu = 1500);
    UDPSink(const UDPSink &) = delete;
    UDPSink(UDPSink &&) noexcept;
    UDPSink &operator=(const UDPSink &) = delete;
    UDPSink &operator=(UDPSink &&) noexcept;
    ~UDPSink();

private:
    class Impl;
    std::unique_ptr<Impl> _impl;

    friend bool sink_write_invoke(UDPSink &, LogLevel, std::string_view) noexcept;
    friend bool sink_flush_invoke(UDPSink &) noexcept;
};

//! 异步队列溢出策略
enum class LogOverflowPolicy : uint8_t {
    Block,      //!< 阻塞日志生产线程，直至队列有足够空间
    DropNewest, //!< 丢弃当前日志
    DropOldest  //!< 丢弃队列中最早的日志
};

//! 日志公共配置
struct LoggerOptions {
    LogLevel level{LogLevel::Debug};                                  //!< 最低输出级别
    std::chrono::milliseconds flush_interval{10000};                  //!< 定期持久化间隔
    std::size_t queue_memory_limit{1024U * 1024U};                    //!< 触发持久化的缓存上限，默认 1 MiB
    std::size_t max_queue_memory{8U * 1024U * 1024U};                 //!< 异步队列日志文本上限，默认 8 MiB，0 表示无限制
    LogOverflowPolicy overflow_policy{LogOverflowPolicy::DropOldest}; //!< 异步队列溢出策略
    std::function<void(std::string_view)> error_handler{};            //!< 后端错误处理器，在工作线程调用；为空时输出至 stderr
};

//! @cond

namespace details {

//! 日志后端类型擦除接口
class SinkBase {
public:
    virtual ~SinkBase() = default;
    virtual bool write(LogLevel level, std::string_view record) = 0;
    virtual bool flush() = 0;
};

//! 日志后端运行时适配器
template <typename SinkType>
class Sink final : public SinkBase {
public:
    explicit Sink(SinkType sink) : _sink(std::move(sink)) {}

    bool write(LogLevel level, std::string_view record) override { return sink_write(_sink, level, record); }
    bool flush() override { return sink_flush(_sink); }

private:
    SinkType _sink;
};

} // namespace details

//! @endcond

/**
 * @brief 异步线程安全日志器
 * @details
 * 日志先进入内存队列，再由内部工作线程写入指定后端。文件、串口和 UDP
 * 后端会在达到缓存阈值、定时器到期或调用 flush() 时持久化。
 */
class Logger {
public:
    //! 创建 stdout 日志器
    Logger();

    /**
     * @brief 使用指定公共配置创建 stdout 日志器
     *
     * @param[in] options 日志器公共配置
     */
    explicit Logger(LoggerOptions options);

    /**
     * @brief 使用指定日志后端和公共配置创建日志器
     *
     * @tparam SinkType 满足 sink_write 和 sink_flush 定制点的后端类型
     * @param[in] sink 日志后端
     * @param[in] options 日志器公共配置
     */
    template <typename SinkType>
    Logger(SinkType sink, LoggerOptions options = {})
        : Logger(std::unique_ptr<details::SinkBase>(std::make_unique<details::Sink<SinkType>>(std::move(sink))), std::move(options)) {}

    Logger(const Logger &) = delete;
    Logger(Logger &&) = delete;
    Logger &operator=(const Logger &) = delete;
    Logger &operator=(Logger &&) = delete;
    ~Logger();

    /**
     * @brief 获取进程内默认 stdout 日志器
     *
     * @return 默认 stdout 日志器
     */
    static Logger &basic();

    /**
     * @brief 记录指定级别的日志
     *
     * @param[in] level 日志级别
     * @param[in] msg 日志消息
     */
    void log(LogLevel level, std::string_view msg);

    /**
     * @brief 记录 DEBUG 级别日志
     *
     * @param[in] msg 日志消息
     */
    void debug(std::string_view msg) { log(LogLevel::Debug, msg); }

    /**
     * @brief 记录 DEBUG 级别日志
     *
     * @tparam Args 可变参数包
     * @param[in] format 格式化字符串
     * @param[in] args 可变参数包
     */
    template <typename... Args>
    void debug(fmt::format_string<Args...> format, Args &&...args) { log(LogLevel::Debug, fmt::format(format, std::forward<Args>(args)...)); }

    /**
     * @brief 记录 INFO 级别日志
     *
     * @param[in] msg 日志消息
     */
    void info(std::string_view msg) { log(LogLevel::Info, msg); }

    /**
     * @brief 记录 INFO 级别日志
     *
     * @tparam Args 可变参数包
     * @param[in] format 格式化字符串
     * @param[in] args 可变参数包
     */
    template <typename... Args>
    void info(fmt::format_string<Args...> format, Args &&...args) { log(LogLevel::Info, fmt::format(format, std::forward<Args>(args)...)); }

    /**
     * @brief 记录 WARN 级别日志
     *
     * @param[in] msg 日志消息
     */
    void warn(std::string_view msg) { log(LogLevel::Warn, msg); }

    /**
     * @brief 记录 WARN 级别日志
     *
     * @tparam Args 可变参数包
     * @param[in] format 格式化字符串
     * @param[in] args 可变参数包
     */
    template <typename... Args>
    void warn(fmt::format_string<Args...> format, Args &&...args) { log(LogLevel::Warn, fmt::format(format, std::forward<Args>(args)...)); }

    /**
     * @brief 记录 ERROR 级别日志
     *
     * @param[in] msg 日志消息
     */
    void error(std::string_view msg) { log(LogLevel::Error, msg); }

    /**
     * @brief 记录 ERROR 级别日志
     *
     * @tparam Args 可变参数包
     * @param[in] format 格式化字符串
     * @param[in] args 可变参数包
     */
    template <typename... Args>
    void error(fmt::format_string<Args...> format, Args &&...args) { log(LogLevel::Error, fmt::format(format, std::forward<Args>(args)...)); }

    /**
     * @brief 记录 CRITICAL 级别日志
     *
     * @param[in] msg 日志消息
     */
    void critical(std::string_view msg) { log(LogLevel::Critical, msg); }

    /**
     * @brief 记录 CRITICAL 级别日志
     *
     * @tparam Args 可变参数包
     * @param[in] format 格式化字符串
     * @param[in] args 可变参数包
     */
    template <typename... Args>
    void critical(fmt::format_string<Args...> format, Args &&...args) { log(LogLevel::Critical, fmt::format(format, std::forward<Args>(args)...)); }

    /**
     * @brief 设置最低输出级别
     *
     * @param[in] level 最低输出级别
     */
    void setLevel(LogLevel level) noexcept;

    /**
     * @brief 获取最低输出级别
     *
     * @return 最低输出级别
     */
    [[nodiscard]] LogLevel level() const noexcept;

    //! 阻塞等待队列中的日志全部持久化
    void flush();

    /**
     * @brief 获取当前尚未持久化的日志字节数
     *
     * @return 尚未持久化的日志字节数
     */
    [[nodiscard]] std::size_t pendingBytes() const noexcept;

    /**
     * @brief 获取因异步队列溢出而丢弃的日志数
     *
     * @return 丢弃的日志数
     */
    [[nodiscard]] std::size_t droppedLogs() const noexcept;

    /**
     * @brief 获取日志后端累计发生的错误数
     *
     * @return 后端错误数
     */
    [[nodiscard]] std::size_t backendErrors() const noexcept;

private:
    Logger(std::unique_ptr<details::SinkBase> sink, LoggerOptions options);

    class Impl;
    std::unique_ptr<Impl> _impl;
};

/**
 * @brief 默认日志器代理
 * @details 代理自身不持有资源，首次调用成员函数时才构造 Logger::basic()。
 */
class BasicLoggerProxy {
public:
    /**
     * @brief 记录 DEBUG 级别日志
     *
     * @param[in] msg 日志消息
     */
    void debug(std::string_view msg) const { Logger::basic().debug(msg); }

    /**
     * @brief 记录 DEBUG 级别日志
     *
     * @tparam Args 可变参数包
     * @param[in] format 格式化字符串
     * @param[in] args 可变参数包
     */
    template <typename... Args>
    void debug(fmt::format_string<Args...> format, Args &&...args) const { Logger::basic().debug(format, std::forward<Args>(args)...); }

    /**
     * @brief 记录 INFO 级别日志
     *
     * @param[in] msg 日志消息
     */
    void info(std::string_view msg) const { Logger::basic().info(msg); }

    /**
     * @brief 记录 INFO 级别日志
     *
     * @tparam Args 可变参数包
     * @param[in] format 格式化字符串
     * @param[in] args 可变参数包
     */
    template <typename... Args>
    void info(fmt::format_string<Args...> format, Args &&...args) const { Logger::basic().info(format, std::forward<Args>(args)...); }

    /**
     * @brief 记录 WARN 级别日志
     *
     * @param[in] msg 日志消息
     */
    void warn(std::string_view msg) const { Logger::basic().warn(msg); }

    /**
     * @brief 记录 WARN 级别日志
     *
     * @tparam Args 可变参数包
     * @param[in] format 格式化字符串
     * @param[in] args 可变参数包
     */
    template <typename... Args>
    void warn(fmt::format_string<Args...> format, Args &&...args) const { Logger::basic().warn(format, std::forward<Args>(args)...); }

    /**
     * @brief 记录 ERROR 级别日志
     *
     * @param[in] msg 日志消息
     */
    void error(std::string_view msg) const { Logger::basic().error(msg); }

    /**
     * @brief 记录 ERROR 级别日志
     *
     * @tparam Args 可变参数包
     * @param[in] format 格式化字符串
     * @param[in] args 可变参数包
     */
    template <typename... Args>
    void error(fmt::format_string<Args...> format, Args &&...args) const { Logger::basic().error(format, std::forward<Args>(args)...); }

    /**
     * @brief 记录 CRITICAL 级别日志
     *
     * @param[in] msg 日志消息
     */
    void critical(std::string_view msg) const { Logger::basic().critical(msg); }

    /**
     * @brief 记录 CRITICAL 级别日志
     *
     * @tparam Args 可变参数包
     * @param[in] format 格式化字符串
     * @param[in] args 可变参数包
     */
    template <typename... Args>
    void critical(fmt::format_string<Args...> format, Args &&...args) const { Logger::basic().critical(format, std::forward<Args>(args)...); }

    /**
     * @brief 记录指定级别的日志
     *
     * @param[in] level 日志级别
     * @param[in] msg 日志消息
     */
    void log(LogLevel level, std::string_view msg) const { Logger::basic().log(level, msg); }

    /**
     * @brief 设置最低输出级别
     *
     * @param[in] level 最低输出级别
     */
    void setLevel(LogLevel level) const noexcept { Logger::basic().setLevel(level); }

    /**
     * @brief 获取最低输出级别
     *
     * @return 最低输出级别
     */
    [[nodiscard]] LogLevel level() const noexcept { return Logger::basic().level(); }

    //! 阻塞等待队列中的日志全部持久化
    void flush() const { Logger::basic().flush(); }

    /**
     * @brief 获取当前尚未持久化的日志字节数
     *
     * @return 尚未持久化的日志字节数
     */
    [[nodiscard]] std::size_t pendingBytes() const noexcept { return Logger::basic().pendingBytes(); }

    /**
     * @brief 获取因异步队列溢出而丢弃的日志数
     *
     * @return 丢弃的日志数
     */
    [[nodiscard]] std::size_t droppedLogs() const noexcept { return Logger::basic().droppedLogs(); }

    /**
     * @brief 获取日志后端累计发生的错误数
     *
     * @return 后端错误数
     */
    [[nodiscard]] std::size_t backendErrors() const noexcept { return Logger::basic().backendErrors(); }
};

//! stdout 默认线程安全日志器，首次使用时懒加载
constexpr BasicLoggerProxy std_logger{};

//! @} logger

} // namespace rm

/**
 * @file logger_impl.hpp
 * @author zhaoxi (535394140@qq.com)
 * @brief 日志后端与异步日志器内部实现声明
 * @version 1.0
 * @date 2026-08-11
 *
 * @copyright Copyright 2026 (c), zhaoxi
 *
 */

#pragma once

#include <atomic>
#include <condition_variable>
#include <fstream>
#include <mutex>
#include <queue>
#include <thread>

#include <rmvl/io/socket.hpp>
#include <rmvl/logger.hpp>

namespace rm {

class FileSink::Impl {
public:
    Impl(std::string directory, std::string log_name, std::size_t file_limit, std::size_t file_count);

    bool write(std::string_view record) noexcept;
    bool flush() noexcept;

private:
    std::string currentPath() const;
    bool writeIndex() const noexcept;
    void openFile() noexcept;
    bool rotate() noexcept;

    std::string path;
    std::string name;
    std::size_t max_file_size;
    std::size_t max_files;
    std::string index_path;
    std::ofstream file;
    std::size_t file_size{};
    std::size_t index{};
    bool healthy{};
};

class SerialSink::Impl {
public:
    Impl(std::string device, BaudRate baud_rate);
    bool write(std::string_view record);

private:
    std::unique_ptr<SerialPort> serial;
};

class UDPSink::Impl {
public:
    Impl(std::string target_address, uint16_t target_port, std::size_t link_mtu);
    bool write(std::string_view record) noexcept;

private:
    std::string address;
    uint16_t port;
    std::size_t mtu;
    std::unique_ptr<DgramSocket> socket;
};

class Logger::Impl {
public:
    Impl(std::unique_ptr<details::SinkBase> output, LoggerOptions opts);
    ~Impl();

    void push(LogLevel log_level, std::string_view message);
    void flush();
    void setLevel(LogLevel level) noexcept;
    [[nodiscard]] LogLevel level() const noexcept;
    [[nodiscard]] std::size_t pendingBytes() const noexcept;
    [[nodiscard]] std::size_t droppedLogs() const noexcept;
    [[nodiscard]] std::size_t backendErrors() const noexcept;

private:
    struct Entry {
        LogLevel level;
        std::string text;
    };

    void normalizeOptions() noexcept;
    [[nodiscard]] std::size_t threshold() const noexcept;
    void reportError(std::string_view message) noexcept;
    bool write(const Entry &entry) noexcept;
    bool flushSink() noexcept;
    void run();

    std::unique_ptr<details::SinkBase> sink;
    LoggerOptions options;
    std::atomic<LogLevel> min_level;
    mutable std::mutex mutex;
    std::condition_variable work_cv;
    std::condition_variable flush_cv;
    std::condition_variable space_cv;
    std::queue<Entry> queue;
    std::size_t queued_bytes{};
    std::size_t pending_bytes{};
    bool stopping{};
    bool force_flush{};
    bool writing{};
    std::thread worker;
    std::atomic_size_t dropped_logs{};
    std::atomic_size_t backend_errors{};
};

} // namespace rm

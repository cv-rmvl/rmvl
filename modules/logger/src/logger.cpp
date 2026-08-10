/**
 * @file logger.cpp
 * @author zhaoxi (535394140@qq.com)
 * @brief 异步线程安全日志库公开接口实现
 * @version 1.0
 * @date 2026-08-08
 *
 * @copyright Copyright 2026 (c), zhaoxi
 *
 */

#include "logger_impl.hpp"

#include <cstdio>

namespace rm {

namespace {

const char *levelColor(LogLevel level) noexcept {
    switch (level) {
    case LogLevel::Debug:
        return "\033[90m";
    case LogLevel::Info:
        return "\033[32m";
    case LogLevel::Warn:
        return "\033[33m";
    case LogLevel::Error:
        return "\033[31m";
    case LogLevel::Critical:
        return "\033[1;31m";
    case LogLevel::Off:
        return "";
    }
    return "";
}

} // namespace

bool sink_write_invoke(StdoutSink &, LogLevel level, std::string_view record) noexcept {
    try {
        fmt::print(stdout, "{}{}\033[0m", levelColor(level), record);
        return std::fflush(stdout) == 0;
    } catch (...) {
        return false;
    }
}

bool sink_flush_invoke(StdoutSink &) noexcept { return std::fflush(stdout) == 0; }

FileSink::FileSink(std::string path, std::string name, std::size_t max_file_size, std::size_t max_files)
    : _impl(std::make_unique<Impl>(std::move(path), std::move(name), max_file_size, max_files)) {}

FileSink::FileSink(FileSink &&) noexcept = default;

FileSink &FileSink::operator=(FileSink &&) noexcept = default;

FileSink::~FileSink() = default;

bool sink_write_invoke(FileSink &sink, LogLevel, std::string_view record) noexcept { return sink._impl && sink._impl->write(record); }

bool sink_flush_invoke(FileSink &sink) noexcept { return sink._impl && sink._impl->flush(); }

SerialSink::SerialSink(std::string device, BaudRate baud_rate) : _impl(std::make_unique<Impl>(std::move(device), baud_rate)) {}

SerialSink::SerialSink(SerialSink &&) noexcept = default;

SerialSink &SerialSink::operator=(SerialSink &&) noexcept = default;

SerialSink::~SerialSink() = default;

bool sink_write_invoke(SerialSink &sink, LogLevel, std::string_view record) noexcept {
    try {
        return sink._impl && sink._impl->write(record);
    } catch (...) {
        return false;
    }
}

bool sink_flush_invoke(SerialSink &) noexcept { return true; }

UDPSink::UDPSink(std::string address, uint16_t port, std::size_t mtu) : _impl(std::make_unique<Impl>(std::move(address), port, mtu)) {}

UDPSink::UDPSink(UDPSink &&) noexcept = default;

UDPSink &UDPSink::operator=(UDPSink &&) noexcept = default;

UDPSink::~UDPSink() = default;

bool sink_write_invoke(UDPSink &sink, LogLevel, std::string_view record) noexcept { return sink._impl && sink._impl->write(record); }

bool sink_flush_invoke(UDPSink &) noexcept { return true; }

constexpr StdoutSink stdout_sink{};

Logger::Logger() : Logger(stdout_sink) {}

Logger::Logger(LoggerOptions options) : Logger(stdout_sink, std::move(options)) {}

Logger::Logger(std::unique_ptr<details::SinkBase> sink, LoggerOptions options)
    : _impl(std::make_unique<Impl>(std::move(sink), std::move(options))) {}
Logger::~Logger() = default;

Logger &Logger::basic() {
    static Logger default_logger;
    return default_logger;
}

void Logger::log(LogLevel level, std::string_view message) { _impl->push(level, message); }

void Logger::setLevel(LogLevel level) noexcept { _impl->setLevel(level); }

LogLevel Logger::level() const noexcept { return _impl->level(); }

void Logger::flush() { _impl->flush(); }

std::size_t Logger::pendingBytes() const noexcept { return _impl->pendingBytes(); }

std::size_t Logger::droppedLogs() const noexcept { return _impl->droppedLogs(); }

std::size_t Logger::backendErrors() const noexcept { return _impl->backendErrors(); }

} // namespace rm

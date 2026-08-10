/**
 * @file logger_impl.cpp
 * @author zhaoxi (535394140@qq.com)
 * @brief 日志后端与异步日志器内部实现
 * @version 1.0
 * @date 2026-08-11
 *
 * @copyright Copyright 2026 (c), zhaoxi
 *
 */

#include "logger_impl.hpp"

#include <algorithm>
#include <cstdio>

#include <fmt/chrono.h>

namespace rm {

namespace {

constexpr std::size_t IPV4_UDP_HEADER_SIZE = 28;

const char *levelName(LogLevel level) noexcept {
    switch (level) {
    case LogLevel::Debug:
        return "DEBUG";
    case LogLevel::Info:
        return "INFO";
    case LogLevel::Warn:
        return "WARN";
    case LogLevel::Error:
        return "ERROR";
    case LogLevel::Critical:
        return "CRITICAL";
    case LogLevel::Off:
        return "OFF";
    }
    return "UNKNOWN";
}

std::tm localTime(std::time_t value) noexcept {
    std::tm result{};
#ifdef _WIN32
    localtime_s(&result, &value);
#else
    localtime_r(&value, &result);
#endif
    return result;
}

std::string makeRecord(LogLevel level, std::string_view message) {
    const auto now = std::chrono::system_clock::now();
    const auto timestamp = std::chrono::system_clock::to_time_t(now);
    const auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count() % 1000;
    const auto time = localTime(timestamp);
    return fmt::format("[{:%Y-%m-%d %H:%M:%S}.{:03}] [{}] {}\n", time, millis, levelName(level), message);
}

std::string joinPath(std::string_view path, std::string_view filename) {
    if (path.empty() || path == ".")
        return std::string(filename);
    const auto last = path.back();
    return last == '/' || last == '\\' ? fmt::format("{}{}", path, filename) : fmt::format("{}/{}", path, filename);
}

} // namespace

FileSink::Impl::Impl(std::string directory, std::string log_name, std::size_t file_limit, std::size_t file_count)
    : path(directory.empty() ? "." : std::move(directory)), name(log_name.empty() ? "rmvl" : std::move(log_name)),
      max_file_size(std::max<std::size_t>(file_limit, 1)), max_files(std::max<std::size_t>(file_count, 1)),
      index_path(joinPath(path, name + "_index")) {
    if (std::FILE *index_file = std::fopen(index_path.c_str(), "rb")) {
        unsigned long long value{};
        if (std::fscanf(index_file, "%llu", &value) == 1)
            index = static_cast<std::size_t>(value);
        std::fclose(index_file);
    }
    healthy = writeIndex();
    openFile();
    healthy = static_cast<bool>(file) && healthy;
}

bool FileSink::Impl::write(std::string_view record) noexcept {
    if (!healthy)
        return false;
    if (!file.is_open())
        openFile();
    if (!file)
        return false;

    std::size_t offset{};
    while (file && offset < record.size()) {
        while (file && file_size >= max_file_size) {
            if (!rotate())
                return false;
        }
        if (!file)
            break;
        const auto count = std::min(max_file_size - file_size, record.size() - offset);
        file.write(record.data() + offset, static_cast<std::streamsize>(count));
        file_size += count;
        offset += count;
    }
    return static_cast<bool>(file) && offset == record.size();
}

bool FileSink::Impl::flush() noexcept {
    file.flush();
    return static_cast<bool>(file);
}

std::string FileSink::Impl::currentPath() const { return joinPath(path, fmt::format("{}_{}.log", name, index)); }

bool FileSink::Impl::writeIndex() const noexcept {
    std::FILE *index_file = std::fopen(index_path.c_str(), "wb");
    if (index_file == nullptr)
        return false;
    const bool written = std::fprintf(index_file, "%zu", index) > 0;
    const bool closed = std::fclose(index_file) == 0;
    return written && closed;
}

void FileSink::Impl::openFile() noexcept {
    file.clear();
    file.open(currentPath(), std::ios::binary | std::ios::app);
    if (!file) {
        file_size = 0;
        return;
    }
    file.seekp(0, std::ios::end);
    const auto position = file.tellp();
    file_size = position < 0 ? 0U : static_cast<std::size_t>(position);
}

bool FileSink::Impl::rotate() noexcept {
    file.close();
    ++index;
    const bool index_written = writeIndex();
    if (index >= max_files) {
        const auto expired = joinPath(path, fmt::format("{}_{}.log", name, index - max_files));
        std::remove(expired.c_str());
    }
    file_size = 0;
    openFile();
    healthy = index_written && static_cast<bool>(file);
    return healthy;
}

SerialSink::Impl::Impl(std::string device, BaudRate baud_rate) {
    if (!device.empty())
        serial = std::make_unique<SerialPort>(device, baud_rate);
}

bool SerialSink::Impl::write(std::string_view record) { return serial && serial->isOpened() && serial->write(record); }

UDPSink::Impl::Impl(std::string target_address, uint16_t target_port, std::size_t link_mtu)
    : address(std::move(target_address)), port(target_port), mtu(std::max(link_mtu, IPV4_UDP_HEADER_SIZE + 1)) {
    socket = std::make_unique<DgramSocket>(Sender(ip::udp::v4()).create());
    if (!socket->invalid())
        socket->setOption(ip::udp::Broadcast{});
}

bool UDPSink::Impl::write(std::string_view record) noexcept {
    if (!socket || socket->invalid())
        return false;
    const auto payload = mtu - IPV4_UDP_HEADER_SIZE;
    const Endpoint endpoint(ip::udp::v4(), port);
    for (std::size_t offset = 0; offset < record.size(); offset += payload)
        if (!socket->write(address, endpoint, record.substr(offset, std::min(payload, record.size() - offset))))
            return false;
    return true;
}

Logger::Impl::Impl(std::unique_ptr<details::SinkBase> output, LoggerOptions opts)
    : sink(std::move(output)), options(std::move(opts)), min_level(options.level) {
    normalizeOptions();
    worker = std::thread([this] { run(); });
}

Logger::Impl::~Impl() {
    {
        const std::lock_guard lock(mutex);
        stopping = true;
        force_flush = true;
    }
    work_cv.notify_one();
    space_cv.notify_all();
    if (worker.joinable())
        worker.join();
}

void Logger::Impl::push(LogLevel log_level, std::string_view message) {
    if (log_level < min_level.load(std::memory_order_relaxed) || log_level == LogLevel::Off)
        return;

    auto text = makeRecord(log_level, message);
    {
        std::unique_lock lock(mutex);
        if (options.max_queue_memory != 0 && text.size() > options.max_queue_memory) {
            ++dropped_logs;
            return;
        }
        const auto has_space = [this, size = text.size()] {
            return stopping || options.max_queue_memory == 0 || queued_bytes + pending_bytes + size <= options.max_queue_memory;
        };
        if (options.overflow_policy == LogOverflowPolicy::Block) {
            space_cv.wait(lock, has_space);
            if (stopping)
                return;
        } else if (!has_space()) {
            if (options.overflow_policy == LogOverflowPolicy::DropNewest) {
                ++dropped_logs;
                return;
            }
            while (!queue.empty() && !has_space()) {
                queued_bytes -= queue.front().text.size();
                queue.pop();
                ++dropped_logs;
            }
            if (!has_space()) {
                ++dropped_logs;
                return;
            }
        }
        queued_bytes += text.size();
        queue.push({log_level, std::move(text)});
    }
    work_cv.notify_one();
}

void Logger::Impl::flush() {
    std::unique_lock lock(mutex);
    force_flush = true;
    work_cv.notify_one();
    flush_cv.wait(lock, [this] { return queue.empty() && pending_bytes == 0 && !writing && !force_flush; });
}

void Logger::Impl::setLevel(LogLevel level) noexcept { min_level.store(level, std::memory_order_relaxed); }
LogLevel Logger::Impl::level() const noexcept { return min_level.load(std::memory_order_relaxed); }

std::size_t Logger::Impl::pendingBytes() const noexcept {
    const std::lock_guard lock(mutex);
    return queued_bytes + pending_bytes;
}

std::size_t Logger::Impl::droppedLogs() const noexcept { return dropped_logs.load(std::memory_order_relaxed); }
std::size_t Logger::Impl::backendErrors() const noexcept { return backend_errors.load(std::memory_order_relaxed); }

void Logger::Impl::normalizeOptions() noexcept {
    if (options.flush_interval.count() <= 0)
        options.flush_interval = std::chrono::milliseconds{1};
    if (options.queue_memory_limit == 0)
        options.queue_memory_limit = 1;
}

std::size_t Logger::Impl::threshold() const noexcept {
    return options.max_queue_memory == 0 ? options.queue_memory_limit : std::min(options.queue_memory_limit, options.max_queue_memory);
}

void Logger::Impl::reportError(std::string_view message) noexcept {
    ++backend_errors;
    try {
        if (options.error_handler)
            options.error_handler(message);
        else
            fmt::print(stderr, "[RMVL logger error] {}\n", message);
    } catch (...) {
        // 错误处理器不能影响日志工作线程
    }
}

bool Logger::Impl::write(const Entry &entry) noexcept {
    try {
        return sink && sink->write(entry.level, entry.text);
    } catch (...) {
        return false;
    }
}

bool Logger::Impl::flushSink() noexcept {
    try {
        return sink && sink->flush();
    } catch (...) {
        return false;
    }
}

void Logger::Impl::run() {
    auto deadline = std::chrono::steady_clock::now() + options.flush_interval;
    std::unique_lock lock(mutex);
    while (true) {
        work_cv.wait_until(lock, deadline, [this] { return stopping || force_flush || !queue.empty(); });

        while (!queue.empty()) {
            Entry entry = std::move(queue.front());
            queue.pop();
            queued_bytes -= entry.text.size();
            space_cv.notify_all();
            writing = true;
            lock.unlock();
            const bool success = write(entry);
            if (!success)
                reportError("failed to write log sink");
            lock.lock();
            writing = false;
            if (success)
                pending_bytes += entry.text.size();
        }

        const bool timed_out = std::chrono::steady_clock::now() >= deadline;
        const bool threshold_reached = pending_bytes >= threshold();
        if (pending_bytes != 0 && (force_flush || stopping || timed_out || threshold_reached)) {
            writing = true;
            lock.unlock();
            const bool success = flushSink();
            if (!success)
                reportError("failed to flush log sink");
            lock.lock();
            writing = false;
            pending_bytes = 0;
            space_cv.notify_all();
        }

        if (timed_out)
            deadline = std::chrono::steady_clock::now() + options.flush_interval;
        if (force_flush) {
            force_flush = false;
            flush_cv.notify_all();
        }
        if (queue.empty() && pending_bytes == 0 && !writing)
            flush_cv.notify_all();
        if (stopping && queue.empty() && pending_bytes == 0)
            break;
    }
}

} // namespace rm

/**
 * @file test_logger.cpp
 * @author zhaoxi (535394140@qq.com)
 * @brief Logger 单元测试
 * @version 1.0
 * @date 2026-08-09
 *
 * @copyright Copyright 2026 (c), zhaoxi
 *
 */

#include <algorithm>
#include <cstdio>
#include <string>
#include <thread>
#include <vector>

#include <gtest/gtest.h>

#include <rmvl/logger.hpp>

namespace rm_test {

struct TestSink {
    std::string *output;
    std::size_t *flushes;
};

bool sink_write_invoke(TestSink &sink, rm::LogLevel, std::string_view record) noexcept {
    sink.output->append(record);
    return true;
}

bool sink_flush_invoke(TestSink &sink) noexcept {
    ++*sink.flushes;
    return true;
}

class Logger_fixture : public testing::Test {
protected:
    void SetUp() override {
        name = "ts_logger_" + std::string(testing::UnitTest::GetInstance()->current_test_info()->name());
    }

    void TearDown() override {
        std::remove((name + "_index").c_str());
        for (std::size_t index = 0; index < 32; ++index)
            std::remove(logPath(index).c_str());
    }

    std::string logPath(std::size_t index) const { return name + "_" + std::to_string(index) + ".log"; }

    std::string readLog(std::size_t index = 0) const {
        std::FILE *file = std::fopen(logPath(index).c_str(), "rb");
        if (file == nullptr)
            return {};
        std::string result;
        char buffer[512]{};
        while (const auto count = std::fread(buffer, 1, sizeof(buffer), file))
            result.append(buffer, count);
        std::fclose(file);
        return result;
    }

    static bool fileExists(const std::string &file_path) {
        std::FILE *file = std::fopen(file_path.c_str(), "rb");
        if (file == nullptr)
            return false;
        std::fclose(file);
        return true;
    }

    static long fileSize(const std::string &file_path) {
        std::FILE *file = std::fopen(file_path.c_str(), "rb");
        if (file == nullptr)
            return -1;
        std::fseek(file, 0, SEEK_END);
        const auto size = std::ftell(file);
        std::fclose(file);
        return size;
    }

    std::size_t readIndex() const {
        std::FILE *file = std::fopen((name + "_index").c_str(), "rb");
        if (file == nullptr)
            return 0;
        std::size_t index{};
        if (std::fscanf(file, "%zu", &index) != 1)
            index = 0;
        std::fclose(file);
        return index;
    }

    rm::FileSink fileSink() const { return {".", name}; }

    rm::LoggerOptions loggerOptions() const {
        rm::LoggerOptions options;
        options.flush_interval = std::chrono::hours{1};
        return options;
    }

    std::string name;
};

TEST_F(Logger_fixture, format_and_filter) {
    auto options = loggerOptions();
    options.level = rm::LogLevel::Info;
    rm::Logger logger(fileSink(), options);

    logger.debug("hidden {}", 1);
    logger.info("answer = {}", 42);
    const std::string warning = "warning";
    logger.warn(warning);
    logger.flush();

    const auto content = readLog();
    EXPECT_EQ(content.find("hidden"), std::string::npos);
    EXPECT_NE(content.find("[INFO] answer = 42"), std::string::npos);
    EXPECT_NE(content.find("[WARN] warning"), std::string::npos);
    EXPECT_EQ(logger.pendingBytes(), 0U);
}

TEST_F(Logger_fixture, thread_safe_queue) {
    rm::Logger logger(fileSink(), loggerOptions());
    std::vector<std::thread> workers;
    for (int thread_id = 0; thread_id < 4; ++thread_id) {
        workers.emplace_back([&, thread_id] {
            for (int index = 0; index < 100; ++index)
                logger.info("thread={}, index={}", thread_id, index);
        });
    }
    for (auto &worker : workers)
        worker.join();
    logger.flush();

    const auto content = readLog();
    EXPECT_EQ(std::count(content.begin(), content.end(), '\n'), 400);
}

TEST_F(Logger_fixture, rotate_files) {
    constexpr std::size_t max_file_size = 128;
    constexpr std::size_t max_files = 3;
    rm::Logger logger(rm::FileSink{".", name, max_file_size, max_files}, loggerOptions());
    for (int index = 0; index < 20; ++index)
        logger.info("rotation record {:02}: abcdefghijklmnopqrstuvwxyz", index);
    logger.flush();

    const auto current = readIndex();
    ASSERT_GT(current, 1U);
    if (current >= max_files) {
        EXPECT_FALSE(fileExists(logPath(current - max_files)));
    }
    for (std::size_t index = current - 2; index <= current; ++index) {
        EXPECT_TRUE(fileExists(logPath(index)));
        EXPECT_LE(fileSize(logPath(index)), static_cast<long>(max_file_size));
    }
}

TEST_F(Logger_fixture, drop_oversized_record) {
    auto options = loggerOptions();
    options.max_queue_memory = 32;
    options.overflow_policy = rm::LogOverflowPolicy::DropNewest;
    rm::Logger logger(fileSink(), options);
    logger.info("{}", std::string(128, 'x'));
    logger.flush();

    EXPECT_EQ(logger.droppedLogs(), 1U);
    EXPECT_TRUE(readLog().empty());
}

TEST_F(Logger_fixture, report_backend_failure) {
    rm::LoggerOptions options;
    std::size_t reported_errors{};
    options.error_handler = [&reported_errors](std::string_view) { ++reported_errors; };
    rm::Logger logger(rm::FileSink{name, "rmvl"}, options);
    logger.error("cannot be persisted");
    logger.flush();

    EXPECT_EQ(logger.backendErrors(), 1U);
    EXPECT_EQ(reported_errors, 1U);
}

TEST_F(Logger_fixture, simple_file_constructor) {
    rm::Logger logger(rm::FileSink{".", name});
    logger.info("simple file logger");
    logger.flush();

    EXPECT_TRUE(fileExists(name + "_index"));
    EXPECT_EQ(readIndex(), 0U);
    EXPECT_NE(readLog().find("simple file logger"), std::string::npos);
}

TEST_F(Logger_fixture, resume_indexed_file) {
    {
        rm::Logger logger(rm::FileSink{".", name});
        logger.info("before restart");
        logger.flush();
    }
    {
        rm::Logger logger(rm::FileSink{".", name});
        logger.info("after restart");
        logger.flush();
    }

    EXPECT_EQ(readIndex(), 0U);
    const auto content = readLog();
    EXPECT_NE(content.find("before restart"), std::string::npos);
    EXPECT_NE(content.find("after restart"), std::string::npos);
}

TEST(Logger_cpo, custom_sink) {
    std::string output;
    std::size_t flushes{};
    rm::Logger logger(TestSink{&output, &flushes});
    logger.info("custom sink {}", 42);
    logger.flush();

    EXPECT_NE(output.find("custom sink 42"), std::string::npos);
    EXPECT_EQ(flushes, 1U);
}

TEST(Logger_proxy, lazy_global_logger) {
    rm::std_logger.setLevel(rm::LogLevel::Off);
    rm::std_logger.info("filtered global message {}", 1);
    rm::std_logger.flush();
    EXPECT_EQ(rm::std_logger.pendingBytes(), 0U);
    rm::std_logger.setLevel(rm::LogLevel::Debug);
}

} // namespace rm_test

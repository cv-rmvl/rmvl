/**
 * @file test_toml.cpp
 * @brief TOML 加载、类型转换、错误和句柄生命周期测试
 */
#include <cstdio>
#include <type_traits>

#include <gtest/gtest.h>
#include "rmvl/core/toml.hpp"

namespace rm_test {
namespace {
using namespace rm::toml;
static_assert(!std::is_convertible_v<Node, bool>);
static_assert(!std::is_convertible_v<Node, int>);

struct Config { int port{}; std::string host; };
bool toml_decode(const Node &node, Config &value) {
    return node["port"].read(value.port) && node["host"].read(value.host);
}

TEST(TOML, loads_and_lookup) {
    auto root = loads(R"(
title = "RMVL"
enabled = false
"a.b" = 42
[server]
port = 8080
host = "localhost"
ports = [8000, 8001]
)");
    ASSERT_TRUE(root);
    EXPECT_TRUE(root.valid());
    EXPECT_TRUE(root.isTable());
    EXPECT_EQ(root.type(), NodeType::Table);
    EXPECT_EQ(root.size(), 4u);
    EXPECT_EQ(root["title"].as<std::string>(), "RMVL");
    EXPECT_TRUE(root["enabled"]); // 存在的 false 值仍然是有效节点
    EXPECT_EQ(root["enabled"].as<bool>(), false);
    EXPECT_EQ(root["a.b"].as<int>(), 42);
    EXPECT_FALSE(root["a"]["b"]);
    EXPECT_TRUE(root.contains("server"));
    EXPECT_FALSE(root.contains("missing"));
    EXPECT_FALSE(root["missing"]["nested"]);
    EXPECT_FALSE(root[0]);
    EXPECT_FALSE(root["title"]["x"]);
    EXPECT_FALSE(root["server"]["ports"][99]);
    EXPECT_TRUE(root["server"]["ports"].isArray());
    EXPECT_EQ(root["server"]["ports"][1].as<int>(), 8001);
    EXPECT_EQ(root["missing"].valueOr(17), 17);
    EXPECT_EQ(root["title"].size(), 0u);
    EXPECT_TRUE(root["title"].keys().empty());
    EXPECT_EQ(root.keys().size(), root.size());
    Node invalid;
    EXPECT_FALSE(invalid);
    EXPECT_EQ(invalid.type(), NodeType::Undefined);
    EXPECT_EQ(invalid.size(), 0u);
    EXPECT_TRUE(loads("").isTable());
    EXPECT_EQ(loads("").size(), 0u);
}

TEST(TOML, strict_numeric_conversion) {
    auto root = loads("large = 256\nnegative = -1\nreal = 1.5\ntext = '42'\nmax = 9223372036854775807\nnan = nan\ninf = +inf\nhuge = 1e300");
    uint8_t out = 12;
    EXPECT_FALSE(root["large"].read(out));
    EXPECT_EQ(out, 12);
    EXPECT_EQ(root["large"].as<uint16_t>(), 256);
    EXPECT_FALSE(root["negative"].as<unsigned>());
    EXPECT_FALSE(root["large"].as<double>());
    EXPECT_FALSE(root["real"].as<int>());
    EXPECT_FALSE(root["text"].as<int>());
    EXPECT_FALSE(root["negative"].as<bool>());
    EXPECT_EQ(root["max"].as<uint64_t>(), uint64_t{9223372036854775807});
    EXPECT_FLOAT_EQ(root["real"].valueOr(0.0f), 1.5f);
    EXPECT_TRUE(std::isnan(root["nan"].valueOr(0.0)));
    EXPECT_TRUE(std::isinf(root["inf"].valueOr(0.0f)));
    EXPECT_FALSE(root["huge"].as<float>());
}

TEST(TOML, containers_and_custom_types) {
    auto root = loads(R"(
numbers = [1, 2, 3]
flags = [true, false]
mixed = [1, "two"]
port = 8080
host = "localhost"
[values]
a = 1
b = 2
)");
    EXPECT_EQ(root["numbers"].as<std::vector<int>>(), (std::vector<int>{1, 2, 3}));
    EXPECT_EQ((root["numbers"].as<std::array<int, 3>>()), (std::array<int, 3>{1, 2, 3}));
    EXPECT_FALSE((root["numbers"].as<std::array<int, 2>>()));
    EXPECT_EQ(root["flags"].as<std::vector<bool>>(), (std::vector<bool>{true, false}));
    EXPECT_EQ((root["values"].as<std::map<std::string, int>>()), (std::map<std::string, int>{{"a", 1}, {"b", 2}}));
    EXPECT_EQ((root["values"].as<std::unordered_map<std::string, int>>()), (std::unordered_map<std::string, int>{{"a", 1}, {"b", 2}}));
    std::vector<int> output{7};
    EXPECT_FALSE(root["mixed"].read(output));
    EXPECT_EQ(output, (std::vector<int>{7}));
    auto optional = root["missing"].as<std::optional<int>>();
    ASSERT_TRUE(optional);
    EXPECT_FALSE(*optional);
    auto config = root.as<Config>();
    ASSERT_TRUE(config);
    EXPECT_EQ(config->port, 8080);
    EXPECT_EQ(config->host, "localhost");
    Config unchanged{123, "original"};
    EXPECT_FALSE(loads("port = 12\nhost = 42").read(unchanged));
    EXPECT_EQ(unchanged.port, 123);
    EXPECT_EQ(unchanged.host, "original");
}

TEST(TOML, syntax_and_tables) {
    auto root = loads(R"(
hex = 0xff
binary = 0b1010
underscores = 1_000
escaped = "line\n\u4E2D"
literal = 'C:\robot'
multiline = """
hello
world"""
inline = { name = "camera", gain = 1.5 }
[[devices]]
id = 1
[[devices]]
id = 2
[devices.settings]
active = true
)");
    EXPECT_EQ(root["hex"].as<int>(), 255);
    EXPECT_EQ(root["binary"].as<int>(), 10);
    EXPECT_EQ(root["underscores"].as<int>(), 1000);
    EXPECT_EQ(root["escaped"].as<std::string>(), "line\n中");
    EXPECT_EQ(root["literal"].as<std::string>(), "C:\\robot");
    EXPECT_EQ(root["multiline"].as<std::string>(), "hello\nworld");
    EXPECT_EQ(root["inline"]["name"].as<std::string>(), "camera");
    ASSERT_EQ(root["devices"].size(), 2u);
    EXPECT_EQ(root["devices"][1]["settings"]["active"].as<bool>(), true);
}

TEST(TOML, date_and_time) {
    auto root = loads(R"(
date = 2024-02-29
time = 12:34:56.123456789
local = 2024-02-29T12:34:56
utc = 2024-02-29T12:34:56Z
offset = 2024-02-29T12:34:56+08:00
)");
    auto date = root["date"].as<Date>();
    ASSERT_TRUE(date);
    EXPECT_EQ(date->year, 2024);
    EXPECT_EQ(date->day, 29u);
    auto time = root["time"].as<Time>();
    ASSERT_TRUE(time);
    EXPECT_EQ(time->nanosecond, 123456789u);
    auto local = root["local"].as<DateTime>();
    ASSERT_TRUE(local);
    EXPECT_FALSE(local->offsetMinutes);
    auto utc = root["utc"].as<DateTime>();
    ASSERT_TRUE(utc);
    EXPECT_EQ(utc->offsetMinutes, 0);
    auto offset = root["offset"].as<DateTime>();
    ASSERT_TRUE(offset);
    EXPECT_EQ(offset->offsetMinutes, 480);
    EXPECT_FALSE(root["date"].as<std::string>());
}

TEST(TOML, errors) {
    for (auto source : {"x =", "x = 1\nx = 2", "[a]\n[a]", "a = 1\na.b = 2", "x = null",
                        "x = 9223372036854775808", "x = 2023-02-29", "x = { a = 1, a = 2 }"})
        EXPECT_THROW(loads(source), ParseError) << source;
    try {
        loads("valid = 1\nbroken =\n");
        FAIL() << "expected parse error";
    } catch (const ParseError &error) {
        EXPECT_EQ(error.line, 2u);
        EXPECT_GT(error.column, 0u);
        EXPECT_NE(std::string(error.what()), "");
    }
    EXPECT_THROW(load(""), IoError);
    EXPECT_THROW(load(std::string("bad\0path", 8)), IoError);
    const std::string invalid_utf8 = "key = \"\xff\"";
    EXPECT_THROW(loads(invalid_utf8), ParseError);
}

TEST(TOML, shared_lifetime_and_move) {
    Node child = loads("[server]\nport = 8080")["server"];
    auto copy = child;
    auto moved = std::move(child);
    EXPECT_FALSE(child);
    EXPECT_EQ(child.type(), NodeType::Undefined);
    EXPECT_EQ(moved["port"].as<int>(), 8080);
    copy = {};
    EXPECT_EQ(moved["port"].as<int>(), 8080);
    const auto keys = moved.keys();
    EXPECT_EQ(keys.front(), "port");
}

TEST(TOML, file_load) {
    const auto path = ::testing::TempDir() + "rmvl_core_toml_test.toml";
    struct Cleanup { std::string path; ~Cleanup() { std::remove(path.c_str()); } } cleanup{path};
    auto *file = std::fopen(path.c_str(), "wb");
    ASSERT_NE(file, nullptr);
    const std::string source = "[server]\nport = 8080\n#" + std::string(20000, 'x');
    const auto written = std::fwrite(source.data(), 1, source.size(), file);
    const auto closed = std::fclose(file);
    ASSERT_EQ(written, source.size());
    ASSERT_EQ(closed, 0);
    EXPECT_EQ(load(path)["server"]["port"].as<int>(), 8080);
    file = std::fopen(path.c_str(), "wb");
    ASSERT_NE(file, nullptr);
    std::fputs("invalid =", file);
    std::fclose(file);
    EXPECT_THROW(load(path), ParseError);
}
} // namespace
} // namespace rm_test

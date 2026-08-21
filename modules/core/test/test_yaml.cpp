/**
 * @file test_yaml.cpp
 * @brief YAML 模块单元测试
 */

#include <cstdio>
#include <optional>
#include <string>
#include <vector>

#include <gtest/gtest.h>

#include "rmvl/core/yaml.hpp"

namespace rm_test {

using namespace rm::yaml;

namespace {

struct Point {
    double x{};
    double y{};
};

bool yaml_decode(const Node &node, Point &point) {
    return node["x"].read(point.x) && node["y"].read(point.y);
}

bool yaml_encode(Node &node, const Point &point) {
    node.makeMap();
    return node.set("x", point.x) && node.set("y", point.y);
}

} // namespace

TEST(YAML, parse_and_access) {
    const auto result = parse(R"(
camera:
  name: front
  width: 1280
  enabled: true
  exposure: 3.25
labels: [robot, armor]
empty: null
quoted_null: "null"
)");

    ASSERT_TRUE(result) << result.error.message;
    EXPECT_TRUE(result->isMap());
    EXPECT_TRUE(result->contains("camera"));
    EXPECT_EQ(result->size(), 4u);
    EXPECT_EQ((*result)["camera"]["name"].as<std::string>(), "front");
    EXPECT_EQ((*result)["camera"]["width"].as<int>(), 1280);
    EXPECT_EQ((*result)["camera"]["enabled"].as<bool>(), true);
    EXPECT_DOUBLE_EQ((*result)["camera"]["exposure"].valueOr(0.0), 3.25);
    ASSERT_TRUE((*result)["labels"].isSequence());
    EXPECT_EQ((*result)["labels"][1].scalar(), "armor");
    EXPECT_TRUE((*result)["empty"].isNull());
    EXPECT_TRUE((*result)["quoted_null"].isScalar());
    EXPECT_FALSE((*result)["missing"].valid());
}

TEST(YAML, standard_type_conversion) {
    auto root = Node::createMap();
    ASSERT_TRUE(root.set("numbers", std::vector<int>{1, 2, 3}));
    ASSERT_TRUE(root.set("array", std::array<double, 2>{1.5, 2.5}));
    ASSERT_TRUE(root.set("mapping", std::map<std::string, int>{{"a", 4}, {"b", 5}}));
    ASSERT_TRUE(root.set("optional", std::optional<int>{}));

    EXPECT_EQ(root["numbers"].as<std::vector<int>>(), (std::vector<int>{1, 2, 3}));
    EXPECT_EQ((root["array"].as<std::array<double, 2>>()), (std::array<double, 2>{1.5, 2.5}));
    EXPECT_EQ((root["mapping"].as<std::map<std::string, int>>()), (std::map<std::string, int>{{"a", 4}, {"b", 5}}));
    const auto optional = root["optional"].as<std::optional<int>>();
    ASSERT_TRUE(optional.has_value());
    EXPECT_FALSE(optional->has_value());
}

TEST(YAML, checked_numeric_conversion) {
    auto value = Node::createScalar("256");
    EXPECT_FALSE(value.as<uint8_t>().has_value());
    EXPECT_EQ(value.as<uint16_t>(), 256);
    value.setScalar("-1");
    EXPECT_FALSE(value.as<unsigned>().has_value());
    value.setScalar("12px");
    EXPECT_FALSE(value.as<int>().has_value());
    value.setScalar("1e1000");
    EXPECT_FALSE(value.as<double>().has_value());
}

TEST(YAML, mutate_dump_and_round_trip) {
    auto root = Node::createMap();
    ASSERT_TRUE(root.set("name", "demo"));
    ASSERT_TRUE(root.set("point", Point{1.25, -2.5}));
    auto values = root.ensure("values");
    ASSERT_TRUE(values.push(7));
    ASSERT_TRUE(values.push(9));
    EXPECT_TRUE(root.erase("name"));
    EXPECT_FALSE(root.erase("missing"));
    EXPECT_TRUE(values.erase(0));

    const auto source = dump(root);
    ASSERT_FALSE(source.empty());
    const auto parsed = parse(source);
    ASSERT_TRUE(parsed) << parsed.error.message << "\n" << source;
    EXPECT_EQ((*parsed)["values"].as<std::vector<int>>(), (std::vector<int>{9}));
    const auto point = (*parsed)["point"].as<Point>();
    ASSERT_TRUE(point.has_value());
    EXPECT_DOUBLE_EQ(point->x, 1.25);
    EXPECT_DOUBLE_EQ(point->y, -2.5);
}

TEST(YAML, child_keeps_tree_alive) {
    Node child;
    {
        const auto result = parse("root:\n  answer: 42\n");
        ASSERT_TRUE(result);
        child = (*result)["root"]["answer"];
    }
    EXPECT_EQ(child.as<int>(), 42);
}

TEST(YAML, parse_error) {
    const auto result = parse("root: [1, 2\n");
    EXPECT_FALSE(result);
    EXPECT_EQ(result.error.code, ErrorCode::Parse);
    EXPECT_FALSE(result.error.message.empty());
    EXPECT_GT(result.error.line, 0u);
}

TEST(YAML, file_round_trip) {
    constexpr auto path = "rmvl_yaml_round_trip.yml";
    auto root = Node::createMap();
    ASSERT_TRUE(root.set("name", "file"));
    ASSERT_TRUE(root.set("value", 42));

    Error error{ErrorCode::Io, "stale error", 1, 1};
    ASSERT_TRUE(save(path, root, error)) << error.message;
    EXPECT_FALSE(error);
    const auto loaded = load(path);
    std::remove(path);
    ASSERT_TRUE(loaded) << loaded.error.message;
    EXPECT_EQ((*loaded)["name"].as<std::string>(), "file");
    EXPECT_EQ((*loaded)["value"].as<int>(), 42);

    EXPECT_FALSE(save(path, Node{}));

    const auto missing = load("rmvl_yaml_file_that_does_not_exist.yml");
    EXPECT_FALSE(missing);
    EXPECT_EQ(missing.error.code, ErrorCode::Io);
}

} // namespace rm_test

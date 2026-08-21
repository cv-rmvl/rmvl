/**
 * @file perf_yaml.cpp
 * @brief YAML 解析、访问、构建与序列化性能测试
 */

#include <string>

#include <benchmark/benchmark.h>

#include "rmvl/core/yaml.hpp"

namespace rm_test {

using namespace rm::yaml;

namespace {

std::string makeYaml(std::size_t count) {
    std::string source{"items:\n"};
    source.reserve(count * 48);
    for (std::size_t i = 0; i < count; ++i) {
        source.append("  item_").append(std::to_string(i)).append(":\n");
        source.append("    id: ").append(std::to_string(i)).append("\n");
        source.append("    enabled: true\n");
    }
    return source;
}

} // namespace

static void BM_YamlParse(benchmark::State &state) {
    const auto source = makeYaml(static_cast<std::size_t>(state.range(0)));
    for (auto _ : state) {
        auto result = parse(source);
        if (!result) {
            state.SkipWithError(result.error.message.c_str());
            break;
        }
        benchmark::DoNotOptimize(result.root);
    }
    state.SetBytesProcessed(state.iterations() * static_cast<int64_t>(source.size()));
}

BENCHMARK(BM_YamlParse)->Arg(32)->Arg(256)->Arg(1024);

static void BM_YamlLookup(benchmark::State &state) {
    const auto count = static_cast<std::size_t>(state.range(0));
    const auto result = parse(makeYaml(count));
    if (!result) {
        state.SkipWithError(result.error.message.c_str());
        return;
    }
    const auto items = (*result)["items"];
    const auto key = std::string("item_") + std::to_string(count - 1);
    for (auto _ : state) {
        const auto value = items[key]["id"].as<std::size_t>();
        benchmark::DoNotOptimize(value.has_value());
    }
}

BENCHMARK(BM_YamlLookup)->Arg(32)->Arg(256)->Arg(1024);

static void BM_YamlBuild(benchmark::State &state) {
    const auto count = static_cast<std::size_t>(state.range(0));
    for (auto _ : state) {
        auto root = Node::createMap();
        auto items = root.ensure("items");
        items.makeSequence();
        for (std::size_t i = 0; i < count; ++i) {
            auto item = items.append();
            item.makeMap();
            item.set("id", i);
            item.set("enabled", true);
        }
        benchmark::DoNotOptimize(root);
    }
    state.SetItemsProcessed(state.iterations() * static_cast<int64_t>(count));
}

BENCHMARK(BM_YamlBuild)->Arg(32)->Arg(256)->Arg(1024);

static void BM_YamlDump(benchmark::State &state) {
    const auto result = parse(makeYaml(static_cast<std::size_t>(state.range(0))));
    if (!result) {
        state.SkipWithError(result.error.message.c_str());
        return;
    }
    for (auto _ : state) {
        const auto source = dump(*result);
        benchmark::DoNotOptimize(source.data());
        benchmark::DoNotOptimize(source.size());
    }
}

BENCHMARK(BM_YamlDump)->Arg(32)->Arg(256)->Arg(1024);

} // namespace rm_test

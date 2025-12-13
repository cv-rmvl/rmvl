#include <benchmark/benchmark.h>
#include <string>

#include "rmvl/io/netapp.hpp"

namespace rm_test {

static rm::HTTPMethod get_method_from_hash(std::string_view str) {
    static std::unordered_map<std::string_view, rm::HTTPMethod> method_map = {
        {"GET", rm::HTTPMethod::Get}, {"POST", rm::HTTPMethod::Post}, {"PUT", rm::HTTPMethod::Put}, {"DELETE", rm::HTTPMethod::Delete}, {"PATCH", rm::HTTPMethod::Patch}, {"HEAD", rm::HTTPMethod::Head}, {"OPTIONS", rm::HTTPMethod::Options}, {"TRACE", rm::HTTPMethod::Trace}, {"CONNECT", rm::HTTPMethod::Connect}};
    auto it = method_map.find(str);
    return it != method_map.end() ? it->second : rm::HTTPMethod::Unknown;
}

static constexpr rm::HTTPMethod get_method_from_if(std::string_view str) {
    if (str == "GET")
        return rm::HTTPMethod::Get;
    else if (str == "POST")
        return rm::HTTPMethod::Post;
    else if (str == "PUT")
        return rm::HTTPMethod::Put;
    else if (str == "DELETE")
        return rm::HTTPMethod::Delete;
    else if (str == "PATCH")
        return rm::HTTPMethod::Patch;
    else if (str == "HEAD")
        return rm::HTTPMethod::Head;
    else if (str == "OPTIONS")
        return rm::HTTPMethod::Options;
    else if (str == "TRACE")
        return rm::HTTPMethod::Trace;
    else if (str == "CONNECT")
        return rm::HTTPMethod::Connect;
    else
        return rm::HTTPMethod::Unknown;
}

const char *datas[] = {"GET", "POST", "PUT", "DELETE", "PATCH", "HEAD", "OPTIONS", "TRACE", "CONNECT", "Test"};

void use_hash(benchmark::State &state) {
    for (auto _ : state)
        for (std::size_t i = 0; i < std::size(datas); ++i)
            benchmark::DoNotOptimize(get_method_from_hash(datas[rand() % std::size(datas)]));
}

void use_if(benchmark::State &state) {
    for (auto _ : state)
        for (std::size_t i = 0; i < std::size(datas); ++i)
            benchmark::DoNotOptimize(get_method_from_if(datas[rand() % std::size(datas)]));
}

BENCHMARK(use_hash)->Name("get method by using hash")->Iterations(100000);
BENCHMARK(use_if)->Name("get method by using if")->Iterations(100000);

} // namespace rm_test
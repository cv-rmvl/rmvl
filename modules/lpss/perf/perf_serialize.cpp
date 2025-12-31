#include <benchmark/benchmark.h>
#include <nlohmann/json.hpp>

namespace rm::msg {

class TestData {
public:
    std::string serialize() {
        std::string _res__;
        _res__.resize(sizeof(a) + sizeof(b) + sizeof(uint8_t) + c.size());
        _res__.append(reinterpret_cast<const char *>(&a), sizeof(a));
        _res__.append(reinterpret_cast<const char *>(&b), sizeof(b));
        uint8_t c_size__ = static_cast<uint8_t>(c.size());
        _res__.append(reinterpret_cast<const char *>(&c_size__), sizeof(uint8_t));
        _res__.append(c.data(), c.size());

        return _res__;
    }

    static TestData deserialize(std::string_view _str__) {
        TestData _msg__{};
        const char *_p__ = _str__.data();
        _msg__.a = *reinterpret_cast<const int16_t *>(_p__);
        _p__ += sizeof(int16_t);
        _msg__.b = *reinterpret_cast<const uint32_t *>(_p__);
        _p__ += sizeof(uint32_t);
        auto c_size__ = *reinterpret_cast<const uint8_t *>(_p__);
        _p__ += sizeof(uint8_t);
        _msg__.c.assign(_p__, c_size__);
        _p__ += c_size__;

        return _msg__;
    }

    int16_t a;
    uint32_t b;
    std::string c;
};

} // namespace rm::msg

using namespace rm;

namespace rm_test {

void data_serialize(benchmark::State &state) {
    msg::TestData msg;
    msg.a = 42;
    msg.b = 2024;
    msg.c = "Hello, LPSS!";
    for (auto _ : state) {
        benchmark::DoNotOptimize(msg.serialize());
    }
}

void data_deserialize(benchmark::State &state) {
    msg::TestData msg;
    msg.a = 42;
    msg.b = 2024;
    msg.c = "Hello, LPSS!";
    auto serialized = msg.serialize();
    for (auto _ : state) {
        auto data = msg::TestData::deserialize(serialized);
        benchmark::DoNotOptimize(data);
    }
}

namespace msg_json {

class TestData {
public:
    int16_t a;
    uint32_t b;
    std::string c;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(TestData, a, b, c)
};

} // namespace msg_json

void json_data_serialize(benchmark::State &state) {
    msg_json::TestData msg;
    msg.a = 42;
    msg.b = 2024;
    msg.c = "Hello, LPSS!";
    for (auto _ : state) {
        benchmark::DoNotOptimize(json(msg).dump());
    }
}

void json_data_deserialize(benchmark::State &state) {
    msg_json::TestData msg;
    msg.a = 42;
    msg.b = 2024;
    msg.c = "Hello, LPSS!";
    auto serialized = json(msg).dump();
    for (auto _ : state) {
        benchmark::DoNotOptimize(json::parse(serialized).get<msg_json::TestData>());
    }
}

BENCHMARK(data_serialize)->Name("LPSS Data Serialize")->Iterations(100000);
BENCHMARK(data_deserialize)->Name("LPSS Data Deserialize")->Iterations(100000);
BENCHMARK(json_data_serialize)->Name("JSON Data Serialize")->Iterations(100000);
BENCHMARK(json_data_deserialize)->Name("JSON Data Deserialize")->Iterations(100000);

} // namespace rm_test
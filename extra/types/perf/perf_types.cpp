/**
 * @file perf_types.cpp
 * @author zhaoxi (535394140@qq.com)
 * @brief 状态类型系统性能测试
 * @version 1.0
 * @date 2025-01-04
 * 
 * @copyright Copyright 2025 (c), zhaoxi
 * 
 */

#include <benchmark/benchmark.h>

#include "rmvl/types.hpp"

namespace rm::rm_test
{

enum class Test1Type : uint8_t
{
    TEST1_1,
    TEST1_2,
    TEST1_3,
};

enum class Test2Type : uint8_t
{
    TEST2_1,
    TEST2_2,
    TEST2_3,
};

enum class Test3Type : uint8_t
{
    TEST3_1,
    TEST3_2,
    TEST3_3,
};

struct OldRMStatus
{
    Test1Type Test1TypeID{};
    Test2Type Test2TypeID{};
    Test3Type Test3TypeID{};
};

static void types_enum(benchmark::State &state)
{
    for (auto _ : state)
    {
        OldRMStatus state;
        state.Test1TypeID = Test1Type::TEST1_1;
        benchmark::DoNotOptimize(state);
        state.Test2TypeID = Test2Type::TEST2_1;
        benchmark::DoNotOptimize(state);
        state.Test3TypeID = Test3Type::TEST3_1;
        benchmark::DoNotOptimize(state);
        state.Test1TypeID = Test1Type::TEST1_2;
        benchmark::DoNotOptimize(state);
        state.Test2TypeID = Test2Type::TEST2_2;
        benchmark::DoNotOptimize(state);
        state.Test3TypeID = Test3Type::TEST3_2;
        benchmark::DoNotOptimize(state);
    }
}

static void str_types_state_info(benchmark::State &state)
{
    for (auto _ : state)
    {
        auto state = StateInfo();
        state["test1"] = "test1_1";
        benchmark::DoNotOptimize(state);
        state["test2"] = "test2_1";
        benchmark::DoNotOptimize(state);
        state["test3"] = "test3_1";
        benchmark::DoNotOptimize(state);
        state["test1"] = "test1_2";
        benchmark::DoNotOptimize(state);
        state["test2"] = "test2_2";
        benchmark::DoNotOptimize(state);
        state["test3"] = "test3_2";
        benchmark::DoNotOptimize(state);
    }
}

static void num_types_state_info(benchmark::State &state)
{
    for (auto _ : state)
    {
        auto state = StateInfo();
        state["test1"] = 1.1;
        benchmark::DoNotOptimize(state);
        state["test2"] = 2.1;
        benchmark::DoNotOptimize(state);
        state["test3"] = 3.1;
        benchmark::DoNotOptimize(state);
        state["test1"] = 1.2;
        benchmark::DoNotOptimize(state);
        state["test2"] = 2.2;
        benchmark::DoNotOptimize(state);
        state["test3"] = 3.2;
        benchmark::DoNotOptimize(state);
    }
}

BENCHMARK(types_enum)->Name("enum types (init x3, modify x3)")->Iterations(50000);
BENCHMARK(str_types_state_info)->Name("string states (init x3, modify x3)")->Iterations(50000);
BENCHMARK(num_types_state_info)->Name("numeric states (init x3, modify x3)")->Iterations(50000);

} // namespace rm::rm_test

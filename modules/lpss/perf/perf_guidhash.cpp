#include <benchmark/benchmark.h>

#include "rmvl/lpss/node_rsd.hpp"

using namespace rm;

namespace rm_test {

void guid_hash(benchmark::State &state) {
    lpss::GuidHash guid_hasher;
    for (auto _ : state) {
        benchmark::DoNotOptimize(guid_hasher({}));
    }
}

BENCHMARK(guid_hash)->Name("LPSS GUID Hash")->Iterations(100000);

} // namespace rm_test
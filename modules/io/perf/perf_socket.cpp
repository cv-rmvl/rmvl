#include <benchmark/benchmark.h>
#include <string>

#include "rmvl/io/socket.hpp"

using namespace rm;

// ==============================================================================
// 对比测试 1：拼接拷贝发送 (Copy & Concatenate)
// ==============================================================================
static void BM_SocketWrite(benchmark::State &state) {
    Sender sender(ip::udp::v4());
    auto client = sender.create();
    Endpoint target_ep(ip::udp::v4(), 18080); // 本地空闲端口
    std::string_view localhost = "127.0.0.1";

    const std::string MOCK_HEADER = "RMVL_SYNC_HEADER_V1";
    // 动态生成负载大小（通过 state.range 传入，测试不同量级下的性能表现）
    const std::string MOCK_PAYLOAD(state.range(0), 'X');

    // Google Benchmark 核心循环
    for (auto _ : state) {
        // 堆内存分配和数据拷贝
        std::string concat_buffer;
        concat_buffer.reserve(MOCK_HEADER.size() + MOCK_PAYLOAD.size());
        concat_buffer.append(MOCK_HEADER);
        concat_buffer.append(MOCK_PAYLOAD);

        client.write(localhost, target_ep, concat_buffer);
    }
}

// 分别测试 1KB, 10KB, 60KB 负载下的传统发送性能
BENCHMARK(BM_SocketWrite)->Arg(1024)->Arg(10240)->Arg(60000);

// ==============================================================================
// 对比测试 2：Scatter-Gather 聚集写零拷贝发送 (MultiWrite)
// ==============================================================================
static void BM_SocketMultiWrite(benchmark::State &state) {
    Sender sender(ip::udp::v4());
    auto client = sender.create();
    Endpoint target_ep(ip::udp::v4(), 18081); // 错开端口避免干扰
    std::string_view localhost = "127.0.0.1";

    const std::string MOCK_HEADER = "RMVL_SYNC_HEADER_V1";
    const std::string MOCK_PAYLOAD(state.range(0), 'X');

    for (auto _ : state) {
        client.multiwrite(localhost, target_ep, MOCK_HEADER, MOCK_PAYLOAD);
    }
}

// 同样测试 1KB, 10KB, 60KB 负载下的聚集写性能
BENCHMARK(BM_SocketMultiWrite)->Arg(1024)->Arg(10240)->Arg(60000);
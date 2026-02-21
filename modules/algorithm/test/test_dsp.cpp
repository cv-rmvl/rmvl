/**
 * @file test_dsp.cpp
 * @author zhaoxi (535394140@qq.com)
 * @brief 数字信号处理单元测试
 * @version 1.0
 * @date 2024-08-31
 *
 * @copyright Copyright 2024 (c), zhaoxi
 *
 */

#include <gtest/gtest.h>

#include "rmvl/algorithm/dsp.hpp"
#include "rmvl/algorithm/math.hpp"

namespace rm_test {

TEST(Algorithm_cal, dsp_dft) {
    constexpr int f = 16;
    // 1024 个点的正弦波
    rm::ComplexSignal x(1024);
    for (std::size_t i = 0; i < 1024; ++i)
        x[i] = std::sin(2 * rm::PI * i * f / 1024);
    auto X = rm::dft(x);
    auto Gx = rm::Gx(X, rm::GxType::Amp);
    // 获取幅度谱最大值对应的频率
    std::ptrdiff_t max_it = std::max_element(Gx.begin(), Gx.end()) - Gx.begin();
    EXPECT_EQ(max_it, static_cast<std::ptrdiff_t>(f));
}

} // namespace rm_test

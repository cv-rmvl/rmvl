/**
 * @file test_math.cpp
 * @author zhaoxi (535394140@qq.com)
 * @brief 基础数学工具单元测试
 * @version 1.0
 * @date 2025-03-10
 *
 * @copyright Copyright (c) 2025
 *
 */

#include <gtest/gtest.h>

#include "rmvl/algorithm/math.hpp"

namespace rm_test {

TEST(Algorithm_math, mean) {
    std::vector vec = {1, 2, 3, 4, 5};
    auto mean = rm::mean(vec.begin(), vec.end());
    EXPECT_EQ(mean, 3);

    std::vector vec2 = {1.1, 2.2, 3.3, 4.4, 5.5};
    auto mean2 = rm::mean(vec2.begin(), vec2.end());
    EXPECT_NEAR(mean2, 3.3, 1e-8);

    double arr[] = {-1., 0., 1., 3., 5.};
    auto mean3 = rm::mean(std::begin(arr), std::end(arr));
    EXPECT_NEAR(mean3, 1.6, 1e-8);

    double arr2[] = {1e-4, 2e-4, 3e-4, 4e-4, 5e-4};
    double mean4 = rm::mean(std::begin(arr2), std::end(arr2));
    EXPECT_NEAR(mean4, 3e-4, 1e-8);

#if __cplusplus >= 202002L
    constexpr double carr[] = {3., 4., 5., 6., 7.};
    static_assert(rm::mean(std::begin(carr), std::end(carr)) == 5.);
#endif
}

TEST(Algorithm_math, variance) {
    std::vector vec = {1., 2., 3., 4., 5.};
    auto var = rm::variance(vec.begin(), vec.end());
    EXPECT_NEAR(var, 2, 1e-8);

    std::vector vec2 = {1.1, 2.2, 3.3, 4.4, 5.5};
    auto var2 = rm::variance(vec2.begin(), vec2.end());
    EXPECT_NEAR(var2, 2.42, 1e-8);

    double arr[] = {-1., 0., 3., 3., 5.};
    auto var3 = rm::variance(std::begin(arr), std::end(arr));
    EXPECT_NEAR(var3, 4.8, 1e-8);

    double arr2[] = {1e-3, 2e-3, 3e-3, 4e-3, 5e-3};
    auto var4 = rm::variance(std::begin(arr2), std::end(arr2));
    EXPECT_NEAR(var4, 2e-6, 1e-8);

#if __cplusplus >= 202002L
    constexpr double carr[] = {3., 4., 5., 6., 7.};
    static_assert(rm::variance(std::begin(carr), std::end(carr)) == 2.);
#endif
}

} // namespace rm_test

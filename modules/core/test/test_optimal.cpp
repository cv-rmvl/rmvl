/**
 * @file test_optimal.cpp
 * @author zhaoxi (535394140@qq.com)
 * @brief 最优化算法库测试
 * @version 1.0
 * @date 2024-05-04
 *
 * @copyright Copyright 2024 (c), zhaoxi
 *
 */

#include <cmath>
#include <gtest/gtest.h>

#include <rmvl/core/numcal.hpp>

namespace rm_test
{

static inline double f1d(double x) { return x * x - 4 * x + 7; }
static inline double rosenbrock(const std::vector<double> &x)
{
    const auto &x1 = x[0], &x2 = x[1];
    return 100 * (x2 - x1 * x1) * (x2 - x1 * x1) + (1 - x1) * (1 - x1);
}
static inline double quadratic(const std::vector<double> &x)
{
    const auto &x1 = x[0], &x2 = x[1];
    return 60 - 10 * x1 - 4 * x2 + x1 * x1 + x2 * x2 - x1 * x2;
}

TEST(Optimal, derivative)
{
    auto func = [](double x) { return std::pow(x, 7) - std::pow(x, 5); };
    auto val_central = rm::derivative(func, 1);
    EXPECT_NEAR(val_central, 2, 1e-3);
    auto val_ridders = rm::derivative(func, 1, rm::Diff_Ridders);
    EXPECT_NEAR(val_ridders, 2, 1e-12);
}

TEST(Optimal, fminbnd)
{
    auto [x1, x2] = rm::region(f1d, -5);
    EXPECT_LE(x1, 2);
    EXPECT_GE(x2, 2);
    auto [x, fval] = rm::fminbnd(f1d, x1, x2);
    EXPECT_NEAR(x, 2, 1e-4);
    EXPECT_NEAR(fval, 3, 1e-4);
}

TEST(Optimal, fminunc_simplex)
{
    rm::OptimalOptions options;
    options.optm_mode = rm::Optm_Simplex;
    auto [x, fval] = rm::fminunc(rosenbrock, {1, -2}, options);
    EXPECT_NEAR(x[0], 1, 1e-2);
    EXPECT_NEAR(x[1], 1, 1e-2);
    EXPECT_NEAR(fval, 0, 1e-2);
}

TEST(Optimal, fminunc_conjgrad)
{
    rm::OptimalOptions options;
    auto [x, fval] = rm::fminunc(quadratic, {0, 0}, options);
    EXPECT_NEAR(x[0], 8, 1e-4);
    EXPECT_NEAR(x[1], 6, 1e-4);
    EXPECT_NEAR(fval, 8, 1e-4);
}

} // namespace rm_test

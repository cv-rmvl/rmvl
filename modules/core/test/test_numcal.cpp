/**
 * @file test_numcal.cpp
 * @author zhaoxi (535394140@qq.com)
 * @brief Number calculation module unit test 数值计算模块单元测试
 * @version 1.0
 * @date 2024-01-14
 *
 * @copyright Copyright 2023 (c), zhaoxi
 *
 */

#include <cmath>

#include <gtest/gtest.h>

#include "rmvl/core/numcal.hpp"

namespace rm_test
{

TEST(NumberCalculation, polynomial)
{
    rm::Polynomial foo = {1, 2, 3};
    EXPECT_EQ(foo(0), 1);  // 1 + 2*0 + 3*0*0 = 1
    EXPECT_EQ(foo(1), 6);  // 1 + 2*1 + 3*1*1 = 6
    EXPECT_EQ(foo(2), 17); // 1 + 2*2 + 3*2*2 = 17
}

TEST(NumberCalculation, function_interpolator)
{
    rm::Interpolator foo({1, 2, 3}, {0, 1, 0});
    EXPECT_EQ(foo(0), -3); // a0 = -3, a1 = 4, a2 = -1
    foo.add(0, 1);
    EXPECT_EQ(foo(4), -7); // a0 = 1, a1=-10/3, a2=3, a3=-2/3
}

TEST(NumberCalculation, curve_fitter)
{
    rm::CurveFitter foo({1, 2, 3, 4}, {0, 2, 1, 3}, 0b11);
    EXPECT_LE(abs(foo(0.625)), 1e-5);
    EXPECT_LE(foo(0) + 0.5, 1e-5);
}

TEST(NumberCalculation, nonlinear_solver)
{
    rm::NonlinearSolver foo;
    foo = [](double x) { return x * x - 4; }; // f(x)
    EXPECT_LE(foo(2.5) - 2, 1e-5);            // fo(2) = 0
    EXPECT_LE(foo(1.5) - 2, 1e-5);            // fo(2) = 0
    EXPECT_LE(foo(-1.5) + 2, 1e-5);           // fo(-2) = 0
    EXPECT_LE(foo(-1.5) + 2, 1e-5);           // fo(-2) = 0
}

TEST(NumberCalculation, runge_kutta_1_order)
{
    auto f = []([[maybe_unused]] double t, const std::vector<double> &xs) { return -2 * xs[0] - 2; }; // e^{-2x} - 1
    std::vector<rm::ODE> fs = {f};

    rm::RungeKutta rkb(fs, {0.0, 2.0 / 3.0}, {0.25, 0.75}, {{0.0, 0.0}, {2.0 / 3.0, 0.0}});
    auto resb = rkb(0, {0}, 0.01, 100);
    EXPECT_LE(std::abs(resb.front() - std::expm1(-2)), 1e-4);

    rm::RungeKutta<rm::RkType::RK2> rk2(fs);
    auto res2 = rk2(0, {0}, 0.01, 100);
    EXPECT_LE(std::abs(res2.front() - std::expm1(-2)), 1e-4);

    rm::RungeKutta<rm::RkType::RK3> rk3(fs);
    auto res3 = rk3(0, {0}, 0.01, 100);
    EXPECT_LE(std::abs(res3.front() - std::expm1(-2)), 1e-5);

    rm::RungeKutta<rm::RkType::RK4> rk4(fs);
    auto res4 = rk4(0, {0}, 0.01, 100);
    EXPECT_LE(std::abs(res4.front() - std::expm1(-2)), 1e-6);
}

} // namespace rm_test

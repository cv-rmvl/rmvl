/**
 * @file test_numcal.cpp
 * @author zhaoxi (535394140@qq.com)
 * @brief Number calculation module unit test 数值计算模块单元测试
 * @version 1.0
 * @date 2024-01-14
 *
 * @copyright Copyright 2024 (c), zhaoxi
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

TEST(NumberCalculation, curve_fitter_ax_b)
{
    rm::CurveFitter foo({1, 2, 3, 4}, {0, 2, 1, 3}, 0b11);
    EXPECT_LE(abs(foo(0.625)), 1e-5);
    EXPECT_LE(foo(0) + 0.5, 1e-5);
}

TEST(NumberCalculation, curve_fitter_ax2_bx_c)
{
    // 2x^2 + 3x - 1
    rm::CurveFitter foo({0, 1, 2}, {-1, 4, 13}, 0b111);
    // 2*3^2 + 3*3 - 1 = 26
    EXPECT_LE(foo(3) - 26, 1e-5);
}

TEST(NumberCalculation, curve_fitter_ax3_cx_d)
{
    // x^3 - 4x + 1
    rm::CurveFitter foo({0, 1, 2}, {1, -2, 1}, 0b1011);
    // 3^3 - 4*3 + 1 = 16
    EXPECT_LE(foo(3) - 16, 1e-5);
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

TEST(NumberCalculation, runge_kutta_ode)
{
    auto f = [](double, const std::vector<double> &xs) { return -2 * xs[0] - 2; }; // e^{-2x} - 1
    std::vector<rm::Ode> fs = {f};

    rm::RungeKutta rkb(fs, {0.0, 2.0 / 3.0}, {0.25, 0.75}, {{0.0, 0.0}, {2.0 / 3.0, 0.0}});
    rkb.init(0, {0});
    auto resb = rkb.solve(0.01, 100).back();
    EXPECT_LE(std::abs(resb.front() - std::expm1(-2)), 1e-4);

    rm::RungeKutta2 rk2(fs);
    rk2.init(0, {0});
    auto res2 = rk2.solve(0.01, 100).back();
    EXPECT_LE(std::abs(res2.front() - std::expm1(-2)), 1e-4);

    rm::RungeKutta3 rk3(fs);
    rk3.init(0, {0});
    auto res3 = rk3.solve(0.01, 100).back();
    EXPECT_LE(std::abs(res3.front() - std::expm1(-2)), 1e-5);

    rm::RungeKutta4 rk4(fs);
    rk4.init(0, {0});
    auto res4 = rk4.solve(0.01, 100).back();
    EXPECT_LE(std::abs(res4.front() - std::expm1(-2)), 1e-6);
}

TEST(NumberCalculation, runge_kutta_odes)
{
    rm::Ode dot_x1 = [](double t, const std::vector<double> &x) { return 2 * x[1] + t; };
    rm::Ode dot_x2 = [](double, const std::vector<double> &x) { return -x[0] - 3 * x[1]; };
    rm::Odes fs = {dot_x1, dot_x2};
    //     ┌  3/4 ┐          ┌  2 ┐         ┌  3/2 ┐    ┌ -7/4 ┐
    // X = │      │e^{-2t} + │    │e^{-t} + │      │t + │      │
    //     └ -3/4 ┘          └ -1 ┘         └ -1/2 ┘    └  3/4 ┘
    double real_x1 = 3.0 / 4.0 * std::exp(-2) + 2 * std::exp(-1) + 3.0 / 2.0 - 7.0 / 4.0;
    double real_x2 = -3.0 / 4.0 * std::exp(-2) - std::exp(-1) - 1.0 / 2.0 + 3.0 / 4.0;

    rm::RungeKutta2 rk2(fs);
    rk2.init(0, {1, -1});
    auto res2 = rk2.solve(0.01, 100).back();
    EXPECT_EQ(res2.size(), 2);
    EXPECT_LE(std::abs(res2[0] - real_x1), 1e-4);
    EXPECT_LE(std::abs(res2[1] - real_x2), 1e-4);

    rm::RungeKutta4 rk4(fs);
    rk4.init(0, {1, -1});
    auto res4 = rk4.solve(0.01, 100).back();
    EXPECT_LE(std::abs(res4[0] - real_x1), 1e-6);
    EXPECT_LE(std::abs(res4[1] - real_x2), 1e-6);
}

#if __cpp_lib_generator >= 202207L
TEST(NumberCalculation, runge_kutta_ode_generator)
{
    auto f = [](double, const std::vector<double> &xs) { return 1; }; // x + 1
    std::vector<rm::Ode> fs = {f};

    rm::RungeKutta2 rk2(fs);
    rk2.init(0, {1});
    std::vector<std::vector<double>> all_res;
    all_res.reserve(10);
    all_res.push_back({1}); // 初值
    for (auto res2 : rk2.generate(1, 9))
        all_res.push_back(res2);

    for (std::size_t i = 0; i < 10; i++)
        EXPECT_EQ(all_res[i].front(), i + 1);
}
#endif

} // namespace rm_test

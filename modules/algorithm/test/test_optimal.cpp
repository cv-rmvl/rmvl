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

#include <rmvl/algorithm/numcal.hpp>

namespace rm_test {

static inline double f1d(double x) { return x * x - 4 * x + 7; }
static inline double rosenbrock(const std::valarray<double> &x) {
    const auto &x1 = x[0], &x2 = x[1];
    return 100 * (x2 - x1 * x1) * (x2 - x1 * x1) + (1 - x1) * (1 - x1);
}
static inline double quadratic(const std::valarray<double> &x) {
    const auto &x1 = x[0], &x2 = x[1];
    return 60 - 10 * x1 - 4 * x2 + x1 * x1 + x2 * x2 - x1 * x2;
}

TEST(Optimal, derivative) {
    auto func = [](double x) { return std::pow(x, 7) - std::pow(x, 5); };
    auto val_central = rm::derivative(func, 1);
    EXPECT_NEAR(val_central, 2, 1e-3);
    auto val_ridders = rm::derivative(func, 1, rm::DiffMode::Ridders);
    EXPECT_NEAR(val_ridders, 2, 1e-12);
}

TEST(Optimal, fminbnd) {
    auto [x1, x2] = rm::region(f1d, -5);
    EXPECT_LE(x1, 2);
    EXPECT_GE(x2, 2);
    auto [x, fval] = rm::fminbnd(f1d, x1, x2);
    EXPECT_NEAR(x, 2, 1e-4);
    EXPECT_NEAR(fval, 3, 1e-4);
}

TEST(Optimal, fminunc_simplex) {
    rm::OptimalOptions options;
    options.fmin_mode = rm::FminMode::Simplex;
    auto [x, fval] = rm::fminunc(rosenbrock, {1, -2}, options);
    EXPECT_NEAR(x[0], 1, 1e-2);
    EXPECT_NEAR(x[1], 1, 1e-2);
    EXPECT_NEAR(fval, 0, 1e-2);
}

TEST(Optimal, fminunc_conjgrad) {
    auto [x, fval] = rm::fminunc(quadratic, {0, 0});
    EXPECT_NEAR(x[0], 8, 1e-4);
    EXPECT_NEAR(x[1], 6, 1e-4);
    EXPECT_NEAR(fval, 8, 1e-4);
}

TEST(Optimal, fmincon_degredate_to_fminunc) {
    auto [x, fval] = rm::fmincon(quadratic, {0, 0}, {}, {});
    EXPECT_NEAR(x[0], 8, 1e-4);
    EXPECT_NEAR(x[1], 6, 1e-4);
    EXPECT_NEAR(fval, 8, 1e-4);
}

static inline std::valarray<double> ceq(const std::valarray<double> &x) { return {x[0] + x[1] - 10}; }

TEST(Optimal, fmincon_exterior_equation_con) {
    rm::OptimalOptions options;
    options.tol = 1e-3;
    auto [x, fval] = rm::fmincon(quadratic, {0, 0}, {}, ceq);
    EXPECT_NEAR(x[0], 6, 1e-2);
    EXPECT_NEAR(x[1], 4, 1e-2);
    EXPECT_NEAR(fval, 12, 1e-2);
}

static inline std::valarray<double> cle(const std::valarray<double> &x) { return {-x[0] - x[1] + 10,
                                                                                  2 * x[0] + x[1] - 30,
                                                                                  -x[0] + x[1] - 5}; }

TEST(Optimal, fmincon_exterior_inequality_con) {
    auto [x, fval] = rm::fmincon(quadratic, {5, 5}, cle, {});
    EXPECT_NEAR(x[0], 8, 1e-3);
    EXPECT_NEAR(x[1], 6, 1e-3);
    EXPECT_NEAR(fval, 8, 1e-3);
}

#ifdef HAVE_OPENCV

TEST(Optimal, fmincon_lagrange) {
    rm::OptimalOptions options;
    options.cons_mode = rm::ConsMode::Lagrange;
    options.tol = 1e-6;
    auto [x, fval] = rm::fmincon(quadratic, {0, 0}, {}, ceq, options);
    EXPECT_NEAR(x[0], 6, 1e-4);
    EXPECT_NEAR(x[1], 4, 1e-4);
    EXPECT_NEAR(fval, 12, 1e-4);
}

static inline std::valarray<double> lsq_linear(const std::valarray<double> &x) { return {x[0] + x[1] - 6,
                                                                                         x[0] - x[1] - 4}; }

TEST(Optimal, lsqnonlin_linear) {
    rm::OptimalOptions options;
    options.lsq_mode = rm::LsqMode::GN;
    auto x = rm::lsqnonlin(lsq_linear, {0, 0}, options);
    EXPECT_NEAR(x[0], 5, 1e-3);
    EXPECT_NEAR(x[1], 1, 1e-3);
    options.lsq_mode = rm::LsqMode::LM;
    x = rm::lsqnonlin(lsq_linear, {0, 0}, options);
    EXPECT_NEAR(x[0], 5, 1e-3);
    EXPECT_NEAR(x[1], 1, 1e-3);
}

// 待拟合曲线: 0.8sin(1.9FPSx) + 2.09 - 0.8
static inline double real_f(double x) {
    constexpr double FPS = 100;
    return 0.8 * std::sin(1.9 / FPS * x - 0.2) + 1.29;
}

TEST(Optimal, lsqnonlin_sine) {
    std::array<rm::FuncNd, 5> lsq_sine;
    for (std::size_t i = 0; i < lsq_sine.size(); ++i)
        lsq_sine[i] = [=](const std::valarray<double> &x) { return x[0] * std::sin(x[1] * i + x[2]) + x[3] - real_f(i); };

    rm::FuncNds lsq_sine_f = [&](const std::valarray<double> &x) {
        std::valarray<double> ret(lsq_sine.size());
        for (std::size_t i = 0; i < lsq_sine.size(); ++i)
            ret[i] = lsq_sine[i](x);
        return ret;
    };

    rm::OptimalOptions options;
    // SGN
    options.max_iter = 50;
    options.lsq_mode = rm::LsqMode::SGN;
    auto x = rm::lsqnonlin(lsq_sine_f, {1, 0.02, 0, 1.09}, options);
    EXPECT_NEAR(x[0], 0.8, 1e-4);
    EXPECT_NEAR(x[1], 0.019, 1e-4);
    EXPECT_NEAR(x[2], -0.2, 1e-4);
    EXPECT_NEAR(x[3], 2.09 - 0.8, 1e-4);
    // GN
    options.max_iter = 100;
    options.lsq_mode = rm::LsqMode::GN;
    x = rm::lsqnonlin(lsq_sine_f, {1, 0.02, 0, 1.09}, options);
    EXPECT_NEAR(x[0], 0.8, 1e-4);
    EXPECT_NEAR(x[1], 0.019, 1e-4);
    EXPECT_NEAR(x[2], -0.2, 1e-4);
    EXPECT_NEAR(x[3], 2.09 - 0.8, 1e-4);
    // LM
    options.max_iter = 2000;
    options.lsq_mode = rm::LsqMode::LM;
    x = rm::lsqnonlin(lsq_sine_f, {1, 0.02, 0, 1.09}, options);
    EXPECT_NEAR(x[0], 0.8, 1e-4);
    EXPECT_NEAR(x[1], 0.019, 1e-4);
    EXPECT_NEAR(x[2], -0.2, 1e-4);
    EXPECT_NEAR(x[3], 2.09 - 0.8, 1e-4);
}

TEST(Optimal, lsqnonlinRKF_sine) {
    std::array<rm::FuncNd, 5> lsq_sine;
    for (std::size_t i = 0; i < lsq_sine.size(); ++i)
        lsq_sine[i] = [=](const std::valarray<double> &x) { return x[0] * std::sin(x[1] * i + x[2]) + x[3] - real_f(i); };

    rm::FuncNds lsq_sine_f = [&](const std::valarray<double> &x) {
        std::valarray<double> ret(lsq_sine.size());
        for (std::size_t i = 0; i < lsq_sine.size(); ++i)
            ret[i] = lsq_sine[i](x);
        return ret;
    };

    auto x = rm::lsqnonlinRKF(lsq_sine_f, {1, 0.02, 0, 1.09}, rm::RobustMode::Huber);
    EXPECT_NEAR(x[0], 0.8, 1e-4);
    EXPECT_NEAR(x[1], 0.019, 1e-4);
    EXPECT_NEAR(x[2], -0.2, 1e-4);
    EXPECT_NEAR(x[3], 2.09 - 0.8, 1e-4);
    x = rm::lsqnonlinRKF(lsq_sine_f, {1, 0.02, 0, 1.09}, rm::RobustMode::Tukey);
    EXPECT_NEAR(x[0], 0.8, 1e-4);
    EXPECT_NEAR(x[1], 0.019, 1e-4);
    EXPECT_NEAR(x[2], -0.2, 1e-4);
    EXPECT_NEAR(x[3], 2.09 - 0.8, 1e-4);
}

#endif // HAVE_OPENCV

} // namespace rm_test

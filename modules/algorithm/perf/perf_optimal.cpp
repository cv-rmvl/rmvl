/**
 * @file perf_optimal.cpp
 * @author zhaoxi (535394140@qq.com)
 * @brief 最优化算法库基准测试
 * @version 1.0
 * @date 2024-05-04
 *
 * @copyright Copyright 2024 (c), zhaoxi
 *
 */

#include <benchmark/benchmark.h>
#include <opencv2/core.hpp>

#include "rmvl/algorithm/numcal.hpp"

namespace rm_test
{

/////////////////////// Quadratic Function ///////////////////////

static double quadraticFunc(const std::valarray<double> &x)
{
    const auto &x1 = x[0], &x2 = x[1];
    return 60 - 10 * x1 - 4 * x2 + x1 * x1 + x2 * x2 - x1 * x2;
};

static void cg_quadratic_rmvl(benchmark::State &state)
{
    rm::OptimalOptions options;
    options.fmin_mode = rm::FminMode::ConjGrad;
    options.max_iter = 5000;
    for (auto _ : state)
    {
        auto data = rm::fminunc(quadraticFunc, {0, 0}, options);
        benchmark::DoNotOptimize(data);
    }
}

class Quadratic : public cv::MinProblemSolver::Function
{
public:
    int getDims() const override { return 2; }

    double calc(const double *x) const override
    {
        const auto &x1 = x[0], &x2 = x[1];
        return 60 - 10 * x1 - 4 * x2 + x1 * x1 + x2 * x2 - x1 * x2;
    }
};

static void cg_quadratic_cv(benchmark::State &state)
{
    auto quadratic = cv::makePtr<Quadratic>();
    auto solver = cv::ConjGradSolver::create(quadratic);
    for (auto _ : state)
    {
        cv::Vec2d x;
        solver->minimize(x);
        benchmark::DoNotOptimize(x);
    }
}

BENCHMARK(cg_quadratic_rmvl)->Name("fminunc (conj_grad, quadratic) - by rmvl  ")->Iterations(50);
BENCHMARK(cg_quadratic_cv)->Name("fminunc (conj_grad, quadratic) - by opencv")->Iterations(50);

static inline double cle1(const std::valarray<double> &x) { return -x[0] - x[1] + 10; }
static inline double cle2(const std::valarray<double> &x) { return 2 * x[0] + x[1] - 30; }
static inline double cle3(const std::valarray<double> &x) { return -x[0] + x[1] - 5; }

static void com_quadratic_rmvl(benchmark::State &state)
{
    for (auto _ : state)
        rm::fmincon(quadraticFunc, {0, 0}, {cle1, cle2, cle3}, {});
}

BENCHMARK(com_quadratic_rmvl)->Name("fmincon (conj_grad, quadratic) - by rmvl  ")->Iterations(50);

/////////////////////// Rosenbrock Function ///////////////////////

static double rosenbrockFunc(const std::valarray<double> &x)
{
    const auto &x1 = x[0], &x2 = x[1];
    return 100 * (x2 - x1 * x1) * (x2 - x1 * x1) + (1 - x1) * (1 - x1);
};

void splx_rosenbrock_rmvl(benchmark::State &state)
{
    rm::OptimalOptions options;
    options.max_iter = 5000;
    options.fmin_mode = rm::FminMode::Simplex;
    for (auto _ : state)
    {
        auto data = rm::fminunc(rosenbrockFunc, {5, 3}, options);
        benchmark::DoNotOptimize(data);
    }
}

class Rosenbrock : public cv::DownhillSolver::Function
{
public:
    int getDims() const override { return 2; }

    double calc(const double *x) const override
    {
        const auto &x1 = x[0], &x2 = x[1];
        return 100 * (x2 - x1 * x1) * (x2 - x1 * x1) + (1 - x1) * (1 - x1);
    }
};

void splx_rosenbrock_cv(benchmark::State &state)
{
    auto rosenbrock = cv::makePtr<Rosenbrock>();
    auto solver = cv::DownhillSolver::create(rosenbrock);
    for (auto _ : state)
    {
        cv::Vec2d x(5, 3);
        solver->setInitStep(x);
        solver->minimize(x);
        benchmark::DoNotOptimize(x);
    }
}

BENCHMARK(splx_rosenbrock_rmvl)->Name("fminunc (simplex, rosenbrock) - by rmvl  ")->Iterations(50);
BENCHMARK(splx_rosenbrock_cv)->Name("fminunc (simplex, rosenbrock) - by opencv")->Iterations(50);

// 待拟合曲线: 0.8sin(1.9FPSx) + 2.09 - 0.8
static inline double real_f(double x)
{
    constexpr double FPS = 100;
    return 0.8 * std::sin(1.9 / FPS * x - 0.2) + 1.29;
};

void lsqnonlin_rmvl(benchmark::State &state)
{
    for (auto _ : state)
    {
        rm::FuncNds lsq_sine(20);
        for (std::size_t i = 0; i < lsq_sine.size(); ++i)
            lsq_sine[i] = [=](const std::valarray<double> &x) { return x[0] * std::sin(x[1] * i + x[2]) + x[3] - real_f(i); };

        auto x = rm::lsqnonlin(lsq_sine, {1, 0.02, 0, 1.09});
        benchmark::DoNotOptimize(x);
    }
}

BENCHMARK(lsqnonlin_rmvl)->Name("lsqnonlin (sine) - by rmvl")->Iterations(50);

} // namespace rm_test

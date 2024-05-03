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

#include "rmvl/core/numcal.hpp"

namespace rm_test
{

/////////////////////// Quadratic Function ///////////////////////

static double quadraticFunc(const std::vector<double> &x)
{
    const auto &x1 = x[0], &x2 = x[1];
    return 60 - 10 * x1 - 4 * x2 + x1 * x1 + x2 * x2 - x1 * x2;
};

static void cg_quadratic_rmvl(benchmark::State &state)
{
    for (auto _ : state)
        rm::fminunc(quadraticFunc, {0, 0});
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
    for (auto _ : state)
    {
        auto quadratic = cv::makePtr<Quadratic>();
        auto solver = cv::ConjGradSolver::create(quadratic);
        cv::Vec2d x;
        solver->minimize(x);
    }
}

BENCHMARK(cg_quadratic_rmvl)->Name("fminunc (conj_grad, quadratic) - by rmvl  ")->Iterations(50);
BENCHMARK(cg_quadratic_cv)->Name("fminunc (conj_grad, quadratic) - by opencv")->Iterations(50);

/////////////////////// Rosenbrock Function ///////////////////////

static double rosenbrockFunc(const std::vector<double> &x)
{
    const auto &x1 = x[0], &x2 = x[1];
    return 100 * (x2 - x1 * x1) * (x2 - x1 * x1) + (1 - x1) * (1 - x1);
};

void splx_rosenbrock_rmvl(benchmark::State &state)
{
    for (auto _ : state)
    {
        rm::OptimalOptions options;
        options.optm_mode = rm::Optm_Simplex;
        rm::fminunc(rosenbrockFunc, {5, 3}, options);
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
    for (auto _ : state)
    {
        auto rosenbrock = cv::makePtr<Rosenbrock>();
        auto solver = cv::DownhillSolver::create(rosenbrock);
        cv::Vec2d x(5, 3);
        solver->setInitStep(x);
        solver->minimize(x);
    }
}

BENCHMARK(splx_rosenbrock_rmvl)->Name("fminunc (simplex, rosenbrock) - by rmvl  ")->Iterations(50);
BENCHMARK(splx_rosenbrock_cv)->Name("fminunc (simplex, rosenbrock) - by opencv")->Iterations(50);

} // namespace rm_test

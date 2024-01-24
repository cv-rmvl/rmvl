/**
 * @file numcal.cpp
 * @author zhaoxi (535394140@qq.com)
 * @brief Numerical Calculation Module 数值计算模块
 * @version 1.0
 * @date 2024-01-10
 *
 * @copyright Copyright 2023 (c), zhaoxi
 *
 */

#include <numeric>

#include <opencv2/core.hpp>

#include "rmvl/core/numcal.hpp"
#include "rmvl/core/util.hpp"

#include "rmvlpara/core.hpp"

namespace rm
{

///////////////////// 函数插值 /////////////////////

Interpolator::Interpolator(const std::vector<double> &xs, const std::vector<double> &ys)
{
    RMVL_Assert(xs.size() == ys.size());
    _xs = xs;
    _diffquot.resize(xs.size());
    for (size_t i = 0; i < xs.size(); i++)
    {
        _diffquot[i].resize(i + 1);
        // 0 阶差商，即函数值
        _diffquot[i].front() = ys[i];
    }
    // 迭代计算第 k 阶差商
    for (size_t k = 1; k < xs.size(); k++)
        for (size_t i = k; i < xs.size(); i++)
            _diffquot[i][k] = (_diffquot[i][k - 1] - _diffquot[i - 1][k - 1]) / (_xs[i] - _xs[i - k]);
}

Interpolator &Interpolator::add(double x, double y)
{
    _xs.push_back(x);
    _diffquot.emplace_back(_xs.size());
    // 0 阶差商，即函数值
    _diffquot.back().front() = y;
    // k 阶差商
    for (size_t k = 1; k < _xs.size(); k++)
        _diffquot.back()[k] = (_diffquot.back()[k - 1] - _diffquot[_xs.size() - 2][k - 1]) / (_xs.back() - _xs[_xs.size() - 1 - k]);

    return *this;
}

double Interpolator::operator()(double x) const
{
    double y{};
    double xprod{1.0};
    for (size_t i = 0; i < _xs.size(); i++)
    {
        y += _diffquot[i][i] * xprod;
        xprod *= (x - _xs[i]);
    }
    return y;
}

CurveFitter::CurveFitter(const std::vector<double> &xs, const std::vector<double> &ys, std::bitset<8> order)
{
    RMVL_Assert(xs.size() == ys.size());
    if (order.count() > xs.size())
        RMVL_Error(RMVL_StsBadArg, "The number of 1 in \"order\" must be less than or equal to the number of nodes.");
    // 提取 order 中为 1 的位的索引（从低到高）
    _idx.reserve(order.count());
    for (std::size_t i = 0; i < order.size(); i++)
        if (order.test(i))
            _idx.push_back(i);
    // 构建法方程系数矩阵 G
    cv::Mat G(_idx.size(), _idx.size(), CV_64FC1);
    std::for_each(_idx.cbegin(), _idx.cend(), [&](std::size_t i) {
        std::for_each(_idx.cbegin(), _idx.cend(), [&](std::size_t j) {
            G.at<double>(i, j) = 0.0;
            std::for_each(xs.cbegin(), xs.cend(), [&](std::size_t val) { G.at<double>(i, j) += std::pow(val, i + j); });
        });
    });
    // 构建法方程系数矩阵 b
    cv::Mat b(_idx.size(), 1, CV_64FC1);
    std::for_each(_idx.cbegin(), _idx.cend(), [&](std::size_t i) {
        b.at<double>(i) = 0.0;
        for (std::size_t k = 0; k < xs.size(); k++)
            b.at<double>(i) += std::pow(xs[k], i) * ys[k];
    });
    // 求解法方程
    _coeffs.reserve(_idx.size());
    cv::Mat coeffs_mat;
    cv::solve(G, b, coeffs_mat, cv::DECOMP_CHOLESKY);
    std::for_each(coeffs_mat.begin<double>(), coeffs_mat.end<double>(), [&](double val) { _coeffs.push_back(val); });
}

double CurveFitter::operator()(double x) const
{
    double retval{};
    for (std::size_t i = 0; i < _idx.size(); i++)
        retval += _coeffs[i] * std::pow(x, _idx[i]);
    return retval;
}

double NonlinearSolver::operator()(double x0, double eps, std::size_t max_iter) const
{
    double xk{x0};
    for (std::size_t i = 0; i < max_iter; i++)
    {
        double yk = _func(xk);
        if (std::abs(yk) < eps)
            break;
        xk -= para::core_param.SECANT_STEP * yk / (_func(xk + para::core_param.SECANT_STEP) - yk);
        if (std::isinf(xk) || std::isnan(xk))
            RMVL_Error(RMVL_StsDivByZero, "The iteration is divergent");
    }
    return xk;
}

RungeKutta<RkType::Butcher>::RungeKutta(
    const Odes &fs, const std::vector<double> &p,
    const std::vector<double> &lambda, const std::vector<std::vector<double>> &r)
    : _ks(p.size()), _fs(fs), _p(p), _lambda(lambda), _r(r)
{
    // Initialize "ks"
    std::for_each(_ks.begin(), _ks.end(), [this](auto &k) { k.resize(_fs.size()); });
    // Check the size of p, lambda and R.rows
    if (_p.size() != _r.size() || _p.size() != _lambda.size())
        RMVL_Error(RMVL_StsBadArg, "The size of \"p\", \"lambda\" and \"R.rows\" must be equal.");
    for (std::size_t i = 0; i < _r.size(); i++)
        if (_r[i].size() < _r.size())
            RMVL_Error(RMVL_StsBadArg, "\"r[i].size()\" must be greater than or equal to the \"i\".");
}

// 加法
template <typename T>
inline static std::vector<T> operator+(const std::vector<T> &vec1, const std::vector<T> &vec2)
{
    std::vector<T> retval(vec1.size());
    std::transform(vec1.cbegin(), vec1.cend(), vec2.cbegin(), retval.begin(), std::plus<T>());
    return retval;
}

// 自加
template <typename T>
inline static std::vector<T> &operator+=(std::vector<T> &vec1, const std::vector<T> &vec2)
{
    std::transform(vec1.cbegin(), vec1.cend(), vec2.cbegin(), vec1.begin(), std::plus<T>());
    return vec1;
}

// 数乘
template <typename T>
inline static std::vector<T> operator*(const std::vector<T> &vec, T val)
{
    std::vector<T> retval(vec.size());
    std::transform(vec.cbegin(), vec.cend(), retval.begin(), [val](T x) { return x * val; });
    return retval;
}

// 数乘
template <typename T>
inline static std::vector<T> operator*(T val, const std::vector<T> &vec) { return vec * val; }

std::vector<double> RungeKutta<RkType::Butcher>::solve(double t0, const std::vector<double> &x0, double h, std::size_t n)
{
    double t{t0};
    std::vector<double> x{x0};
    for (std::size_t idx = 0; idx < n; idx++)
    {
        // 依次计算每个加权平均系数 k_i
        for (std::size_t i = 0; i < _ks.size(); i++)
        {
            // 计算 (a_i, k)
            std::vector<double> inner_prod(_fs.size());
            for (std::size_t n = 0; n < i; n++)
                inner_prod += _r[i][n] * _ks[n]; // \sum_r^i a_{ir}k_r
            // 计算 F
            for (std::size_t j = 0; j < _fs.size(); j++)
                _ks[i][j] = _fs[j](t + _p[i] * h, x + h * inner_prod);
        }
        // 更新 t 和 x
        t += h;
        for (std::size_t j = 0; j < _fs.size(); j++)
        {
            double inner_prod{};
            for (std::size_t i = 0; i < _ks.size(); i++)
                inner_prod += h * _lambda[i] * _ks[i][j];
            x[j] += inner_prod;
        }
    }
    return x;
}

RungeKutta<RkType::RK2>::RungeKutta(const Odes &fs)
    : RungeKutta<RkType::Butcher>(fs, {0.0, 0.5}, {0.0, 1.0},
                                  {{0.0, 0.0},
                                   {0.5, 0.0}}) {}

RungeKutta<RkType::RK3>::RungeKutta(const Odes &fs)
    : RungeKutta<RkType::Butcher>(fs, {0.0, 0.5, 1.0}, {1.0 / 6.0, 2.0 / 3.0, 1.0 / 6.0},
                                  {{0.0, 0.0, 0.0},
                                   {0.5, 0.0, 0.0},
                                   {-1.0, 2.0, 0.0}}) {}

RungeKutta<RkType::RK4>::RungeKutta(const Odes &fs)
    : RungeKutta<RkType::Butcher>(fs, {0.0, 0.5, 0.5, 1.0}, {1.0 / 6.0, 1.0 / 3.0, 1.0 / 3.0, 1.0 / 6.0},
                                  {{0.0, 0.0, 0.0, 0.0},
                                   {0.5, 0.0, 0.0, 0.0},
                                   {0.0, 0.5, 0.0, 0.0},
                                   {0.0, 0.0, 1.0, 0.0}}) {}

} // namespace rm

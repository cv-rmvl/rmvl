/**
 * @file numcal.cpp
 * @author zhaoxi (535394140@qq.com)
 * @brief Numerical Calculation Module 数值计算模块
 * @version 1.0
 * @date 2024-01-10
 *
 * @copyright Copyright 2024 (c), zhaoxi
 *
 */

#include <opencv2/core.hpp>

#include "rmvl/algorithm/math.hpp"
#include "rmvl/algorithm/numcal.hpp"
#include "rmvl/core/util.hpp"

#include "rmvlpara/core.hpp"

namespace rm
{

double Polynomial::operator()(double x) const noexcept
{
    double y = _coeffs.back();
    std::for_each(_coeffs.rbegin() + 1, _coeffs.rend(), [&](double val) { y = y * x + val; });
    return y;
}

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
    for (size_t i = 0; i < _idx.size(); ++i)
    {
        for (size_t j = 0; j < _idx.size(); ++j)
        {
            G.at<double>(i, j) = 0.0;
            for (size_t k = 0; k < xs.size(); ++k)
                G.at<double>(i, j) += std::pow(xs[k], _idx[i] + _idx[j]);
        }
    }
    // 构建法方程系数矩阵 b
    cv::Mat b(_idx.size(), 1, CV_64FC1);
    for (size_t i = 0; i < _idx.size(); ++i)
    {
        b.at<double>(i) = 0.0;
        for (size_t k = 0; k < xs.size(); ++k)
            b.at<double>(i) += std::pow(xs[k], _idx[i]) * ys[k];
    }
    // 求解法方程
    _coeffs.reserve(_idx.size());
    cv::Mat coeffs_mat;
    cv::solve(G, b, coeffs_mat, cv::DECOMP_CHOLESKY);
    std::for_each(coeffs_mat.begin<double>(), coeffs_mat.end<double>(), [this](double val) { _coeffs.push_back(val); });
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

RungeKutta::RungeKutta(
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

/**
 * @brief 计算数值解
 *
 * @param[in] r Butcher 表 R 矩阵
 * @param[in] p Butcher 表 p 向量
 * @param[in] lambda Butcher 表 λ 向量
 * @param[in] fs 一阶常微分方程组的函数对象
 * @param[in] h 步长
 * @param[in out] t 初始位置的自变量
 * @param[in out] x 初始位置的因变量
 * @param[in out] ks 加权平均系数 k
 */
static inline void calcRK(const std::vector<std::vector<double>> &r, const std::vector<double> &p,
                          const std::vector<double> &lambda, const Odes &fs, const double h,
                          double &t, std::vector<double> &x, std::vector<std::vector<double>> &ks)
{
    // 依次计算每个加权平均系数 k_i
    for (std::size_t i = 0; i < ks.size(); i++)
    {
        // 计算 (a_i, k)
        std::vector<double> inner_prod(fs.size());
        for (std::size_t n = 0; n < i; n++)
            inner_prod += r[i][n] * ks[n]; // \sum_r^i a_{ir}k_r
        // 计算 F
        for (std::size_t j = 0; j < fs.size(); j++)
            ks[i][j] = fs[j](t + p[i] * h, x + h * inner_prod);
    }
    // 更新 t 和 x
    t += h;
    for (std::size_t j = 0; j < fs.size(); j++)
    {
        double inner_prod{};
        for (std::size_t i = 0; i < ks.size(); i++)
            inner_prod += h * lambda[i] * ks[i][j];
        x[j] += inner_prod;
    }
}

std::vector<std::vector<double>> RungeKutta::solve(double h, std::size_t n)
{
    if (_x0.empty())
        RMVL_Error(RMVL_StsBadArg, "The initial value must be set.");
    double t{_t0};
    std::vector<double> x{_x0};
    std::vector<std::vector<double>> retval(n + 1);
    retval[0] = x;
    for (std::size_t idx = 0; idx < n; idx++)
    {
        calcRK(_r, _p, _lambda, _fs, h, t, x, _ks);
        // 保存结果
        retval[idx + 1] = x;
    }
    return retval;
}

#if __cpp_lib_generator >= 202207L
std::generator<std::vector<double>> RungeKutta::generate(double h, std::size_t n)
{
    if (_x0.empty())
        RMVL_Error(RMVL_StsBadArg, "The initial value must be set.");
    double t{_t0};
    std::vector<double> x{_x0};
    for (std::size_t idx = 0; idx < n; idx++)
    {
        calcRK(_r, _p, _lambda, _fs, h, t, x, _ks);
        co_yield x;
    }
}
#endif

RungeKutta2::RungeKutta2(const Odes &fs) : RungeKutta(fs, {0.0, 0.5}, {0.0, 1.0},
                                                      {{0.0, 0.0},
                                                       {0.5, 0.0}}) {}

RungeKutta3::RungeKutta3(const Odes &fs) : RungeKutta(fs, {0.0, 0.5, 1.0}, {1.0 / 6.0, 2.0 / 3.0, 1.0 / 6.0},
                                                      {{0.0, 0.0, 0.0},
                                                       {0.5, 0.0, 0.0},
                                                       {-1.0, 2.0, 0.0}}) {}

RungeKutta4::RungeKutta4(const Odes &fs) : RungeKutta(fs, {0.0, 0.5, 0.5, 1.0}, {1.0 / 6.0, 1.0 / 3.0, 1.0 / 3.0, 1.0 / 6.0},
                                                      {{0.0, 0.0, 0.0, 0.0},
                                                       {0.5, 0.0, 0.0, 0.0},
                                                       {0.0, 0.5, 0.0, 0.0},
                                                       {0.0, 0.0, 1.0, 0.0}}) {}

} // namespace rm

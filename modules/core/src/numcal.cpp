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

#include <rmvl/core/numcal.hpp>
#include <rmvl/core/util.hpp>

namespace rm
{

///////////////////// 函数插值 /////////////////////

Interpolation::Interpolation(const std::vector<double> &xs, const std::vector<double> &ys)
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

Interpolation &Interpolation::add(double x, double y)
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

double Interpolation::operator()(double x) const
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

} // namespace rm

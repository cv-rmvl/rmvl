/**
 * @file numcal.hpp
 * @author zhaoxi (535394140@qq.com)
 * @brief Numerical Calculation Module 数值计算模块
 * @version 1.0
 * @date 2024-01-06
 *
 * @copyright Copyright 2023 (c), zhaoxi
 *
 */

#pragma once

#include <algorithm>
#include <array>
#include <bitset>
#include <vector>

//! @addtogroup core
//! @{
//! @defgroup core_numcal 数值计算模块
//! @{
//! @brief 包含函数插值、曲线拟合、递推最小二乘、非线性方程（组）数值解、常微分方程数值解等数值计算算法
//! @} core_numcal
//! @} core

namespace rm
{

//! @addtogroup core_numcal
//! @{

//! N 次多项式
class Polynomial
{
    std::vector<double> _coeffs; //!< 多项式系数

public:
    /**
     * @brief 创建多项式对象
     * @brief
     * - 多项式系数 \f$a_0,a_1,\cdots,a_{N-1}\f$ 用来表示多项式 \f$f(x)=a_0+a_1x+\cdots+a_{N-1}x^{N-1}\f$
     *
     * @param[in] coeffs 多项式系数 \f$a_0,a_1,\cdots,a_{N-1}\f$
     */
    Polynomial(const std::initializer_list<double> &coeffs) : _coeffs(coeffs) {}

    /**
     * @brief 指定多项式阶数创建多项式对象
     *
     * @param[in] order 多项式阶数，即多项式的最高次数
     */
    Polynomial(size_t order) : _coeffs(order + 1) {}

    /**
     * @brief 计算多项式在指定点的函数值
     *
     * @param[in] x 指定点的 x 坐标
     * @return 多项式在指定点的函数值
     */
    inline double operator()(double x) const noexcept
    {
        double y = _coeffs.back();
        std::for_each(_coeffs.rbegin() + 1, _coeffs.rend(), [&](double val) { y = y * x + val; });
        return y;
    }
};

///////////////////// 函数插值 /////////////////////

/**
 * @brief 函数插值器
 * @brief
 * - 由于插值多项式具有唯一性，为了提高新增节点时算法的简易性，这里使用 Newton 插值多项式
 */
class Interpolator
{
    std::vector<double> _xs;                    //!< 插值节点
    std::vector<std::vector<double>> _diffquot; //!< 差商表

public:
    Interpolator() = default;

    /**
     * @brief 创建插值器对象，初始化差商表
     *
     * @param[in] xs 已知节点的 x 坐标 \f$x_0,x_1,\cdots,x_n\f$
     * @param[in] ys 已知节点的 y 坐标 \f$f(x_0),f(x_1),\cdots,f(x_n)\f$
     */
    Interpolator(const std::vector<double> &xs, const std::vector<double> &ys);

    /**
     * @brief 添加新的插值节点
     *
     * @param[in] x 新的插值节点的 x 坐标
     * @param[in] y 新的插值节点的 y 坐标
     * @code{.cpp}
     * Interpolator interf;
     * // 可以链式添加多个插值节点
     * interf.add(4, 16).add(5, 25).add(6, 36);
     * @endcode
     */
    Interpolator &add(double x, double y);

    /**
     * @brief 计算插值多项式在指定点的函数值
     *
     * @param[in] x 指定点的 x 坐标
     * @return 插值多项式在指定点的函数值
     */
    double operator()(double x) const;
};

///////////////////// 曲线拟合 /////////////////////

/**
 * @brief 曲线拟合器
 * @brief
 * - 使用最小二乘法拟合曲线，详见 @ref tutorial_modules_least_square
 */
class CurveFitter
{
    const std::size_t _num; //!< 节点个数
    std::vector<double> _g; //!< 铺平的法方程系数矩阵 G

public:
    /**
     * @brief 创建曲线拟合器对象
     *
     * @param[in] xs 已知节点的 x 坐标 \f$x_0,x_1,\cdots,x_n\f$
     * @param[in] ys 已知节点的 y 坐标 \f$f(x_0),f(x_1),\cdots,f(x_n)\f$
     * @param[in] order 拟合曲线的阶数，参数从最高位到最低位依次为 \f$a_0,a_1,\cdots,a_7\f$，例如
     *                  `0b10100011` 表示拟合曲线为 \f[f(x)=a_0+a_2x^2+a_6x^6+a_7x^7\f]
     */
    CurveFitter(const std::vector<double> &xs, const std::vector<double> &ys, std::bitset<8> order);

private:
    inline double &operator()(std::size_t i, std::size_t j) noexcept { return _g[i * _num + j]; }
    inline double operator()(std::size_t i, std::size_t j) const noexcept { return _g[i * _num + j]; }
};

/////////////////// 递推最小二乘 ///////////////////

////////////// 非线性方程（组）数值解 //////////////

///////////////// 常微分方程数值解 /////////////////

//! @} core_numcal

} // namespace rm

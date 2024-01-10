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

///////////////////// 函数插值 /////////////////////

/**
 * @brief 函数插值算法
 * @brief
 * - 由于插值多项式具有唯一性，为了提高新增节点时算法的简易性，这里使用 Newton 插值多项式
 */
class Interpolation
{
    std::vector<double> _xs;                    //!< 插值节点
    std::vector<std::vector<double>> _diffquot; //!< 差商表

public:
    Interpolation() = default;

    /**
     * @brief 创建插值算法对象，初始化差商表
     *
     * @param[in] xs 已知节点的 x 坐标 \f$x_0,x_1,\cdots,x_n\f$
     * @param[in] ys 已知节点的 y 坐标 \f$f(x_0),f(x_1),\cdots,f(x_n)\f$
     */
    Interpolation(const std::vector<double> &xs, const std::vector<double> &ys);

    /**
     * @brief 添加新的插值节点
     *
     * @param[in] x 新的插值节点的 x 坐标
     * @param[in] y 新的插值节点的 y 坐标
     * @code{.cpp}
     * Interpolation interf;
     * // 可以链式添加多个插值节点
     * interf.add(4, 16).add(5, 25).add(6, 36);
     * @endcode
     */
    Interpolation &add(double x, double y);

    /**
     * @brief 计算插值多项式在指定点的函数值
     *
     * @param[in] x 指定点的 x 坐标
     * @return 插值多项式在指定点的函数值
     */
    double operator()(double x) const;
};

///////////////////// 曲线拟合 /////////////////////

/////////////////// 递推最小二乘 ///////////////////

////////////// 非线性方程（组）数值解 //////////////

///////////////// 常微分方程数值解 /////////////////

//! @} core_numcal

} // namespace rm

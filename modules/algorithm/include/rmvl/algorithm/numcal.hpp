/**
 * @file numcal.hpp
 * @author zhaoxi (535394140@qq.com)
 * @brief Numerical Calculation Module 数值计算与最优化模块
 * @version 1.0
 * @date 2024-01-06
 *
 * @copyright Copyright 2024 (c), zhaoxi
 *
 */

#pragma once

#include <bitset>
#include <cstdint>
#include <functional>
#include <vector>

#if __cplusplus >= 202302L
#include <generator>
#endif

#include "rmvl/core/rmvldef.hpp"

//! @addtogroup algorithm
//! @{
//! @defgroup algorithm_numcal 数值计算模块
//! @{
//! @brief 包含函数插值、曲线拟合、非线性方程（组）数值解、常微分方程数值解等数值计算算法
//! @} algorithm_numcal
//! @defgroup algorithm_optimal 最优化算法库
//! @{
//! @brief 包含一维函数最小值搜索、无约束多维函数最小值搜索等最优化算法
//! @} algorithm_optimal
//! @} algorithm

namespace rm
{

//! @addtogroup algorithm_numcal
//! @{

//! N 次多项式
class RMVL_EXPORTS_W Polynomial
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
    RMVL_W Polynomial(const std::vector<double> &coeffs) : _coeffs(coeffs) {}

    /**
     * @brief 计算多项式在指定点的函数值
     *
     * @param[in] x 指定点的 x 坐标
     * @return 多项式在指定点的函数值
     */
    RMVL_W double operator()(double x) const noexcept;
};

///////////////////// 函数插值 /////////////////////

/**
 * @brief 函数插值器
 * @brief
 * - 由于插值多项式具有唯一性，为了提高新增节点时算法的简易性，这里使用 Newton 插值多项式
 */
class RMVL_EXPORTS_W Interpolator
{
    std::vector<double> _xs;                    //!< 插值节点
    std::vector<std::vector<double>> _diffquot; //!< 差商表

public:
    RMVL_W Interpolator() = default;

    /**
     * @brief 创建插值器对象，初始化差商表
     *
     * @param[in] xs 已知节点的 x 坐标 \f$x_0,x_1,\cdots,x_n\f$
     * @param[in] ys 已知节点的 y 坐标 \f$f(x_0),f(x_1),\cdots,f(x_n)\f$
     */
    RMVL_W Interpolator(const std::vector<double> &xs, const std::vector<double> &ys);

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
    RMVL_W Interpolator &add(double x, double y);

    /**
     * @brief 计算插值多项式在指定点的函数值
     *
     * @param[in] x 指定点的 x 坐标
     * @return 插值多项式在指定点的函数值
     */
    RMVL_W double operator()(double x) const;
};

////////////////// 多项式曲线拟合 //////////////////

/**
 * @brief 曲线拟合器
 * @brief
 * - 使用最小二乘法拟合曲线，详见 @ref tutorial_modules_least_square
 */
class RMVL_EXPORTS_W CurveFitter
{
    std::vector<std::size_t> _idx; //!< 多项式拟合曲线的阶数（从低到高）
    std::vector<double> _coeffs;   //!< 多项式拟合曲线的系数

public:
    /**
     * @brief 创建多项式曲线拟合器对象
     *
     * @param[in] xs 已知节点的 x 坐标列表 \f$\text{xs}=\{x_0,x_1,\cdots,x_n\}\f$
     * @param[in] ys 已知节点的 y 坐标列表 \f$\text{ys}=\{f(x_0),f(x_1),\cdots,f(x_n)\}\f$
     * @param[in] order 拟合曲线的阶数，参数从最 **低** 位到最 **高** 位依次为 \f$a_0\f$ ~ \f$a_7\f$，即
     *                  \f[f(x)=a_0+a_1x+\cdots+a_7x^7\tag1\f]例如 `0b01000101` 表示拟合曲线为
     *                  \f[f(x)=a_0+a_2x^2+a_6x^6\tag2\f]
     */
    RMVL_W CurveFitter(const std::vector<double> &xs, const std::vector<double> &ys, std::bitset<8> order);

    /**
     * @brief 计算拟合的多项式曲线在指定点的函数值
     *
     * @param[in] x 指定点的 x 坐标
     * @return 拟合的多项式曲线在指定点的函数值
     */
    RMVL_W double operator()(double x) const;
};

///////////////// 非线性方程数值解 /////////////////

/**
 * @brief 非线性方程求解器
 * @brief
 * - 使用离散 Newton 迭代法求解非线性方程，详见 @ref tutorial_modules_func_iteration
 */
class RMVL_EXPORTS_W NonlinearSolver
{
public:
    RMVL_W_RW std::function<double(double)> func; //!< 非线性方程函数对象

    RMVL_W NonlinearSolver() = default;

    /**
     * @brief 创建非线性方程求解器对象
     *
     * @param[in] f 非线性方程 \f$f(x)=0\f$ 的函数对象 \f$f(x)\f$
     * @note
     * - 可以是函数指针、函数对象、lambda 表达式等，可隐式转换为 `double (*)(double)`
     */
    RMVL_W NonlinearSolver(const std::function<double(double)> &f) : func(f) {}

    /**
     * @brief 使用离散 Newton 迭代法求解非线性方程 \f$f(x)=0\f$
     *
     * @param[in] x0 迭代初始值
     * @param[in] eps 精度要求
     * @param[in] max_iter 最大迭代次数
     * @return 迭代结果
     */
    RMVL_W double operator()(double x0, double eps = 1e-5, std::size_t max_iter = 50) const;
};

////////////// 常微分方程（组）数值解 //////////////

//! 常微分方程
using Ode = std::function<double(double, const std::vector<double> &)>;
//! 常微分方程组
using Odes = std::vector<std::function<double(double, const std::vector<double> &)>>;

/**
 * @brief Butcher 表形式的常微分方程（组）数值求解器
 * @brief
 * - 使用 Runge-Kutta 法求解常微分方程（组），算法介绍见 @ref tutorial_modules_runge_kutta
 * @brief
 * - 该 Runge-Kutta 数值求解器为通用求解器，提供了一般的 Runge-Kutta 法求解常微分方程（组）的接口
 */
class RMVL_EXPORTS_W RungeKutta
{
    // 加权系数，`k[i][j]`: `i` 表示第 `i` 个加权系数组，`j` 表示来自第 `j` 条方程
    std::vector<std::vector<double>> _ks;

protected:
    //! 一阶常微分方程组的函数对象 \f$\dot{\pmb x}=\pmb F(t, \pmb x)\f$
    Odes _fs;
    double _t0;              //!< 初值的自变量 \f$t\f$
    std::vector<double> _x0; //!< 初值的因变量 \f$\pmb x(t)\f$

    std::vector<double> _p;              //!< Butcher 表 \f$\pmb p\f$ 向量
    std::vector<double> _lambda;         //!< Butcher 表 \f$\pmb\lambda\f$ 向量
    std::vector<std::vector<double>> _r; //!< Butcher 表 \f$R\f$ 矩阵

public:
    /**
     * @brief 创建一阶常微分方程（组）数值求解器对象，设置初值请参考 @ref init 方法
     *
     * @param[in] fs 常微分方程（组）\f$\pmb x'=\pmb F(t,\pmb x)\f$ 的函数对象 \f$\pmb F(t,\pmb x)\f$
     * @param[in] p Butcher 表 \f$\pmb p\f$ 向量
     * @param[in] lam Butcher 表 \f$\pmb\lambda\f$ 向量
     * @param[in] r Butcher 表 \f$R\f$ 矩阵
     */
    RMVL_W RungeKutta(const Odes &fs, const std::vector<double> &p, const std::vector<double> &lam, const std::vector<std::vector<double>> &r);

    /**
     * @brief 设置常微分方程（组）的初值
     *
     * @param[in] t0 初始位置的自变量 \f$t_0\f$
     * @param[in] x0 初始位置的因变量 \f$\pmb x(t_0)\f$
     */
    RMVL_W inline void init(double t0, const std::vector<double> &x0) { _t0 = t0, _x0 = x0; }

    /**
     * @brief 设置常微分方程（组）的初值
     *
     * @param[in] t0 初始位置的自变量 \f$t_0\f$
     * @param[in] x0 初始位置的因变量 \f$\pmb x(t_0)\f$
     */
    inline void init(double t0, std::vector<double> &&x0) { _t0 = t0, _x0 = std::move(x0); }

    /**
     * @brief 计算常微分方程（组）的数值解
     *
     * @param[in] h 步长
     * @param[in] n 迭代次数
     *
     * @return 从初始位置开始迭代 \f$n\f$ 次后共 \f$n+1\f$ 个数值解，自变量可通过 \f$t_0+ih\f$ 计算得到
     */
    RMVL_W std::vector<std::vector<double>> solve(double h, std::size_t n);

#if __cpp_lib_generator >= 202207L
    /**
     * @brief 常微分方程（组）数值解生成器
     *
     * @param[in] h 步长
     * @param[in] n 迭代次数
     * @return 从初始位置开始迭代计算的生成器，初值不会被 `co_yield`，共生成 \f$n\f$ 个数值解
     */
    std::generator<std::vector<double>> generate(double h, std::size_t n);
#endif
};

//! 2 阶 2 级 Runge-Kutta 求解器
class RMVL_EXPORTS_W RungeKutta2 : public RungeKutta
{
public:
    /**
     * @brief 创建 2 阶 2 级 Runge-Kutta 常微分方程（组）数值求解器对象，设置初值请参考 @ref init 方法
     *
     * @param[in] fs 常微分方程（组）\f$\pmb x'=\pmb F(t,\pmb x)\f$ 的函数对象 \f$\pmb F(t,\pmb x)\f$
     */
    RMVL_W RungeKutta2(const Odes &fs);
};

//! 3 阶 3 级 Runge-Kutta 求解器
class RMVL_EXPORTS_W RungeKutta3 : public RungeKutta
{
public:
    /**
     * @brief 创建 3 阶 3 级 Runge-Kutta 常微分方程（组）数值求解器对象，设置初值请参考 @ref init 方法
     *
     * @param[in] fs 常微分方程（组）\f$\pmb x'=\pmb F(t,\pmb x)\f$ 的函数对象 \f$\pmb F(t,\pmb x)\f$
     */
    RMVL_W RungeKutta3(const Odes &fs);
};

//! 4 阶 4 级 Runge-Kutta 求解器
class RMVL_EXPORTS_W RungeKutta4 : public RungeKutta
{
public:
    /**
     * @brief 创建 4 阶 4 级 Runge-Kutta 常微分方程（组）数值求解器对象，设置初值请参考 @ref init 方法
     *
     * @param[in] fs 常微分方程（组）\f$\pmb x'=\pmb F(t,\pmb x)\f$ 的函数对象 \f$\pmb F(t,\pmb x)\f$
     */
    RMVL_W RungeKutta4(const Odes &fs);
};

//! @} algorithm_numcal

//! @addtogroup algorithm_optimal
//! @{

//! 一元函数
using Func1d = std::function<double(double)>;
//! 一元函数组
using Func1ds = std::vector<std::function<double(double)>>;
//! 多元函数
using FuncNd = std::function<double(const std::vector<double> &)>;
//! 多元函数组
using FuncNds = std::vector<std::function<double(const std::vector<double> &)>>;

//! 梯度/导数计算模式
enum class DiffMode : uint8_t
{
    Central, //!< 中心差商
    Ridders, //!< Richardson 外推
};

//! 多维函数最优化模式
enum class FminMode : uint8_t
{
    ConjGrad, //!< 共轭梯度法
    Simplex,  //!< 单纯形法
};

//! 无约束多维函数优化选项
struct RMVL_EXPORTS_W_AG OptimalOptions
{
    RMVL_W_RW DiffMode diff_mode{}; //!< 梯度计算模式，默认为中心差商 `DiffMode::Central`
    RMVL_W_RW FminMode fmin_mode{}; //!< 多维函数最优化模式，默认为共轭梯度法 `FminMode::ConjGrad`
    RMVL_W_RW int max_iter{1000};   //!< 最大迭代次数
    RMVL_W_RW double exterior{1e3}; //!< 外罚函数系数
    RMVL_W_RW double dx{1e-2};      //!< 求解步长
    RMVL_W_RW double tol{1e-6};     //!< 误差容限
};

/**
 * @brief 计算一元函数的导数
 *
 * @param[in] func 一元函数
 * @param[in] x 指定位置的自变量
 * @param[in] mode 导数计算模式，默认为中心差商 `Diff_Central`
 * @param[in] dx 坐标的微小增量，默认为 `1e-3`
 * @return 函数在指定点的导数
 */
RMVL_EXPORTS_W double derivative(Func1d func, double x, DiffMode mode = DiffMode::Central, double dx = 1e-3);

/**
 * @brief 计算多元函数的梯度
 *
 * @param[in] func 多元函数
 * @param[in] x 指定位置的自变量
 * @param[in] mode 梯度计算模式，默认为中心差商 `Diff_Central`
 * @param[in] dx 计算偏导数时，坐标的微小增量，默认为 `1e-3`
 * @return 函数在指定点的梯度向量
 */
RMVL_EXPORTS_W std::vector<double> grad(FuncNd func, const std::vector<double> &x, DiffMode mode = DiffMode::Central, double dx = 1e-3);

/**
 * @brief 采用进退法确定搜索区间
 *
 * @param[in] func 一维函数
 * @param[in] x0 初始点
 * @param[in] delta 搜索步长
 * @return 搜索区间
 */
RMVL_EXPORTS_W std::pair<double, double> region(Func1d func, double x0, double delta = 1);

/**
 * @brief 一维函数最小值搜索
 *
 * @param[in] func 一维约束函数
 * @param[in] x1 搜索区间左端点
 * @param[in] x2 搜索区间右端点
 * @param[in] options 优化选项
 * @return `[x, fval]` 最小值点和最小值
 */
RMVL_EXPORTS_W std::pair<double, double> fminbnd(Func1d func, double x1, double x2, const OptimalOptions &options = {});

/**
 * @brief 无约束多维函数的最小值搜索 \cite ConjGrad \cite NelderMead ，可参考 @ref tutorial_modules_fminunc
 * @param[in] func 多维约束函数
 * @param[in] x0 初始点
 * @param[in] options 优化选项
 * @return `[x, fval]` 最小值点和最小值
 */
RMVL_EXPORTS_W std::pair<std::vector<double>, double> fminunc(FuncNd func, const std::vector<double> &x0, const OptimalOptions &options = {});

/**
 * @brief 有约束多维函数的最小值搜索
 *
 * @param[in] func 多维函数
 * @param[in] x0 初始点
 * @param[in] c 不等式约束 \f$f_c(x)\le0\f$
 * @param[in] ceq 等式约束 \f$f_{ceq}(x)=0\f$
 * @param[in] options options 优化选项
 * @return `[x, fval]` 最小值点和最小值
 */
RMVL_EXPORTS_W std::pair<std::vector<double>, double> fmincon(FuncNd func, const std::vector<double> &x0, FuncNds c, FuncNds ceq, const OptimalOptions &options = {});

/**
 * @brief 无约束非线性最小二乘求解
 *
 * @param[in] funcs 多维最小二乘约束函数，满足 \f[F(\pmb x_k)=\frac12\|\pmb f(\pmb x_k)\|_2^2=\frac12
 *                  \left(\texttt{funcs}[0]^2+\texttt{funcs}[1]^2+\cdots+\texttt{funcs}[n]^2\right)\f]
 * @param[in] x0 初始点
 * @param[in] options 优化选项
 * @return 最小二乘解
 */
RMVL_EXPORTS_W std::vector<double> lsqnonlin(const FuncNds &funcs, const std::vector<double> &x0, const OptimalOptions &options = {});

//! @} algorithm_optimal

} // namespace rm

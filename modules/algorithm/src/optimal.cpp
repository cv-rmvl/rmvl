/**
 * @file optimal.cpp
 * @author zhaoxi (535394140@qq.com)
 * @brief 最优化算法库
 * @version 1.0
 * @date 2024-05-02
 *
 * @copyright Copyright 2024 (c), zhaoxi
 *
 */

#include <numeric>

#ifdef HAVE_OPENCV
#include <opencv2/core.hpp>
#endif // HAVE_OPENCV

#include "rmvl/algorithm/math.hpp"
#include "rmvl/algorithm/numcal.hpp"

#include <iostream>

namespace rm
{

// 中心差商计算一元函数导数
static inline double partial(Func1d func, double x_dx, double dx)
{
    x_dx += dx;
    double f1 = func(x_dx);
    x_dx -= 2 * dx;
    double f2 = func(x_dx);
    x_dx += dx;
    return (f1 - f2) / (2 * dx);
}

// 中心差商计算多元函数偏导数
static inline double partial(FuncNd func, std::vector<double> &x_dx, std::size_t idx, double dx)
{
    x_dx[idx] += dx;
    double f1 = func(x_dx);
    x_dx[idx] -= 2 * dx;
    double f2 = func(x_dx);
    x_dx[idx] += dx;
    return (f1 - f2) / (2 * dx);
}

double derivative(Func1d func, double x, DiffMode mode, double dx)
{
    double x_dx{x};
    if (mode == DiffMode::Ridders)
    {
        double T00{partial(func, x_dx, 2 * dx)};
        double T10{partial(func, x_dx, dx)};
        double T20{partial(func, x_dx, dx / 2)};
        double T11{(4. * T10 - T00) / 3.};
        double T21{(4. * T20 - T10) / 3.};
        return (16. * T21 - T11) / 15.;
    }
    else
        return partial(func, x_dx, dx);
}

/**
 * @brief 计算多元函数的梯度（无返回值）
 *
 * @param[in] func 多元函数
 * @param[in] x 指定位置的自变量
 * @param[out] xgrad 函数在指定点的梯度向量
 * @param[in] mode 梯度计算模式
 * @param[in] dx 计算偏导数时的步长
 * @return 函数在指定点的梯度向量
 */
static void calcGrad(FuncNd func, const std::vector<double> &x, std::vector<double> &xgrad, DiffMode mode, double dx)
{
    auto x_dx{x};
    if (mode == DiffMode::Ridders)
        for (std::size_t i = 0; i < x_dx.size(); ++i)
        {
            double T00{partial(func, x_dx, i, 2 * dx)};
            double T10{partial(func, x_dx, i, dx)};
            double T20{partial(func, x_dx, i, dx / 2)};
            double T11{(4. * T10 - T00) / 3.};
            double T21{(4. * T20 - T10) / 3.};
            xgrad[i] = (16. * T21 - T11) / 15.;
        }
    else
        for (std::size_t i = 0; i < x_dx.size(); ++i)
            xgrad[i] = partial(func, x_dx, i, dx);
}

std::vector<double> grad(FuncNd func, const std::vector<double> &x, DiffMode mode, double dx)
{
    std::vector<double> ret(x.size());
    calcGrad(func, x, ret, mode, dx);
    return ret;
}

/**
 * @brief 计算向量的二范数
 *
 * @param[in] x 向量
 * @return 二范数
 */
static inline double normL2(const std::vector<double> &x)
{
    double retval{};
    for (auto &&v : x)
        retval += v * v;
    return std::sqrt(retval);
}

std::pair<double, double> region(Func1d func, double x0, double delta)
{
    double f1{func(x0)}, f2{func(x0 + delta)};
    if (f1 > f2)
    {
        f1 = f2;
        f2 = func(x0 + 3 * delta);
        return f1 < f2 ? std::make_pair(x0, x0 + 3 * delta) : (f1 == f2 ? std::make_pair(x0 + delta, x0 + 3 * delta) : region(func, x0 + delta, 2 * delta));
    }
    else if (f1 < f2)
    {
        f2 = f1;
        f1 = func(x0 - delta);
        return f1 > f2 ? std::make_pair(x0 - delta, x0 + delta) : (f1 == f2 ? std::make_pair(x0 - delta, x0) : region(func, x0 - 3 * delta, 2 * delta));
    }
    else
        return {x0, x0 + delta};
}

std::pair<double, double> fminbnd(Func1d func, double x1, double x2, const OptimalOptions &options)
{
    constexpr double phi = 0.618033988749895;
    double a1{x1 + (1.0 - phi) * (x2 - x1)}, a2{x1 + phi * (x2 - x1)};
    double f1{func(a1)}, f2{func(a2)};
    for (int i = 0; i < options.max_iter; ++i)
    {
        if (std::abs(x1 - x2) < options.tol)
            break;
        if (f1 < f2) // 替换右端点
        {
            x2 = a2, a2 = a1, f2 = f1;
            a1 = x1 + (1.0 - phi) * (x2 - x1);
            f1 = func(a1);
        }
        else // 替换左端点
        {
            x1 = a1, a1 = a2, f1 = f2;
            a2 = x1 + phi * (x2 - x1);
            f2 = func(a2);
        }
    }
    return {0.5 * (x1 + x2), func(0.5 * (x1 + x2))};
}

// 共轭梯度法
static double fminunc_cg(FuncNd func, std::vector<double> &xk, const OptimalOptions &options)
{
    std::vector<double> s = -xk;
    std::vector<double> xk_grad = grad(func, xk, options.diff_mode, options.dx), xk2_grad(xk_grad.size());
    // 判断是否收敛
    double nbl_xk = normL2(xk_grad);
    if (nbl_xk < options.tol)
        return func(xk);
    // 一维搜索函数
    auto func_alpha = [&](double alpha) {
        auto xk2 = xk + alpha * s;
        return func(xk2);
    };
    double retfval{};
    for (int i = 0; i < options.max_iter; ++i)
    {
        // 一维搜索 alpha
        auto [a, b] = region(func_alpha, 1);
        auto [alpha, fval] = fminbnd(func_alpha, a, b, options);
        // 更新 xk，并计算对应的梯度
        for (std::size_t j = 0; j < xk.size(); ++j)
            xk[j] += alpha * s[j];
        retfval = fval;
        calcGrad(func, xk, xk2_grad, options.diff_mode, options.dx);
        xk2_grad = grad(func, xk, options.diff_mode, options.dx);
        auto nbl_xk2 = normL2(xk2_grad);
        if (nbl_xk2 < options.tol)
            break;
        // 计算 beta
        double nbl_xk_xk2 = std::inner_product(xk_grad.begin(), xk_grad.end(), xk2_grad.begin(), 0.0);
        double beta = std::max((nbl_xk2 * nbl_xk2 - nbl_xk_xk2) / (nbl_xk * nbl_xk), 0.0);
        // 更新搜索方向
        for (std::size_t j = 0; j < s.size(); ++j)
            s[j] = -xk2_grad[j] + beta * s[j];
        xk_grad = xk2_grad;
    }
    return retfval;
}

// 单纯形法
static double fminunc_splx(FuncNd func, std::vector<double> &xk, const OptimalOptions &options)
{
    const std::size_t dim = xk.size(); // 维度
    const std::size_t N = dim + 1;     // 单纯形点的个数
    std::vector<std::pair<std::vector<double>, double>> splx(N);
    for (auto &p : splx)
        p = {xk, 0};
    for (std::size_t i = 0; i < dim; ++i)
        splx[i + 1].first[i] += 100 * options.dx;
    for (std::size_t i = 0; i < N; ++i)
        splx[i].second = func(splx[i].first);
    // 单纯形迭代
    for (int i = 0; i < options.max_iter; ++i)
    {
        std::sort(splx.begin(), splx.end(), [](const auto &a, const auto &b) { return a.second < b.second; });
        // 求出除最大值点外的所有点的中心
        std::vector<double> xc(dim);
        std::for_each(splx.begin(), splx.end() - 1, [&](const auto &vp) { xc += vp.first; });
        xc /= static_cast<double>(N - 1);
        // 反射 xr = xc + alpha * (xc - xh)，其中 alpha = 1
        std::vector<double> xr(dim);
        for (std::size_t j = 0; j < dim; ++j)
            xr[j] = 2 * xc[j] - splx.back().first[j];

        double fxr = func(xr);
        // f(xn-1) <= f(xr)，反射点函数值大于最差点，则重新计算反射点（反压缩）
        if (splx.back().second <= fxr)
        {
            // 压缩 xs = xc - beta * (xc - xh)，其中 beta = 0.5
            xr = xc + splx.back().first;
            xr /= 2.0;
            fxr = func(xr);
        }

        // f(xr) < f(x0)，反射点函数值小于最优点
        if (fxr < splx[0].second)
        {
            // 扩展 xe = xc + gamma * (xc - xh)，其中 gamma = 2
            std::vector<double> xe(dim);
            for (std::size_t j = 0; j < dim; ++j)
                xe[j] = 3 * xc[j] - 2 * splx.back().first[j];
            double fxe = func(xe);
            splx.back() = fxe < fxr ? std::make_pair(xe, fxe) : std::make_pair(xr, fxr);
        }
        // f(x0) <= f(xr) < f(xn-2)，反射点函数值大于最优点，小于次差点
        else if (splx[0].second <= fxr && fxr < splx[N - 2].second)
            splx.back() = {xr, fxr};
        // f(xn-2) <= f(xr) < f(xn)，反射点函数值大于次差点，小于最差点
        else
        {
            // 压缩 xs = xc + beta * (xc - xh)，其中 beta = 0.5
            std::vector<double> xs(dim);
            for (std::size_t j = 0; j < dim; ++j)
                xs[j] = 1.5 * xc[j] - 0.5 * splx.back().first[j];
            double fxs = func(xs);
            if (fxs < splx.back().second)
                splx.back() = {xs, fxs};
            else
            {
                for (std::size_t i = 1; i < N; ++i)
                {
                    for (std::size_t j = 0; j < dim; ++j)
                        splx[i].first[j] = 0.5 * (splx[i].first[j] + splx[0].first[j]);
                    splx[i].second = func(splx[i].first);
                }
            }
        }
        // 判断是否收敛
        if (std::abs(splx.back().second - splx.front().second) < options.tol)
            break;
    }
    xk = std::move(splx[0].first);
    return splx[0].second;
}

std::pair<std::vector<double>, double> fminunc(FuncNd func, const std::vector<double> &x0, const OptimalOptions &options)
{
    if (x0.empty())
        RMVL_Error(RMVL_StsBadArg, "x0 is empty");
    std::vector<double> xk{x0};
    double fval{};
    switch (options.fmin_mode)
    {
    case FminMode::Simplex:
        fval = fminunc_splx(func, xk, options);
        break;
    default:
        fval = fminunc_cg(func, xk, options);
        break;
    }
    return {xk, fval};
}

std::pair<std::vector<double>, double> fmincon(FuncNd func, const std::vector<double> &x0, FuncNds c, FuncNds ceq, const OptimalOptions &options)
{
    if (x0.empty())
        RMVL_Error(RMVL_StsBadArg, "x0 is empty");
    if (c.empty() && ceq.empty())
        return fminunc(func, x0, options);
    // 外罚函数
    const double M{options.exterior};
    FuncNd farg = [&](const std::vector<double> &xk) -> double {
        double fval = func(xk), ceqval{}, cval{};
        for (const auto &v : ceq)
            ceqval += v(xk) * v(xk);
        for (const auto &v : c)
            cval += std::pow(std::max(v(xk), 0.0), 2);
        return fval + M * (ceqval + cval);
    };

    return fminunc(farg, x0, options);
}

#ifdef HAVE_OPENCV

/**
 * @brief 计算某点处的雅可比矩阵
 *
 * @param[in] funcs 多元函数集合
 * @param[in] xk 指定位置的自变量
 * @param[in] options 优化选项
 * @param[out] jac 雅可比矩阵
 */
static inline void calcJacobi(const FuncNds &funcs, const std::vector<double> &xk, const OptimalOptions &options, cv::Mat &jac)
{
    for (std::size_t i = 0; i < funcs.size(); ++i)
    {
        auto xgrad = grad(funcs[i], xk, options.diff_mode, options.dx);
        for (std::size_t j = 0; j < xgrad.size(); ++j)
            jac.at<double>(i, j) = xgrad[j];
    }
}

/**
 * @brief 计算函数值
 *
 * @param[in] funcs 多元函数集合
 * @param[in] xk 指定位置的自变量
 * @param[out] phi 函数值
 */
static inline void calcFs(const FuncNds &funcs, const std::vector<double> &xk, std::vector<double> &phi)
{
    for (std::size_t i = 0; i < funcs.size(); ++i)
        phi[i] = funcs[i](xk);
}

std::vector<double> lsqnonlin(FuncNds funcs, const std::vector<double> &x0, const OptimalOptions &options)
{
    if (x0.empty())
        RMVL_Error(RMVL_StsBadArg, "x0 is empty");
    std::vector<double> xk(x0);
    cv::Mat J(funcs.size(), x0.size(), CV_64FC1); // Jacobian 矩阵 (M×N)
    std::vector<double> phi(funcs.size());        // 函数值 (M×1)
    for (int idx = 0; idx < options.max_iter; ++idx)
    {
        // 计算函数值
        calcFs(funcs, xk, phi);
        // 计算搜索方向
        calcJacobi(funcs, xk, options, J);
        auto Jt = J.t();
        cv::Mat fvals(phi, false);
        std::vector<double> s = cv::Mat{-(Jt * J).inv() * Jt * fvals};
        if (normL2(s) < options.tol)
            break;
        // 一维搜索 alpha
        auto func_alpha = [&](double alpha) {
            auto xk2 = xk + alpha * s;
            std::vector<double> fvals2(funcs.size());
            for (std::size_t i = 0; i < funcs.size(); ++i)
                fvals2[i] = funcs[i](xk2);
            return normL2(fvals2);
        };
        auto [a, b] = region(func_alpha, 1);
        double alpha = fminbnd(func_alpha, a, b, options).first;
        // 更新 xk
        for (std::size_t i = 0; i < xk.size(); ++i)
            xk[i] += alpha * s[i];
    }
    return xk;
}

#endif // HAVE_OPENCV

} // namespace rm

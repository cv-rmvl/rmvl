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
#include <unsupported/Eigen/NonLinearOptimization>
#else
#include <algorithm>
#endif

#include "rmvl/algorithm/numcal.hpp"
#include "rmvl/core/util.hpp"

#include "rmvlpara/algorithm.hpp"

namespace rm {

// 中心差商计算一元函数导数
static inline double partial(Func1d func, double x_dx, double dx) {
    x_dx += dx;
    double f1 = func(x_dx);
    x_dx -= 2 * dx;
    double f2 = func(x_dx);
    x_dx += dx;
    return (f1 - f2) / (2 * dx);
}

// 中心差商计算多元函数偏导数
static inline double partial(FuncNd func, std::valarray<double> &x_dx, std::size_t idx, double dx) {
    x_dx[idx] += dx;
    double f1 = func(x_dx);
    x_dx[idx] -= 2 * dx;
    double f2 = func(x_dx);
    x_dx[idx] += dx;
    return (f1 - f2) / (2 * dx);
}

double derivative(Func1d func, double x, DiffMode mode, double dx) {
    double x_dx{x};
    if (mode == DiffMode::Ridders) {
        double T00{partial(func, x_dx, 2 * dx)};
        double T10{partial(func, x_dx, dx)};
        double T20{partial(func, x_dx, dx / 2)};
        double T11{(4. * T10 - T00) / 3.};
        double T21{(4. * T20 - T10) / 3.};
        return (16. * T21 - T11) / 15.;
    } else
        return partial(func, x_dx, dx);
}

std::valarray<double> grad(FuncNd func, const std::valarray<double> &x, DiffMode mode, double dx) {
    std::valarray<double> ret(x.size());
    auto x_dx{x};
    if (mode == DiffMode::Ridders)
        for (std::size_t i = 0; i < x_dx.size(); ++i) {
            double T00{partial(func, x_dx, i, 2 * dx)};
            double T10{partial(func, x_dx, i, dx)};
            double T20{partial(func, x_dx, i, dx / 2)};
            double T11{(4. * T10 - T00) / 3.};
            double T21{(4. * T20 - T10) / 3.};
            ret[i] = (16. * T21 - T11) / 15.;
        }
    else
        for (std::size_t i = 0; i < x_dx.size(); ++i)
            ret[i] = partial(func, x_dx, i, dx);
    return ret;
}

/**
 * @brief 计算向量的二范数
 *
 * @param[in] x 向量
 * @return 二范数
 */
static inline double normL2(const std::valarray<double> &x) {
    return std::sqrt(std::inner_product(std::begin(x), std::end(x), std::begin(x), 0.0));
}

std::pair<double, double> region(Func1d func, double x0, double delta) {
    double f1{func(x0)}, f2{func(x0 + delta)};
    if (f1 > f2) {
        f1 = f2;
        f2 = func(x0 + 3 * delta);
        return f1 < f2 ? std::make_pair(x0, x0 + 3 * delta) : (f1 == f2 ? std::make_pair(x0 + delta, x0 + 3 * delta) : region(func, x0 + delta, 2 * delta));
    } else if (f1 < f2) {
        f2 = f1;
        f1 = func(x0 - delta);
        return f1 > f2 ? std::make_pair(x0 - delta, x0 + delta) : (f1 == f2 ? std::make_pair(x0 - delta, x0) : region(func, x0 - 3 * delta, 2 * delta));
    } else
        return {x0, x0 + delta};
}

std::pair<double, double> fminbnd(Func1d func, double x1, double x2, const OptimalOptions &options) {
    constexpr double phi = 0.618033988749895;
    double a1{x1 + (1.0 - phi) * (x2 - x1)}, a2{x1 + phi * (x2 - x1)};
    double f1{func(a1)}, f2{func(a2)};
    for (int i = 0; i < options.max_iter; ++i) {
        if (std::abs(x1 - x2) < options.tol)
            break;
        if (f1 < f2) { // 替换右端点
            x2 = a2, a2 = a1, f2 = f1;
            a1 = x1 + (1.0 - phi) * (x2 - x1);
            f1 = func(a1);
        } else { // 替换左端点
            x1 = a1, a1 = a2, f1 = f2;
            a2 = x1 + phi * (x2 - x1);
            f2 = func(a2);
        }
    }
    return {0.5 * (x1 + x2), func(0.5 * (x1 + x2))};
}

// 共轭梯度法
static double fminunc_cg(FuncNd func, std::valarray<double> &xk, const OptimalOptions &options) {
    std::valarray<double> s = -xk;
    std::valarray<double> xk_grad = grad(func, xk, options.diff_mode, options.dx), xk2_grad(xk_grad.size());
    // 判断是否收敛
    double nbl_xk = normL2(xk_grad);
    if (nbl_xk < options.tol)
        return func(xk);
    // 一维搜索函数
    auto func_alpha = [&](double alpha) {
        std::valarray<double> xk2 = xk + alpha * s;
        return func(xk2);
    };
    double retfval{};
    for (int i = 0; i < options.max_iter; ++i) {
        // 一维搜索 alpha
        auto [a, b] = region(func_alpha, 1);
        auto [alpha, fval] = fminbnd(func_alpha, a, b, options);
        // 更新 xk，并计算对应的梯度
        for (std::size_t j = 0; j < xk.size(); ++j)
            xk[j] += alpha * s[j];
        retfval = fval;
        xk2_grad = grad(func, xk, options.diff_mode, options.dx);
        auto nbl_xk2 = normL2(xk2_grad);
        if (nbl_xk2 < options.tol)
            break;
        // 计算 beta
        double nbl_xk_xk2 = std::inner_product(std::begin(xk_grad), std::end(xk_grad), std::begin(xk2_grad), 0.0);
        double beta = std::max((nbl_xk2 * nbl_xk2 - nbl_xk_xk2) / (nbl_xk * nbl_xk), 0.0);
        // 更新搜索方向
        for (std::size_t j = 0; j < s.size(); ++j)
            s[j] = -xk2_grad[j] + beta * s[j];
        xk_grad = xk2_grad;
    }
    return retfval;
}

// 单纯形法
static double fminunc_splx(FuncNd func, std::valarray<double> &xk, const OptimalOptions &options) {
    const std::size_t dim = xk.size(); // 维度
    const std::size_t N = dim + 1;     // 单纯形点的个数
    std::vector<std::pair<std::valarray<double>, double>> splx(N);
    for (auto &p : splx)
        p = {xk, 0};
    for (std::size_t i = 0; i < dim; ++i)
        splx[i + 1].first[i] += 100 * options.dx;
    for (std::size_t i = 0; i < N; ++i)
        splx[i].second = func(splx[i].first);
    // 单纯形迭代
    for (int i = 0; i < options.max_iter; ++i) {
        std::sort(splx.begin(), splx.end(), [](const auto &a, const auto &b) { return a.second < b.second; });
        // 求出除最大值点外的所有点的中心
        std::valarray<double> xc(dim);
        std::for_each(splx.begin(), splx.end() - 1, [&](const auto &vp) { xc += vp.first; });
        xc /= static_cast<double>(N - 1);
        // 反射 xr = xc + alpha * (xc - xh)，其中 alpha = 1
        std::valarray<double> xr = 2.0 * xc - splx.back().first;

        double fxr = func(xr);
        // f(xn-1) <= f(xr)，反射点函数值大于最差点，则重新计算反射点（反压缩）
        if (splx.back().second <= fxr) {
            // 压缩 xs = xc - beta * (xc - xh)，其中 beta = 0.5
            xr = xc + splx.back().first;
            xr /= 2.0;
            fxr = func(xr);
        }

        // f(xr) < f(x0)，反射点函数值小于最优点
        if (fxr < splx[0].second) {
            // 扩展 xe = xc + gamma * (xc - xh)，其中 gamma = 2
            std::valarray<double> xe = 3.0 * xc - 2.0 * splx.back().first;
            double fxe = func(xe);
            splx.back() = fxe < fxr ? std::make_pair(std::move(xe), fxe) : std::make_pair(std::move(xr), fxr);
        }
        // f(x0) <= f(xr) < f(xn-2)，反射点函数值大于最优点，小于次差点
        else if (splx[0].second <= fxr && fxr < splx[N - 2].second)
            splx.back() = {std::move(xr), fxr};
        // f(xn-2) <= f(xr) < f(xn)，反射点函数值大于次差点，小于最差点
        else {
            // 压缩 xs = xc + beta * (xc - xh)，其中 beta = 0.5
            std::valarray<double> xs = 1.5 * xc - 0.5 * splx.back().first;
            double fxs = func(xs);
            if (fxs < splx.back().second)
                splx.back() = {std::move(xs), fxs};
            else {
                for (std::size_t i = 1; i < N; ++i) {
                    splx[i].first = 0.5 * (splx[i].first + splx[0].first);
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

std::pair<std::valarray<double>, double> fminunc(FuncNd func, const std::valarray<double> &x0, const OptimalOptions &options) {
    RMVL_DbgAssert(x0.size() > 0);
    std::valarray<double> xk{x0};
    double fval{};
    switch (options.fmin_mode) {
    case FminMode::Simplex:
        fval = fminunc_splx(func, xk, options);
        break;
    default:
        fval = fminunc_cg(func, xk, options);
        break;
    }
    return {xk, fval};
}

#ifdef HAVE_OPENCV

//! 计算多元函数组的雅可比矩阵
static cv::Mat jacobian(FuncNds func, const std::valarray<double> &x, DiffMode, double dx) {
    const std::size_t n = x.size();     // 自变量个数
    std::valarray<double> fx = func(x); // 函数值
    const std::size_t m = fx.size();    // 函数值个数
    cv::Mat retval(m, n, CV_64FC1);     // 雅可比矩阵

    std::valarray<double> x_dx = x;
    for (std::size_t i = 0; i < n; ++i) {
        x_dx[i] += dx;
        auto fx_plus = func(x_dx);
        x_dx[i] -= 2 * dx;
        auto fx_minus = func(x_dx);
        for (std::size_t j = 0; j < m; ++j)
            retval.at<double>(j, i) = (fx_plus[j] - fx_minus[j]) / (2 * dx);
        x_dx[i] += dx;
    }

    return retval;
}

#endif // HAVE_OPENCV

static std::pair<std::valarray<double>, double> fmincon_lagrange(FuncNd func, const std::valarray<double> &x0, FuncNds ceq, const OptimalOptions &options) {
#ifdef HAVE_OPENCV
    const auto n_x = x0.size();
    const auto n_lambda = ceq(x0).size();
    const auto n_total = n_x + n_lambda;

    // 初值
    std::valarray<double> z0(0.0, n_total);
    for (size_t i = 0; i < n_x; ++i)
        z0[i] = x0[i];

    FuncNds lagrange_eqs = [&](const std::valarray<double> &z) -> std::valarray<double> {
        std::valarray<double> x(n_x);
        std::valarray<double> lambda(n_lambda);
        for (std::size_t i = 0; i < n_x; ++i)
            x[i] = z[i];
        for (std::size_t i = 0; i < n_lambda; ++i)
            lambda[i] = z[n_x + i];

        // ∇_x L = ∇f(x) + J_ceq(x)ᵀ * λ
        auto grad_f = grad(func, x, options.diff_mode, options.dx);
        auto jac_ceq_T = jacobian(ceq, x, options.diff_mode, options.dx).t();
        cv::Mat lambda_mat(n_lambda, 1, CV_64FC1, &lambda[0]);
        cv::Mat grad_L_mat = jac_ceq_T * lambda_mat;
        for (std::size_t i = 0; i < n_x; ++i)
            grad_f[i] += grad_L_mat.at<double>(static_cast<int>(i));

        // ∇_λ L = ceq(x)
        auto ceq_val = ceq(x);

        std::valarray<double> f_vals(n_total);
        for (std::size_t i = 0; i < n_x; ++i)
            f_vals[i] = grad_f[i];
        for (std::size_t i = 0; i < n_lambda; ++i)
            f_vals[n_x + i] = ceq_val[i];

        return f_vals;
    };

    auto z_opt = lsqnonlin(lagrange_eqs, z0, options);

    std::valarray<double> x_opt(n_x);
    for (std::size_t i = 0; i < n_x; ++i)
        x_opt[i] = z_opt[i];

    return {x_opt, func(x_opt)};
#else
    RMVL_Error(RMVL_StsBadFunc, "Lagrange multiplier method requires OpenCV and Eigen3, please recompile RMVL.");
    return {};
#endif
}

static std::pair<std::valarray<double>, double> fmincon_exterior(FuncNd func, const std::valarray<double> &x0, FuncNds c, FuncNds ceq, const OptimalOptions &options) {
    const double M{rm::para::algorithm_param.EXTERIOR};
    FuncNd farg = [&](const std::valarray<double> &xk) -> double {
        double fval = func(xk), ceqval{}, cval{};
        if (ceq != nullptr) {
            auto ceq_fval = ceq(xk);
            ceqval = std::inner_product(std::begin(ceq_fval), std::end(ceq_fval), std::begin(ceq_fval), 0.0);
        }
        if (c != nullptr) {
            auto c_fval = c(xk);
            std::for_each(std::begin(c_fval), std::end(c_fval), [&](double v) {
                cval += std::pow(std::max(v, 0.0), 2);
            });
        }
        return fval + M * (ceqval + cval);
    };

    return fminunc(farg, x0, options);
}

std::pair<std::valarray<double>, double> fmincon(FuncNd func, const std::valarray<double> &x0, FuncNds c, FuncNds ceq, const OptimalOptions &options) {
    RMVL_DbgAssert(x0.size() > 0);
    RMVL_DbgAssert(c.size() > 0 || ceq.size() > 0);

    if (options.cons_mode == ConsMode::Lagrange)
        return fmincon_lagrange(func, x0, ceq, options);
    else
        return fmincon_exterior(func, x0, c, ceq, options);
}

#ifdef HAVE_OPENCV

// 获取鲁棒加权
template <typename RobustCallable, typename Enable = std::enable_if_t<std::is_convertible_v<RobustCallable, std::function<double(double)>>>>
static cv::Mat robustW(const cv::Mat &fvals, RobustCallable fn) {
    // 使用中位绝对偏差（MAD）计算 sigma
    std::vector<double> tmpfs = fvals;
    const std::size_t N = tmpfs.size();
    std::sort(tmpfs.begin(), tmpfs.end());
    double median = (N % 2) ? tmpfs[N / 2] : (tmpfs[N / 2 - 1] + tmpfs[N / 2]) / 2;
    std::for_each(tmpfs.begin(), tmpfs.end(), [&](double &v) { v = std::abs(v - median); });
    std::sort(tmpfs.begin(), tmpfs.end());
    double mad = (N % 2) ? tmpfs[N / 2] : (tmpfs[N / 2 - 1] + tmpfs[N / 2]) / 2;
    double sigma{1.4826 * mad};
    // 计算鲁棒加权
    std::vector<double> w(fvals.rows);
    for (int i = 0; i < fvals.rows; ++i)
        w[i] = fn(fvals.at<double>(i) / sigma);
    return cv::Mat::diag(cv::Mat(w));
}

// 获取包含 Robust 核函数的 `JtJ` 和 `Jtf` 计算可调用对象
static std::function<cv::Mat(const cv::Mat &)> robustSelect(RobustMode rb) {
    constexpr double HUBER_K{1.345};
    constexpr double TUKEY_K{4.685};
    constexpr double CAUCHY_K{2.3849};

    switch (rb) {
    case RobustMode::Huber:
        return [](const cv::Mat &fs) {
            return robustW(fs, [](double val) { return val < HUBER_K ? 1 : HUBER_K / val; });
        };
    case RobustMode::Tukey:
        return [](const cv::Mat &fs) {
            return robustW(fs, [](double val) { return val < TUKEY_K ? std::pow(1 - val * val / (TUKEY_K * TUKEY_K), 2) : 0; });
        };
    case RobustMode::GM:
        return [](const cv::Mat &fs) {
            return robustW(fs, [](double val) { return 1 / std::pow(1 + val * val, 2); });
        };
    case RobustMode::Cauchy:
        return [](const cv::Mat &fs) {
            return robustW(fs, [](double val) { return 1 / (1 + val * val / (CAUCHY_K * CAUCHY_K)); });
        };
    default:
        return [](const cv::Mat &fs) { return cv::Mat::eye(fs.rows, fs.rows, CV_64F); };
    }
}

// Gauss-Newton 法
static std::valarray<double> lsqnonlin_gn(const FuncNds &func, const std::valarray<double> &x0, RobustMode rb, const OptimalOptions &options) {
    RMVL_DbgAssert(x0.size() > 0);
    std::valarray<double> xk(x0);

    auto fnW = robustSelect(rb);
    for (int idx = 0; idx < options.max_iter; ++idx) {
        // 计算函数值和搜索方向
        auto phi = func(xk);
        if (normL2(phi) < options.tol)
            break;
        auto J = jacobian(func, xk, options.diff_mode, options.dx);
        auto Jt = J.t();
        cv::Mat fvals(phi.size(), 1, CV_64FC1, &phi[0]);
        cv::Mat s;
        // JᵀWJs = JᵀWf
        cv::Mat W = fnW(fvals);
        cv::solve(Jt * W * J, Jt * W * fvals, s, cv::DECOMP_CHOLESKY);
        // 更新 xk
        for (std::size_t i = 0; i < xk.size(); ++i)
            xk[i] -= s.at<double>(static_cast<int>(i));
    }
    return xk;
}

// 改进的 Gauss-Newton 法
static std::valarray<double> lsqnonlin_sgn(const FuncNds &func, const std::valarray<double> &x0, RobustMode rb, const OptimalOptions &options) {
    RMVL_DbgAssert(x0.size() > 0);
    std::valarray<double> xk(x0);

    auto fnW = robustSelect(rb);
    for (int idx = 0; idx < options.max_iter; ++idx) {
        // 计算函数值和搜索方向
        auto phi = func(xk);
        if (normL2(phi) < options.tol)
            break;
        auto J = jacobian(func, xk, options.diff_mode, options.dx);
        auto Jt = J.t();
        cv::Mat fvals(phi.size(), 1, CV_64FC1, &phi[0]);
        cv::Mat s;
        // JᵀWJs = JᵀWf
        cv::Mat W = fnW(fvals);
        cv::solve(Jt * W * J, Jt * W * fvals, s, cv::DECOMP_CHOLESKY);
        // 一维搜索 alpha
        auto func_alpha = [&](double alpha) {
            auto xk2 = xk;
            for (std::size_t i = 0; i < xk.size(); ++i)
                xk2[i] -= alpha * s.at<double>(static_cast<int>(i), 0);
            auto fvals2 = func(xk2);
            return normL2(fvals2);
        };
        auto [a, b] = region(func_alpha, 1);
        double alpha = fminbnd(func_alpha, a, b, options).first;
        // 更新 xk
        for (std::size_t i = 0; i < xk.size(); ++i)
            xk[i] -= alpha * s.at<double>(static_cast<int>(i));
    }
    return xk;
}

class LMFunctor {
public:
    LMFunctor(const FuncNds &func, const std::valarray<double> &x0, DiffMode diff_mode, double dx)
        : _func(func), M(func(x0).size()), N(x0.size()), _diff_mode(diff_mode), _dx(dx) {}

    int operator()(const Eigen::VectorXd &x, Eigen::VectorXd &fvec) const {
        auto fval = _func(std::valarray<double>(x.data(), x.size()));
        for (std::size_t i = 0; i < fval.size(); i++)
            fvec(i) = fval[i];
        return 0;
    }

    int df(const Eigen::VectorXd &x, Eigen::MatrixXd &fjac) const {
        std::valarray<double> xk(x.data(), x.size());
        auto J = jacobian(_func, xk, _diff_mode, _dx);
        for (int i = 0; i < J.rows; ++i)
            for (int j = 0; j < J.cols; ++j)
                fjac(i, j) = J.at<double>(i, j);
        return 0;
    }

    int inputs() const { return N; }
    int values() const { return M; }

private:
    FuncNds _func{};
    int M{};
    int N{};
    DiffMode _diff_mode{};
    double _dx{};
};

// Levenberg-Marquardt 法
static std::valarray<double> lsqnonlin_lm(const FuncNds &func, const std::valarray<double> &x0, RobustMode, const OptimalOptions &options) {
    LMFunctor functor(func, x0, options.diff_mode, options.dx);
    Eigen::LevenbergMarquardt<LMFunctor> lm(functor);
    lm.parameters.maxfev = options.max_iter;
    lm.parameters.xtol = options.tol;
    lm.parameters.ftol = options.tol;
    Eigen::VectorXd res = Eigen::Map<const Eigen::VectorXd>(&x0[0], x0.size());
    lm.minimize(res);
    return std::valarray<double>(res.data(), res.size());
}

std::valarray<double> lsqnonlinRKF(const FuncNds &func, const std::valarray<double> &x0, RobustMode rb, const OptimalOptions &options) {
    RMVL_DbgAssert(x0.size() > 0);
    switch (options.lsq_mode) {
    case LsqMode::LM:
        return lsqnonlin_lm(func, x0, rb, options);
    case LsqMode::GN:
        return lsqnonlin_gn(func, x0, rb, options);
    default: // LsqMode::SGN
        return lsqnonlin_sgn(func, x0, rb, options);
    };
}

#else

std::valarray<double> lsqnonlinRKF(const FuncNds &, const std::valarray<double> &, RobustMode, const OptimalOptions &) {
#if _WIN32
    RMVL_Error(RMVL_StsBadFunc, "this function must be used with opencv_worldxxx.dll, please recompile "
                                "RMVL by setting \"WITH_OPENCV=ON\" and \"WITH_EIGEN3=ON\" in CMake");
#else
    RMVL_Error(RMVL_StsBadFunc, "this function must be used with libopencv_core.so, please recompile "
                                "RMVL by setting \"WITH_OPENCV=ON\" and \"WITH_EIGEN3=ON\" in CMake");
#endif
    return {};
}

#endif // HAVE_OPENCV

std::valarray<double> lsqnonlin(const FuncNds &func, const std::valarray<double> &x0, const OptimalOptions &options) {
    return lsqnonlinRKF(func, x0, RobustMode::L2, options);
}

} // namespace rm

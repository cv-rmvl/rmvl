/**
 * @file kalman.hpp
 * @author RoboMaster Vision Community
 * @brief 包含轻量级 `cv::Matx` 的卡尔曼滤波模块
 * @version 2.0
 * @date 2022-10-05
 *
 * @copyright Copyright 2022 (c), RoboMaster Vision Community
 *
 */

#pragma once

#ifdef HAVE_OPENCV

#include <cmath>
#include <functional>
#include <limits>
#include <type_traits>

#include <opencv2/core.hpp>

//! @addtogroup algorithm_cal
//! @{
//! @defgroup algorithm_kalman 卡尔曼滤波模块
//! @{
//! @brief 使用 `cv::Matx` 改写的轻量级卡尔曼滤波和扩展卡尔曼滤波模块
//! @brief
//! - 考虑到 OpenCV 中提供的 `cv::KalmanFilter` 是基于 `cv::Mat` 实现的，`cv::Mat`
//!   的内存操作在运行时是在堆上打开的，因此会消耗大量的时间，本模块使用 `cv::Matx`
//!   来实现 KF 和 EKF 的功能，以达到轻量化的目的。
//! @brief
//! - 相关知识点可参考说明文档 @ref tutorial_modules_kalman 以及 @ref tutorial_modules_ekf
//! @} algorithm_kalman
//! @} algorithm_cal

namespace rm {

//! @addtogroup algorithm_kalman
//! @{

/**
 * @brief 卡尔曼滤波静态数据
 *
 * @tparam FloatTp 数据类型
 * @tparam StateDim 状态量个数
 * @tparam MeasureDim 观测量个数
 */
template <typename FloatTp, int StateDim, int MeasureDim>
class KalmanFilterStaticDatas {
    static_assert(std::is_floating_point_v<FloatTp>, "\"FloatTp\" must be floating point value.");

public:
    //! 构造新的卡尔曼滤波静态数据
    KalmanFilterStaticDatas() : Q(Q.eye()), R(R.eye()), P(P.eye()), I(I.eye()) {}

    /**
     * @brief 初始化状态以及对应的误差协方差矩阵（常数对角矩阵）
     *
     * @param[in] x0 初始化的状态向量
     * @param[in] error 状态误差系数
     */
    void init(const cv::Matx<FloatTp, StateDim, 1> &x0, FloatTp error) {
        this->x_ = this->x = x0;
        this->P_ = this->P = this->P.eye() * error;
    }

    /**
     * @brief 初始化状态以及对应的误差协方差矩阵（对角矩阵）
     *
     * @param[in] x0 初始化的状态向量
     * @param[in] error 状态误差矩阵的对角线元素
     */
    void init(const cv::Matx<FloatTp, StateDim, 1> &x0, const cv::Matx<FloatTp, StateDim, 1> &error) {
        this->x_ = this->x = x0;
        this->P_ = this->P = this->P.diag(error);
    }

    /**
     * @brief 设置测量噪声协方差矩阵 \f$R\f$
     *
     * @param[in] measure_err 测量噪声协方差矩阵 \f$R\f$
     */
    inline void setR(const cv::Matx<FloatTp, MeasureDim, MeasureDim> &measure_err) { R = measure_err; }

    /**
     * @brief 设置过程噪声协方差矩阵 \f$Q\f$
     *
     * @param[in] process_err 过程噪声协方差矩阵 \f$Q\f$
     */
    inline void setQ(const cv::Matx<FloatTp, StateDim, StateDim> &process_err) { Q = process_err; }

    /**
     * @brief 设置误差协方差矩阵 \f$P\f$
     *
     * @param[in] state_err 误差协方差矩阵 \f$P\f$
     */
    inline void setP(const cv::Matx<FloatTp, StateDim, StateDim> &state_err) { P_ = P = state_err; }

protected:
    cv::Matx<FloatTp, StateDim, 1> x;            //!< 状态的后验估计 \f$\hat{\boldsymbol x}\f$
    cv::Matx<FloatTp, StateDim, 1> x_;           //!< 状态的先验估计 \f$\hat{\boldsymbol x}^-\f$
    cv::Matx<FloatTp, MeasureDim, 1> z;          //!< 观测向量 \f$\boldsymbol z\f$
    cv::Matx<FloatTp, StateDim, StateDim> Q;     //!< 过程噪声协方差矩阵 \f$Q\f$
    cv::Matx<FloatTp, MeasureDim, MeasureDim> R; //!< 测量噪声协方差矩阵 \f$R\f$
    cv::Matx<FloatTp, StateDim, StateDim> P;     //!< 后验误差协方差矩阵 \f$P\f$
    cv::Matx<FloatTp, StateDim, StateDim> P_;    //!< 先验误差协方差矩阵 \f$P^-\f$
    cv::Matx<FloatTp, StateDim, StateDim> I;     //!< 单位矩阵 \f$I\f$
    cv::Matx<FloatTp, StateDim, MeasureDim> K;   //!< 卡尔曼增益 \f$K\f$
};

/**
 * @brief 卡尔曼滤波器
 *
 * @tparam Tp 数据类型
 * @tparam StateDim 状态量个数
 * @tparam MeasureDim 观测量个数
 */
template <typename FloatTp, int StateDim, int MeasureDim>
class KalmanFilter : public KalmanFilterStaticDatas<FloatTp, StateDim, MeasureDim> {
    static_assert(StateDim > 0, "StateDim of \"rm::KalmanFilter\" must greater than 0.");
    static_assert(MeasureDim > 0, "MeasureDim of \"rm::KalmanFilter\" must greater than 0.");

public:
    //! 构造新的 KalmanFilter 对象
    KalmanFilter() : KalmanFilterStaticDatas<FloatTp, StateDim, MeasureDim>(), A(A.eye()), At(At.eye()), H(H.eye()), Ht(Ht.eye()) {}

    /**
     * @brief 设置状态转移矩阵 \f$A\f$
     * @brief
     * 包含 `x` 方向位置、`y` 方向位置、`x` 方向速度和 `y` 方向速度的运动过程一般可以描述为
     * \f[\left\{\begin{align}x_{n+1}&=x_n+{v_x}_nt+\frac12{a_x}_nt^2\\y_{n+1}&=y_n+{v_y}_nt
     * +\frac12{a_y}_nt^2\\{v_x}_{n+1}&={v_x}_n+{a_x}_nt\\{v_y}_{n+1}&={v_y}_n+{a_y}_nt
     * \end{align}\right.\tag1\f]使用矩阵表示为\f[\begin{bmatrix}x_{n+1}\\y_{n+1}\\{v_x}_{n+1}\\
     * {v_y}_{n+1}\end{bmatrix}=\begin{bmatrix}1&0&t&0\\0&1&0&t\\0&0&1&0\\0&0&0&1\end{bmatrix}
     * \begin{bmatrix}x_n\\y_n\\{v_x}_n\\{v_y}_n\end{bmatrix}+\begin{bmatrix}\frac12t^2&0\\
     * 0&\frac12t^2\\t&0\\0&t\end{bmatrix}\begin{bmatrix}{a_x}_n\\{a_y}_n\end{bmatrix}\tag2\f]
     * 即可以写成\f[\boldsymbol{x}_{n+1}=A\boldsymbol{x}_n+B\boldsymbol{u}_n\tag3\f]
     * 在这条公式中
     * - \f$A\f$ 是状态转移矩阵
     * - \f$\boldsymbol x\f$ 是状态向量
     * - \f$n\f$ 和 \f$n+1\f$ 表示当前帧和上一帧的数据
     * - \f$B\f$ 是控制矩阵
     * - \f$\boldsymbol u\f$ 是控制向量
     *
     * @param[in] state_tf 状态转移矩阵
     */
    inline void setA(const cv::Matx<FloatTp, StateDim, StateDim> &state_tf) { this->A = state_tf, this->At = state_tf.t(); }

    /**
     * @brief 设置观测矩阵 \f$H\f$
     * @brief
     * 若状态向量包含以下内容：\f$[p, v, a]^T\f$ ，然而观测向量仅包含 \f$[p, v]^T\f$ ，
     * 在这种情况下，需要使用一个观测矩阵 \f$H_{2\times3}\f$。在上述例子中可表示为
     * \f[\begin{bmatrix}p\\v\end{bmatrix}=\begin{bmatrix}1&0&0\\0&1&0\end{bmatrix}
     * \begin{bmatrix}p\\v\\a\end{bmatrix}\f]
     * @param[in] observe_tf 观测矩阵
     */
    inline void setH(const cv::Matx<FloatTp, MeasureDim, StateDim> &observe_tf) { this->H = observe_tf, this->Ht = observe_tf.t(); }

    /**
     * @brief 卡尔曼滤波的预测部分，包括状态量的先验估计和误差协方差的先验估计
     * @brief 公式如下 \f[\begin{align}\hat{\boldsymbol x}_k^-&=A\hat{\boldsymbol x}_{k-1}\\P_k^-&=AP_{k-1}A^T+Q\end{align}\f]
     *
     * @return 先验状态估计
     */
    inline auto predict() {
        // 先验状态估计
        this->x_ = A * this->x;
        // 先验误差协方差
        this->P_ = A * this->P * At + this->Q;
        return this->x_;
    }

    /**
     * @brief 卡尔曼滤波器校正部分，包含卡尔曼增益的计算、状态量的后验估计和误差协方差的后验估计
     * @brief 公式如下 \f[\begin{align}K_k&=P_k^-H^T\left(HP_k^-H^T+R\right)^{-1}\\\hat{\boldsymbol x}_k&=\hat{\boldsymbol
     *         x}_k^-+K\left(\boldsymbol z_k-H\hat{\boldsymbol x}_k^-\right)\\P_k&=\left(I-K_kH\right)P_k^-\end{align}\f]
     *
     * @param[in] zk 观测量
     * @return 后验状态估计
     */
    inline auto correct(const cv::Matx<FloatTp, MeasureDim, 1> &zk) {
        this->z = zk;
        // 计算卡尔曼增益
        this->K = this->P_ * Ht * (H * this->P_ * Ht + this->R).inv();
        // 后验状态估计
        this->x = this->x_ + this->K * (this->z - this->H * this->x_);
        // 后验误差协方差
        this->P = (this->I - this->K * H) * this->P_;
        return this->x;
    }

private:
    cv::Matx<FloatTp, StateDim, StateDim> A;    //!< 状态转移矩阵 \f$A\f$
    cv::Matx<FloatTp, StateDim, StateDim> At;   //!< 状态转移矩阵的转置 \f$A^T\f$
    cv::Matx<FloatTp, MeasureDim, StateDim> H;  //!< 观测矩阵 \f$H\f$
    cv::Matx<FloatTp, StateDim, MeasureDim> Ht; //!< 观测矩阵的转置 \f$H^T\f$
};

using KF21f = KalmanFilter<float, 2, 1>;  //!< 2 × 1 卡尔曼滤波器
using KF21d = KalmanFilter<double, 2, 1>; //!< 2 × 1 卡尔曼滤波器
using KF31f = KalmanFilter<float, 3, 1>;  //!< 3 × 1 卡尔曼滤波器
using KF31d = KalmanFilter<double, 3, 1>; //!< 3 × 1 卡尔曼滤波器
using KF32f = KalmanFilter<float, 3, 2>;  //!< 3 × 2 卡尔曼滤波器
using KF32d = KalmanFilter<double, 3, 2>; //!< 3 × 2 卡尔曼滤波器
using KF42f = KalmanFilter<float, 4, 2>;  //!< 4 × 2 卡尔曼滤波器
using KF42d = KalmanFilter<double, 4, 2>; //!< 4 × 2 卡尔曼滤波器
using KF63f = KalmanFilter<float, 6, 3>;  //!< 6 × 3 卡尔曼滤波器
using KF63d = KalmanFilter<double, 6, 3>; //!< 6 × 3 卡尔曼滤波器
using KF64f = KalmanFilter<float, 6, 4>;  //!< 6 × 4 卡尔曼滤波器
using KF64d = KalmanFilter<double, 6, 4>; //!< 6 × 4 卡尔曼滤波器
using KF73f = KalmanFilter<float, 7, 3>;  //!< 7 × 3 卡尔曼滤波器
using KF73d = KalmanFilter<double, 7, 3>; //!< 7 × 3 卡尔曼滤波器

/**
 * @brief 扩展卡尔曼滤波器
 *
 * @tparam FloatTp 浮点数据类型
 * @tparam StateDim 状态量个数
 * @tparam MeasureDim 观测量个数
 */
template <typename FloatTp, int StateDim, int MeasureDim>
class ExtendedKalmanFilter : public KalmanFilterStaticDatas<FloatTp, StateDim, MeasureDim> {
public:
    //! 构造新的 ExtendedKalmanFilter 对象
    ExtendedKalmanFilter() : KalmanFilterStaticDatas<FloatTp, StateDim, MeasureDim>(), Ja(Ja.eye()), Jat(Jat.eye()),
                             Jh(Jh.eye()), Jht(Jht.eye()), W(W.eye()), Wt(Wt.eye()), V(V.eye()), Vt(Vt.eye()) {}

    /**
     * @brief 设置状态方程雅可比矩阵 \f$J_A\f$
     *
     * @deprecated 雅可比矩阵会在 predict() 中根据 Fa 自动计算，手动设置无效
     * @param[in] state_jac 状态方程雅可比矩阵
     */
    [[deprecated("Jacobian is calculated from Fa in predict().")]]
    inline void setJa(const cv::Matx<FloatTp, StateDim, StateDim> &state_jac) { Ja = state_jac, Jat = state_jac.t(); }

    /**
     * @brief 设置观测方程雅可比矩阵 \f$J_H\f$
     *
     * @deprecated 雅可比矩阵会在 correct() 中根据 Fh 自动计算，手动设置无效
     * @param[in] observe_jac 观测方程雅可比矩阵
     */
    [[deprecated("Jacobian is calculated from Fh in correct().")]]
    inline void setJh(const cv::Matx<FloatTp, MeasureDim, StateDim> &observe_jac) { Jh = observe_jac, Jht = observe_jac.t(); }

    /**
     * @brief 设置非线性的离散状态方程 \f$\boldsymbol f_A(\boldsymbol x)\f$
     *
     * @param[in] state_func 状态方程
     */
    inline void setFa(const std::function<cv::Matx<FloatTp, StateDim, 1>(const cv::Matx<FloatTp, StateDim, 1> &)> &state_func) { Fa = state_func; }

    /**
     * @brief 设置非线性的离散观测方程 \f$\boldsymbol f_H(\boldsymbol x)\f$
     *
     * @param[in] observe_func 观测方程
     */
    inline void setFh(const std::function<cv::Matx<FloatTp, MeasureDim, 1>(const cv::Matx<FloatTp, StateDim, 1> &)> &observe_func) { Fh = observe_func; }

    /**
     * @brief 设置过程噪声协方差雅可比矩阵 \f$W\f$
     *
     * @see @ref ekf_state_function_linearization
     * @param[in] process_jac 过程噪声协方差雅可比矩阵
     */
    inline void setW(const cv::Matx<FloatTp, StateDim, StateDim> &process_jac) { W = process_jac, Wt = process_jac.t(); }

    /**
     * @brief 设置测量噪声协方差雅可比矩阵 \f$V\f$
     *
     * @see @ref ekf_observation_function_linearization
     * @param[in] measure_jac 测量噪声协方差雅可比矩阵
     */
    inline void setV(const cv::Matx<FloatTp, MeasureDim, MeasureDim> &measure_jac) { V = measure_jac, Vt = measure_jac.t(); }

    /**
     * @brief 扩展卡尔曼滤波的预测部分，包括状态量的先验估计和误差协方差的先验估计
     * @brief 公式如下 \f[\begin{align}\hat{\boldsymbol x_k}^-&=\boldsymbol f(\hat{\boldsymbol x}_{k-1})\\
     *        P_k^-&=J_AP_{k-1}J_A^T+WQW^T\end{align}\f]
     *
     * @return 先验状态估计
     */
    inline auto predict() {
        Ja = calcJ(Fa, this->x);
        Jat = Ja.t();
        // 先验状态估计
        this->x_ = Fa(this->x);
        // 先验误差协方差
        this->P_ = Ja * this->P * Jat + W * this->Q * Wt;
        return this->x_;
    }

    /**
     * @brief 扩展卡尔曼滤波的校正部分，包含卡尔曼增益的计算、状态量的后验估计和误差协方差的后验估计
     * @brief 公式如下 \f[\begin{align}K_k&=P_k^-J_H^T\left(J_HP_k^-J_H^T+VRV^T\right)^{-1}\\\hat{\boldsymbol x}
     *        &=\hat{\boldsymbol x}_k^-+K_k\left[\boldsymbol z_k-\boldsymbol f_H(\hat{\boldsymbol x}_k^-)\right]\\P_k&=\left(I-K_kJ_H
     *        \right)P_k^-\end{align}\f]
     *
     * @param[in] zk 观测量
     * @return 后验状态估计
     */
    inline auto correct(const cv::Matx<FloatTp, MeasureDim, 1> &zk) {
        this->z = zk;
        Jh = calcJ(Fh, this->x_);
        Jht = Jh.t();
        // 计算卡尔曼增益
        this->K = this->P_ * Jht * (Jh * this->P_ * Jht + V * this->R * Vt).inv();
        // 后验状态估计
        this->x = this->x_ + this->K * (this->z - Fh(this->x_));
        // 后验误差协方差
        this->P = (this->I - this->K * Jh) * this->P_;
        return this->x;
    }

protected:
    template <int OutputDim>
    using StateFunction = std::function<cv::Matx<FloatTp, OutputDim, 1>(const cv::Matx<FloatTp, StateDim, 1> &)>;

    /**
     * @brief 根据非线性函数计算当前状态处的雅可比矩阵
     *
     * @param[in] func 非线性状态方程或观测方程
     * @param[in] x 线性化点
     * @return 当前状态处的雅可比矩阵
     */
    template <int OutputDim>
    static cv::Matx<FloatTp, OutputDim, StateDim> calcJ(const StateFunction<OutputDim> &func, const cv::Matx<FloatTp, StateDim, 1> &x) {
        auto jac = cv::Matx<FloatTp, OutputDim, StateDim>::zeros();
        const FloatTp eps = std::sqrt(std::numeric_limits<FloatTp>::epsilon());
        for (int col = 0; col < StateDim; ++col) {
            auto x_plus = x;
            auto x_minus = x;
            const FloatTp scale = std::abs(x(col)) > FloatTp(1) ? std::abs(x(col)) : FloatTp(1);
            const FloatTp delta = eps * scale;
            x_plus(col) += delta;
            x_minus(col) -= delta;
            const auto diff = (func(x_plus) - func(x_minus)) * (FloatTp(0.5) / delta);
            for (int row = 0; row < OutputDim; ++row)
                jac(row, col) = diff(row);
        }
        return jac;
    }

    cv::Matx<FloatTp, StateDim, StateDim> Ja;     //!< 状态方程雅可比矩阵 \f$J_A\f$
    cv::Matx<FloatTp, StateDim, StateDim> Jat;    //!< 状态方程雅可比矩阵的转置 \f$J_A^T\f$
    cv::Matx<FloatTp, MeasureDim, StateDim> Jh;   //!< 观测方程雅可比矩阵 \f$J_H\f$
    cv::Matx<FloatTp, StateDim, MeasureDim> Jht;  //!< 观测方程雅可比矩阵的转置 \f$J_H^T\f$
    cv::Matx<FloatTp, StateDim, StateDim> W;      //!< 过程噪声协方差雅可比矩阵 \f$W\f$
    cv::Matx<FloatTp, StateDim, StateDim> Wt;     //!< 过程噪声协方差雅可比矩阵的转置 \f$W^T\f$
    cv::Matx<FloatTp, MeasureDim, MeasureDim> V;  //!< 测量噪声协方差雅可比矩阵 \f$V\f$
    cv::Matx<FloatTp, MeasureDim, MeasureDim> Vt; //!< 测量噪声协方差雅可比矩阵的转置 \f$V^T\f$

    //! 非线性的离散状态方程
    std::function<cv::Matx<FloatTp, StateDim, 1>(const cv::Matx<FloatTp, StateDim, 1> &)> Fa;
    //! 非线性的离散观测方程
    std::function<cv::Matx<FloatTp, MeasureDim, 1>(const cv::Matx<FloatTp, StateDim, 1> &)> Fh;
};

using EKF31f = ExtendedKalmanFilter<float, 3, 1>;  //!< 3 × 1 扩展卡尔曼滤波器
using EKF31d = ExtendedKalmanFilter<double, 3, 1>; //!< 3 × 1 扩展卡尔曼滤波器
using EKF32f = ExtendedKalmanFilter<float, 3, 2>;  //!< 3 × 2 扩展卡尔曼滤波器
using EKF32d = ExtendedKalmanFilter<double, 3, 2>; //!< 3 × 2 扩展卡尔曼滤波器
using EKF42f = ExtendedKalmanFilter<float, 4, 2>;  //!< 4 × 2 扩展卡尔曼滤波器
using EKF42d = ExtendedKalmanFilter<double, 4, 2>; //!< 4 × 2 扩展卡尔曼滤波器
using EKF52f = ExtendedKalmanFilter<float, 5, 2>;  //!< 5 × 2 扩展卡尔曼滤波器
using EKF52d = ExtendedKalmanFilter<double, 5, 2>; //!< 5 × 2 扩展卡尔曼滤波器
using EKF53f = ExtendedKalmanFilter<float, 5, 3>;  //!< 5 × 3 扩展卡尔曼滤波器
using EKF53d = ExtendedKalmanFilter<double, 5, 3>; //!< 5 × 3 扩展卡尔曼滤波器
using EKF63f = ExtendedKalmanFilter<float, 6, 3>;  //!< 6 × 3 扩展卡尔曼滤波器
using EKF63d = ExtendedKalmanFilter<double, 6, 3>; //!< 6 × 3 扩展卡尔曼滤波器
using EKF64f = ExtendedKalmanFilter<float, 6, 4>;  //!< 6 × 4 扩展卡尔曼滤波器
using EKF64d = ExtendedKalmanFilter<double, 6, 4>; //!< 6 × 4 扩展卡尔曼滤波器
using EKF73f = ExtendedKalmanFilter<float, 7, 3>;  //!< 7 × 3 扩展卡尔曼滤波器
using EKF73d = ExtendedKalmanFilter<double, 7, 3>; //!< 7 × 3 扩展卡尔曼滤波器
using EKF74f = ExtendedKalmanFilter<float, 7, 4>;  //!< 7 × 4 扩展卡尔曼滤波器
using EKF74d = ExtendedKalmanFilter<double, 7, 4>; //!< 7 × 4 扩展卡尔曼滤波器
using EKF83f = ExtendedKalmanFilter<float, 8, 3>;  //!< 8 × 3 扩展卡尔曼滤波器
using EKF83d = ExtendedKalmanFilter<double, 8, 3>; //!< 8 × 3 扩展卡尔曼滤波器
using EKF84f = ExtendedKalmanFilter<float, 8, 4>;  //!< 8 × 4 扩展卡尔曼滤波器
using EKF84d = ExtendedKalmanFilter<double, 8, 4>; //!< 8 × 4 扩展卡尔曼滤波器
using EKF94f = ExtendedKalmanFilter<float, 9, 4>;  //!< 9 × 4 扩展卡尔曼滤波器
using EKF94d = ExtendedKalmanFilter<double, 9, 4>; //!< 9 × 4 扩展卡尔曼滤波器

/**
 * @brief 紧耦合迭代扩展卡尔曼滤波器
 *
 * @tparam FloatTp 浮点数数据类型
 * @tparam StateDim
 * @tparam MeasureDim
 */
template <typename FloatTp, int StateDim, int MeasureDim>
class IteratedExtendedKalmanFilter : public ExtendedKalmanFilter<FloatTp, StateDim, MeasureDim> {
public:
    //! 构造新的 IteratedExtendedKalmanFilter 对象
    IteratedExtendedKalmanFilter(int max_iter = 5, FloatTp eps = FloatTp(1e-3))
        : ExtendedKalmanFilter<FloatTp, StateDim, MeasureDim>(), _max_iter(max_iter > 0 ? max_iter : 1), _eps(eps > FloatTp(0) ? eps : FloatTp(1e-3)) {}

    /**
     * @brief 迭代扩展卡尔曼滤波的校正部分
     *
     * @param[in] zk 观测量
     * @return 后验状态估计
     */
    inline auto correct(const cv::Matx<FloatTp, MeasureDim, 1> &zk) {
        this->z = zk;
        auto xi = this->x_;

        for (int i = 0; i < _max_iter; ++i) {
            this->Jh = this->calcJ(this->Fh, xi);
            this->Jht = this->Jh.t();
            this->K = this->P_ * this->Jht * (this->Jh * this->P_ * this->Jht + this->V * this->R * this->Vt).inv();

            const auto x_prev = xi;
            xi = this->x_ + this->K * (this->z - this->Fh(x_prev) + this->Jh * (x_prev - this->x_));
            if (calcError(xi - x_prev) < _eps)
                break;
        }

        this->x = xi;
        this->Jh = this->calcJ(this->Fh, this->x);
        this->Jht = this->Jh.t();
        this->K = this->P_ * this->Jht * (this->Jh * this->P_ * this->Jht + this->V * this->R * this->Vt).inv();
        this->P = (this->I - this->K * this->Jh) * this->P_;
        return this->x;
    }

private:
    static FloatTp calcError(const cv::Matx<FloatTp, StateDim, 1> &dx) {
        FloatTp err{};
        for (int i = 0; i < StateDim; ++i)
            err += dx(i) * dx(i);
        return err;
    }

    int _max_iter; //!< 最大迭代次数
    FloatTp _eps;  //!< 迭代终止阈值
};

using IEKF31f = IteratedExtendedKalmanFilter<float, 3, 1>;  //!< 3 × 1 迭代扩展卡尔曼滤波器
using IEKF31d = IteratedExtendedKalmanFilter<double, 3, 1>; //!< 3 × 1 迭代扩展卡尔曼滤波器
using IEKF32f = IteratedExtendedKalmanFilter<float, 3, 2>;  //!< 3 × 2 迭代扩展卡尔曼滤波器
using IEKF32d = IteratedExtendedKalmanFilter<double, 3, 2>; //!< 3 × 2 迭代扩展卡尔曼滤波器
using IEKF42f = IteratedExtendedKalmanFilter<float, 4, 2>;  //!< 4 × 2 迭代扩展卡尔曼滤波器
using IEKF42d = IteratedExtendedKalmanFilter<double, 4, 2>; //!< 4 × 2 迭代扩展卡尔曼滤波器
using IEKF52f = IteratedExtendedKalmanFilter<float, 5, 2>;  //!< 5 × 2 迭代扩展卡尔曼滤波器
using IEKF52d = IteratedExtendedKalmanFilter<double, 5, 2>; //!< 5 × 2 迭代扩展卡尔曼滤波器
using IEKF53f = IteratedExtendedKalmanFilter<float, 5, 3>;  //!< 5 × 3 迭代扩展卡尔曼滤波器
using IEKF53d = IteratedExtendedKalmanFilter<double, 5, 3>; //!< 5 × 3 迭代扩展卡尔曼滤波器
using IEKF63f = IteratedExtendedKalmanFilter<float, 6, 3>;  //!< 6 × 3 迭代扩展卡尔曼滤波器
using IEKF63d = IteratedExtendedKalmanFilter<double, 6, 3>; //!< 6 × 3 迭代扩展卡尔曼滤波器
using IEKF64f = IteratedExtendedKalmanFilter<float, 6, 4>;  //!< 6 × 4 迭代扩展卡尔曼滤波器
using IEKF64d = IteratedExtendedKalmanFilter<double, 6, 4>; //!< 6 × 4 迭代扩展卡尔曼滤波器
using IEKF73f = IteratedExtendedKalmanFilter<float, 7, 3>;  //!< 7 × 3 迭代扩展卡尔曼滤波器
using IEKF73d = IteratedExtendedKalmanFilter<double, 7, 3>; //!< 7 × 3 迭代扩展卡尔曼滤波器
using IEKF74f = IteratedExtendedKalmanFilter<float, 7, 4>;  //!< 7 × 4 迭代扩展卡尔曼滤波器
using IEKF74d = IteratedExtendedKalmanFilter<double, 7, 4>; //!< 7 × 4 迭代扩展卡尔曼滤波器
using IEKF83f = IteratedExtendedKalmanFilter<float, 8, 3>;  //!< 8 × 3 迭代扩展卡尔曼滤波器
using IEKF83d = IteratedExtendedKalmanFilter<double, 8, 3>; //!< 8 × 3 迭代扩展卡尔曼滤波器
using IEKF84f = IteratedExtendedKalmanFilter<float, 8, 4>;  //!< 8 × 4 迭代扩展卡尔曼滤波器
using IEKF84d = IteratedExtendedKalmanFilter<double, 8, 4>; //!< 8 × 4 迭代扩展卡尔曼滤波器
using IEKF94f = IteratedExtendedKalmanFilter<float, 9, 4>;  //!< 9 × 4 迭代扩展卡尔曼滤波器
using IEKF94d = IteratedExtendedKalmanFilter<double, 9, 4>; //!< 9 × 4 迭代扩展卡尔曼滤波器

//! @} algorithm_kalman

} // namespace rm

#endif // HAVE_OPENCV

/**
 * @file kalman.hpp
 * @author RoboMaster Vision Community
 * @brief 包含轻量级 `cv::Matx` 的卡尔曼滤波模块
 * @details 考虑到 OpenCV 中提供的 `cv::KalmanFilter` 是基于 `cv::Mat` 实现的，并且 `cv::Mat`
 *          的内存操作在运行时是在堆上打开的，因此会消耗大量的时间，所以现在使用 `cv::Matx`
 *          来复现卡尔曼滤波的功能。并简化部分功能的实现，以达到更方便使用的目的。
 * @version 2.0
 * @date 2022-10-05
 *
 * @copyright Copyright 2022 (c), RoboMaster Vision Community
 *
 */

#pragma once

#include <opencv2/core/matx.hpp>

//! @addtogroup core
//! @{
//!     @defgroup core_kalman 卡尔曼滤波器库
//! @}

//! @addtogroup core_kalman 卡尔曼滤波器库
//! @{

/**
 * @brief 使用 `cv::Matx` 的轻量级卡尔曼滤波器库
 *
 * @tparam Tp 数据类型
 * @tparam StateDim 状态向量的维度，类型是 `uint16_t`
 * @tparam MeasureDim The 观测向量的维度，类型是 `uint16_t`
 */
template <typename Tp, uint16_t StateDim, uint16_t MeasureDim>
class KalmanFilterX
{
    cv::Matx<Tp, StateDim, StateDim> A;     //!< 状态转移矩阵 \f$A\f$
    cv::Matx<Tp, StateDim, StateDim> At;    //!< 状态转移矩阵 `A` 的转置矩阵 \f$A^T\f$
    cv::Matx<Tp, MeasureDim, StateDim> H;   //!< 观测转换矩阵 \f$H\f$
    cv::Matx<Tp, StateDim, MeasureDim> Ht;  //!< 观测转换矩阵 `H` 的转置矩阵 \f$H^T\f$
    cv::Matx<Tp, StateDim, 1> xhat;         //!< 后验状态估计 \f$\hat x\f$
    cv::Matx<Tp, StateDim, 1> xhat_;        //!< 先验状态估计 \f$\hat x^-\f$
    cv::Matx<Tp, StateDim, StateDim> Q;     //!< 过程噪声协方差矩阵 \f$Q\f$
    cv::Matx<Tp, MeasureDim, MeasureDim> R; //!< 测量噪声协方差矩阵 \f$R\f$
    cv::Matx<Tp, StateDim, StateDim> P;     //!< 后验状态误差协方差矩阵 \f$P\f$
    cv::Matx<Tp, StateDim, StateDim> P_;    //!< 先验状态误差协方差矩阵 \f$P^-\f$
    cv::Matx<Tp, StateDim, StateDim> I;     //!< 单位矩阵 \f$I\f$
    cv::Matx<Tp, StateDim, MeasureDim> K;   //!< 卡尔曼增益 \f$K\f$
    cv::Matx<Tp, MeasureDim, 1> z;          //!< 观测向量 \f$z\f$

public:
    //! 构造新的 KalmanFilterX 对象
    KalmanFilterX() : A(A.eye()), At(At.eye()), H(H.eye()), Ht(Ht.eye()),
                      Q(Q.eye()), R(R.eye()), P(P.eye()), I(I.eye()) {}

    /**
     * @brief 初始化状态以及对应的误差协方差矩阵（常数对角矩阵）
     *
     * @param[in] state 初始化的状态向量
     * @param[in] error 状态误差系数
     */
    void init(const cv::Matx<Tp, StateDim, 1> &state, Tp error)
    {
        xhat_ = xhat = state;
        P_ = P = P.eye() * error;
    }

    /**
     * @brief 初始化状态以及对应的误差协方差矩阵（对角矩阵）
     *
     * @param[in] state 初始化的状态向量
     * @param[in] error 状态误差矩阵的对角线元素
     */
    void init(const cv::Matx<Tp, StateDim, 1> &state, const cv::Matx<Tp, StateDim, 1> &error)
    {
        xhat_ = xhat = state;
        P_ = P = P.diag(error);
    }

    /**
     * @brief 设置状态转移矩阵 `A`
     * @note 包含 `x` 方向位置、`y` 方向位置、`x` 方向速度和 `y` 方向速度的运动过程一般可以描述为
     *       \f[\left\{\begin{array}{rl}x_{n+1}&=&x_n+v_{x_n}t\\y_{n+1}&=&y_n+v_{y_n}t\\v_{x_{n+1}}&=&
     *       v_{x_n}\\v_{y_{n+1}}&=&v_{y_n}\end{array}\right.\f] Using the matrix formula denoted as
     *       \f[\begin{bmatrix}x_{n+1}\\y_{n+1}\\v_{x_{n+1}}\\v_{y_{n+1}}\end{bmatrix}=
     *       \begin{bmatrix}1&0&t&0\\0&1&0&t\\0&0&1&0\\0&0&0&1\end{bmatrix}
     *       \begin{bmatrix}x_n\\y_n\\v_{x_n}\\v_{y_n}\end{bmatrix}\f] Which is \f$X_{n+1}=AX_n\f$.
     *       在这条公式中，矩阵 \f$A\f$ 被称为状态转移矩阵
     *
     * @param[in] _A 状态转移矩阵
     */
    inline void setA(const cv::Matx<Tp, StateDim, StateDim> &_A) { A = _A, At = _A.t(); }

    /**
     * @brief 设置观测转换矩阵 `H`
     * @note 若状态向量包含以下内容：\f$[p, v, a]\f$ ，然而观测向量仅包含 \f$[p, v]\f$，
     *       在这种情况下，需要使用一个观测转换矩阵 \f$H_{2\times3}\f$。在上述例子中克表示为
     *       \f[\begin{bmatrix}p\\v\end{bmatrix}=\begin{bmatrix}1&0&0\\0&1&0\end{bmatrix}
     *       \begin{bmatrix}p\\v\\a\end{bmatrix}\f]
     *
     * @param[in] _H 观测转换矩阵
     */
    inline void setH(const cv::Matx<Tp, MeasureDim, StateDim> &_H) { H = _H, Ht = _H.t(); }

    /**
     * @brief 设置测量噪声协方差矩阵 `R`
     *
     * @param[in] measure_err 测量噪声协方差矩阵 `R`
     */
    inline void setR(const cv::Matx<Tp, MeasureDim, MeasureDim> &measure_err) { R = measure_err; }

    /**
     * @brief 设置过程噪声协方差矩阵 `Q`
     *
     * @param[in] process_err 过程噪声协方差矩阵 `Q`
     */
    inline void setQ(const cv::Matx<Tp, StateDim, StateDim> &process_err) { Q = process_err; }

    /**
     * @brief 设置状态误差协方差矩阵 `P`
     *
     * @param[in] state_err 状态误差协方差矩阵 `P`
     */
    inline void setP(const cv::Matx<Tp, StateDim, StateDim> &state_err) { P_ = P = state_err; }

    /**
     * @brief 卡尔曼滤波的预测部分，包括状态向量的先验估计和误差协方差的先验估计
     *
     * @return 先验状态估计
     */
    inline auto predict()
    {
        // Estimate the prior state vector
        xhat_ = A * xhat;
        // Estimate the prior error covariance
        P_ = A * P * At + Q;
        return xhat_;
    }

    /**
     * @brief 卡尔曼滤波器的校正部分，包括卡尔曼增益 `K` 的计算、状态向量的后验估计和误差协方差的校正
     *
     * @param[in] measurement 观测向量
     * @return 后验状态估计
     */
    inline auto correct(const cv::Matx<Tp, MeasureDim, 1> &measurement)
    {
        z = measurement;
        // Calculate the Kalman gain: K
        K = P_ * Ht * (H * P_ * Ht + R).inv();
        // Estimate the posterior state. In other words, update the state estimation
        xhat = xhat_ + K * (z - H * xhat_);
        // Correction the error covariance
        P = (I - K * H) * P_;
        return xhat;
    }
};

using KF11f = KalmanFilterX<float, 1U, 1U>;  //!< 1 × 1 卡尔曼滤波类
using KF11d = KalmanFilterX<double, 1U, 1U>; //!< 1 × 1 卡尔曼滤波类
using KF22f = KalmanFilterX<float, 2U, 2U>;  //!< 2 × 2 卡尔曼滤波类
using KF22d = KalmanFilterX<double, 2U, 2U>; //!< 2 × 2 卡尔曼滤波类
using KF33f = KalmanFilterX<float, 3U, 3U>;  //!< 3 × 3 卡尔曼滤波类
using KF33d = KalmanFilterX<double, 3U, 3U>; //!< 3 × 3 卡尔曼滤波类
using KF44f = KalmanFilterX<float, 4U, 4U>;  //!< 4 × 4 卡尔曼滤波类
using KF44d = KalmanFilterX<double, 4U, 4U>; //!< 4 × 4 卡尔曼滤波类
using KF66f = KalmanFilterX<float, 6U, 6U>;  //!< 6 × 6 卡尔曼滤波类
using KF66d = KalmanFilterX<double, 6U, 6U>; //!< 6 × 6 卡尔曼滤波类

//! @} kalman

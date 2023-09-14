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
#include <type_traits>

//! @addtogroup core
//! @{
//!     @defgroup core_kalman 卡尔曼滤波器库
//! @}

namespace rm
{

//! @addtogroup core_kalman 卡尔曼滤波器库
//! @{

/**
 * @brief 轻量级 `cv::Matx` 的卡尔曼滤波模块
 *
 * @note 相关知识点说明文档可参考 @ref tutorial_modules_kalman
 * @tparam Tp 数据类型
 * @tparam StateDim 状态向量的维度，类型是 `uint16_t`
 * @tparam MeasureDim 观测向量的维度，类型是 `uint16_t`
 * @tparam ControlDim 控制向量的维度，类型是 `uint16_t`，默认为 `1`
 */
template <typename Tp, uint16_t StateDim, uint16_t MeasureDim, uint16_t ControlDim = 1>
class KalmanFilter
{
#if __cplusplus >= 201703L
    static_assert(std::is_floating_point_v<Tp>, "\"Tp\" must be floating point value.");
    static_assert(ControlDim != 0, "ControlDim of \"rm::KalmanFilter\" must greater than 0.");
#endif // C++17

    cv::Matx<Tp, StateDim, StateDim> A;   //!< 状态转移矩阵 \f$A\f$
    cv::Matx<Tp, StateDim, StateDim> At;  //!< 状态转移矩阵 `A` 的转置矩阵 \f$A^T\f$
    cv::Matx<Tp, StateDim, 1> x;          //!< 后验状态估计 \f$\hat x\f$
    cv::Matx<Tp, StateDim, 1> x_;         //!< 先验状态估计 \f$\hat x^-\f$
    cv::Matx<Tp, StateDim, ControlDim> B; //!< 控制量直接对状态量作用的矩阵：控制矩阵 `B` \f$B\f$
    cv::Matx<Tp, ControlDim, 1> u;        //!< 控制向量 `u` \f$\vec{u}\f$

    cv::Matx<Tp, MeasureDim, StateDim> H;   //!< 观测转换矩阵 \f$H\f$
    cv::Matx<Tp, StateDim, MeasureDim> Ht;  //!< 观测转换矩阵 `H` 的转置矩阵 \f$H^T\f$
    cv::Matx<Tp, StateDim, StateDim> Q;     //!< 过程噪声协方差矩阵 \f$Q\f$
    cv::Matx<Tp, MeasureDim, MeasureDim> R; //!< 测量噪声协方差矩阵 \f$R\f$
    cv::Matx<Tp, StateDim, StateDim> P;     //!< 后验状态误差协方差矩阵 \f$P\f$
    cv::Matx<Tp, StateDim, StateDim> P_;    //!< 先验状态误差协方差矩阵 \f$P^-\f$
    cv::Matx<Tp, StateDim, StateDim> I;     //!< 单位矩阵 `I` \f$I\f$
    cv::Matx<Tp, StateDim, MeasureDim> K;   //!< 卡尔曼增益 `K` \f$K\f$
    cv::Matx<Tp, MeasureDim, 1> z;          //!< 观测向量 `z` \f$\vec{z}\f$

public:
    //! 构造新的 KalmanFilter 对象
    KalmanFilter() : A(A.eye()), At(At.eye()), H(H.eye()), Ht(Ht.eye()),
                     Q(Q.eye()), R(R.eye()), P(P.eye()), I(I.eye()) {}

    /**
     * @brief 初始化状态以及对应的误差协方差矩阵（常数对角矩阵）
     *
     * @param[in] state 初始化的状态向量
     * @param[in] error 状态误差系数
     */
    void init(const cv::Matx<Tp, StateDim, 1> &state, Tp error)
    {
        x_ = x = state;
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
        x_ = x = state;
        P_ = P = P.diag(error);
    }

    /**
     * @brief 设置状态转移矩阵 `A`
     * @details
     * 包含 `x` 方向位置、`y` 方向位置、`x` 方向速度和 `y` 方向速度的运动过程一般可以描述为
     * \f[\left\{\begin{align}x_{n+1}&=x_n+{v_x}_nt+\frac12{a_x}_nt^2\\y_{n+1}&=y_n+{v_y}_nt
     * +\frac12{a_y}_nt^2\\{v_x}_{n+1}&={v_x}_n+{a_x}_nt\\{v_y}_{n+1}&={v_y}_n+{a_y}_nt
     * \end{align}\right.\tag1\f]使用矩阵表示为\f[\begin{bmatrix}x_{n+1}\\y_{n+1}\\{v_x}_{n+1}\\
     * {v_y}_{n+1}\end{bmatrix}=\begin{bmatrix}1&0&t&0\\0&1&0&t\\0&0&1&0\\0&0&0&1\end{bmatrix}
     * \begin{bmatrix}x_n\\y_n\\{v_x}_n\\{v_y}_n\end{bmatrix}+\begin{bmatrix}\frac12t^2&0\\
     * 0&\frac12t^2\\t&0\\0&t\end{bmatrix}\begin{bmatrix}{a_x}_n\\{a_y}_n\end{bmatrix}\tag2\f]
     * 即可以写成\f[\pmb{x}_{n+1}=A\pmb{x}_n+B\pmb{u}_n\tag3\f]
     * 在这条公式中
     * - \f$A\f$ 是状态转移矩阵
     * - \f$\pmb x\f$ 是状态向量
     * - \f$n\f$ 和 \f$n+1\f$ 表示当前帧和上一帧的数据
     * - \f$B\f$ 是控制矩阵
     * - \f$\pmb u\f$ 是控制向量
     *
     * @param[in] state_tf 状态转移矩阵
     */
    inline void setA(const cv::Matx<Tp, StateDim, StateDim> &state_tf) { A = state_tf, At = state_tf.t(); }

    /**
     * @brief 设置控制矩阵 `B`
     * @details 输入直接对状态向量的作用，由控制矩阵 `B` 定义
     * @see setA
     *
     * @param[in] control_matrix 控制矩阵
     */
    inline void setB(const cv::Matx<Tp, StateDim, ControlDim> &control_matrix) { B = control_matrix; }

    /**
     * @brief 设置观测转换矩阵 `H`
     * @details
     * 若状态向量包含以下内容：\f$[p, v, a]^T\f$ ，然而观测向量仅包含 \f$[p, v]^T\f$，
     * 在这种情况下，需要使用一个观测转换矩阵 \f$H_{2\times3}\f$。在上述例子中克表示为
     * \f[\begin{bmatrix}p\\v\end{bmatrix}=\begin{bmatrix}1&0&0\\0&1&0\end{bmatrix}
     * \begin{bmatrix}p\\v\\a\end{bmatrix}\f]
     * @note `H` 若不加以设置，则默认是单位矩阵 \f$I\f$
     * @param[in] observe_tf 观测转换矩阵
     */
    inline void setH(const cv::Matx<Tp, MeasureDim, StateDim> &observe_tf) { H = observe_tf, Ht = observe_tf.t(); }

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
     * @brief 含控制量的卡尔曼滤波的预测部分，包括状态向量的先验估计和误差协方差的先验估计
     * @details 预测部分公式如下
     *          \f[\begin{align}\hat{\pmb x}^-&=A\hat{\pmb x}+B\pmb u\\P^-&=APA^T+Q\end{align}\f]
     *
     * @param[in] control_vec 控制向量
     * @return 先验状态估计
     */
    inline auto predict(const cv::Matx<Tp, ControlDim, 1> &control_vec)
    {
        u = control_vec;
        // 估计先验状态，考虑控制向量
        x_ = A * x + B * u;
        // 估计先验误差协方差
        P_ = A * P * At + Q;
        return x_;
    }

    /**
     * @brief 不含控制量的卡尔曼滤波的预测部分，包括状态向量的先验估计和误差协方差的先验估计
     * @see predict(const cv::Matx<Tp, ControlDim, 1> &control)
     * @note 公式中 \f$B\pmb u = \pmb 0\f$
     *
     * @return 先验状态估计
     */
    inline auto predict()
    {
        // 估计先验状态
        x_ = A * x;
        // 估计先验误差协方差
        P_ = A * P * At + Q;
        return x_;
    }

    /**
     * @brief 卡尔曼滤波器的校正部分，包括卡尔曼增益 `K` 的计算、状态向量的后验估计和误差协方差的校正
     * @details
     * 更新部分公式如下 \f[\begin{align}K&=P^-H^T\left(HP^-H^T+R\right)^{-1}\\\hat{\pmb x}&=
     * \hat{\pmb x}^-+K\left(\pmb z-H\hat{\pmb x}^-\right)\\P&=\left(I-KH\right)P^-\end{align}\f]
     *
     * @param[in] measurement 观测向量
     * @return 后验状态估计
     */
    inline auto correct(const cv::Matx<Tp, MeasureDim, 1> &measurement)
    {
        z = measurement;
        // 计算卡尔曼增益 `K`
        K = P_ * Ht * (H * P_ * Ht + R).inv();
        // 估计后验状态，即完成状态的更新
        x = x_ + K * (z - H * x_);
        // 估计后验误差协方差，即校正、更新误差协方差
        P = (I - K * H) * P_;
        return x;
    }
};

using KF11f = KalmanFilter<float, 1U, 1U>;  //!< 1 × 1 卡尔曼滤波器，无控制量
using KF11d = KalmanFilter<double, 1U, 1U>; //!< 1 × 1 卡尔曼滤波器，无控制量
using KF22f = KalmanFilter<float, 2U, 2U>;  //!< 2 × 2 卡尔曼滤波器，无控制量
using KF22d = KalmanFilter<double, 2U, 2U>; //!< 2 × 2 卡尔曼滤波器，无控制量
using KF33f = KalmanFilter<float, 3U, 3U>;  //!< 3 × 3 卡尔曼滤波器，无控制量
using KF33d = KalmanFilter<double, 3U, 3U>; //!< 3 × 3 卡尔曼滤波器，无控制量
using KF44f = KalmanFilter<float, 4U, 4U>;  //!< 4 × 4 卡尔曼滤波器，无控制量
using KF44d = KalmanFilter<double, 4U, 4U>; //!< 4 × 4 卡尔曼滤波器，无控制量
using KF66f = KalmanFilter<float, 6U, 6U>;  //!< 6 × 6 卡尔曼滤波器，无控制量
using KF66d = KalmanFilter<double, 6U, 6U>; //!< 6 × 6 卡尔曼滤波器，无控制量

//! @} kalman

} // namespace rm

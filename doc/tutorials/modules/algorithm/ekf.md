扩展卡尔曼滤波 {#tutorial_modules_ekf}
============

@author 赵曦
@date 2024/04/19
@version 1.0
@brief 扩展卡尔曼滤波

@prev_tutorial{tutorial_modules_kalman}

@next_tutorial{tutorial_modules_dft}

@tableofcontents

------

相关类 rm::ExtendedKalmanFilter

\f[
\def\red#1{\color{red}{#1}}
\def\teal#1{\color{teal}{#1}}
\def\green#1{\color{green}{#1}}
\def\transparent#1{\color{transparent}{#1}}
\def\orange#1{\color{orange}{#1}}
\def\Var{\mathrm{Var}}
\def\Cov{\mathrm{Cov}}
\def\tr{\mathrm{tr}}
\def\fml#1{\text{(#1)}}
\def\ptl#1#2{\frac{\partial#1}{\partial#2}}
\f]

在阅读本教程前，请确保已经熟悉标准的 @ref tutorial_modules_kalman ，因为核心公式不变，只是在原来的基础上增加了非线性函数线性化的部分。

### 1. 非线性函数的线性化

对于一个线性系统，可以用状态空间方程描述其运动过程

\f[\begin{align}\dot{\pmb x}&=A\pmb x+B\pmb u\\\pmb y&=C\pmb x\end{align}\tag{1-1}\f]

离散化，并考虑噪声后可以写为

\f[\begin{align}\dot{\pmb x}_k&=A\pmb x_{k-1}+B\pmb u_{k-1}+\pmb w_{k-1}&&\pmb w_{k-1}\sim N(0,Q)\tag{1-2a}\\
\pmb z_k&=H\pmb x_{k-1}+\pmb v_k&&\pmb v_k\sim N(0,R)\tag{1-2b}\end{align}\f]

但对于一个非线性系统，我们就无法使用矩阵来表示了，我们需要写为

\f[\left\{\begin{align}\dot{\pmb x}_k&=\pmb f_A(\pmb x_{k-1},\pmb u_{k-1},\pmb w_{k-1})\\
\pmb z_k&=\pmb f_H(\pmb x_{k-1},\pmb v_{k-1})\end{align}\right.\tag{1-3}\f]

其中，\f$\pmb f_A\f$ 和 \f$\pmb f_H\f$ 都为非线性函数。我们在非线性函数中同样考虑了噪声，但是对于状态量以及观测量本身的噪声而言，<span style="color: red">正态分布的随机变量通过非线性系统后就不再服从正态分布了</span>。因此我们可以利用 **泰勒展开** ，将非线性系统线性化，即

\f[f(x)\approx f(x_0)+\frac{\mathrm df}{\mathrm dx}(x-x_0)\tag{1-4}\f]

对于多元函数而言，泰勒展开可以写为

\f[f(x,y,z)\approx f(x_0,y_0,z_0)+\begin{bmatrix}f'_x(x_0,y_0,z_0)&f'_y(x_0,y_0,z_0)&f'_z(x_0,y_0,z_0)\end{bmatrix}\begin{bmatrix}x-x_0\\y-y_0\\z-z_0\end{bmatrix}\tag{1-5a}\f]

即

\f[f(\pmb x)\approx f(\pmb x_0)+\ptl fx(\pmb x-\pmb x_0)=f(\pmb x_0)+\nabla f(\pmb x_0)(\pmb x-\pmb x_0)\tag{1-5b}\f]

#### 1.1 状态方程线性化 {#ekf_state_function_linearization}

对公式 \f$\fml{1-2a}\f$ 在 \f$\hat{\pmb x}_{k-1}\f$ 处进行线性化，即选取 \f$\text{k-1}\f$ 时刻的后验状态估计作为展开点，有

\f[\pmb x_k=\pmb f_A(\hat{\pmb x}_{k-1},\pmb u_{k-1},\pmb w_{k-1})+J_A(\pmb x_{k-1}-\hat x_{k-1})+W\pmb w_{k-1}\tag{1-6}\f]

令 \f$\pmb w_{k-1}=\pmb 0\f$，则 \f$f_A(\hat{\pmb x}_{k-1},\pmb u_{k-1},\pmb w_{k-1})=f_A(\hat{\pmb x}_{k-1},\pmb u_{k-1},\pmb 0)\stackrel{\triangle}=\tilde{\pmb x}_{k-1}\f$，有

\f[\red{\pmb x_k=\tilde{\pmb x}_{k-1}+J_A(\pmb x_{k-1}-\hat x_{k-1})+W\pmb w_{k-1}\qquad W\pmb w_{k-1}\sim N(0,WQW^T)\tag{1-7}}\f]

其中

\f[\begin{align}J_A&=\left.\ptl{f_A}{\pmb x}\right|_{(\hat{\pmb x}_{k-1},\pmb u_{k-1})}=\begin{bmatrix}\ptl{{f_A}_1}{x_1}&\ptl{{f_A}_1}{x_2}&\cdots&\ptl{{f_A}_1}{x_n}\\\ptl{{f_A}_2}{x_1}&\ptl{{f_A}_2}{x_2}&\cdots&\ptl{{f_A}_2}{x_n}\\\vdots&\vdots&\ddots&\vdots\\\ptl{{f_A}_n}{x_1}&\ptl{{f_A}_n}{x_2}&\cdots&\ptl{{f_A}_n}{x_n}\end{bmatrix}\\
W&=\left.\ptl{f_A}{\pmb w}\right|_{(\hat{\pmb w}_{k-1},\pmb u_{k-1})}\end{align}\f]

#### 1.2 观测方程线性化 {#ekf_observation_function_linearization}

对公式 \f$\fml{1-2b}\f$ 在 \f$\hat{\pmb x}_k\f$ 处进行线性化，有

\f[\pmb z_k=\pmb f_H(\tilde{\pmb x}_k,\pmb v_k)+J_H(\pmb x_k-\tilde x_k)+V\pmb v_k\tag{1-8}\f]

令 \f$\pmb v_k=\pmb 0\f$，则 \f$f_H(\tilde{\pmb x}_k,\pmb v_k)=f_H(\tilde{\pmb x}_k,\pmb 0)\stackrel{\triangle}=\tilde{\pmb z}_k\f$，有

\f[\red{\pmb z_k=\tilde{\pmb z}_k+J_H(\pmb x_k-\tilde x_k)+V\pmb v_k\qquad V\pmb v_k\sim N(0,VRV^T)\tag{1-9}}\f]

其中

\f[J_H=\left.\ptl{f_H}{\pmb x}\right|_{\tilde{\pmb x}_k},\qquad V=\left.\ptl{f_H}{\pmb v}\right|_{\tilde{\pmb x}_k}\f]

### 2. 扩展卡尔曼滤波

#### 2.1 公式汇总

根据卡尔曼滤波的 @ref kalman_filter_formulas 可以相应的改写非线性系统下的卡尔曼滤波公式，从而得到如下的扩展卡尔曼滤波公式。

**① 预测**

1. <span style="color: teal">先验状态估计</span>
   \f[\hat{\pmb x}_k^-=\pmb f_A(\pmb x_{k-1},\pmb u_{k-1},\pmb 0)\f]

2. <span style="color: teal">计算先验误差协方差</span>
   \f[P_k^-=J_AP_{k-1}J_A^T+WQW^T\f]

**② 校正（更新）**

1. <span style="color: teal">计算卡尔曼增益</span>
   \f[K_k=P_k^-J_H^T\left(J_HP_k^-J_H^T+VRV^T\right)^{-1}\f]

2. <span style="color: teal">后验状态估计</span>
   \f[\hat{\pmb x}_k=\hat{\pmb x}_k^-+K_k\left[\pmb z_k-\pmb f_H(\hat{\pmb x}_k^-,\pmb 0)\right]\f]

3. <span style="color: teal">更新后验误差协方差</span>
   \f[P_k=(I-K_kJ_H)P_k^-\f]

#### 2.2 EKF 模块的使用

下面拿扩展卡尔曼模块单元测试的内容举例子

```cpp
#include <cstdio>
#include <random>
#include <rmvl/algorithm/kalman.hpp>

int main()
{
    // 状态量 x = [ cx, cy, θ, ω, r ]ᵀ
    //
    // 状态方程
    //           ┌ cx                  ┌ 1  0  0  0  0 ┐
    //           │ cy                  │ 0  1  0  0  0 │
    //       F = │ θ + ωT         Ja = │ 0  0  1  T  0 │ = A
    //           │ ω                   │ 0  0  0  1  0 │
    //           └ r                   └ 0  0  0  0  1 ┘
    //
    // 观测量 z = [ px, py, θ ]ᵀ
    //
    // 观测方程
    //           ┌ cx + rcosθ          ┌ 1  0 -rsinθ  0  cosθ ┐
    //       H = │ cy + rsinθ     Jh = │ 0  1  rcosθ  0  sinθ │
    //           └ θ                   └ 0  0    1    0    0  ┘

    // 正态分布噪声
    std::default_random_engine ng;
    std::normal_distribution<double> err{0, 1};

    // 创建扩展卡尔曼滤波
    rm::EKF53d ekf;
    ekf.init({0, 0, 0, 0, 150}, 1e5);
    ekf.setQ(1e-1 * cv::Matx<double, 5, 5>::eye());
    ekf.setR(cv::Matx33d::diag({1e-3, 1e-3, 1e-3}));
    double t{0.01};
    // 设置状态方程（这里的例子是线性的，但一般都是非线性的）
    ekf.setFa([=](const cv::Matx<double, 5, 1> &x) -> cv::Matx<double, 5, 1> {
        return {x(0),
                x(1),
                x(2) + x(3) * t,
                x(3),
                x(4)};
    });
    // 设置观测方程
    ekf.setFh([=](const cv::Matx<double, 5, 1> &x) -> cv::Matx<double, 3, 1> {
        return {x(0) + x(4) * std::cos(x(2)),
                x(1) + x(4) * std::sin(x(2)),
                x(2)};
    });

    while (true)
    {
        // 预测部分，设置状态方程 Jacobi 矩阵，获取先验状态估计
        ekf.setJa({1, 0, 0, 0, 0,
                   0, 1, 0, 0, 0,
                   0, 0, 1, t, 0,
                   0, 0, 0, 1, 0,
                   0, 0, 0, 0, 1});
        auto x_ = ekf.predict();
        // 更新部分，设置观测方程 Jacobi 矩阵，获取后验状态估计
        ekf.setJh({1, 0, -x_(4) * std::sin(x_(2)), 0, std::cos(x_(2)),
                   0, 1, x_(4) * std::cos(x_(2)), 0, std::sin(x_(2)),
                   0, 0, 1, 0, 0});
        // 以 20 为半径，0.02/T 为角速度的圆周运动（图像上是顺时针），并人为加上观测噪声
        auto x = ekf.correct({500 + 200 * std::cos(0.02 * i) + err(ng),
                              500 + 200 * std::sin(0.02 * i) + err(ng),
                              0.02 * i + 0.01 * err(ng)});
        printf("x = [%.3f, %.3f, %.3f, %.3f, %.3f]\n", x(0), x(1), x(2), x(3), x(4));
    }
}
```

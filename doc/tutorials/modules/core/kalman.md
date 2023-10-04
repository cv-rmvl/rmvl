卡尔曼滤波模块 {#tutorial_modules_kalman}
============

@author 张华铨
@date 2022/10/11

@prev_tutorial{tutorial_modules_serial}

@next_tutorial{tutorial_modules_union_find}

@tableofcontents

------

相关类 rm::KalmanFilter

## 1. 卡尔曼滤波理论

### 1.1 卡尔曼滤波器是做什么的？

估计目标的当前位置，并以最高置信度获得当前位置。

如果有一辆车以恒定速度行驶，现在我得到两个数据：汽车的当前位置和汽车的当前距离传感器数据，根据匀速公式计算

<img src="https://s1.ax1x.com/2022/10/11/xNlnw6.png" width=680 height=380 />

由于自身的原因，它的位置不是由匀速运动公式得到的精确位置，它的距离传感器数据也不完全准确。两者都有一定的误差。那么我们现在如何估计汽车的实际位置呢？

对于两者的置信度，不难想象，环境越复杂，公式得到的位置置信度越低。传感器越先进，置信度就越高。如果我们现在有了两条信息的置信度，我们就可以根据卡尔曼滤波对这两条数据进行处理，得到具有最高数学置信度的实际位置。

### 1.2 重要变量的推导

#### 1.2.1 方差和协方差

方差

\f[
    Var(x)=\frac{\sum\limits_{i=1}^n(x_i-\bar{x})^2}{n}\tag1
\f]

协方差

\f[
    Cov(x,y)=\frac{\sum\limits_{i=1}^n(x_i-\bar{x})(y_i-\bar{y})}{n}\tag2
\f]

#### 1.2.2 递归

如果使用同一个传感器多次测量一个数据（每次都会产生一个随机误差），就可以得到

\f[
z_1,\ z_2,\ z_3\ \dots\ z_n
\f]

计算平均

\f[
    \begin{array}{lll}
    \hat x_n&=&\frac1n\sum\limits_{i=1}^nz_i\\
    &=&\frac1n\sum\limits_{i=1}^{n-1}z_i+\frac1nz_n\\
    &=&\frac{n-1}n\frac1{n-1}\sum\limits_{i=1}^{n-1}+\frac{1}{n}z_n\\
    &=&\frac{n-1}{n}\hat{x}_{n-1}+\frac{1}{n}z_n\\
    &=&\hat{x}_{n-1}+\frac{1}{n}(z_n-\hat{x}_{n-1})\\
    &=&\hat{x}_{n-1}+K_n(z_n-\hat{x}_{n-1})\\
    \end{array}\tag3
\f]

\f$K_n\f$: 卡尔曼增益

卡尔曼滤波的特点：只需要最后一帧的数据， **不需要** 保留更多的数据。

**若：**

- Error of estimation: \f$e_{EST}\f$
- Error of meaturement: \f$e_{MEA}\f$

**则：**

\f[
    K_n=\frac{e_{EST_{n-1}}}{e_{EST_{n-1}}+e_{MEA_n}}\tag4
\f]

- 根据公式\f$(3)\f$，可以得出以下结论

1. \f$e_{EST_{n-1}}\gg e_{MEA_n}\f$

\f[
    K_n\rightarrow1\\
    e\hat{x}_{n-1}=\hat{x}_{n-1}+z_n-\hat{x}_{n-1}=z_n\tag5
\f]

2. \f$e_{EST_{n-1}}\ll e_{MEA_n}\f$

\f[
    K_n\rightarrow0\\
    \hat{x}_n=\hat{x}_{n-1}\tag6
\f]

- Update:

\f[
    e_{EST_n}=(1-K_n)e_{EST_{n-1}}\tag7
\f]

@note \f$K_n\f$ 的导出过程与下面的 Kalman 公式的导出过程类似，这里不需要展开说明

#### 1.2.3 数据融合

如果用两种不同的仪器同时测量一个数据（测量误差方差不同但已知），并且得到两个测量值:\f$z_1，\ z_2\f$，我们如何选择最优估计值？

例如，如果\f$z_1，\ z_2\f$的误差满足正态分布：

\f[
    z_1=30,\ S_1=2\\
    z_2=32,\ S_2=4
\f]

\f[
    \begin{array}{lll}
    \hat{z}&=&z_1+K(z_2-z_1)\qquad K\text{ is the parameter to be solved}\\
    S_z^2&=&\text{Var}(z_1+K(z_2-z_1))\\
    &=&\text{Var}((1-K)z_1+Kz_2)\\
    &=&(1-K)^2\text{Var}(z_1)+K^2\text{Var}(z_2)\\
    &=&(1-K)^2S_1^2+K^2S_2^2
    \end{array}\tag8
\f]

令

\f[
    \frac{\text{d}}{\text{d}x}((1-K)^2S_1^2+K^2S_2^2)=0\tag9
\f]

求解公式\f$(9)\f$，我们可以得到

\f[
    K=\frac{S_1^2}{S_1^2+S_2^2}\tag{10}
\f]

将\f$K\f$代入下列公式

\f[
    \hat{z}=z_1+K(z_2-z_1)
\f]

则

\f[
    S_z^2=(1-K)^2S_1^2+K^2S_2^2\tag{11}
\f]

最终我们可以计算：

\f[
    K=0.2,\ z=30.4,\ S_z=1.79 
\f]

#### 1.2.4 状态空间方程

\f[
    \begin{array}{rrll}
    x_k&=&Ax_{k-1}+Bu_{k-1}+w_{k-1}&w\sim N(0,Q)\\
    z_k&=&Hx_k+v_k&v\sim N(0, R)
    \end{array}\tag{12}
\f]

例如

\f[
    \begin{array}{rrl}
    p_k&=&p_{k-1}+v_{k-1}t+\frac12at^2\\v_k&=&v_{k-1}+at
    \end{array}\tag{13}
\f]

将公式\f$(13)\f$转换为矩阵表示

\f[
    \begin{bmatrix}p_k\\v_k\end{bmatrix}
    =\begin{bmatrix}1&t\\0&1\end{bmatrix}
    \begin{bmatrix}p_{k-1}\\v_{k-1}\end{bmatrix}+
    \begin{bmatrix}\frac12t^2\\t\end{bmatrix}
    a+\begin{bmatrix}w_{p_k}\\w_{v_k}\end{bmatrix}\tag{14-1}
\f]

\f[
    \begin{bmatrix}z_{p_k}\\z_{v_k}\end{bmatrix}=
    \begin{bmatrix}1&0\\0&1\end{bmatrix}
    \begin{bmatrix}p_k\\v_k\end{bmatrix}+
    \begin{bmatrix}v_{p_k}\\v_{v_k}\end{bmatrix}\tag{14-2}
\f]

### 1.3 卡尔曼滤波的公式

#### 1.3.1 预测

\f[
    \begin{array}{rml}
    \hat x^-&=&A\hat x\\
    P^-&=&APA^T+Q
    \end{array}\tag{15}
\f]

#### 1.3.2 更新

\f[
    \begin{array}{rml}
    K&=&P^-H^T(HP^-H^T+R)^{-1}\\
    \hat x&=&\hat x^-+K(z-H\hat x^-)\\
    P&=&(I-KH)P^-\end{array}\tag{16}
\f]

## 2. 卡尔曼滤波模块的用法

### 2.1 如何配置

首先必须要寻找 RMVL 包，即 `find_package(RMVL [OPTIONS])`，之后可直接在中使用在 CMakeLists.txt 中链接库

```cmake
target_link_libraries(
    xxx
    rmvl_core
)
```

### 2.2 如何使用

#### 2.2.1 包含头文件

```cpp
#include <rmvl/core/kalman.hpp>
```

#### 2.2.2 创建并初始化卡尔曼滤波器对象

取
- 数据类型 `Tp` = `double`
- 状态量阶数 `StateDim` = `4`
- 观测量阶数 `MeatureDim` = `4`
- 控制量阶数 `ControlDim` = `1`

可以得到以下代码

```cpp
rm::KalmanFilter<double, 4, 4, 1> filter; // 简写可以写成 rm::KF44d filter

filter.setR(/* ... */);
filter.setQ(/* ... */);
cv::Vec<float, state> init_vec = {/* ... */};
filter.init(init_vec, pred_err);
```

#### 2.2.3 运行

```cpp
filter.setA(/* ... */);
// Prediction
filter.predict();
// Correction
auto corr = filter.correct(/* ... */);
```

---------

### 参考文献 {#ref_paper}

- 卡尔曼滤波 @cite kalman
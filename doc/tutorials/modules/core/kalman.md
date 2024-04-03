卡尔曼滤波 {#tutorial_modules_kalman}
============

@author 张华铨
@author 赵曦
@date 2024/03/24
@version 2.0
@brief 卡尔曼滤波详细公式推导

@prev_tutorial{tutorial_modules_runge_kutta}

@next_tutorial{tutorial_modules_union_find}

@tableofcontents

------

相关类 rm::KalmanFilter

\f[
\def\red#1{\color{red}{#1}}
\def\teal#1{\color{teal}{#1}}
\def\green#1{\color{green}{#1}}
\def\transparent#1{\color{transparent}{#1}}
\def\orange#1{\color{orange}{#1}}
\def\Var{\mathrm{Var}}
\def\Cov{\mathrm{Cov}}
\def\tr{\mathrm{tr}}
\def\formular#1{\text{(#1)}}
\f]

### 1. 卡尔曼滤波

#### 1.1 简介

卡尔曼滤波器是一个 Optimal Recursive Data Processing Algorithm，即最优化递归数字处理算法，与其说滤波器，倒不如说这是一个 **观测器** 。简单来说，卡尔曼滤波器能够估计目标的当前位置，并结合观测结果，以最高置信度获得当前位置。

#### 1.2 初尝递归算法 {#kalman_recursive}

<span style="color: green">**示例**</span>

我们需要测量一个硬币的长度\f$x\f$，现测出了如下结果

\f[\begin{align}z_1&=30.2\mathrm{mm}\\z_2&=29.7\mathrm{mm}\\z_3&=30.1\mathrm{mm}\\&\vdots\end{align}\f]

我们要利用这些观测结果来估计硬币的真实长度，很自然的我们可以想到使用<span style="color: red">取平均</span>的方式，我们可以利用 \f$k\f$ 个观测值，求出硬币长度在第 \f$k\f$ 次的估计值 \f$\hat x_k\f$。

\f[\hat x_k=\frac{z_1+z_2+\cdots+z_k}k\tag{1-1}\f]

这是一条非常简单的取算数平均的公式，但是我们为了估计硬币的长度，需要用到所有的观测值，例如我们已经测了 5 次硬币的长度，并且使用公式 \f$\formular{1-1}\f$ 得到了第 5 次的平均值（长度的估计值），在测完第 6 次长度准备计算第 6 次估计值的时候，使用公式 \f$\formular{1-1}\f$ 还需要重新使用前 5 次的观测值。当观测次数非常高的时候，计算的压力就逐渐高起来了。

针对这一问题，我们可以改写公式 \f$\formular{1-1}\f$

\f[\begin{align}\hat x_k&=\frac1k(z_1+z_2+\cdots+z_k)\\
&=\frac1k(z_1+z_2+\cdots+z_{k-1})+\frac1kz_k\\
&=\frac{k-1}k·\frac1{\red{k-1}}\red{(z_1+z_2+\cdots+z_{k-1})}+\frac1kz_k\\
&=\frac{k-1}k\red{\hat x_{k-1}}-\frac1kz_k\\&=\hat x_{k-1}-\frac1k\hat x_{k-1}+\frac1kz_k\\
&=\hat x_{k-1}+\frac1k(z_k-\hat x_{k-1})\tag{1-2a}\end{align}\f]

这样我们就把硬币长度的估计值，改写成由上一次估计值和当前观测值共同作用的形式。并且我们发现，随着测量次数 \f$k\f$ 增大，\f$\frac1k\f$ 趋向于 \f$0\f$，\f$\hat x_k\f$ 趋向于 \f$\hat x_{k-1}\f$，这也就是说，随着 \f$k\f$ 增长，测量结果将不再重要。

为了不失一般性，我们把公式 \f$\formular{1-2a}\f$ 的结果改写成以下形式。

\f[\boxed{\hat x_k=\hat x_{k-1}+\red{K_k}(z_k-\hat x_{k-1})}\tag{1-2b}\f]

其中 \f$K_k\f$ 表示 Kalman Gain，即卡尔曼增益。对于 \f$K_k\f$，我们先给出他的计算结果。

\f[K_k=\frac{e_{{EST}_{k-1}}}{e_{{EST}_{k-1}}+e_{{MEA}_k}}\tag{1-3}\f]

其中，\f$e_{{EST}_{k-1}}\f$ 表示第 \f$k-1\f$ 次的估计误差，\f$e_{{MEA}_k}\f$ 表示第 \f$k\f$ 次的测量误差。可以得到，在 \f$k\f$ 时刻，

1. 若 \f$e_{{EST}_{k-1}}\gg e_{{MEA}_k}\f$，则 \f$K_k\to1\f$，\f$\hat x_k=\hat x_{k-1}+1\times(z_k-\hat x_{k-1})=z_k\f$
2. 若 \f$e_{{EST}_{k-1}}\ll e_{{MEA}_k}\f$，则 \f$K_k\to0\f$，\f$\hat x_k=\hat x_{k-1}+0\times(z_k-\hat x_{k-1})=\hat x_{k-1}\f$

#### 1.3 数据融合 {#kalman_data_fusion}

现在我们可以使用公式 \f$\formular{1-2b}\f$ 的思想来研究数据融合。如果有一辆车以恒定速度行驶，现在得到了两个数据：

- 根据匀速公式计算得到的汽车当前位置（算出来的，记作 \f$\teal{x_1}\f$）
- 汽车的当前距离传感器数据（测出来的，记作 \f$\red{x_2}\f$）

<img src="https://s1.ax1x.com/2022/10/11/xNlnw6.png" width=680 height=380 />

由于自身的原因，它的位置并不是由匀速运动公式得到的精确位置，它的距离传感器数据也不完全准确。两者都有一定的误差。那么我们现在如何估计汽车的实际位置呢？

我们使用公式 \f$\formular{1-2b}\f$ 的思想，得到估计值

\f[\hat x=\teal{x_1}+\green{K_k}(\red{x_2}-\teal{x_1})\tag{1-4}\f]

我们的目标很明确，希望求出这个 \f$\green{K_k}\f$，使得估计值 \f$\hat x\f$ 是最优的，即 \f$\hat x\f$ 的方差 \f$\Var(\hat x)\f$ 最小，即 \f$\hat x\to x_{实际值}\f$。

不妨令 \f$\left\{\begin{align}\teal{x_1=30\mathrm m\qquad\sigma_1=2\mathrm m}\\\red{x_2=32\mathrm m\qquad\sigma_2=4\mathrm m}\end{align}\right.\f$，则有

\f[\begin{align}\Var(\hat x)&=\Var[x_1+K_k(x_2-x_1)]\\&=\Var(x_1-K_kx_1+K_kx_2)\\
&=\Var[(1-K_k)x_1+K_kx_2]\\由x_1和x_2相互独立\quad&=\Var[(1-k)x_1]+\Var(K_kx_2)\\
&=(1-K_k)^2\Var(x_1)+K_k^2\Var(x_2)\\
&=(1-K_k)^2\sigma_1^2+K_k^2\sigma_2^2\tag{1-5}\end{align}\f]

要求 \f$\Var(\hat x)\f$ 的最小值，只需令 \f$\mathrm \Var(\hat x)'|_{K_k}=0\f$ 即可，即：

\f[\frac{\mathrm d\Var(\hat x)}{\mathrm dK_k}=-2(1-K_k)\sigma_1^2+2K_k\sigma_2^2=0\tag{1-6}\f]

即

\f[K_k=\frac{\sigma_1^2}{\sigma_1^2+\sigma_2^2}\tag{1-7}\f]

代入数据后，有

\f[\left\{\begin{align}
K_k&=\frac{2^2}{2^2+4^2}=0.2\\
\hat x&=30+0.2(32-30)=30.4\mathrm m\quad\orange{理论最优解}\\
\sigma_{\hat x}^2&=(1-0.2)^22^2+0.2^24^2=3.2\quad(\sigma_{\hat x}=1.79)
\end{align}\right.\f]

#### 1.4 协方差矩阵 {#kalman_covariance_matrix}

<center>

|  数据   |   \f$x\f$    |   \f$y\f$    |   \f$z\f$    |
| :-----: | :----------: | :----------: | :----------: |
| \f$1\f$ |  \f$x_1\f$   |  \f$y_1\f$   |  \f$z_1\f$   |
| \f$2\f$ |  \f$x_2\f$   |  \f$y_2\f$   |  \f$z_2\f$   |
| \f$3\f$ |  \f$x_3\f$   |  \f$y_3\f$   |  \f$z_3\f$   |
| 平均值  | \f$\bar x\f$ | \f$\bar y\f$ | \f$\bar z\f$ |

</center>

**方差**

表明单个变量的波动程度

\f[\begin{align}
\Var(x)&=\frac13\left[(x_1-\bar x)^2+(x_2-\bar x)^2+(x_3-\bar x)^2\right]=\frac13\sum_{i=1}^3(x-\bar x)^2\stackrel{\triangle}{=}\sigma_x^2\\
\Var(y)&=\frac13\left[(y_1-\bar y)^2+(y_2-\bar y)^2+(y_3-\bar y)^2\right]=\frac13\sum_{i=1}^3(y-\bar y)^2\stackrel{\triangle}{=}\sigma_y^2\\
\Var(z)&=\frac13\left[(z_1-\bar z)^2+(z_2-\bar z)^2+(z_3-\bar z)^2\right]=\frac13\sum_{i=1}^3(z-\bar z)^2\stackrel{\triangle}{=}\sigma_z^2
\end{align}\f]

一般的，方差具有以下推导公式

\f[\begin{align}\Var(X)&=\frac1n\sum_{i=1}^n(X-\bar X)^2\\&=\frac1n\sum_{i=1}^n(X-EX)^2\\
定义\quad&=\green{E\left[(X-EX)^2\right]}\\
&=E\left[X^2-2X·EX+(EX)^2\right]\\&=EX^2-2EX·EX+(EX)^2\\
&=\red{EX^2-(EX)^2}\tag{1-8}\end{align}\f]

**协方差**

表明变量之间的相关性，为正数表示正相关，为负数表示负相关，绝对值越大表示相关性越强

\f[\begin{align}
\Cov(x,y)&=\frac13\left[(x_1-\bar x)(y_1-\bar y)+(x_2-\bar x)(y_2-\bar y)+(x_3-\bar x)(y_3-\bar y)\right]\\
&=\frac13\sum_{i=1}^3(x-\bar x)(y-\bar y)\stackrel{\triangle}{=}\sigma_x\sigma_y\\
\Cov(y,z)&=\frac13\left[(y_1-\bar y)(z_1-\bar z)+(y_2-\bar y)(z_2-\bar z)+(y_3-\bar y)(z_3-\bar z)\right]\\
&=\frac13\sum_{i=1}^3(y-\bar y)(z-\bar z)\stackrel{\triangle}{=}\sigma_y\sigma_z\\
\Cov(x,z)&=\frac13\left[(x_1-\bar x)(z_1-\bar z)+(x_2-\bar x)(z_2-\bar z)+(x_3-\bar x)(z_3-\bar z)\right]\\
&=\frac13\sum_{i=1}^3(x-\bar x)(z-\bar z)\stackrel{\triangle}{=}\sigma_x\sigma_z
\end{align}\f]

一般的，协方差具有以下推导公式

\f[\begin{align}\Cov(X,Y)
&=\frac1n\sum_{i=1}^n(X-\bar X)(Y-\bar Y)\\
&=\frac1n\sum_{i=1}^n(X-EX)(Y-EY)\\
定义\quad&=\green{E\left[(X-EX)(Y-EY)\right]}\\
&=E(XY-X·EY-Y·EX+EX·EY)\\
&=E(XY)-EX·EY-EY·EX+EX·EY\\
&=\red{E(XY)-EXEY}
\tag{1-9}\end{align}\f]

**协方差矩阵**

按照协方差的定义出发，可以有

\f[\begin{align}\Cov(x_i,x_j)
&=E\left[(x_i-Ex_i)(x_j-Ex_j)\right]=\sigma_{x_i}\sigma_{x_j}\\
令误差e_i=x_i-E(x_i)\quad&=\green{E(e_ie_j)}
\tag{1-10a}\end{align}\f]

若取 \f$i,j=1,2,\cdots,n\f$，则可以得到协方差矩阵 \f$P\f$，表示为

\f[\begin{align}P&=\begin{bmatrix}
\sigma_{x_1}^2&\sigma_{x_1}\sigma_{x_2}&\cdots&\sigma_{x_1}\sigma_{x_n}\\
\sigma_{x_2}\sigma_{x_1}&\sigma_{x_2}^2&\cdots&\sigma_{x_2}\sigma_{x_n}\\
\vdots&\vdots&\ddots&\vdots\\
\sigma_{x_n}\sigma_{x_1}&\sigma_{x_n}\sigma_{x_2}&\cdots&\sigma_{x_n}^2
\end{bmatrix}\\&=\begin{bmatrix}Ee_1^2&Ee_1Ee_2&\cdots&Ee_1Ee_n\\
Ee_2Ee_1&Ee_2^2&\cdots&Ee_2Ee_n\\\vdots&\vdots&\ddots&\vdots\\
Ee_nEe_1&Ee_nEe_2&\cdots&Ee_n^2\end{bmatrix}\\&=
E\begin{bmatrix}e_1^2&e_1e_2&\cdots&e_1e_n\\
e_2e_1&e_2^2&\cdots&e_2e_n\\\vdots&\vdots&\ddots&\vdots\\
e_ne_1&e_ne_2&\cdots&e_n^2\end{bmatrix}=\green{E\left(\pmb e\pmb e^T\right)}\tag{1-10b}\end{align}\f]

#### 1.5 卡尔曼增益推导 {#kalman_gain_derivate}

对于一个系统，可以使用状态空间方程来描述其运动

\f[\left\{\begin{align}
\dot{\pmb x}&=F\pmb x+G\pmb u\\
\pmb z&=H\pmb x
\end{align}\right.\tag{1-11}\f]

根据 @ref equations_runge_kutta 中精确解的公式 \f$\text{(b)}\f$，我们可以知道状态空间方程的解的表达式，即

\f[\pmb x(t)=e^{F(t-t_0)}\pmb x(t_0)+\int_{t_0}^te^{F(t-\tau)}G\pmb u(\tau)\mathrm d\tau\tag{1-12}\f]

以\f$T\f$为周期离散采样+零阶保持器，可以得到

\f[\begin{align}
\pmb x(t_k)&=e^{F(t_k-t_{k-1})}\pmb x(t_{k-1})+\int_{t_{k-1}}^{t_k}e^{F(t_k-\tau)}G\pmb u(\tau)\mathrm d\tau\\
&=e^{FT}\pmb x(t_{k-1})+\int_{t_{k-1}}^{t_k}e^{F(t_k-\tau)}\mathrm d\tau·G\pmb u(t_{k-1})\\
&=e^{FT}\pmb x(t_{k-1})+\left[-F^{-1}e^{F(t_k-\tau)}\right]_{t_{k-1}}^{t_k}·G\pmb u(t_{k-1})\\
&=e^{FT}\pmb x(t_{k-1})+F^{-1}(e^{FT}-I)G\pmb u(t_{k-1})\\
\pmb x_k&=\red{e^{FT}}\pmb x_{k-1}+\green{F^{-1}(e^{FT}-I)G}\pmb u_{k-1}\\
简记为\quad\pmb x_k&=\red A\pmb x_{k-1}+\green B\pmb u_{k-1}\tag{1-13a}\end{align}\f]

因此我们可以得到离散系统的状态空间方程

\f[\left\{\begin{align}
\pmb x_k&=A\pmb x_{k-1}+B\pmb u_{k-1}\\
\pmb z_k&=H\pmb x_k
\end{align}\right.\tag{1-13b}\f]

这其实是个不准确的结果，因为如果我们考虑上噪声，公式 \f$\formular{1-13b}\f$ 应该改写为

\f[\left\{\begin{align}
\pmb x_k&=A\pmb x_{k-1}+B\pmb u_{k-1}\red{+\pmb w_{k-1}}&p(\pmb w)\sim N(0,Q)\\
\pmb z_k&=H\pmb x_k\red{+\pmb v_k}&p(\pmb v)\sim N(0,R)
\end{align}\right.\tag{1-13c}\f]

- \f$w_{k-1}\f$ 称为过程噪声，来自于算不准的部分，其中 \f$Q\f$ 满足以下形式
  \f[Q=E(\pmb w\pmb w^T)=\begin{bmatrix}\sigma_{w_1}^2&\sigma_{w_1}\sigma_{w_2}&\cdots&\sigma_{w_1}\sigma_{w_n}\\
  \sigma_{w_2}\sigma_{w_1}&\sigma_{w_2}^2&\cdots&\sigma_{w_2}\sigma_{w_n}\\
  \vdots&\vdots&\ddots&\vdots\\\sigma_{w_n}\sigma_{w_1}&\sigma_{w_n}\sigma_{w_2}&\cdots&\sigma_{w_n}^2\end{bmatrix}\f]
  称为过程噪声协方差矩阵
- \f$v_{k-1}\f$ 称为测量噪声，来自于测不准的部分，其中 \f$R\f$ 满足以下形式
  \f[R=E(\pmb v\pmb v^T)=\begin{bmatrix}\sigma_{v_1}^2&\sigma_{v_1}\sigma_{v_2}&\cdots&\sigma_{v_1}\sigma_{v_n}\\
  \sigma_{v_2}\sigma_{v_1}&\sigma_{v_2}^2&\cdots&\sigma_{v_2}\sigma_{v_n}\\
  \vdots&\vdots&\ddots&\vdots\\\sigma_{v_n}\sigma_{v_1}&\sigma_{v_n}\sigma_{v_2}&\cdots&\sigma_{v_n}^2\end{bmatrix}\f]
  称为测量噪声协方差矩阵

但是，这两个误差我们无从得知，我们只能使用公式 \f$\formular{1-13b}\f$ 的形式进行近似估计，通过 \f$\pmb x_k=A\pmb x_{k-1}+B\pmb u_{k-1}\f$ 算出来的 \f$\pmb x_k\f$ 称为<span style="color: red">先验状态估计</span>，一般写为

\f[\red{\hat{\pmb x}_k^-=A\pmb x_{k-1}+B\pmb u_{k-1}\tag{1-14}}\f]

通过 \f$\pmb z_k=H\pmb x_k\f$ 得到的 \f$\pmb x_k\f$ 是测出来的结果，记为 \f$\hat{\pmb x}_{k_{MEA}}\f$，即

\f[\hat{\pmb x}_{k_{MEA}}=H^+\pmb z_k\tag{1-15}\f]

因为 \f$H^{-1}\f$ 可能不存在，这里先使用 M-P 广义逆 \f$H^+\f$ 来表示。

@note 对于一个线性方程组 \f[A\pmb x=\pmb b\f]必定存在最小二乘解 \f[\pmb x=(A^TA)^{-1}A^T\pmb b\f]可以令 \f$A^+=(A^TA)^{-1}A^T\f$ 来表示 Moore-Penrose 广义逆，即\f[\pmb x=A^+\pmb b\f]当 \f$A\f$ 可逆时，\f$A^+=A^{-1}\f$.

目前的两个结果 \f$\hat{\pmb x}_k^-\f$ 和 \f$\hat{\pmb x}_{k_{MEA}}\f$ 都不准确，因此可以回顾 @ref kalman_data_fusion 的部分，在公式 \f$\formular{1-4}\f$ 中使用了算出来的 \f$\teal{x_1}\f$ 和测出来的 \f$\red{x_2}\f$ 得到了最优估计值 \f$\hat x\f$，为此我们可以仿照这一步骤来求出离散系统状态的最优估计值 \f$\hat{\pmb x}_k\f$，称为<span style="color: red">后验状态估计</span>。

\f[\begin{align}\hat{\pmb x}_k&=\hat{\pmb x}_k^-+\green{G_k}(\hat{\pmb x}_{k_{MEA}}-\hat{\pmb x}_k^-)\\
&=\hat{\pmb x}_k^-+\green{G_k}(H^+\pmb z_k-\hat{\pmb x}_k^-)\tag{1-16}\end{align}\f]

令 \f$G_k=K_kH\f$，可以得到

\f[\red{\hat{\pmb x}_k=\hat{\pmb x}_k^-+K_k(\pmb z_k-H\hat{\pmb x}_k^-)\tag{1-17}}\f]

我们的目标依然很明确，希望求出这个 \f$\green{K_k}\f$，使得后验状态估计值 \f$\hat{\pmb x}_k\f$ 是最优的，即 \f$\hat{\pmb x}_k\f$ 的方差 \f$\Var(\hat{\pmb x}_k)\f$ 最小，即 \f$\hat{\pmb x}_k\to \pmb x_k\f$。如何衡量 \f$\hat{\pmb x}_k\f$ 的方差 \f$\Var(\hat{\pmb x}_k)\f$？我们可以令状态误差 \f$\pmb e_k=\pmb x_k-\hat{\pmb x}_k\f$，那么此时 \f$p(\pmb e_k)\sim N(0,P_k)\f$，其中 \f$P_k\f$ 是误差协方差矩阵，并且满足

\f[P_k=\begin{bmatrix}\sigma_{e_1}^2&\cdots&\sigma_{e_1}\sigma_{e_n}\\\vdots&&\vdots\\\sigma_{e_n}\sigma_{e_1}&\cdots&\sigma_{e_n}^2\end{bmatrix}\tag{1-18}\f]

方差 \f$\Var(\hat{\pmb x}_k)\f$ 最小，即 \f$\sum\limits_{i=1}^{n}\sigma_{e_i}^2\f$ 最小，即 \f$P_k\f$ 的迹 \f$\tr(P_k)\f$ 最小。

对于状态误差，有

\f[\begin{align}
\pmb e_k&=\pmb x_k-\hat{\pmb x}_k=\pmb x_k-\left(\hat{\pmb x}_k^-+K_k(\pmb z_k-H\hat{\pmb x}_k^-)\right)\\
&=\pmb x_k-\hat{\pmb x}_k^--K_k\pmb z_k+K_kH\hat{\pmb x}_k^-\\
&=\pmb x_k-\hat{\pmb x}_k^--K_k(H\pmb x_k+\pmb v_k)+K_kH\hat{\pmb x}_k^-\\
&=\pmb x_k-\hat{\pmb x}_k^--K_kH\pmb x_k-K_k\pmb v_k+K_kH\hat{\pmb x}_k^-\\
&=(\pmb x_k-\hat{\pmb x}_k^-)-K_kH(\pmb x_k-\hat{\pmb x}_k^-)-K_k\pmb v_k\\
&=(I-K_kH)\teal{(\pmb x_k-\hat{\pmb x}_k^-)}-K_k\pmb v_k\\
&=(I-K_kH)\teal{\pmb e_k^-}-K_k\pmb v_k\tag{1-19}\end{align}\f]

则误差协方差矩阵可以表示为

\f[\begin{align}P_k
&=E\left(\pmb e_k\pmb e_k^T\right)=E\left[(\pmb x_k-\hat{\pmb x}_k)(\pmb x_k-\hat{\pmb x}_k)^T\right]\\
&=E\left[\left((I-K_kH)\teal{\pmb e_k^-}-K_k\pmb v_k\right)\left((I-K_kH)\teal{\pmb e_k^-}-K_k\pmb v_k\right)^T\right]\\
&=E\left[\left(\green{(I-K_kH)\pmb e_k^-}-\red{K_k\pmb v_k}\right)\left(\teal{{\pmb e_k^-}^T(I-K_kH)^T}-\orange{\pmb v_k^TK_k^T}\right)\right]\\
&=E\left[\green{(I-K_kH)\pmb e_k^-}\teal{{\pmb e_k^-}^T(I-K_kH)^T}-\green{(I-K_kH)\pmb e_k^-}\orange{\pmb v_k^TK_k^T}-\right.\\
&\transparent=\left.\red{K_k\pmb v_k}\teal{{\pmb e_k^-}^T(I-K_kH)^T}+\red{K_k\pmb v_k}\orange{\pmb v_k^TK_k^T}\right]\\
&=\red{E(}(I-K_kH)\pmb e_k^-{\pmb e_k^-}^T(I-K_kH)^T\red{)}-\\
&\transparent=\red{E(}(I-K_kH)\pmb e_k^-\pmb v_k^TK_k^T\red{)}-\\
&\transparent=\red{E(}K_k\pmb v_k{\pmb e_k^-}^T(I-K_kH)^T\red{)}+\\
&\transparent=\red{E(}K_k\pmb v_k\pmb v_k^TK_k^T\red{)}\\
提出常数的期望\quad&=(I-K_kH)\red{E(}\pmb e_k^-{\pmb e_k^-}^T\red{)}(I-K_kH)^T-\\
&\transparent=(I-K_kH)\red{E(}\pmb e_k^-\pmb v_k^T\red{)}K_k^T-\\
&\transparent=K_k\red{E(}\pmb v_k{\pmb e_k^-}^T\red{)}(I-K_kH)^T+\\
&\transparent=K_k\red{E(}\pmb v_k\pmb v_k^T\red{)}K_k^T\\
&=(I-K_kH)\red{P_k^-}(I-K_kH)^T+\\
\pmb e_k^-和{\pmb v_k}^T相互独立\quad&\transparent=0+\\
且期望为0\quad&\transparent=0+\\
&\transparent=K_k\red{R}K_k^T\\
&=\left(\green{P_k^-}-\red{K_kHP_k^-}\right)\left(\teal I-\orange{H^TK_k^T}\right)+K_kRK_k^T\\
&=\green{P_k^-}-\red{K_kHP_k^-}-\green{P_k^-}\orange{H^TK_k^T}+\red{K_kHP_k^-}\orange{H^TK_k^T}+K_kRK_k^T\\
P_k^-对称\quad&=P_k^--K_kHP_k^--\left(K_kHP_k^-\right)^T+K_kHP_k^-H^TK_k^T+K_kRK_k^T
\tag{1-20}\end{align}\f]

因此，\f$P_k\f$ 的迹可以表示为

\f[\tr(P_k)=\tr(P_k^-)-2\tr(K_kHP_k^-)+\tr(K_kHP_k^-H^TK_k^T)+\tr(K_kRK_k^T)\tag{1-21}\f]

希望 \f$\tr(P_k)\f$ 有最小值，则需要对 \f$K_k\f$ 求导，则有

\f[\frac{\mathrm d\tr(P_k)}{\mathrm dK_k}=0-2\frac{\mathrm d\tr(K_kHP_k^-)}{\mathrm dK_k}+\frac{\mathrm d\tr(K_kHP_k^-H^TK_k^T)}{\mathrm dK_k}+\frac{\mathrm d\tr(K_kRK_k^T)}{\mathrm dK_k}=0\tag{1-22}\f]

对于形如 \f$\frac{\mathrm d\tr(AB)}{A}\f$ 的导数计算，可以通过拿一个 2 阶矩阵作为例子

> 令 \f[A=\begin{bmatrix}a_{11}&a_{12}\\a_{21}&a_{22}\end{bmatrix},\quad B=\begin{bmatrix}b_{11}&b_{12}\\b_{21}&b_{22}\end{bmatrix}\f]
> 则 \f[AB=\begin{bmatrix}a_{11}b_{11}+a_{12}b_{21}&a_{11}b_{21}+a_{12}b_{22}\\a_{21}b_{11}+a_{22}b_{21}&a_{21}b_{12}+a_{22}b_{22}\end{bmatrix}\f]
> 则 \f[\tr(AB)=a_{11}b_{11}+a_{12}b_{21}+a_{21}b_{12}+a_{22}b_{22}\f]
> 则 \f[\frac{\mathrm d\tr(AB)}{\mathrm dA}=\begin{bmatrix}\frac{\partial\tr(AB)}{\partial a_{11}}&\frac{\partial\tr(AB)}{\partial a_{12}}\\\frac{\partial\tr(AB)}{\partial a_{21}}&\frac{\partial\tr(AB)}{\partial a_{22}}\end{bmatrix}=\begin{bmatrix}b_{11}&b_{21}\\b_{12}&b_{22}\end{bmatrix}=B^T\f]

因此可以得到

\f[\frac{\mathrm d\tr(AB)}{A}=B^T\tag{1-23a}\f]

同理，我们也能验证如下结论

\f[\begin{align}\frac{\mathrm d\tr(ABA^T)}{A}&=AB+AB^T\\
当B对称时\quad&=2AB\end{align}\tag{1-23b}\f]

那么，公式\f$\formular{1-22}\f$可以写为

\f[\begin{align}2\frac{\mathrm d\tr(K_kHP_k^-)}{\mathrm dK_k}
&=\frac{\mathrm d\tr(K_kHP_k^-H^TK_k^T)}{\mathrm dK_k}+\frac{\mathrm d\tr(K_kRK_k^T)}{\mathrm dK_k}\\
2{P_k^-}^TH^T&=2K_kHP_k^-H^T+2K_kR\\
P_k^-H^T&=K_k\left(HP_k^-H^T+R\right)
\tag{1-24}\end{align}\f]

最终可以得到误差协方差矩阵的迹最小时的<span style="color: red">卡尔曼增益</span> \f$K_k\f$ 的表达式

\f[\red{K_k=P_k^-H^T\left(HP_k^-H^T+R\right)^{-1}\tag{1-25}}\f]

**讨论**

- 当 \f$R\f$ 很大时，\f$K_k\to0,\quad\hat{\pmb x}_k=\hat{\pmb x}_k^-+0(\pmb z_k-H\hat{\pmb x}_k^-)=\red{\hat{\pmb x}_k^-}\f$
- 当 \f$R\f$ 很小时，\f$K_k\to H^+,\quad\hat{\pmb x}_k=\hat{\pmb x}_k^-+H^+(\pmb z_k-H\hat{\pmb x}_k^-)=H^+\pmb z_k=\red{\hat{\pmb x}_{k_{MEA}}}\f$

#### 1.6 误差协方差矩阵 {#kalman_error_covairance_matrix}

我们在计算卡尔曼增益 \f$K_k\f$ 的时候出现了 \f$E\left(\pmb e_k^-{\pmb e_k^-}^T\right)\f$，并使用 \f$P_k^-\f$ 进行表示，代表先验的误差协方差矩阵，现在要做的就是求出先验误差协方差矩阵 \f$P_k^-\f$，同样可以由协方差矩阵的定义出发。

首先先求 \f$\pmb e_k^-\f$

\f[\begin{align}\pmb e_k^-&=\pmb x_k-\hat{\pmb x}_k^-\\
&=A\pmb x_{k-1}+B\pmb u_{k-1}+\pmb w_{k-1}-A\hat{\pmb x}_{k-1}-B\pmb u_{k-1}\\
&=A(\pmb x_{k-1}-\hat{\pmb x}_{k-1})+\pmb w_{k-1}\\
&=A\pmb e_{k-1}+\pmb w_{k-1}\tag{1-26}\end{align}\f]

因此先验误差协方差矩阵 \f$P_k^-\f$ 可以表示为

\f[\begin{align}P_k^-
&=E\left(\pmb e_k^-{\pmb e_k^-}^T\right)=E\left[(A\pmb e_{k-1}+\pmb w_{k-1})(A\pmb e_{k-1}+\pmb w_{k-1})^T\right]\\
&=E\left[(A\pmb e_{k-1}+\pmb w_{k-1})(\pmb e_{k-1}^TA^T+\pmb w_{k-1}^T)\right]\\
&=E\left[A\pmb e_{k-1}\pmb e_{k-1}^TA^T+A\pmb e_{k-1}\pmb w_{k-1}^T+\pmb w_{k-1}\pmb e_{k-1}^TA^T+\pmb w_{k-1}\pmb w_{k-1}^T\right]\\
&=AE(\pmb e_{k-1}\pmb e_{k-1}^T)A^T+AE\pmb e_{k-1}E\pmb w_{k-1}^T+E\pmb w_{k-1}E\pmb e_{k-1}^TA^T+E(\pmb w_{k-1}\pmb w_{k-1}^T)\\
&=AP_{k-1}A^T+0+0+Q
\tag{1-27}\end{align}\f]

最终可以得到<span style="color: red">先验误差协方差矩阵</span>的表达式

\f[\red{P_k^-=AP_{k-1}A^T+Q\tag{1-28}}\f]

在求解 \f$P_k^-\f$ 的时候用到了 \f$P_{k-1}\f$，因此需要进一步求解 \f$P_k\f$，从而为下一次 \f$P_{k+1}^-\f$ 所使用。

由 @ref kalman_gain_derivate 的公式 \f$\formular{1-20}\f$ 可以得到

\f[\begin{align}P_k
&=\green{P_k^-}-\red{K_kHP_k^-}-\green{P_k^-}\orange{H^TK_k^T}+\red{K_kHP_k^-}\orange{H^TK_k^T}+K_kRK_k^T\\
&=P_k^--K_kHP_k^--P_k^-H^TK_k^T+K_k(HP_k^-H^T+R)K_k^T\\
代入\formular{1-25}\quad&=P_k^--K_kHP_k^--P_k^-H^TK_k^T+P_k^-H^TK_k^T\\
&=P_k^--K_kHP_k^-\tag{1-29}\end{align}\f]

即所谓后验误差协方差矩阵 \f$P_k\f$

\f[\red{P_k=(I-K_kH)P_k^-\tag{1-30}}\f]

#### 1.7 汇总 {#kalman_filter_fomulars}

至此，Kalman Filter 的 5 大公式已经全部求出，分别是公式 \f$\formular{1-14}\f$、公式 \f$\formular{1-17}\f$、公式 \f$\formular{1-25}\f$、公式 \f$\formular{1-28}\f$ 和公式 \f$\formular{1-30}\f$

按照处理顺序，卡尔曼滤波器划分为两个部分

**① 预测**

1. <span style="color: teal">先验状态估计</span>
   \f[\hat{\pmb x}_k^-=A\pmb x_{k-1}+B\pmb u_{k-1}\f]

2. <span style="color: teal">计算先验误差协方差</span>
   \f[P_k^-=AP_{k-1}A^T+Q\f]

**② 校正（更新）**

1. <span style="color: teal">计算卡尔曼增益</span>
   \f[K_k=P_k^-H^T\left(HP_k^-H^T+R\right)^{-1}\f]

2. <span style="color: teal">后验状态估计</span>
   \f[\hat{\pmb x}_k=\hat{\pmb x}_k^-+K_k(\pmb z_k-H\hat{\pmb x}_k^-)\f]

3. <span style="color: teal">更新后验误差协方差</span>
   \f[P_k=(I-K_kH)P_k^-\f]

---

### 2. 卡尔曼滤波模块的用法

#### 2.1 如何配置

首先必须要寻找 RMVL 包，即 `find_package(RMVL [OPTIONS])`，之后可直接在中使用在 CMakeLists.txt 中链接库

```cmake
target_link_libraries(
  xxx
  PUBLIC rmvl_core
)
```

#### 2.2 如何使用

##### 2.2.1 包含头文件

```cpp
#include <rmvl/core/kalman.hpp>
```

##### 2.2.2 创建并初始化卡尔曼滤波器对象

取
- 数据类型 `Tp` = `double`
- 状态量阶数 `StateDim` = `4`
- 观测量阶数 `MeatureDim` = `2`
- 无系统输入

可以得到以下代码

```cpp
rm::KalmanFilter<double, 4, 2> filter; // 简写可以写成 rm::KF42d filter

filter.setR(/* ... */);
filter.setQ(/* ... */);
cv::Vec4f init_vec = {/* ... */};
filter.init(init_vec, pred_err);
```

##### 2.2.3 运行

```cpp
filter.setA(/* ... */);
filter.setH(/* ... */);
// Prediction
filter.predict();
// Correction
cv::Vec2f zk = {/* ... */};
auto corr = filter.correct(zk);
```

---------

#### 参考文献 {#ref_paper}

- 卡尔曼滤波 @cite kalman
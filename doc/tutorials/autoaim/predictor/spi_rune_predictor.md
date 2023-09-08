k 步前向预估神符预测 {#tutorial_autoaim_spi_rune_predictor}
============

@author 黄昊睿
@author 赵曦
@date 2023/07/16

@prev_tutorial{tutorial_autoaim_gyro_predictor}

@next_tutorial{tutorial_autoaim_how_to_use_decider}

@tableofcontents

------

相关类 rm::SpiRunePredictor

### 使用此方法的前提

#### 前提 1 {#premise_1}

能量机关角度位置因为不受其他输入影响，所以可以视为一个输入信号为\f$1\f$的系统对应的输出信号，输入信号固定，系统固定，因此输出信号固定。

#### 前提 2 {#premise_2}

连续信号可以通过\f$z\f$变换转变为离散信号，此时神符系统输出信号的关系可用一个 **差分方程** 来表示，根据\f$k\f$步前向预估的原理，迭代差分方程即可求得任意\f$k\f$步之后的神符系统输出信号，其中\f$k\f$步代表\f$kT\f$，\f$T\f$为采样周期，如果要预测非整数步可通过\f$k\f$步和\f$k+1\f$步线性插值求得。

#### 前提 3 {#premise_3}

由于\f$k\f$步前向预估公式迭代可求解，通过数学归纳法可证得任意步可求，且此公式可以被拆分成任意个数的项。

#### 前提 4 {#premise_4}

数据有误差，不同采样中的误差假设成立，\f$k\f$步前向预估用到的项数越多，数据方差越小，估算越准确。

根据以上 4 个前提，可以用当前时刻之前任意长度的数据来估计任意时间之后的神符位置，此位置进度由前向预估步数和用到的数据长度决定，考虑到子弹射速相对固定，预估步数基本固定，若精度不够可以考虑增加数据长度来提高精度。

### 差分方程

**k 步前向预估**

由 @ref premise_2 可以得到如下公式

\f[
x(k)=c_0+\sum_{i=n_f}^{n+n_f-1}c_ix(k-i)\quad n_f\in{N^*}\tag{1-1}
\f]

不难发现，其公式与自回归模型 (AR) 一致，因此，以下公式推导则间接证明了在神符预测中 AR 模型的正确性。

\f$n_f\f$可以为任意大小的正整数值，上述公式表明\f$x(k)\f$的结果可以通过计算\f$x(k-n_f-n+1)\f$，\f$x(k-n_f-n+2)\f$，\f$\dots\f$，\f$x(k-n_f)\f$共\f$n\f$个数的线性组合来得到，换句话说，可以根据前\f$n\f$帧的采样结果计算出\f$n_f\f$帧之后的结果。

在神符预测中，\f$n_f\f$表示为子弹飞行时间所需要的帧数，若子弹飞行时间不为整数帧，那么可以使用线性插值的方式计算出\f$y(k)\f$。设子弹飞行时间为\f$t_f\f$，帧间隔为\f$T\f$，可以使用以下公式近似计算。

计算帧数帧的\f$n_f\f$，其中\f$floor\f$表示向下取整函数

\f[
n_f=floor\left(\frac{t_f}T\right)\tag{1-2}
\f]

计算插值系数\f$k_1\f$和\f$k_2\f$

\f[
\left\{\begin{align}
k_1&=\frac{t_f}T-n_f\\
k_2&=1-k_1
\end{align}\right.\tag{1-3}
\f]

修正后的预测值\f$x_p\f$的计算公式为

\f[
x_p=k_2y(n_f)+k_1y(n_f+1)\tag{1-4}
\f]

### 最小二乘递推算法

#### 1. 最小二乘

对于一个方程组

\f[
\left\{\begin{matrix}
a_{11}x_1+a_{12}x_2+\dots+a_{1n}x_n&=&b_1\\
a_{21}x_1+a_{22}x_2+\dots+a_{2n}x_n&=&b_2\\
\vdots&\vdots&\vdots\\
a_{m1}x_1+a_{m2}x_2+\dots+a_{mn}x_n&=&b_m\\
\end{matrix}\right.\tag{2-1}
\f]

写为矩阵表达即为

\f[
A\pmb{x}=\pmb{b}\tag{2-2}
\f]

其中，\f$A\f$为方程组的系数矩阵，\f$\pmb{x}\f$为待辨识的未知向量，\f$\pmb{b}\f$为系数向量，一般情况下，\f$A\f$、\f$\pmb{b}\f$的系数均可以通过给定或测量系统的输入或输出值来得到。为使得误差平方和最小，即

\f[
\sqrt{\sum_{i=1}^n\left(\pmb{a_i}\hat{x}-b_i\right)^2}=\parallel A\hat{x}-\pmb{b}\parallel_2=
\min_x{\parallel A\pmb{x}-\pmb{b}\parallel_2}\tag{2-3}
\f]

可对\f$\parallel A\pmb{x}-\pmb{b}\parallel_2\f$求偏导数，即

\f[
A^T\left(A\pmb{x}-\pmb{b}\right)=0\tag{2-4}
\f]

即

\f[
\hat{x}=\left(A^TA\right)^{-1}A^T\pmb{b}\tag{2-5}
\f]

#### 2. 递推公式

对上式的\f$\left(A^TA\right)^{-1}\f$部分，现设\f$P_m^{-1}=A_m^TA_m\f$，其中\f$A_m=\left[\pmb{a_1}\quad \pmb{a_2}\quad \dots\quad \pmb{a_m}\right]^T\f$，\f$\pmb{a_i}\f$为上文出现过的行向量：\f$\pmb{a_i}=\left[a_{i1}\quad a_{i2}\quad \dots\quad a_{in}\right]\f$，则有

\f[
\begin{align}
P_m^{-1}&=A_m^TA_m=\sum_{i=1}^{m-1}\pmb{a_i}^T\pmb{a_i}+\pmb{a_m}^T\pmb{a_m}\\
&=P_{m-1}^{-1}+\pmb{a_m}^T\pmb{a_m}
\end{align}\tag{2-6}
\f]

同样，对于\f$A^Tb\f$的部分也能用相同的方法，现设\f$Q_m=A_m^T\pmb{b}\f$，其中\f$\pmb{b}=\left[b_1\quad b_2\quad \dots\quad b_m\right]^T\f$，则有

\f[
\begin{align}
Q_m&=A_m^T\pmb{b}=\sum_{i=1}^{m-1}\pmb{a_i}^Tb_i+\pmb{a_m}^Tb_m\\
&=Q_{m-1}+\pmb{a_m}^Tb_m
\end{align}\tag{2-7}
\f]

因此，第\f$m\f$轮迭代的估计值\f$\hat{x}\f$可表示为

\f[
\begin{align}
\hat{x}_m&=P_mQ_m\\
&=P_m(Q_{m-1}+\pmb{a_m}^Tb_m)\\
&=P_m(P_{m-1}^{-1}\hat{x}_{m-1}+\pmb{a_m}^Tb_m)\\
&=P_m(P_m^{-1}\hat{x}_{m-1}-\pmb{a_m}^T\pmb{a_m}\hat{x}_{m-1}+\pmb{a_m}^Tb_m)\\
&=\hat{x}_{m-1}+P_m\pmb{a_m}^T(b_m-\pmb{a_m}\hat{x}_{m-1})
\end{align}\tag{2-8}
\f]

上式即为递推最小二乘法的公式，但在其中，\f$P_m=\left(P_{m-1}^{-1}+\pmb{a_m}^T\pmb{a_m}\right)^{-1}\f$，求解\f$P_m\f$的递推形式需要进行两次求逆操作，会显著增加时间复杂度，需要对这一部分进行修改，在矩阵求逆过程中，有以下变换公式

\f[
\left(A+BC\right)^{-1}=A^{-1}-A^{-1}B\left(I+CA^{-1}B\right)^{-1}CA^{-1}\tag{2-9}
\f]

因此，\f$P_m\f$可以表示为

\f[
P_m=P_{m-1}-P_{m-1}\pmb{a_m}^T\left(I+\pmb{a_m}P_{m-1}\pmb{a_m}^T\right)^{-1}\pmb{a_m}P_{m-1}\tag{2-10}
\f]

@note 由于\f$\pmb{a_m}\f$为行向量，因此\f$I\f$为\f$1\times1\f$矩阵，即数字\f$1\f$。

汇总以上结果，最后可以得到

\f[
\left\{\begin{align}
P_m&=P_{m-1}-\frac{P_{m-1}\pmb{a_m}^T\pmb{a_m}P_{m-1}}{1-\pmb{a_m}P_{m-1}\pmb{a_m}^T}\\
\hat{x}_m&=\hat{x}_{m-1}+P_m\pmb{a_m}^T\left(b_m-\pmb{a_m}\hat{x}_{m-1}\right)
\end{align}\right.\tag{2-11}
\f]

### 预测

首先能够直接观测到神符的角度信息，因此我们直接辨识角度曲线的各系数即可。对每一次角度的观测结果\f$\theta(k)\f$得到的\f$(1-1)\f$式写成方程\f$(2-1)\f$的形式

\f[
c_0+\theta(k-n_f)c_{n_f}+\theta(k-n_f-1)c_{n_f+1}+\dots+\theta(k-n_f-n+1)c_{n_f+n-1}=\theta(k)\tag{3-1}
\f]

结合公式\f$(2-11)\f$，得到

\f[
\left\{\begin{align}
\pmb{a_m}&=\left[1\quad \theta(k-n_f)\quad \theta(k-n_f-1)\quad \dots\quad \theta(k-n_f-n+1)\right]\\
b_m&=\theta(k)\\
\hat{x}_m&=\left[c_0\quad c_{n_f}\quad c_{n_f+1}\quad \dots\quad c_{n_f+n-1}\right]^T
\end{align}\right.\tag{3-2}
\f]

\f$\theta(k)\f$表示当前帧采样得到的神符数据数据，\f$\theta(k-n_f)\f$表示\f$n_f\f$个采样周期（帧）之前采集到的神符角度信息，\f$\theta(k-n_f-n+1)\f$表示从\f$n_f\f$个采样周期之前再往前\f$n\f$个周期的神符角度信息。若要预测当前帧后\f$n_f\f$个采样周期的神符角度，运用公式\f$(1-1)\f$，使用使用前\f$n\f$帧到当前帧的\f$\theta\f$角度，代入迭代公式，即可计算出\f$n_f\f$个采样周期后的角度值。最后使用\f$(1-3)\f$和\f$(1-4)\f$即可得到最终的预测结果\f$y_p\f$。

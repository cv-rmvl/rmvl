k 步前向预估能量机关预测 {#tutorial_extra_spi_rune_predictor}
============

@author 黄昊睿
@author 赵曦
@date 2023/07/16
@version 2.1

@prev_tutorial{tutorial_extra_gyro_predictor}

@next_tutorial{tutorial_extra_how_to_use_decider}

@tableofcontents

------

相关类 rm::SpiRunePredictor

### 使用此方法的前提

#### 前提 1 {#premise_1}

能量机关角度位置因为不受其他输入影响，所以可以视为一个输入信号为\f$1\f$的系统对应的输出信号，输入信号固定，系统固定，因此输出信号固定。

#### 前提 2 {#premise_2}

连续信号可以通过\f$z\f$变换转变为离散信号，此时能量机关系统输出信号的关系可用一个 **差分方程** 来表示，根据\f$k\f$步前向预估的原理，迭代差分方程即可求得任意\f$k\f$步之后的能量机关系统输出信号，其中\f$k\f$步代表\f$kT\f$，\f$T\f$为采样周期，如果要预测非整数步可通过\f$k\f$步和\f$k+1\f$步线性插值求得。

#### 前提 3 {#premise_3}

由于\f$k\f$步前向预估公式迭代可求解，通过数学归纳法可证得任意步可求，且此公式可以被拆分成任意个数的项。

#### 前提 4 {#premise_4}

数据有误差，不同采样中的误差假设成立，\f$k\f$步前向预估用到的项数越多，数据方差越小，估算越准确。

根据以上 4 个前提，可以用当前时刻之前任意长度的数据来估计任意时间之后的能量机关位置，此位置进度由前向预估步数和用到的数据长度决定，考虑到子弹射速相对固定，预估步数基本固定，若精度不够可以考虑增加数据长度来提高精度。

### 差分方程

**k 步前向预估**

由 @ref premise_2 可以得到如下公式

\f[
x(k)=c_0+\sum_{i=n_f}^{n+n_f-1}c_ix(k-i)\quad n_f\in{N^*}\tag{1-1}
\f]

不难发现，其公式与自回归模型 (AR) 一致，因此，以下公式推导则间接证明了在能量机关预测中 AR 模型的正确性。

\f$n_f\f$可以为任意大小的正整数值，上述公式表明\f$x(k)\f$的结果可以通过计算\f$x(k-n_f-n+1)\f$，\f$x(k-n_f-n+2)\f$，\f$\dots\f$，\f$x(k-n_f)\f$共\f$n\f$个数的线性组合来得到，换句话说，可以根据前\f$n\f$帧的采样结果计算出\f$n_f\f$帧之后的结果。

在能量机关预测中，\f$n_f\f$表示为子弹飞行时间所需要的帧数，若子弹飞行时间不为整数帧，那么可以使用线性插值的方式计算出\f$y(k)\f$。设子弹飞行时间为\f$t_f\f$，帧间隔为\f$T\f$，可以使用以下公式近似计算。

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

@note 此部分详细介绍请参考 @ref tutorial_modules_least_square 一文。

对于一个超定方程组，写为矩阵表达即为

\f[
A\boldsymbol{x}=\boldsymbol{b}\tag{2-1}
\f]

有如下最小二乘解

\f[
\hat{x}=\left(A^TA\right)^{-1}A^T\boldsymbol{b}\tag{2-2}
\f]

#### 2. 递推公式

对上式的\f$\left(A^TA\right)^{-1}\f$部分，现设\f$P_m^{-1}=A_m^TA_m\f$，其中\f$A_m=\left[\boldsymbol{a_1}\quad \boldsymbol{a_2}\quad \dots\quad \boldsymbol{a_m}\right]^T\f$，\f$\boldsymbol{a_i}\f$为上文出现过的行向量：\f$\boldsymbol{a_i}=\left[a_{i1}\quad a_{i2}\quad \dots\quad a_{in}\right]\f$，则有

\f[
\begin{align}
P_m^{-1}&=A_m^TA_m=\sum_{i=1}^{m-1}\boldsymbol{a_i}^T\boldsymbol{a_i}+\boldsymbol{a_m}^T\boldsymbol{a_m}\\
&=P_{m-1}^{-1}+\boldsymbol{a_m}^T\boldsymbol{a_m}
\end{align}\tag{2-3}
\f]

同样，对于\f$A^Tb\f$的部分也能用相同的方法，现设\f$Q_m=A_m^T\boldsymbol{b}\f$，其中\f$\boldsymbol{b}=\left[b_1\quad b_2\quad \dots\quad b_m\right]^T\f$，则有

\f[
\begin{align}
Q_m&=A_m^T\boldsymbol{b}=\sum_{i=1}^{m-1}\boldsymbol{a_i}^Tb_i+\boldsymbol{a_m}^Tb_m\\
&=Q_{m-1}+\boldsymbol{a_m}^Tb_m
\end{align}\tag{2-4}
\f]

因此，第\f$m\f$轮迭代的估计值\f$\hat{x}\f$可表示为

\f[
\begin{align}
\hat{x}_m&=P_mQ_m\\
&=P_m(Q_{m-1}+\boldsymbol{a_m}^Tb_m)\\
&=P_m(P_{m-1}^{-1}\hat{x}_{m-1}+\boldsymbol{a_m}^Tb_m)\\
&=P_m(P_m^{-1}\hat{x}_{m-1}-\boldsymbol{a_m}^T\boldsymbol{a_m}\hat{x}_{m-1}+\boldsymbol{a_m}^Tb_m)\\
&=\hat{x}_{m-1}+P_m\boldsymbol{a_m}^T(b_m-\boldsymbol{a_m}\hat{x}_{m-1})
\end{align}\tag{2-5}
\f]

上式即为递推最小二乘法的公式，但在其中，\f$P_m=\left(P_{m-1}^{-1}+\boldsymbol{a_m}^T\boldsymbol{a_m}\right)^{-1}\f$，求解\f$P_m\f$的递推形式需要进行两次求逆操作，会显著增加时间复杂度，需要对这一部分进行修改，对于形如\f$\left(A^{-1}+BC\right)^{-1}\f$的式子，不妨令

\f[\left(A^{-1}+BC\right)^{-1}=A+X\tag{2-6a}\f]

因此满足

\f[\left(A^{-1}+BC\right)(A+X)=I\tag{2-6b}\f]

这是个求解\f$X\f$的过程，详细步骤如下

\f[\begin{align}\left(A^{-1}+BC\right)(A+X)&=I\\
BCA+A^{-1}X+BCX&=\boldsymbol 0\\\left(A^{-1}+BC\right)X&=-BCA\\
X&=-\left(A^{-1}+BC\right)^{-1}BCA\\
&=-\left(BB^{-1}A^{-1}+BC\right)^{-1}BCA\\
&=-\left[B\left(B^{-1}A^{-1}+C\right)\right]^{-1}BCA\\
&=-\left(B^{-1}A^{-1}+C\right)^{-1}CA\\
&=-\left(B^{-1}A^{-1}+CABB^{-1}A^{-1}\right)^{-1}CA\\
&=-\left[\left(I+CAB\right)B^{-1}A^{-1}\right]^{-1}CA\\
&=-AB\left(I+CAB\right)^{-1}CA
\end{align}\f]

因此，最终得到了矩阵求逆过程中有以下的变换公式

\f[\boxed{\left(A^{-1}+BC\right)^{-1}=A-AB\left(I+CAB\right)^{-1}CA}\tag{2-6c}\f]

因此，\f$P_m\f$可以表示为

\f[
P_m=P_{m-1}-P_{m-1}\boldsymbol{a_m}^T\left(I+\boldsymbol{a_m}P_{m-1}\boldsymbol{a_m}^T\right)^{-1}\boldsymbol{a_m}P_{m-1}\tag{2-7}
\f]

@note 由于\f$\boldsymbol{a_m}\f$为行向量，因此\f$I\f$为\f$1\times1\f$矩阵，即数字\f$1\f$。

汇总以上结果，最后可以得到

\f[
\boxed{\left\{\begin{align}
P_m&=P_{m-1}-\frac{P_{m-1}\boldsymbol{a_m}^T\boldsymbol{a_m}P_{m-1}}{1-\boldsymbol{a_m}P_{m-1}\boldsymbol{a_m}^T}\\
\hat{x}_m&=\hat{x}_{m-1}+P_m\boldsymbol{a_m}^T\left(b_m-\boldsymbol{a_m}\hat{x}_{m-1}\right)
\end{align}\right.}\tag{2-8}
\f]

### 预测

首先能够直接观测到能量机关的角度信息，因此我们直接辨识角度曲线的各系数即可。对每一次角度的观测结果\f$\theta(k)\f$得到的\f$\text{(1-1)}\f$式写成方程\f$\text{(2-1)}\f$的形式

\f[
c_0+\theta(k-n_f)c_{n_f}+\theta(k-n_f-1)c_{n_f+1}+\dots+\theta(k-n_f-n+1)c_{n_f+n-1}=\theta(k)\tag{3-1}
\f]

结合公式\f$\text{(2-8)}\f$，得到

\f[
\boxed{\left\{\begin{align}
\boldsymbol{a_m}&=\left[1\quad \theta(k-n_f)\quad \theta(k-n_f-1)\quad \dots\quad \theta(k-n_f-n+1)\right]\\
b_m&=\theta(k)\\
\hat{x}_m&=\left[c_0\quad c_{n_f}\quad c_{n_f+1}\quad \dots\quad c_{n_f+n-1}\right]^T
\end{align}\right.}\tag{3-2}
\f]

\f$\theta(k)\f$表示当前帧采样得到的能量机关数据数据，\f$\theta(k-n_f)\f$表示\f$n_f\f$个采样周期（帧）之前采集到的能量机关角度信息，\f$\theta(k-n_f-n+1)\f$表示从\f$n_f\f$个采样周期之前再往前\f$n\f$个周期的能量机关角度信息。若要预测当前帧后\f$n_f\f$个采样周期的能量机关角度，运用公式\f$\text{(1-1)}\f$，使用前\f$n\f$帧到当前帧的\f$\theta\f$角度，代入迭代公式，即可计算出\f$n_f\f$个采样周期后的角度值。最后使用\f$\text{(1-3)}\f$和\f$\text{(1-4)}\f$即可得到最终的预测结果\f$y_p\f$。

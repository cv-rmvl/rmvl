常微分方程（组）数值解与 Runge-Kutta 算法 {#tutorial_modules_runge_kutta}
============

@author 赵曦
@date 2024/01/12
@version 1.0
@brief 涉及常用的 **Euler 公式** 与 **Runge-Kutta** 算法

@prev_tutorial{tutorial_modules_func_iteration}

@next_tutorial{tutorial_modules_kalman}

@tableofcontents

------

### 1. 常微分方程（组）

一个质量-弹簧-阻尼系统的运动微分方程可以表示为

\f[m\frac{\mathrm d^2x}{\mathrm dt^2}+c\frac{\mathrm dx}{\mathrm dt}+kx=p(t)\tag{1-1a}\f]

即 \f[m\ddot x+c\dot x+kx=p(t)\tag{1-1b}\f]

可写成一阶方程组的形式，令\f$x_1=x,\quad x_2=\dot x\f$，有

\f[\left\{\begin{align}
\dot x_1&=x_2\\
\dot x_2&=-\frac kmx_1-\frac cmx_2+\frac1mp(t)
\end{align}\right.\tag{1-2}\f]

记 \f$\dot{\pmb x}=\begin{bmatrix}\dot x_1\\\dot x_2\end{bmatrix},\quad A=\begin{bmatrix}0&1\\-\frac km&-\frac cm\end{bmatrix},\quad \pmb b(t)=\begin{bmatrix}0\\\frac1mp(t)\end{bmatrix},\quad \pmb x^{(0)}=\begin{bmatrix}x_1(t_0)\\x_2(t_0)\end{bmatrix}\f$，有

\f[\dot{\pmb x}=A\pmb x+\pmb b(t),\quad\pmb x(t_0)=\pmb x^{(0)}\tag{1-3}\f]

\f$\text{(1-3)}\f$在控制系统中能够经常遇见，这种表示一个在时刻\f$t_0\f$带有初始条件的 2 阶线性系统，对于一般的（非线性）方程组，我们可以表示为

\f[\def\rkf#1{\dot x_{#1}=f_{#1}(t,x_1,x_2,\cdots,x_k),\quad x_{#1}(t_0)=x_{#1}^{(0)}}\rkf1\\\rkf2\\\vdots\\\rkf k\f]

即\f[\dot{\pmb x}=\pmb F(t,\pmb x),\quad\pmb x(t_0)=\pmb x^{(0)}\tag{1-4a}\f]

仅有 1 条的常微分方程则表示为\f[\dot x=f(t,x),\quad x(t_0)=x^{(0)}\tag{1-4b}\f]

这就是常微分方程的<b>初值问题</b>，即给定一个一阶常微分方程和对应的初始条件，求解出指定点或者指定范围的函数值。对于\f$\text{(1-4b)}\f$形式的初值问题，我们通常截取等步长，如取

\f[\begin{align}
&t_0<t_1<\cdots<t_n<t_{n+1}<\cdots\\&h=t_{n+1}-t_n\quad或\quad t_{n+1}=t_n+h\quad(h=0,1,\cdots)
\end{align}\f]

### 2. 求解常微分方程的 Euler 方法

#### 2.1 显式 Euler 单步法

先考虑一阶常微分方程\f$\text{(1-4b)}\f$的形式，在取定的\f$t_n\f$处 Taylor 展开，有

\f[\begin{align}
x_{n+1}\approx x(t_{n+1})&=x(t_n)+x'(t_n)h+x''(t_n)\frac{h^2}2+\cdots\\&=x_n+h\dot x_n+\frac{h^2}2\ddot x_n+\cdots\\
忽略h^2以上的高阶项，有\quad&\approx x_n+hf(t_n,x_n)
\end{align}\tag{2-1}\f]

可以得到

\f[x_{n+1}=x_n+hf(t_n,x_n)\tag{2-2}\f]

这便是求解常微分方程的基本方法，公式\f$\text{(2-2)}\f$被称为 Explicit Euler 显式欧拉公式。

#### 2.2 其余常见的 Euler 单步法

① Implicit Euler 隐式欧拉公式

\f[x_{n+1}=x_n+hf(t_{n+1},x_{n+1})\tag{2-3a}\f]

② 梯形公式

\f[x_{n+1}=x_n+\frac h2\left[f(t_n,x_n)+f(t_{n+1},x_{n+1})\right]\tag{2-3b}\f]

③ 改进 Euler 公式

\f[\left\{\begin{align}&\tilde x_{n+1}=x_n+hf(t_n,x_n)\\
&x_{n+1}=x_n+\frac h2\left[f(t_n,x_n)+f(t_{n+1},\tilde x_{n+1})\right]
\end{align}\right.\tag{2-3c}\f]

同样的，对于一阶方程组，公式\f$\text{(2-2)}\f$可以改写成

\f[\pmb x_{n+1}=\pmb x_n+h\pmb F(t_n,\pmb x_n)\tag{2-4}\f]

<span style="color: green">**示例**</span>

用 Euler 方法和步长\f$h=0.1\f$求解常微分方程的初值问题

\f[\left\{\begin{align}y'&=x^3+y^3+1\quad(0\leq x\leq0.8)\\y(0)&=0\end{align}\right.\f]

计算结果保留小数点后 6 位

**解答**

这里，\f$f(x,y)=x^3+y^3+1,\ x_n=nh+x_0=0.1n+0=0.1n\quad(n=0,1,\cdots,8),\ y_0=0\f$。由 Euler 公式计算可得

\f[\def\hf#1#2{y_{#1}+h(x_{#1}^3+y_{#1}^3+1)=#2}
y(0.1)\approx y_1=\hf0{0.100000}\\
y(0.2)\approx y_2=\hf1{0.200200}\\
y(0.3)\approx y_3=\hf2{0.301802}\\
y(0.4)\approx y_4=\hf3{0.407249}\\
y(0.5)\approx y_5=\hf4{0.520403}\\
y(0.6)\approx y_6=\hf5{0.646995}\\
y(0.7)\approx y_7=\hf6{0.795680}\\
y(0.8)\approx y_8=\hf7{0.980355}\f]

#### 2.3 局部阶段误差、精度的 “阶”

上文的 4 种 Euler 单步法公式，使用哪种精度更高？这里要引入局部阶段误差的概念，因为每一次求解\f$x_i\f$都会引入误差，并且误差会进行累积，局部截断误差不考虑迭代求解\f$x_n\f$及之前的累积误差，仅考虑从\f$x_n\f$到\f$x_{n+1}\f$产生的误差，即认为\f$x_n=x(t_n)\f$。对于显式 Euler 单步法，可以计算出其局部截断误差\f$T_{n+1}\f$。

\f[\begin{align}
T_{n+1}&=x(t_{n+1})-x_{n+1}\\
&=x(t_{n+1})-x_n-hf(t_n,x_n)\\
&=x(t_{n+1})-x(t_n)-hx'(t_n)\\
x(t_{n+1})在t_n处\text{ Taylor }展开，有\quad&=x(t_n)+hx'(t_n)+\frac{h^2}2x''(t_n)+o(h^3)-x(t_n)-hx'(t_n)\\
&=\frac12x''(t_n)h^2+o(h^3)
\end{align}\tag{2-5}\f]

可以看出<b>局部截断误差的主项</b>为\f$0.5x''(t_n)h^2\f$，我们称显式 Euler 单步法具有 1 阶精度。

再来考虑梯形公式\f$\text{(2-3b)}\f$的局部截断误差\f$T_{n+1}\f$。

\f[\begin{align}
T_{n+1}&=x(t_{n+1})-x(t_n)-\frac12hf(t_n,x(t_n))-\frac12hf(t_{n+1},x(t_{n+1}))\\
&=x(t_{n+1})-x(t_n)-\frac12hx'(t_n)-\frac12hx'(t_{n+1})\\
&=\left[x(t_n)+hx'(t_n)+\frac{h^2}2x''(t_n)+\frac{h^3}6x'''(t_n)+o(h^4)\right]-x(t_n)-\\
&\qquad\frac12hx'(t_n)-\frac12h\left[x'(t_n)+hx''(t_n)+\frac{h^2}2x'''(t_n)+o(h^3)\right]\\
&=-\frac1{12}x'''(t_n)h^3+o(h^4)
\end{align}\tag{2-6}\f]

<b>局部截断误差的主项</b>为\f$-\frac1{12}x'''(t_n)h^3\f$，我们称梯形公式具有 2 阶精度。

### 3. Runge-Kutta 方法

对于\f$\text{(2-3c)}\f$的改进 Euler 公式，可以改写成

\f[\left\{\begin{align}
x_{n+1}&=x_n+h\left(\frac12k_1+\frac12k_2\right)\\
k_1&=f(t_n,x_n)\\k_2&=f(t_n+h,x_n+hk_1)
\end{align}\right.\tag{3-1}\f]

模仿这一写法，我们继续构造新的 2 阶公式

#### 3.1 二阶 Runge-Kutta 公式

\f[\left\{\begin{align}
x_{n+1}&=x_n+h(\lambda_1k_1+\lambda_2k_2)\\
k_1&=f(t_n,x_n)\\k_2&=f(t_n+ph,x_n+phk_1)
\end{align}\right.\tag{3-2}\f]

其中\f$\lambda_1,\lambda_2,p\f$为待定参数，先分别将\f$x(t_{n+1})\f$和\f$x_{n+1}\f$在\f$t_n\f$处 Taylor 展开。

对\f$x(t_{n+1})\f$，有

\f[\begin{align}
x(t_{n+1})&=x(t_n)+hx'(t_n)+\frac{h^2}2x''(t_n)+o(h^3)\\
&=x(t_n)+hf(t_n,x(t_n))+\frac{h^2}2\frac{\mathrm d}{\mathrm dt}f(t_n,x(t_n))+o(h^3)\\
&=x(t_n)+hf(t_n,x(t_n))+\frac{h^2}2\left[\frac{\partial f(t_n,x(t_n))}{\partial t}+\frac{\partial f(t_n,x(t_n))}{\partial x}·\frac{\mathrm dx}{\mathrm dt}\right]+o(h^3)\\
令f(t_n,x(t_n))=(f)_{(n)},有\quad&=x(t_n)+h(f)_{(n)}+\frac{h^2}2\left[\frac{\partial (f)_{(n)}}{\partial t}+\frac{\partial (f)_{(n)}}{\partial x}(f)_{(n)}\right]+o(h^3)\\
&=x(t_n)+h(f)_{(n)}+\frac{h^2}2(f_t+f_xf)_{(n)}+o(h^3)
\end{align}\tag{3-3}\f]

对\f$x_{n+1}\f$，有

\f[\begin{align}x_{n+1}&=x_n+h\left[\lambda_1f(t_n,x_n)+\lambda_2f(t_n+ph,x_n+phf(t_n,x_n))\right]\\
&=x_n+h\lambda_1f(t_n,x_n)+h\lambda_2\left[f(t_n,x_n)+ph\frac{\partial f(t_n,x_n)}{\partial t}+phf(t_n,x_n)\frac{\partial f(t_n,x_n)}{\partial x}+o(h^2)\right]\\
令f(t_n,x_n)=(f)_n,有\quad&=x_n+h\lambda_1(f)_n+h\lambda_2\left[(f)_n+ph\frac{\partial(f)_n}{\partial t}+ph(f)_n\frac{\partial(f)_n}{\partial x}+o(h^2)\right]\\
&=x_n+h\lambda_1(f)_n+h\lambda_2\left[(f)_n+ph(f_t)_n+ph(f)_n(f_x)_n+o(h^2)\right]\\
&=x_n+h(\lambda_1+\lambda_2)(f)_n+ph^2\lambda_2(f_t+f_xf)_n+o(h^3)
\end{align}\tag{3-4}\f]

为求局部截断误差，则满足\f$x(t_n)=x_n,\ (f)_{(n)}=(f)_n,\ (f_t+f_xf)_{(n)}=(f_t+f_xf)_n\f$，因此\f$T_{n+1}=\text{(3-3)}-\text{(3-4)}\f$，即

\f[\begin{align}
T_{n+1}&=x(t_{n+1})-x_{n+1}\\
&=(1-\lambda_1-\lambda_2)(f)_n+\left(\frac12-p\lambda_2\right)h^2(f_t+f_xf)_n+o(h^3)
\end{align}\tag{3-5}\f]

我们希望这个公式具有 2 阶精度，即局部截断误差的主项为 0，因此有

\f[\left\{\begin{align}
&1-\lambda_1+\lambda_2=1\\
&p\lambda_2=\frac12
\end{align}\right.\tag{3-6}\f]

\f$\text{(3-2)}\f$和\f$\text{(3-6)}\f$联立构成 2 阶精度的单步显式公式族，由于使用到了 2 个斜率值\f$k_1\f$和\f$k_2\f$，因此称为 2 级 2 阶 Runge-Kutta 公式族，最常用的是当\f$\lambda_1=0\f$即\f$\lambda_2=1,\quad p=\frac12\f$，这时得到所谓 2 级 2 阶<span style="color: red">中点公式</span>。

\f[\begin{align}x_{n+1}&=x_n+hk_2\\k_1&=f(t_n,x_n)\\
k_2&=f\left(t_n+\frac h2,x_n+\frac h2k_1\right)\end{align}\tag{3-7}\f]

#### 3.2 Butcher 表

为了便于记录上文的 Runge-Kutta 中点公式，我们将公式\f$\text{(3-2)}\f$改写为

\f[\left\{\begin{align}x_{n+1}&=x_n+h(\lambda_1k_1+\lambda_2k_2)\\k_1&=f(t_n+p_1h,x_n+h(a_{11}
k_1+a_{12}k_2))\\k_2&=f(t_n+p_2h,x_n+h(a_{21}k_1+a_{22}k_2))\end{align}\right.\tag{3-8}\f]

令\f$\pmb p=(p_1,p_2)T,\quad\pmb\lambda=(\lambda_1,\lambda_2),\quad
R=\begin{bmatrix}a_{11}&a_{12}\\a_{21}&a_{22}\end{bmatrix}\f$，则

\f[\begin{array}{c|c}\pmb p&R\\\hline&\pmb\lambda\end{array}=\begin{array}{c|cc}p_1&
a_{11}&a_{12}\\p_2&a_{21}&a_{22}\\\hline&\lambda_1&\lambda_2\end{array}\tag{3-9}\f]

被称为 Butcher 表，例如，上文的中点公式可以表示为

\f[\begin{array}{c|cc}0&0&0\\1&1&0\\\hline&\frac12&\frac12\end{array}\tag{3-10}\f]

一般的，对于以下\f$n\f$阶公式

\f[\left\{\begin{align}
x_{n+1}&=x_n+h(\lambda_1k_1+\lambda_2k_2+\cdots+\lambda_nk_n)\\
k_1&=f(t_n+p_1h,x_n+h(a_{11}k_1+a_{12}k_2+\cdots+a_{1n}k_n))\\
k_2&=f(t_n+p_2h,x_n+h(a_{21}k_1+a_{22}k_2+\cdots+a_{2n}k_n))\\
&\vdots\\
k_n&=f(t_n+p_nh,x_n+h(a_{n1}k_1+a_{n2}k_2+\cdots+a_{nn}k_n))\\
\end{align}\right.\tag{3-11}\f]

同样可以用 Butcher 表来表示：

\f[\begin{array}{c|cccc}
p_1&a_{11}&a_{12}&\cdots&a_{1n}\\
p_2&a_{21}&a_{22}&\cdots&a_{2n}\\
\vdots&\vdots&\vdots&\ddots&\vdots\\
p_n&a_{n1}&a_{n2}&\cdots&a_{nn}\\
\hline&\lambda_1&\lambda_2&\cdots&\lambda_n\end{array}\tag{3-12}\f]

#### 3.3 三阶 / 四阶 Runge-Kutta 公式

这里直接给出对应的 Butcher 表。

一种 3 级 3 阶 Runge-Kutta 方法的具体形式对应的 Butcher 表

\f[\begin{array}{c|ccc}0&0&0&0\\\frac12&\frac12&0&0\\1&-1&2&0\\
\hline&\frac16&\frac23&\frac16\end{array}\tag{3-13a}\f]

经典 4 级 4 阶 Runge-Kutta 方法对应的 Butcher 表

\f[\begin{array}{c|cccc}0&0&0&0&0\\\frac12&\frac12&0&0&0\\\frac12&0&\frac12&0&
0\\1&0&0&1&0\\\hline&\frac16&\frac13&\frac13&\frac16\end{array}\tag{3-13b}\f]
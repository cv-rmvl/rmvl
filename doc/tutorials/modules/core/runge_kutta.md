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

\f[\begin{align}\dot{\pmb x}&=A\pmb x+\pmb b(t)\\\pmb x(t_0)&=\pmb x^{(0)}\end{align}\tag{1-3}\f]

\f$\text{(1-3)}\f$在控制系统中能够经常遇见，这种表示一个在时刻\f$t_0\f$带有初始条件的 2 阶线性系统，对于一般的（非线性）方程组，我们可以表示为

\f[\left\{\begin{align}\dot{\pmb x}&=f(t,\pmb x)\\\pmb x(t_0)&=\pmb x^{(0)}\end{align}\right.\tag{1-4a}\f]

现考虑仅有一个方程的形式

\f[\left\{\begin{align}\dot x&=f(t,x)\\x(t_0)&=x^{(0)}\end{align}\right.\tag{1-4b}\f]

这就是常微分方程的<b>初值问题</b>，即给定一个一阶常微分方程和对应的初始条件，求解出指定点或者指定范围的函数值。对于\f$\text{(1-4b)}\f$形式的初值问题，我们通常截取等步长，如取

\f[\begin{align}
&t_0<t_1<\cdots<t_n<t_{n+1}<\cdots\\&h=t_{n+1}-t_n\quad或\quad t_{n+1}=t_n+h\quad(h=0,1,\cdots)
\end{align}\f]

### 2. Euler 公式

在取定的\f$t_n\f$处 Taylor 展开，有

\f[\begin{align}
x_{n+1}\approx x(t_{n+1})&=x(t_n)+x'(t_n)h+o(h^2)\\&=x_n+h\dot x_n+o(h^2)\\&\approx x_n+hf(t_n,x_n)
\end{align}\tag{2-1}\f]

可以得到

\f[x_{n+1}=x_n+hf(t_n,x_n)\tag{2-2}\f]

这便是求解常微分方程的基本方法，公式\f$\text{2-2}\f被称为 Explicit Euler 显式欧拉公式。下面列举几个常见的公式

① Implicit Euler 隐式欧拉公式

\f[x_{n+1}=x_n+hf(t_{n+1},x_{n+1})\tag{2-3a}\f]

② 梯形公式

\f[x_{n+1}=x_n+\frac h2\left[f(t_n,x_n)+f(t_{n+1},x_{n+1})\right]\tag{2-3b}\f]

③ 改进 Euler 公式

\f[\begin{align}
&\bar x_{n+1}=x_n+hf(t_n,x_n)\\
&x_{n+1}=x_n+\frac h2\left[f(t_n,x_n)+f(t_{n+1},\bar x_{n+1})\right]
\end{align}\tag{2-3c}\f]

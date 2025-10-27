常微分方程（组）数值解与 Runge-Kutta 算法 {#tutorial_modules_runge_kutta}
============

@author 赵曦
@date 2024/01/12
@version 1.0
@brief 涉及常用的 **Euler 公式** 与 **Runge-Kutta** 算法

@prev_tutorial{tutorial_modules_func_iteration}

@next_tutorial{tutorial_modules_ew_topsis}

@tableofcontents

------

\f[
\def\transparent#1{\color{transparent}{#1}}
\f]

### 1 常微分方程（组）

一个质量-弹簧-阻尼系统的运动微分方程可以表示为

\f[m\frac{\mathrm d^2x}{\mathrm dt^2}+c\frac{\mathrm dx}{\mathrm dt}+kx=p(t)\tag{1-1a}\f]

即 \f[m\ddot x+c\dot x+kx=p(t)\tag{1-1b}\f]

可写成一阶方程组的形式，令\f$x_1=x,\quad x_2=\dot x\f$，有

\f[\def\mat#1#2{\begin{bmatrix}#1\\#2\end{bmatrix}}
\left\{\begin{align}\dot x_1&=x_2\\
\dot x_2&=-\frac kmx_1-\frac cmx_2+\frac1mp(t)
\end{align}\right.\tag{1-2}\f]

记 \f$\dot{\boldsymbol x}=\begin{bmatrix}\dot x_1\\\dot x_2\end{bmatrix},\quad A=\begin{bmatrix}0&1\\-\frac km&-\frac cm\end{bmatrix},\quad \boldsymbol b(t)=\begin{bmatrix}0\\\frac1mp(t)\end{bmatrix},\quad \boldsymbol x^{(0)}=\begin{bmatrix}x_1(t_0)\\x_2(t_0)\end{bmatrix}\f$，有

\f[\dot{\boldsymbol x}=A\boldsymbol x+\boldsymbol b(t),\quad\boldsymbol x(t_0)=\boldsymbol x^{(0)}\tag{1-3}\f]

\f$\text{(1-3)}\f$在控制系统中能够经常遇见，这种表示一个在时刻\f$t_0\f$带有初始条件的 2 阶线性系统，对于一般的（非线性）方程组，我们可以表示为

\f[\def\rkf#1{\dot x_{#1}=f_{#1}(t,x_1,x_2,\cdots,x_k),\quad x_{#1}(t_0)=x_{#1}^{(0)}}\rkf1\\\rkf2\\\vdots\\\rkf k\f]

即\f[\dot{\boldsymbol x}=\boldsymbol F(t,\boldsymbol x),\quad\boldsymbol x(t_0)=\boldsymbol x^{(0)}\tag{1-4a}\f]

仅有 1 条的常微分方程则表示为\f[\dot x=f(t,x),\quad x(t_0)=x^{(0)}\tag{1-4b}\f]

这就是常微分方程的<b>初值问题</b>，即给定一个一阶常微分方程和对应的初始条件，求解出指定点或者指定范围的函数值。对于\f$\text{(1-4b)}\f$形式的初值问题，我们通常截取等步长，如取

\f[\begin{align}
&t_0<t_1<\cdots<t_n<t_{n+1}<\cdots\\&h=t_{n+1}-t_n\quad或\quad t_{n+1}=t_n+h\quad(h=0,1,\cdots)
\end{align}\f]

### 2 求解常微分方程的 Euler 方法

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

\f[\boldsymbol x_{n+1}=\boldsymbol x_n+h\boldsymbol F(t_n,\boldsymbol x_n)\tag{2-4}\f]

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

#### 2.3 局部阶段误差

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

### 3 Runge-Kutta 方法

对于\f$\text{(2-3c)}\f$的改进 Euler 公式，可以改写成

\f[\left\{\begin{align}
x_{n+1}&=x_n+h\left(\frac12k_1+\frac12k_2\right)\\
k_1&=f(t_n,x_n)\\k_2&=f(t_n+h,x_n+hk_1)
\end{align}\right.\tag{3-1}\f]

模仿这一写法，我们继续构造新的 2 阶公式

#### 3.1 二阶 Runge-Kutta 公式

@note 此小节为 2 阶 Runge-Kutta 公式族的推导，涉及到多元函数\f$f(x,y)\f$的全导数
\f[\begin{align}\frac{\mathrm df}{\mathrm dx}&=\frac{\partial f}{\partial x}+\frac{\partial f}{\partial y}·\frac{\mathrm dy}{\mathrm dx}\\
f'&=f_x+f_yy'\end{align}\tag{i}\f]
以及多元函数\f$f(x,y)\f$的 Taylor 展开，令\f$\boldsymbol x=(x-x_0,\ y-y_0)^T\f$，则多元函数 Taylor 展开如下
\f[\begin{align}f(x,y)&=f(x_0,y_0)+\begin{bmatrix}f_x(x_0,y_0)&f_y(x_0,y_0)\end{bmatrix}\boldsymbol x+\\
&\transparent=\frac1{2!}\boldsymbol x^T\begin{bmatrix}f_{xx}(x_0,y_0)&f_{xy}(x_0,y_0)\\f_{yx}(x_0,y_0)&f_{yy}(x_0,y_0)\end{bmatrix}\boldsymbol x+o^n\end{align}\tag{ii}\f]
<span style="color: red">若仅想了解最终结果，请跳过此小节</span>

\f[\left\{\begin{align}
x_{n+1}&=x_n+h(\lambda_1k_1+\lambda_2k_2)\\
k_1&=f(t_n,x_n)\\k_2&=f(t_n+ph,x_n+phk_1)
\end{align}\right.\tag{3-2}\f]

其中\f$\lambda_1,\lambda_2,p\f$为待定参数，先分别将\f$x(t_{n+1})\f$和\f$x_{n+1}\f$在\f$t_n\f$处 Taylor 展开。

对\f$x(t_{n+1})\f$，有

\f[\begin{align}
x(t_{n+1})&=x(t_n)+hx'(t_n)+\frac{h^2}2x''(t_n)+o(h^3)\\
&=x(t_n)+hf(t_n,x(t_n))+\frac{h^2}2\frac{\mathrm d}{\mathrm dt}f(t_n,x(t_n))+o(h^3)\\
&=x(t_n)+hf(t_n,x(t_n))+\\
&\transparent=\frac{h^2}2\left[\frac{\partial f(t_n,x(t_n))}{\partial t}+\frac{\partial f(t_n,x(t_n))}{\partial x}·\frac{\mathrm dx}{\mathrm dt}\right]+o(h^3)\\
令f(t_n,x(t_n))=(f)_{(n)}\quad&=x(t_n)+h(f)_{(n)}+\frac{h^2}2\left[\frac{\partial (f)_{(n)}}{\partial t}+\frac{\partial (f)_{(n)}}{\partial x}\frac{\mathrm dx}{\mathrm dt}\right]+o(h^3)\\
由f(t_n,x(t_n))=\frac{\mathrm dx}{\mathrm dt}\quad&=x(t_n)+h(f)_{(n)}+\frac{h^2}2\left[\frac{\partial (f)_{(n)}}{\partial t}+\frac{\partial (f)_{(n)}}{\partial x}(f)_{(n)}\right]+o(h^3)\\
&=x(t_n)+h(f)_{(n)}+\frac{h^2}2(f_t+f_xf)_{(n)}+o(h^3)\tag{3-3}
\end{align}\f]

对\f$x_{n+1}\f$，有

\f[\begin{align}x_{n+1}&=x_n+h\left[\lambda_1f(t_n,x_n)+\lambda_2f(t_n+ph,x_n+phf(t_n,x_n))\right]\\
&=x_n+h\lambda_1f(t_n,x_n)+h\lambda_2\left[f(t_n,x_n)+ph\frac{\partial f(t_n,x_n)}{\partial t}+\right.\\
&\transparent=\left.phf(t_n,x_n)\frac{\partial f(t_n,x_n)}{\partial x}+o(h^2)\right]\\
令f(t_n,x_n)=(f)_n\quad&=x_n+h\lambda_1(f)_n+\\
&\transparent=h\lambda_2\left[(f)_n+ph\frac{\partial(f)_n}{\partial t}+ph(f)_n\frac{\partial(f)_n}{\partial x}+o(h^2)\right]\\
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
&\lambda_1+\lambda_2=1\\
&p\lambda_2=\frac12
\end{align}\right.\tag{3-6}\f]

\f$\text{(3-2)}\f$和\f$\text{(3-6)}\f$联立构成 2 阶精度的单步显式公式族，由于使用到了 2 个斜率值\f$k_1\f$和\f$k_2\f$，因此称为 2 级 2 阶 Runge-Kutta 公式族，最常用的是当\f$\lambda_1=0\f$即\f$\lambda_2=1,\quad p=\frac12\f$，这时得到所谓 2 级 2 阶<span style="color: red">中点公式</span>。

\f[\begin{align}x_{n+1}&=x_n+hk_2\\k_1&=f(t_n,x_n)\\
k_2&=f\left(t_n+\frac h2,x_n+\frac h2k_1\right)\end{align}\tag{3-7}\f]

#### 3.2 Butcher 表

为了便于记录上文的 Runge-Kutta 中点公式，我们将公式\f$\text{(3-2)}\f$改写为

\f[\left\{\begin{align}x_{n+1}&=x_n+h(\lambda_1k_1+\lambda_2k_2)\\k_1&=f(t_n+p_1h,x_n+h(a_{11}
k_1+a_{12}k_2))\\k_2&=f(t_n+p_2h,x_n+h(a_{21}k_1+a_{22}k_2))\end{align}\right.\tag{3-8}\f]

令\f$\boldsymbol p=\mat{p_1}{p_2},\quad\boldsymbol\lambda=(\lambda_1,\lambda_2),\quad
R=\begin{bmatrix}a_{11}&a_{12}\\a_{21}&a_{22}\end{bmatrix}\f$，则

\f[\begin{array}{c|c}\boldsymbol p&R\\\hline&\boldsymbol\lambda\end{array}\Rightarrow\begin{array}{c|cc}p_1&
a_{11}&a_{12}\\p_2&a_{21}&a_{22}\\\hline&\lambda_1&\lambda_2\end{array}\tag{3-9}\f]

被称为 Butcher 表，例如，上文的中点公式可以表示为

\f[\begin{array}{c|cc}0&0&0\\1&1&0\\\hline&\frac12&\frac12\end{array}\tag{3-10}\f]

一般的，对于一阶方程\f$x'=f(t,x),\ x(t_0)=x^{(0)}\f$，有以下\f$n\f$阶公式

\f[\left\{\begin{align}
x_{n+1}&=x_n+h(\lambda_1k_1+\lambda_2k_2+\cdots+\lambda_nk_n)\\
k_1&=f(t_n+p_1h,x_n+h(a_{11}k_1+a_{12}k_2+\cdots+a_{1n}k_n))\\
k_2&=f(t_n+p_2h,x_n+h(a_{21}k_1+a_{22}k_2+\cdots+a_{2n}k_n))\\&\vdots\\
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

#### 3.4 方程组的 Runge-Kutta 公式 {#equations_runge_kutta}

对于一阶方程组\f$\boldsymbol x'=\boldsymbol F(t,\boldsymbol x),\ \boldsymbol x(t_0)=\boldsymbol x^{(0)}\f$，公式\f$\text{(3-11)}\f$可以改写为

\f[\left\{\begin{align}
\boldsymbol x_{n+1}&=\boldsymbol x_n+h(\lambda_1\boldsymbol k_1+\lambda_2\boldsymbol k_2+\cdots+\lambda_n\boldsymbol k_n)\\
\boldsymbol k_1&=\boldsymbol F(t_n+p_1h,\boldsymbol x_n+h(a_{11}\boldsymbol k_1+a_{12}\boldsymbol k_2+\cdots+a_{1n}\boldsymbol k_n))\\
\boldsymbol k_2&=\boldsymbol F(t_n+p_2h,\boldsymbol x_n+h(a_{21}\boldsymbol k_1+a_{22}\boldsymbol k_2+\cdots+a_{2n}\boldsymbol k_n))\\&\vdots\\
\boldsymbol k_n&=\boldsymbol F(t_n+p_nh,\boldsymbol x_n+h(a_{n1}\boldsymbol k_1+a_{n2}\boldsymbol k_2+\cdots+a_{nn}\boldsymbol k_n))\\
\end{align}\right.\tag{3-14}\f]

公式\f$\text{(3-14)}\f$与\f$\text{(3-11)}\f$基本一致，因此同样可以使用 Butcher 表来描述常微分方程组的 Runge-Kutta 公式。

RMVL 的相关类请参考 rm::RungeKutta

<span style="color: green">**示例**</span>

使用 2 阶中点公式求解\f$[0,1]\f$上常微分方程的初值问题，并在\f$t=1\f$处与实际值进行比较

\f[\left\{\begin{align}\dot x_1&=2x_2+t\\\dot x_2&=-x_1-3x_2\end{align}\right.\\
且满足x_1(0)=1,\quad x_2(0)=-1\f]

**解答**

<span style="color: orange">**① 数值解**</span>

使用中点公式：

\f[\begin{align}\boldsymbol x_{n+1}&=\boldsymbol x_n+h\boldsymbol k_2\\\boldsymbol k_1&=\boldsymbol F(t_n,\boldsymbol x_n)\\
\boldsymbol k_2&=\boldsymbol F(t_n+\frac h2,\boldsymbol x_n+\frac h2\boldsymbol k_1)\end{align}\f]

即

\f[\begin{align}x_{1_{n+1}}&=x_{1_n}+hk_2\\x_{2_{n+1}}&=x_{2_n}+hl_2\\
k_1&=f_1(t_n,x_{1_n},x_{2_n})\\l_1&=f_2(t_n,x_{1_n},x_{2_n})\\
k_2&=f_1(t_n+\frac h2,x_{1_n}+\frac h2k_1,x_{2_n}+\frac h2l_1)\\
l_2&=f_2(t_n+\frac h2,x_{1_n}+\frac h2k_1,x_{2_n}+\frac h2l_1)\end{align}\f]

迭代运行结果如下：

<div style="height: 250px; overflow: auto;">
  ```
  t = 0.010000,   x = 0.980250, -0.980200
  t = 0.020000,   x = 0.960992, -0.960793
  t = 0.030000,   x = 0.942218, -0.941772
  t = 0.040000,   x = 0.923921, -0.923131
  t = 0.050000,   x = 0.906093, -0.904863
  t = 0.060000,   x = 0.888727, -0.886961
  t = 0.070000,   x = 0.871815, -0.869420
  t = 0.080000,   x = 0.855350, -0.852232
  t = 0.090000,   x = 0.839325, -0.835393
  t = 0.100000,   x = 0.823734, -0.818895
  t = 0.110000,   x = 0.808570, -0.802734
  t = 0.120000,   x = 0.793825, -0.786903
  t = 0.130000,   x = 0.779494, -0.771396
  t = 0.140000,   x = 0.765569, -0.756209
  t = 0.150000,   x = 0.752045, -0.741335
  t = 0.160000,   x = 0.738916, -0.726770
  t = 0.170000,   x = 0.726174, -0.712507
  t = 0.180000,   x = 0.713815, -0.698543
  t = 0.190000,   x = 0.701833, -0.684871
  t = 0.200000,   x = 0.690221, -0.671487
  t = 0.210000,   x = 0.678973, -0.658386
  t = 0.220000,   x = 0.668085, -0.645563
  t = 0.230000,   x = 0.657551, -0.633014
  t = 0.240000,   x = 0.647365, -0.620734
  t = 0.250000,   x = 0.637521, -0.608717
  t = 0.260000,   x = 0.628016, -0.596961
  t = 0.270000,   x = 0.618843, -0.585460
  t = 0.280000,   x = 0.609998, -0.574210
  t = 0.290000,   x = 0.601475, -0.563207
  t = 0.300000,   x = 0.593269, -0.552447
  t = 0.310000,   x = 0.585377, -0.541926
  t = 0.320000,   x = 0.577792, -0.531639
  t = 0.330000,   x = 0.570511, -0.521584
  t = 0.340000,   x = 0.563529, -0.511755
  t = 0.350000,   x = 0.556841, -0.502149
  t = 0.360000,   x = 0.550443, -0.492763
  t = 0.370000,   x = 0.544331, -0.483592
  t = 0.380000,   x = 0.538499, -0.474634
  t = 0.390000,   x = 0.532945, -0.465884
  t = 0.400000,   x = 0.527664, -0.457340
  t = 0.410000,   x = 0.522652, -0.448997
  t = 0.420000,   x = 0.517904, -0.440853
  t = 0.430000,   x = 0.513418, -0.432904
  t = 0.440000,   x = 0.509188, -0.425147
  t = 0.450000,   x = 0.505212, -0.417579
  t = 0.460000,   x = 0.501485, -0.410196
  t = 0.470000,   x = 0.498004, -0.402997
  t = 0.480000,   x = 0.494765, -0.395977
  t = 0.490000,   x = 0.491765, -0.389133
  t = 0.500000,   x = 0.489000, -0.382464
  t = 0.510000,   x = 0.486466, -0.375966
  t = 0.520000,   x = 0.484161, -0.369635
  t = 0.530000,   x = 0.482081, -0.363471
  t = 0.540000,   x = 0.480222, -0.357469
  t = 0.550000,   x = 0.478582, -0.351627
  t = 0.560000,   x = 0.477157, -0.345943
  t = 0.570000,   x = 0.475944, -0.340414
  t = 0.580000,   x = 0.474941, -0.335037
  t = 0.590000,   x = 0.474143, -0.329810
  t = 0.600000,   x = 0.473548, -0.324731
  t = 0.610000,   x = 0.473154, -0.319797
  t = 0.620000,   x = 0.472956, -0.315006
  t = 0.630000,   x = 0.472954, -0.310356
  t = 0.640000,   x = 0.473142, -0.305844
  t = 0.650000,   x = 0.473520, -0.301468
  t = 0.660000,   x = 0.474084, -0.297226
  t = 0.670000,   x = 0.474831, -0.293116
  t = 0.680000,   x = 0.475759, -0.289136
  t = 0.690000,   x = 0.476865, -0.285283
  t = 0.700000,   x = 0.478148, -0.281556
  t = 0.710000,   x = 0.479603, -0.277953
  t = 0.720000,   x = 0.481229, -0.274471
  t = 0.730000,   x = 0.483024, -0.271109
  t = 0.740000,   x = 0.484985, -0.267865
  t = 0.750000,   x = 0.487110, -0.264737
  t = 0.760000,   x = 0.489396, -0.261723
  t = 0.770000,   x = 0.491841, -0.258822
  t = 0.780000,   x = 0.494443, -0.256031
  t = 0.790000,   x = 0.497199, -0.253349
  t = 0.800000,   x = 0.500109, -0.250774
  t = 0.810000,   x = 0.503169, -0.248304
  t = 0.820000,   x = 0.506377, -0.245939
  t = 0.830000,   x = 0.509731, -0.243676
  t = 0.840000,   x = 0.513230, -0.241513
  t = 0.850000,   x = 0.516870, -0.239449
  t = 0.860000,   x = 0.520652, -0.237483
  t = 0.870000,   x = 0.524571, -0.235613
  t = 0.880000,   x = 0.528627, -0.233838
  t = 0.890000,   x = 0.532818, -0.232156
  t = 0.900000,   x = 0.537141, -0.230565
  t = 0.910000,   x = 0.541595, -0.229065
  t = 0.920000,   x = 0.546178, -0.227653
  t = 0.930000,   x = 0.550889, -0.226329
  t = 0.940000,   x = 0.555725, -0.225091
  t = 0.950000,   x = 0.560685, -0.223938
  t = 0.960000,   x = 0.565768, -0.222869
  t = 0.970000,   x = 0.570971, -0.221881
  t = 0.980000,   x = 0.576292, -0.220975
  t = 0.990000,   x = 0.581732, -0.220149
  t = 1.000000,   x = 0.587286, -0.219401
  ```
</div>

因此，在\f$t=1\f$时，\f$x\approx(0.587286,-0.219401)^T\f$

<span style="color: orange">**② 精确解**</span>

对待求的初值问题写成矩阵形式

\f[\dot X=\begin{bmatrix}0&2\\-1&-3\end{bmatrix}X+\mat10t,\quad X(0)=\mat1{-1}\f]

由矩阵分析的知识，可以知道，对于形如\f[\dot X=AX+B(t)\tag{a}\f]的矩阵微分方程，其解为

\f[X=e^{At}X(0)+\int_0^te^{A(t-\tau)}B(\tau)\mathrm d\tau\tag{b}\f]

其中\f$e^{At}\f$为矩阵函数，可使用

- 矩阵的 Jordan 标准型
- 最小多项式的 Hamilton-Cayley 定理
- Laplace 变换

的方式进行求解，这里使用最小多项式的 Hamilton-Cayley 定理进行求解。

\f$A\f$ 的特征多项式为\f$\varphi(\lambda)=\lambda^2+3\lambda+2=(\lambda+1)(\lambda+2)\f$，无重根，因此\f$A\f$ 的最小多项式为\f$\mu(\lambda)=(\lambda+1)(\lambda+2)\f$，由 Hamilton-Cayley 定理可知\f$A^2+3A+2I=0\f$，对\f$e^{At}\f$进行 Taylor 展开，并使用带余除法，一定可以得到

\f[e^{At}=a_0I+a_1A\tag{c}\f]

将\f$\lambda=-1,-2\f$代入\f$e^{At}\f$，可得

\f[\left\{\begin{align}a_0+a_1(-1)&=e^{-t}\\a_0+a_1(-2)&=e^{-2t}\end{align}\right.\tag{d}\f]

解得\f$\left\{\begin{align}a_0&=2e^{-t}-e^{-2t}\\a_1&=e^{-t}-e^{-2t}\end{align}\right.\f$，因此

\f[\begin{align}e^{At}&=(2e^{-t}-e^{-2t})I+(e^{-t}-e^{-2t})A\\
&=\begin{bmatrix}2&2\\1&1\end{bmatrix}e^{-t}+\begin{bmatrix}-1&-2\\1&2\end{bmatrix}e^{-2t}
\end{align}\tag{e}\f]

可以算出

\f[e^{At}X(0)=\mat00e^{-t}+\mat1{-1}e^{-2t}=\mat1{-1}e^{-2t}\tag{f}\f]

同理有

\f[\int_0^te^{A(t-\tau)}B(\tau)\mathrm d\tau=\mat2{-1}\left(e^{-t}+t-1\right)+
\mat{-1}1\left(\frac14e^{-2t}+\frac12t-\frac14\right)\tag{g}\f]

整理得

\f[\begin{align}X&=e^{At}X(0)+\int_0^te^{A(t-\tau)}B(\tau)\mathrm d\tau\\
&=\mat{0.75}{-0.75}e^{-2t}+\mat2{-1}e^{-t}+\mat{1.5}{-0.5}t+\mat{-1.75}{0.75}\end{align}\tag{h}\f]

把 \f$t=1\f$ 代入上式，可得\f$X=\mat{0.75e^{-2}+2e^{-1}-0.25}{-0.75e^{-2}-e^{-1}+0.25}\approx\mat{0.587260}{-0.219381}\f$，数值解\f$X=\mat{0.587286}{-0.219401}\f$与之相符。

#### 3.5 示例代码

现给出上文使用 2 阶 2 级中点公式求解常微分方程组的示例代码

<div class="tabbed">

- <b class="tab-title">C++</b>

  ```cpp
  #include <rmvl/algorithm.hpp>

  #include <cstdio>

  int main() {
      rm::Ode dot_x1 = [](double t, const std::valarray<double> &x) { return 2 * x[1] + t; };
      rm::Ode dot_x2 = [](double, const std::valarray<double> &x) { return -x[0] - 3 * x[1]; };
      rm::Odes fs = {dot_x1, dot_x2};

      // 计算精确解
      double real_x1 = 0.75 * std::exp(-2) + 2 * std::exp(-1) - 0.25;
      double real_x2 = -0.75 * std::exp(-2) - std::exp(-1) + 0.25;
      // 2 阶 2 级中点公式
      rm::RungeKutta2 rk2(fs);
      // 设置初值
      rk2.init(0, {1, -1});
      // 计算数值解
      auto res2 = rk2.solve(0.01, 100).back();

      printf("x* = (%f, %f)\n", real_x1, real_x2);
      printf("x  = (%f, %f)\n", res2[0], res2[1]);
      // 输出结果为
      // x* = (0.587260, -0.219381)
      // x  = (0.587286, -0.219401)
  }
  ```

- <b class="tab-title">Python</b>

  ```python
  import rm
  import numpy as np

  fs = []
  fs.append(lambda t, x: 2 * x[1] + t)
  fs.append(lambda t, x: -x[0] - 3 * x[1])

  # 计算精确解
  real_x1 = 0.75 * np.exp(-2) + 2 * np.exp(-1) - 0.25
  real_x2 = -0.75 * np.exp(-2) - np.exp(-1) + 0.25
  # 2 阶 2 级中点公式
  rk2 = rm.RungeKutta2(fs)
  # 设置初值
  rk2.init(0, [1, -1])
  # 计算数值解
  res2 = rk2.solve(0.01, 100)[-1]

  print("x* = {:.6f}, {:.6f}".format(real_x1, real_x2))
  print("x  = {:.6f}, {:.6f}".format(res2[0], res2[1]))
  # 输出结果为
  # x* = 0.587260, -0.219381
  # x  = 0.587286, -0.219401
  ```

</div>
非线性方程（组）数值解与迭代法 {#tutorial_modules_func_iteration}
============

@author 赵曦
@date 2024/01/12
@version 1.0
@brief 涉及 **不动点迭代** 与 **牛顿迭代** 两种非线性方程组数值解的求法

@prev_tutorial{tutorial_modules_least_square}

@next_tutorial{tutorial_modules_runge_kutta}

@tableofcontents

------

### 1. 不动点迭代

对于一个函数\f$y=f(x)\f$，希望求其零点，即\f$f(x)=0\f$，可以将其改写成等价的

\f[x=\varphi(x)\tag{1-1}\f]

因此，选定一个初值 \f$x_0\f$，对 \f$\text{(1-1)}\f$ 构建迭代公式

\f[x_{k+1}=\varphi(x_k)\quad(k=0,1,\cdots)\tag{1-2}\f]

如果该迭代最终收敛于精确解，则这种方法被称为<b>不动点迭代法</b>，需要特别注意的是，选择的迭代函数\f$\varphi(x)\f$不能发散，也就是需要满足一定的收敛条件，这里给出<b>局部收敛</b>的判断方法。令\f$(x)=0\f$的精确解为\f$x^*\f$，若满足

\f[\left|\varphi'(x^*)\right|<1\tag{1-3}\f]

则迭代公式 \f$x=\varphi(x)\f$ 在 \f$x^*\f$ 处局部收敛。

<span style="color: green">**示例**</span>

用不动点迭代法求解 \f[x^3+4x^2-10=0\f]

**解答**

上式可以改写为\f[x=\frac12\sqrt{10-x^3}\f]，因此可以构建如下迭代方程

\f[x_{k+1}=\frac12\sqrt{10-x_k^3}\f]

### 2. Newton 迭代

对于方程\f$f(x)=0\f$，其精确解为\f$x^*\f$，对于某一次迭代结果\f$x_k\f$，将\f$f(x^*)\f$在\f$x_k\f$处展开，有

\f[0=f(x^*)=f(x_k)+f'(x_k)(x^*-x_k)+\frac12f''(x_k)(x^*-x_k)^2+\cdots\tag{2-1}\f]

即

\f[\begin{align}
x^*&=x_k-\frac{f(x_k)}{f'(x_k)}-\frac{f''(x_k)}{2f'(x_k)}(x^*-x_k)^2-\cdots\\
&\approx x_k-\frac{f(x_k)}{f'(x_k)}
\end{align}\tag{2-2}\f]

因此可以建立迭代方程

\f[f(x_{k+1})=x_k-\frac{f(x_k)}{f'(x_k)}\tag{2-3}\f]

上式就是 Newton 迭代的迭代公式，对于存在连续二阶导数的函数\f$f(x)\f$，Newton 迭代在用于求解单根（一重零点）的时候，至少具有 2 阶收敛性。在用于求解重根（\f$m\f$重零点）的时候，Newton 迭代仅具备 1 阶收敛性，需要改写为

\f[f(x_{k+1})=x_k-m\frac{f(x_k)}{f'(x_k)}\tag{2-3}\f]

方可实现 2 阶收敛。

### 3. Newton 迭代的简化

对于计算机算法而言，普通的 Newton 迭代需要求导\f$f'(x_k)\f$的操作，这一步可使用向后差商来代替

\f[f'(x_k)\approx\frac{f(x_k-x_{k-1})}{x_k-x_{k-1}}=f\left[x_k,x_{k-1}\right]\tag{3-1}\f]

此时的 Newton 迭代被称为离散 Newton 迭代，也称为弦截法，表示为如下形式，可参考 rm::NonlinearSolver

\f[f(x_{k+1})=x_k-\frac{f(x_k)}{f(x_k-x_{k-1})}(x_k-x_{k-1})\tag{3-2}\f]

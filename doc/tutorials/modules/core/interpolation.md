函数插值方法 {#tutorial_modules_interpolation}
============

@author 赵曦
@date 2024/01/11
@version 1.0
@brief 从 **Lagrange 插值** 与 **Newton 插值** 两种基函数选取方式介绍函数插值

@prev_tutorial{tutorial_modules_opcua}

@next_tutorial{tutorial_modules_least_square}

@tableofcontents

------

### 1. Lagrange 插值多项式

一般对于一个未知函数，若能从已知的一系列点<span style="color: red">近似</span>地使用多项式函数\f$p(x)\f$代替原函数\f$f(x)\f$，并且满足

\f[p(x_i)=f(x_i)=y_i\tag{1-1}\f]

则\f$p(x)\f$称为插值多项式。

也就是说，\f$p(x)\f$可以表示成多项式\f$\mathbb P[x]_n\f$的形式，即

\f[p(x)=a_0+a_1x+a_2x^2+\cdots+a_{n-1}x^{n-1}\tag{1-2}\f]

对于\f$n\f$个点，我们可以列出如下表格

\f[\begin{array}{c|cc}x&x_1&x_2&x_3&\cdots&x_n\\\hline f(x)&y_1&y_2&y_3&\cdots&y_n\end{array}\f]

根据\f$\text{(1-1)}\f$，将\f$n\f$个点分别代入\f$p(x)\f$，可以得到

\f[\left\{\begin{align}
p(x_1)&=a_0+a_1x_1+a_2x_1^2+\cdots+a_{n-1}x_1^{n-1}=f(x_1)=y_1\\
p(x_2)&=a_0+a_1x_2+a_2x_2^2+\cdots+a_{n-1}x_2^{n-1}=f(x_2)=y_2\\
&=\qquad\qquad\vdots\\
p(x_n)&=a_0+a_1x_n+a_2x_n^2+\cdots+a_{n-1}x_n^{n-1}=f(x_n)=y_n
\end{align}\right.\tag{1-3a}\f]

要求解\f$a_0\f$、\f$a_1\f$、\f$\cdots\f$、\f$a_{n-1}\f$，可以将\f$\text{(1-3a)}\f$改写成

\f[X\pmb a=\begin{bmatrix}1&x_1&\cdots&x_1^{n-1}\\1&x_2&\cdots&x_2^{n-1}\\\vdots&\vdots&\ddots&
\vdots\\1&x_n&\cdots&x_n^{n-1}\end{bmatrix}\begin{bmatrix}a_0\\a_1\\\vdots\\a_{n-1}\end{bmatrix}=
\begin{bmatrix}y_1\\y_2\\\vdots\\y_n\end{bmatrix}=\pmb f\tag{1-3b}\f]

我们选取的一组基是\f$\left(1,x,x^2,\cdots,x^n\right)\f$，在这里系数矩阵\f$X\f$是个 Vandermonde 矩阵，求解起来比较复杂。希望寻找一组合适的基，使得\f$\pmb a\f$的求解比较简单，如果在某组基\f$(l_0(x),l_1(x),l_2(x),\cdots,l_{n-1}(x))\f$下，系数矩阵\f$X\f$变为单位矩阵，即\f$X=I\f$，此时\f$\pmb a=\pmb f\f$，此时插值多项式就可以写为

\f[p(x)=y_1l_0(x)+y_2l_1(x)+y_3l_2(x)+\cdots+y_nl_{n-1}(x)\tag{1-4}\f]

要使得\f$X=I\f$，也就是需要满足

\f[l_i(x_j)=\left\{\begin{align}1,\quad i=j\\0,\quad i\neq j\end{align}\right.\tag{1-5}\f]

可以取

\f[l_i(x)=\prod\limits_{j=0,j\neq i}^{n-1}\frac{x-x_j}{x_i-x_j}=\frac{(x-x_0)(x-x_1)\cdots(x-x_{i-1})(x-x_{i+1})\cdots(x-x_{n-1})}{(x_i-x_0)(x_i-x_1)\cdots(x_i-x_{i-1})(x_i-x_{i+1})\cdots(x_i-x_{n-1})}\tag{1-6}\f]

这就是 Lagrange 插值基函数，\f$\text{(1-4)}\f$被称为 Lagrange 插值多项式。

<span style="color: green">**示例**</span>

使用以下节点进行二次插值

|  \f$x\f$   | \f$1\f$ | \f$2\f$ | \f$3\f$ |
| :--------: | :-----: | :-----: | :-----: |
| \f$f(x)\f$ | \f$2\f$ | \f$1\f$ | \f$2\f$ |

\f[\begin{align}
l_0(x)&=\prod\limits_{j=0,j\neq0}^2\frac{x-x_j}{x_i-x_j}=\frac{(x-2)(x-3)}{(1-2)(1-3)}=\frac12(x-2)(x-3)\\
l_1(x)&=\prod\limits_{j=0,j\neq1}^2\frac{x-x_j}{x_i-x_j}=\frac{(x-1)(x-3)}{(2-1)(2-3)}=-(x-1)(x-3)\\
l_2(x)&=\prod\limits_{j=0,j\neq2}^2\frac{x-x_j}{x_i-x_j}=\frac{(x-1)(x-2)}{(3-1)(3-2)}=\frac12(x-1)(x-2)\\
\end{align}\f]

因此，得到：

\f[\begin{align}p(x)&=y_1l_0(x)+y_2l_1(x)+y_3l_2(x)\\&=(x-2)(x-3)-(x-1)(x-3)+(x-1)(x-2)\\&=x^2-4x+5\end{align}\f]

@note 一般的，\f$n\f$点\f$n\f$次插值得到的插值多项式是<span style="color: red">**唯一**</span>的，读者可以自行使用待定系数法验证

### 2. Newton 插值多项式

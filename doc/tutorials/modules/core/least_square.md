最小二乘法 {#tutorial_modules_least_square}
============

@author 赵曦
@date 2023/11/01

@prev_tutorial{tutorial_modules_kalman}

@next_tutorial{tutorial_modules_union_find}

@tableofcontents

------

### 1. 概念

最小二乘法诞生于统计学，其最初的目标是使用一条直线拟合一系列离散的点，那么我们该如何寻找这一直线？最小二乘法的原理是使得拟合直线的误差的平方和最小。这里的误差定义成每一个离散的点\f$x_i\f$与拟合直线的 **距离** 。在这一最初的用法中，点到直线的距离能够很直观的描述成点到直线的 **垂线** 的长度。

<center>
![ls-line-vertical](ls-line-vertical.png)

图 1-1
</center>

如图 1-1 所示，令直线外一点为\f$P\f$，直线为\f$l\f$，垂线为\f$l_0\f$，垂足为点\f$O\f$。最小二乘法就是构建一条直线，使得这些点\f$P_i\f$到这条直线的距离平方和最小，此时\f$O_i\f$点被称为 **最小二乘解** 。

让我们拓宽最小二乘这一概念。对于一个向量\f$\pmb\alpha\f$所表示的直线，在该直线上是否存在一点\f$Y\f$使得\f$Y\f$到直线外一点\f$P\f$的距离最小？答案是肯定的，并且原理与图 1 完全一致，即构造\f$P\f$到直线的垂线，在线性代数中我们引入过 **内积** 运算，并且得知两个向量 **正交** （垂直）的时候，二者内积为\f$0\f$。

@note
设 \f$V\f$ 是实数域 \f$\mathbb{R}\f$ 上的线性空间。如果对 \f$V\f$ 中任意两个向量 \f$\pmb\alpha\f$，\f$\pmb\beta\f$ 都有一个实数（记为\f$(\pmb\alpha,\pmb\beta)\f$）与它们相对应，并且满足下列各个条件，则实数 \f$(\pmb\alpha,\pmb\beta)\f$ 称为向量 \f$\pmb\alpha\f$，\f$\pmb\beta\f$ 的内积，而线性空间 \f$V\f$ 则称为 **实内积空间** ，简称 **内积空间** ：
- \f$(\pmb\alpha, \pmb\beta)=(\pmb\beta,\pmb\alpha)\f$
- \f$(k\pmb\alpha,\pmb\beta)=k(\pmb\alpha,\pmb\beta)\f$
- \f$(\pmb\alpha+\pmb\beta,\pmb\gamma)=(\pmb\alpha+\pmb\gamma,\pmb\beta+\pmb\gamma)\f$
- \f$(\pmb\alpha,\pmb\alpha)\geq0，当且仅当\pmb\alpha=\pmb0，等号成立\f$

在初等几何中，点到直线（或平面）上所有点的距离以垂线最短。同样的，对于一个 **欧式空间** \f$V=\mathbb{R}^n\f$，一个指定向量 \f$\pmb\beta\in V\f$ 和子空间 \f$W\subseteq V\f$ 的各个向量距离也以 **垂线** 最短。换句话说，该向量 \f$\pmb\beta\f$ 与子空间 \f$W\f$ 正交，记作 \f$\pmb\beta\perp W\f$。其中子空间可以表示成

\f[W=L(\pmb\alpha_1,\pmb\alpha_2,\cdots,\pmb\alpha_s)\tag{1-1}\f]

其中 \f$\pmb\alpha_1,\ \pmb\alpha_2,\ \cdots,\ \pmb\alpha_s\f$ 是一 \f$W\f$ 的一组基。从上式可以推导出，若 \f$\exists\pmb\beta\in V\f$，使得 \f$\pmb\beta\perp W\f$，则对于 \f$\forall\pmb\alpha\in W\f$，均有 \f$\pmb\beta\perp\pmb\alpha\f$。因此可以写为

\f[\begin{matrix}
(\pmb\alpha_1,\pmb\beta)&=&\pmb\alpha_1^T\pmb\beta&=&0\\
(\pmb\alpha_2,\pmb\beta)&=&\pmb\alpha_2^T\pmb\beta&=&0\\
\vdots&=&\vdots&=&\vdots\\
(\pmb\alpha_n,\pmb\beta)&=&\pmb\alpha_n^T\pmb\beta&=&0
\end{matrix}\tag{1-2a}\f]

观察中间一列 \f$\pmb\alpha_i^T\pmb\beta\f$，可以写成

\f[
(\pmb\alpha_1,\pmb\alpha_2,\cdots,\pmb\alpha_i)^T\pmb\beta=\pmb 0\tag{1-2b}
\f]

我们先用一个最简单的例子。

<center>
![ls-vector-vertical](ls-vector-vertical.png)

图 1-2
</center>

对于图 1-2 中使用向量表示的方式，最小二乘解可以表示为向量\f$OP\f$的坐标，在这种情况下，\f$BP\f$与\f$OA\f$垂直，即：

\f[\left(OA,BP\right)=0\tag{1-3}\f]

我们令\f$OA=\pmb a\f$，\f$OB=\pmb b\f$，待求向量\f$OP=\pmb y\f$，（\f$\pmb a, \pmb b, \pmb y\f$均为列向量）代入公式 \f$\text{(1-3)}\f$ 可以得到

\f[(\pmb a,\ \pmb y-\pmb b)=0\tag{1-4}\f]

其中 \f$\pmb y-\pmb b\f$ 就是公式 \f$\text{(1-2a)}\f$ 中的 \f$\pmb\beta\f$，写成矩阵形式

\f[
\begin{align}
\pmb a^T(\pmb y-\pmb b)&=0\\
\pmb y&=\left(\pmb a^T\right)^{-1}\pmb a^T\pmb b
\end{align}\tag{1-5}
\f]

为此我们得到了最小二乘解 \f$\pmb y\f$ 的表达式，这也为后文超定线性方程组的最小二乘解提供了理论基础。

### 2. 超定线性方程组

有时候我们会遇到这一类形如 \f$A\pmb x=\pmb b\f$ 的方程组

\f[\left\{\begin{align}2x_1+x_2&=1\\x_1-x_2&=0\\x_1+x_2&=2\end{align}\right.\qquad\Leftrightarrow\qquad
\begin{bmatrix}2&1\\1&-1\\1&1\end{bmatrix}\begin{bmatrix}x_1\\x_2\end{bmatrix}=\begin{bmatrix}1\\0\\2\end{bmatrix}\tag{2-1}\f]

这里 \f$A=\begin{bmatrix}2&1\\1&-1\\1&1\end{bmatrix}\f$，\f$\pmb b=(1,0,2)^T\f$，在图 1-3 中表示如下。

<center>
![ls-equations](ls-equations.png)

图 1-3
</center>

一般的，对于一个系数矩阵 \f$A=(a_{ij})_{s\times{n}}\f$，\f$\pmb b=(b_1,b_2,\cdots,b_s)^T\f$，\f$\pmb x=(x_1,x_2,\cdots,x_s)^T\f$，若满足 \f$\text{rank}(A)\leq s\f$ 时，线性方程组没有数值解，但我们希望找到一组 **最优解** ，衡量此最优解的方法仍然可以采用最小二乘法。设法找出一组解 \f$\hat{\pmb x}=(x_1^0,\ x_2^0,\ x_3^0,\ \cdots,\ x_n^0)\f$ 使得每一项的误差 \f$\delta_i\f$ 平方和最小，如何定量这个误差？能否继续采用最小二乘法的点到直线的最短距离作为出发点？答案是肯定的，这里先给出平方误差的表达式。

\f[
\delta^2=\sum_{i=1}^s(a_{i1}x_1+a_{i2}x_2+\cdots+a_{in}x_n-b_i)^2\tag{2-2}
\f]

上式的基本想法是，将线性方程组的<span style="color: red">每一个方程</span>在带入 \f$\hat{\pmb x}\f$ 后与右端项 \f$b_i\f$ 作差，即可得到每一项的误差 \f$\delta_i\f$，我们令 \f$\pmb y=A\pmb x\f$，则 \f$\pmb y\f$ 当然是个 \f$s\f$ 维列向量，上述平方偏差 \f$\delta^2\f$ 也就是 \f$|\pmb y-\pmb b|^2\f$，而最小二乘法就是要找一组 \f$\hat{\pmb x}\f$ 使得 \f$|\pmb y-\pmb b|^2\f$ 最小，换句话说，就是要使 \f$\pmb y\f$ 与 \f$\pmb b\f$ 的距离最小。

到这里，我们需要进一步强化线性方程组及其生成空间的这些概念。

**基本概念介绍**

1. 对于一般的线性方程组（方程数\f$\leq\f$未知数个数\f$n\f$），\f$A\pmb x=\pmb0\f$ 的解（也叫通解）生成的空间 \f$\pmb x\f$ 称为<span style="color: red">解空间</span>，其极大线性无关组被称为 **基础解系** ，即解空间的任意一个向量都可由基础解系所线性表出，即 \f$\pmb x=k_1\pmb\xi_1+k_2\pmb\xi_2+\cdots+k_t\pmb\xi_t\f$，其中满足 \f[\text{rank}(A)+t=n\tag{2-3}\f]
2. 对于任意的线性方程组（可以是超定的）\f$A\f$ 是\f$s\times n\f$系数矩阵，\f$A\f$ 的列向量组合生成的空间，即 \f$x_1\pmb\alpha_1+x_2\pmb\alpha_2+\cdots+x_n\pmb\alpha_n\f$ 的空间，即代表 \f$A\pmb x\f$ 所生成的空间。同样的，可以写为 \f$L(\pmb\alpha_1,\pmb\alpha_2,\cdots,\pmb\alpha_s)\f$，对于 \f$A\pmb x\f$ 生成的空间，后文简称为<span style="color: red">列空间</span>
3. \f$\pmb b\f$ 是方程组右端项，表示列空间之外的一个向量，即在超定线性方程组中，\f$\pmb b\notin L(\pmb\alpha_1,\pmb\alpha_2,\cdots,\pmb\alpha_s)\f$，因此才具备向量 \f$\pmb b\f$ 到 \f$A\pmb x\f$ 的距离的概念

让我们继续回到最小二乘解的求解过程中，根据 \f$\text{(1-2b)}\f$ 和 \f$\text{(1-4)}\f$，类似的，我们可以构建出

\f[\begin{matrix}
(\pmb\alpha_1,\pmb y-\pmb b)&=&\pmb\alpha_1^T(\pmb y-\pmb b)&=&0\\
(\pmb\alpha_2,\pmb y-\pmb b)&=&\pmb\alpha_2^T(\pmb y-\pmb b)&=&0\\
\vdots&=&\vdots&=&\vdots\\
(\pmb\alpha_s,\pmb y-\pmb b)&=&\pmb\alpha_s^T(\pmb y-\pmb b)&=&0\\
\end{matrix}\tag{2-4}\f]

这表示当 \f$\pmb y-\pmb b\f$ 与列空间正交时，\f$\pmb y\f$ 与 \f$\pmb b\f$ 距离最短，与列空间正交就必须要与生成列空间的每一个向量即 \f$\pmb\alpha_1,\pmb\alpha_2,\cdots,\pmb\alpha_s\f$ 正交。整合后可以得到

\f[(\pmb\alpha_1,\pmb\alpha_2,\cdots,\pmb\alpha_s)^T(\pmb y-\pmb b)=0\tag{2-5}\f]

\f$\pmb\alpha_1,\pmb\alpha_2,\cdots,\pmb\alpha_s\f$ 刚好是系数矩阵按列分块的形式，又有 \f$\pmb y=A\hat{\pmb x}\f$ 我们可以得到如下公式。

\f[
\begin{align}
A^T(A\hat{\pmb x}-\pmb b)&=0\\
A^TA\hat{\pmb x}&=A^T\pmb b
\end{align}
\f]

经过化简我们最终可以得到

\f[
\boxed{\hat{\pmb x}=\left(A^TA\right)^{-1}A^T\pmb b}\tag{2-6}
\f]

这就是最小二乘解所满足的代数方程。

### 3. 例子

下面给出两个例子，用图形的方式表示超定线性方程组的最小二乘解（此处不会当做计算题死板的套公式）

<span style="color: green">**示例 1**</span>

问：求数 \f$1,\ 2\f$ 的最小二乘解

显而易见，求算术平均数即可得到最小二乘解为 \f$\frac32\f$。但为何呢？

我们将其转化为一个线性方程组

\f[\left\{\begin{align}
x_1&=1\\
x_1&=2
\end{align}\right.\tag{3-1}\f]

因此，系数矩阵 \f$A=(1,\ 1)^T\f$，列空间 \f$\pmb y=A\pmb x=(x_1,\ x_1)\f$，右端项为 \f$\pmb b=(1,\ 2)^T\f$。可以画出一个平面笛卡尔坐标系，以表示这一列空间和指定向量 \f$\pmb b\f$，如图 2-1，满足

\f[\left\{\begin{align}
x&=x_1\\
y&=x_1
\end{align}\right.\tag{3-2}\f]

这是一个参数方程，消去 \f$x_1\f$ 可以得到 \f$y=x\f$，这就是系数矩阵 \f$A=(1,\ 1)^T\f$ 所生成的列空间。

<center>
![ls-eg1](ls-eg1.png)

图 2-1
</center>

相当于现在需要在 \f$y=x\f$ 上找到一个点，使得其到 \f$\pmb b\f$ 的距离最短，这是初等几何的内容，做垂线即可，最终得到交点的坐标 

\f[(x,y)=(\frac32,\frac32)\tag{3-3}\f]

代入参数方程 \f$\text{(3-2)}\f$，可以得到

\f[x_1=\frac32\f]

<span style="color: green">**示例 2**</span>

题目：求公式 \f$\text{(2-1)}\f$ 的最小二乘解

系数矩阵：\f$A=\begin{bmatrix}2&1\\1&-1\\1&1\end{bmatrix}\f$，其生成的列空间为图 2-2 中<span style="color: orange">橙色的平面</span>，记作平面 \f$\alpha\f$，并对 \f$A\f$ 作列分块得到 \f$A=(\pmb\alpha_1,\pmb\alpha_2)\f$

列空间外向量（右端项）：\f$\pmb b=(1,0,2)^T\f$

<center>
![ls-eg2](ls-eg2.png)

图 2-2
</center>

需要满足 \f$(\pmb y-\pmb b)\perp\alpha\f$，则需要分别满足 \f$(\pmb y-\pmb b)\perp\alpha_1\f$ 和 \f$(\pmb y-\pmb b)\perp\alpha_2\f$，下面使用几何法对最小二乘法的原理进行验证。

① 求平面 \f$\alpha\f$ 的法向量 \f$\pmb n\f$

\f[
\pmb n=\left|\begin{matrix}\pmb i&\pmb j&\pmb k\\1&-1&1\\2&1&1\end{matrix}\right|
=-2\pmb i+\pmb j+3\pmb k=\begin{bmatrix}-2\\1\\3\end{bmatrix}\tag{3-4}
\f]

② 根据约束条件列方程

满足 2 个 <span style="color: red">**约束条件**</span> ，即

- \f$\pmb y-\pmb b=k\pmb n\f$，表示 \f$\pmb y-\pmb b\f$ 是平面 \f$\alpha\f$ 的一个法向量
- \f$\pmb y=x_1\pmb\alpha_1+x_2\pmb\alpha_2\f$，表示 \f$\pmb y\in L(\pmb\alpha_1,\pmb\alpha_2)\f$，即 \f$\pmb y\f$ 是列空间中的一个向量

联立得到

\f[k\pmb n+\pmb b=x_1\pmb\alpha_1+x_2\pmb\alpha_2\tag{3-5a}\f]

即

\f[\left\{\begin{align}
-2k+1&=2x_1+x_2\\k&=x_1-x_2\\3k+2&=x_1+x_2
\end{align}\right.\tag{3-5b}\f]

③ 求解 \f$\hat{\pmb x}\f$

这是一个关于 \f$k,x_1,x_2\f$ 的线性方程组，解得

\f[X=(-\frac27,\frac37,\frac57)^T\tag{3-6}\f]

因此我们得到了满足以上 <span style="color: red">**约束条件**</span> 的解

\f[\hat{\pmb x}=\left(\frac37,\frac57\right)^T\tag{3-7}\f]

### 4. 部署使用

OpenCV 中提供了最小二乘法求解的函数 `cv::solve()`，并设置有 2 个相关的成员函数，其函数原型如下。此外可参考 [cv::solve](https://docs.opencv.org/4.x/d2/de8/group__core__array.html#ga12b43690dbd31fed96f213eefead2373) 的 OpenCV 手册。

```cpp
bool cv::solve(
    InputArray src1,
    InputArray src2,
    OutputArray dst,
    int flags = DECOMP_LU 
);
```

有关递推最小二乘法的介绍可参考 @ref tutorial_extra_spi_rune_predictor 。

---

### 参考书籍

\cite tongji_linear_algebra \cite luo2013matrix

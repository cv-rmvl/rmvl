最小二乘法 {#tutorial_modules_least_square}
============

@author 赵曦
@date 2023/11/10
@version 1.0
@brief 从 **向量到子空间距离** 和 **构建法方程** 两种方式推导了最小二乘法的矩阵表示

@prev_tutorial{tutorial_modules_interpolation}

@next_tutorial{tutorial_modules_lsqnonlin}

@tableofcontents

------

### 1. 概念

最小二乘法诞生于统计学，其最初的目标是使用一条直线拟合一系列离散的点，那么我们该如何寻找这一直线？最小二乘法的原理是使得拟合直线的误差的平方和最小。这里的误差定义成每一个离散的点\f$x_i\f$与拟合直线的 **距离** 。在这一最初的用法中，点到直线的距离能够很直观的描述成点到直线的 **垂线** 的长度。

<center>

![图 1-1 直线的垂线的长度](lsq/line-vertical.png)

</center>

如图 1-1 所示，令直线外一点为\f$P\f$，直线为\f$l\f$，垂线为\f$l_0\f$，垂足为点\f$O\f$。最小二乘法就是构建一条直线，使得这些点\f$P_i\f$到这条直线的距离平方和最小，此时\f$O_i\f$点被称为 **最小二乘解** 。

让我们拓宽最小二乘这一概念。对于一个向量\f$\boldsymbol\alpha\f$所表示的直线，在该直线上是否存在一点\f$Y\f$使得\f$Y\f$到直线外一点\f$P\f$的距离最小？答案是肯定的，并且原理与图 1 完全一致，即构造\f$P\f$到直线的垂线，在线性代数中我们引入过 **内积** 运算，并且得知两个向量 **正交** （垂直）的时候，二者内积为\f$0\f$。

@note
设 \f$V\f$ 是实数域 \f$\mathbb{R}\f$ 上的线性空间。如果对 \f$V\f$ 中任意两个向量 \f$\boldsymbol\alpha\f$，\f$\boldsymbol\beta\f$ 都有一个实数（记为\f$(\boldsymbol\alpha,\boldsymbol\beta)\f$）与它们相对应，并且满足下列各个条件，则实数 \f$(\boldsymbol\alpha,\boldsymbol\beta)\f$ 称为向量 \f$\boldsymbol\alpha\f$，\f$\boldsymbol\beta\f$ 的内积，而线性空间 \f$V\f$ 则称为 **实内积空间** ，简称 **内积空间** ：
- \f$(\boldsymbol\alpha, \boldsymbol\beta)=(\boldsymbol\beta,\boldsymbol\alpha)\f$
- \f$(k\boldsymbol\alpha,\boldsymbol\beta)=k(\boldsymbol\alpha,\boldsymbol\beta)\f$
- \f$(\boldsymbol\alpha+\boldsymbol\beta,\boldsymbol\gamma)=(\boldsymbol\alpha+\boldsymbol\gamma,\boldsymbol\beta+\boldsymbol\gamma)\f$
- \f$(\boldsymbol\alpha,\boldsymbol\alpha)\geq0，当且仅当\boldsymbol\alpha=\boldsymbol0，等号成立\f$

在初等几何中，点到直线（或平面）上所有点的距离以垂线最短。同样的，对于一个 **欧式空间** \f$V=\mathbb{R}^n\f$，一个指定向量 \f$\boldsymbol\beta\in V\f$ 和子空间 \f$W\subseteq V\f$ 的各个向量距离也以 **垂线** 最短。换句话说，该向量 \f$\boldsymbol\beta\f$ 与子空间 \f$W\f$ 正交，记作 \f$\boldsymbol\beta\perp W\f$。其中子空间可以表示成

\f[W=L(\boldsymbol\alpha_1,\boldsymbol\alpha_2,\cdots,\boldsymbol\alpha_s)\tag{1-1}\f]

其中 \f$\boldsymbol\alpha_1,\ \boldsymbol\alpha_2,\ \cdots,\ \boldsymbol\alpha_s\f$ 是一 \f$W\f$ 的一组基。从上式可以推导出，若 \f$\exists\boldsymbol\beta\in V\f$，使得 \f$\boldsymbol\beta\perp W\f$，则对于 \f$\forall\boldsymbol\alpha\in W\f$，均有 \f$\boldsymbol\beta\perp\boldsymbol\alpha\f$。因此可以写为

\f[\begin{matrix}
(\boldsymbol\alpha_1,\boldsymbol\beta)&=&\boldsymbol\alpha_1^T\boldsymbol\beta&=&0\\
(\boldsymbol\alpha_2,\boldsymbol\beta)&=&\boldsymbol\alpha_2^T\boldsymbol\beta&=&0\\
\vdots&=&\vdots&=&\vdots\\
(\boldsymbol\alpha_s,\boldsymbol\beta)&=&\boldsymbol\alpha_s^T\boldsymbol\beta&=&0
\end{matrix}\tag{1-2a}\f]

观察中间一列 \f$\boldsymbol\alpha_i^T\boldsymbol\beta\f$，可以写成

\f[
(\boldsymbol\alpha_1,\boldsymbol\alpha_2,\cdots,\boldsymbol\alpha_i)^T\boldsymbol\beta=\boldsymbol 0\tag{1-2b}
\f]

我们先用一个最简单的例子。

<center>

![图 1-2 最小二乘解表示为向量 OP](lsq/vector-vertical.png)

</center>

对于图 1-2 中使用向量表示的方式，最小二乘解可以表示为向量\f$OP\f$的坐标，在这种情况下，\f$BP\f$与\f$OA\f$垂直，即：

\f[\left(OA,BP\right)=0\tag{1-3}\f]

我们令\f$OA=\boldsymbol a\f$，\f$OB=\boldsymbol b\f$，待求向量\f$OP=\boldsymbol y\f$，（\f$\boldsymbol a, \boldsymbol b, \boldsymbol y\f$ 均为列向量）代入公式 \f$\text{(1-3)}\f$ 可以得到

\f[(\boldsymbol a,\ \boldsymbol y-\boldsymbol b)=0\tag{1-4}\f]

其中 \f$\boldsymbol y-\boldsymbol b\f$ 就是公式 \f$\text{(1-2a)}\f$ 中的 \f$\boldsymbol\beta\f$，写成矩阵形式

\f[\boldsymbol a^T(\boldsymbol y-\boldsymbol b)=0\tag{1-5}\f]

为此我们得到了最小二乘解 \f$\boldsymbol y\f$ 的表达式，这也为后文超定线性方程组的最小二乘解提供了理论基础。

### 2. 超定线性方程组

有时候我们会遇到这一类形如 \f$A\boldsymbol x=\boldsymbol b\f$ 的方程组

\f[\left\{\begin{align}2x_1+x_2&=1\\x_1-x_2&=0\\x_1+x_2&=2\end{align}\right.\qquad\Leftrightarrow\qquad
\begin{bmatrix}2&1\\1&-1\\1&1\end{bmatrix}\begin{bmatrix}x_1\\x_2\end{bmatrix}=\begin{bmatrix}1\\0\\2\end{bmatrix}\tag{2-1}\f]

这里 \f$A=\begin{bmatrix}2&1\\1&-1\\1&1\end{bmatrix}\f$，\f$\boldsymbol b=(1,0,2)^T\f$，在图 1-3 中表示如下。

<center>

![图 1-3 超定线性方程组](lsq/equations.png)

</center>

一般的，对于一个系数矩阵 \f$A=(a_{ij})_{s\times{n}}\f$，\f$\boldsymbol b=(b_1,b_2,\cdots,b_s)^T\f$，\f$\boldsymbol x=(x_1,x_2,\cdots,x_s)^T\f$，若满足 \f$\text{rank}(A)\leq s\f$ 时，线性方程组没有数值解，但我们希望找到一组 **最优解** ，衡量此最优解的方法仍然可以采用最小二乘法。设法找出一组解 \f$\hat{\boldsymbol x}=(x_1^0,\ x_2^0,\ x_3^0,\ \cdots,\ x_n^0)\f$ 使得每一项的误差 \f$\delta_i\f$ 平方和最小，如何定量这个误差？能否继续采用上文最小二乘法的点到直线的最短距离的思想作为出发点？答案是肯定的，这里先给出误差平方和的表达式。

\f[\delta^2=\sum_{i=0}^{s-1}(a_{i1}x_1+a_{i2}x_2+\cdots+a_{in}x_n-b_i)^2\tag{2-2}\f]

上式也可以写成

\f[\delta^2=\sum_{i=0}^{s-1}\left[\left(\sum_{j=1}^na_{ij}x_j\right)-b_i\right]^2\tag{2-3a}\f]

或

\f[\delta^2=\left\|\left(\sum_{j=1}^na_{ij}x_j\right)-b_i\right\|_2^2\tag{2-3b}\f]

@note 公式 \f$\text{(2-3b)}\f$ 中出现的形如 \f$\left\|A\right\|_2\f$ 的部分也叫做向量的2-范数。

上式的基本想法是，将线性方程组的<span style="color: red">每一个方程</span>在带入 \f$\hat{\boldsymbol x}\f$ 后与右端项 \f$b_i\f$ 作差，即可得到每一项的误差 \f$\delta_i\f$，我们令 \f$\boldsymbol y=A\boldsymbol x\f$，则 \f$\boldsymbol y\f$ 当然是个 \f$s\f$ 维列向量，上述平方偏差 \f$\delta^2\f$ 也就是 \f$|\boldsymbol y-\boldsymbol b|^2\f$，而最小二乘法就是要找一组 \f$\hat{\boldsymbol x}\f$ 使得 \f$|\boldsymbol y-\boldsymbol b|^2\f$ 最小，换句话说，就是要使 \f$\boldsymbol y\f$ 与 \f$\boldsymbol b\f$ 的距离最小。

到这里，我们需要进一步强化线性方程组及其生成空间的这些概念。

**基本概念介绍**

1. 对于任意的线性方程组（可以是超定的）\f$A\f$ 是\f$s\times n\f$系数矩阵，\f$A\f$ 的列向量组合生成的空间，即 \f$x_1\boldsymbol\alpha_1+x_2\boldsymbol\alpha_2+\cdots+x_n\boldsymbol\alpha_n\f$ 的空间，即代表 \f$A\boldsymbol x\f$ 所生成的空间。同样的，可以写为 \f$L(\boldsymbol\alpha_1,\boldsymbol\alpha_2,\cdots,\boldsymbol\alpha_n)\f$，对于 \f$A\boldsymbol x\f$ 生成的空间，后文简称为<span style="color: red">列空间</span>。使用线性变换的角度进行分析，设 \f$T\f$ 是线性空间 \f$V\f$ 的线性变换，则\f[T(V)=\{T\boldsymbol\alpha|\boldsymbol\alpha\in V\}\f]是 \f$V\f$ 的子空间，因此方程组的系数矩阵 \f$A\f$ 就可以当做是某个线性变换 \f$T(V)\f$ 对应的矩阵，表示为\f[(T\boldsymbol\alpha_1,T\boldsymbol\alpha_2,\cdots,T\boldsymbol\alpha_n)=(\boldsymbol\alpha_1,\boldsymbol\alpha_2,\cdots,\boldsymbol\alpha_n)A\f]\f$T(V)\f$ 的维数叫线性变换 \f$T\f$ 的秩，也可以记为 \f$\text{rank}(A)\f$。因此列空间也可以被称为<span style="color: red">象子空间</span>；
2. 对于一般的线性方程组（方程数\f$\leq\f$未知数个数\f$n\f$），\f$A\boldsymbol x=\boldsymbol0\f$ 的解（也叫通解）生成的空间 \f$\boldsymbol x\f$ 称为<span style="color: red">解空间</span>。使用线性变换的角度进行分析，设 \f$T\f$ 是线性空间 \f$V\f$ 的线性变换，则集合\f[K=\{\boldsymbol\alpha\in V|T\boldsymbol\alpha=\boldsymbol0\}\f]是 \f$V\f$ 的子空间，称为线性变换 \f$T\f$ 的核，记作 \f$T^{-1}(\boldsymbol0)\f$，因此解空间也被称为<span style="color: red">核空间</span>。另外，在方程组中，解空间的极大线性无关组被称为 **基础解系** ，即解空间的任意一个向量都可由基础解系所线性表出，即 \f$\boldsymbol x=k_1\boldsymbol\xi_1+k_2\boldsymbol\xi_2+\cdots+k_t\boldsymbol\xi_t\f$，不难得出结论，\f$T^{-1}(\boldsymbol0)\f$ 的维度就是 \f$t\f$。并且可以证明出如下的 **子空间维数定理** ：设 \f$T\f$ 是 \f$n\f$ 维线性空间 \f$V\f$ 的线性变换，则一定满足维数关系\f[\text{dim}T(V)+\text{dim}T^{-1}(\boldsymbol0)=n\f]也可以记为\f[\text{rank}(A)+t=n\tag{2-4}\f]
3. \f$\boldsymbol b\f$ 是方程组右端项，表示列空间之外的一个向量，即在超定线性方程组中，\f$\boldsymbol b\notin L(\boldsymbol\alpha_1,\boldsymbol\alpha_2,\cdots,\boldsymbol\alpha_n)\f$，因此才具备向量 \f$\boldsymbol b\f$ 到 \f$A\boldsymbol x\f$ 的距离的概念

让我们继续回到最小二乘解的求解过程中，根据 \f$\text{(1-2b)}\f$ 和 \f$\text{(1-4)}\f$，类似的，我们可以构建出

\f[\begin{matrix}
(\boldsymbol\alpha_1,\boldsymbol y-\boldsymbol b)&=&\boldsymbol\alpha_1^T(\boldsymbol y-\boldsymbol b)&=&0\\
(\boldsymbol\alpha_2,\boldsymbol y-\boldsymbol b)&=&\boldsymbol\alpha_2^T(\boldsymbol y-\boldsymbol b)&=&0\\
\vdots&=&\vdots&=&\vdots\\
(\boldsymbol\alpha_n,\boldsymbol y-\boldsymbol b)&=&\boldsymbol\alpha_n^T(\boldsymbol y-\boldsymbol b)&=&0\\
\end{matrix}\tag{2-5}\f]

这表示当 \f$\boldsymbol y-\boldsymbol b\f$ 与列空间正交时，\f$\boldsymbol y\f$ 与 \f$\boldsymbol b\f$ 距离最短，与列空间正交就必须要与生成列空间的每一个向量即 \f$\boldsymbol\alpha_1,\boldsymbol\alpha_2,\cdots,\boldsymbol\alpha_n\f$ 正交。整合后可以得到

\f[(\boldsymbol\alpha_1,\boldsymbol\alpha_2,\cdots,\boldsymbol\alpha_n)^T(\boldsymbol y-\boldsymbol b)=\boldsymbol0\tag{2-6}\f]

\f$\boldsymbol\alpha_1,\boldsymbol\alpha_2,\cdots,\boldsymbol\alpha_n\f$ 刚好是系数矩阵按列分块的形式，又有 \f$\boldsymbol y=A\hat{\boldsymbol x}\f$ 我们可以得到如下公式。

\f[
\begin{align}
A^T(A\hat{\boldsymbol x}-\boldsymbol b)&=0\\
A^TA\hat{\boldsymbol x}&=A^T\boldsymbol b
\end{align}
\f]

经过化简我们最终可以得到

\f[
\boxed{\hat{\boldsymbol x}=\left(A^TA\right)^{-1}A^T\boldsymbol b}\tag{2-7}
\f]

这就是最小二乘解所满足的代数方程。

### 3. 示例

下面给出 3 个示例，不直接使用公式 \f$\text{(2-7)}\f$，通过几何或者其他手段来表示最小二乘解。

<span style="color: green">**示例 1**</span>

问：求数 \f$1,\ 2\f$ 的最小二乘解

显而易见，求算术平均数即可得到最小二乘解为 \f$\frac32\f$。但为何呢？

我们将其转化为一个线性方程组

\f[\left\{\begin{align}
x_1&=1\\
x_1&=2
\end{align}\right.\tag{3-1}\f]

因此，系数矩阵 \f$A=(1,\ 1)^T\f$，列空间 \f$\boldsymbol y=A\boldsymbol x=(x_1,\ x_1)\f$，右端项为 \f$\boldsymbol b=(1,\ 2)^T\f$。可以画出一个平面笛卡尔坐标系，以表示这一列空间和指定向量 \f$\boldsymbol b\f$，如图 2-1，满足

\f[\left\{\begin{align}
x&=x_1\\
y&=x_1
\end{align}\right.\tag{3-2}\f]

这是一个参数方程，消去 \f$x_1\f$ 可以得到 \f$y=x\f$，这就是系数矩阵 \f$A=(1,\ 1)^T\f$ 所生成的列空间。

<center>

![图 2-1 二维列空间的最小二乘解几何解释](lsq/eg1.png)

</center>

相当于现在需要在 \f$y=x\f$ 上找到一个点，使得其到 \f$\boldsymbol b\f$ 点的距离最短，这是初等几何的内容，做垂线即可，最终得到交点的坐标 

\f[\hat y=(\frac32,\frac32)\tag{3-3}\f]

代入参数方程 \f$\text{(3-2)}\f$，可以得到

\f[x_1=\frac32\f]

<span style="color: green">**示例 2**</span>

题目：求公式 \f$\text{(2-1)}\f$ 的最小二乘解

系数矩阵：\f$A=\begin{bmatrix}2&1\\1&-1\\1&1\end{bmatrix}\f$，其生成的列空间为图 2-2 中<span style="color: orange">橙色的平面</span>，记作平面 \f$\alpha\f$，并对 \f$A\f$ 作列分块得到 \f$A=(\boldsymbol\alpha_1,\boldsymbol\alpha_2)\f$

列空间外向量（右端项）：\f$\boldsymbol b=(1,0,2)^T\f$

<center>

![图 2-2 三维列空间的最小二乘解几何解释](lsq/eg2.png)

</center>

需要满足 \f$(\boldsymbol y-\boldsymbol b)\perp\alpha\f$，则需要分别满足 \f$(\boldsymbol y-\boldsymbol b)\perp\boldsymbol\alpha_1\f$ 和 \f$(\boldsymbol y-\boldsymbol b)\perp\boldsymbol\alpha_2\f$，下面使用几何法对最小二乘法的原理进行验证。

① 求平面 \f$\alpha\f$ 的法向量 \f$\boldsymbol n\f$

\f[
\boldsymbol n=\left|\begin{matrix}\boldsymbol i&\boldsymbol j&\boldsymbol k\\1&-1&1\\2&1&1\end{matrix}\right|
=-2\boldsymbol i+\boldsymbol j+3\boldsymbol k=\begin{bmatrix}-2\\1\\3\end{bmatrix}\tag{3-4}
\f]

② 根据约束条件列方程

满足 2 个 <span style="color: red">**约束条件**</span> ，即

- \f$\boldsymbol y-\boldsymbol b=k\boldsymbol n\f$，表示 \f$\boldsymbol y-\boldsymbol b\f$ 是平面 \f$\alpha\f$ 的一个法向量
- \f$\boldsymbol y=x_1\boldsymbol\alpha_1+x_2\boldsymbol\alpha_2\f$，表示 \f$\boldsymbol y\in L(\boldsymbol\alpha_1,\boldsymbol\alpha_2)\f$，即 \f$\boldsymbol y\f$ 是列空间中的一个向量

联立得到

\f[k\boldsymbol n+\boldsymbol b=x_1\boldsymbol\alpha_1+x_2\boldsymbol\alpha_2\tag{3-5a}\f]

即

\f[\left\{\begin{align}
-2k+1&=2x_1+x_2\\k&=x_1-x_2\\3k+2&=x_1+x_2
\end{align}\right.\tag{3-5b}\f]

③ 求解 \f$\hat{\boldsymbol x}\f$

这是一个关于 \f$k,x_1,x_2\f$ 的线性方程组，解得

\f[X=(-\frac27,\frac37,\frac57)^T\tag{3-6}\f]

因此我们得到了满足以上 <span style="color: red">**约束条件**</span> 的解

\f[\hat{\boldsymbol x}=\left(\frac37,\frac57\right)^T\tag{3-7}\f]

<span style="color: green">**示例 3**</span>

题目：使用形如 \f$f(t)=a_0+a_1t\f$ 的曲线在已知点集上完成拟合（线性拟合）

<center>
表 3-1 已知点集
</center>

\f[\begin{array}{c|cccc}\text{下标}i&0&1&2&3\\\hline t_i&1&2&3&4\\f(t_i)&0&2&1&3\end{array}\f]

上文研究的 **线性空间** 都是定义在数域 \f$\mathbb P^n\f$ 上的欧式空间，实际上，对于系数属于 \f$\mathbb P\f$，而未定元为 \f$t\f$ 的所有次数小于 \f$n\f$ 的多项式集合也构成一个 **线性空间** ，记作

\f[\mathbb P{[t]}_n=a_0+a_1t+a_2t^2+\cdots+a_{n-1}t^{n-1}=\sum_{i=0}^{n-1}a_it^i\tag{3-8}\f]

可以很容易的找到一组基

\f[\left\{\begin{matrix}
\phi_0(t)&=&1\\\phi_1(t)&=&t\\\phi_2(t)&=&t^2\\\vdots&=&\vdots\\\phi_{n-1}(t)&=&t^{n-1}\\
\end{matrix}\right.\tag{3-9}\f]

@note
- 若不加说明，后文对 \f$\phi_i(t)\f$ 简记为 \f$\phi_i\f$
- 要证明 \f$\phi_0,\phi_1,\cdots,\phi_{n-1}\f$ 是一组基，则要证明他们线性无关。
- 此处的线性无关表示任意的 \f$\phi_i\f$ 都不能被其余所有 \f$\phi_j\f$ 所表示，其中 \f$i\neq j\f$。比如说 \f$t\f$ 不能被 \f$1\f$ 和 \f$t^2,\ t^3,\ \cdots\f$ 所表示。
- 从定义上证明线性无关，需要证明 \f[a_0+a_1t+a_2t^2+\cdots+a_{n-1}t^{n-1}=\sum_{i=0}^{n-1}a_it^i=0\f] **只有零解** ，即 \f$a_0=a_1=a_2=\cdots=a_{n-1}=0\f$，这是显然的。
- 没有定义内积运算，因此这组基 **没有正交的概念** 。

<span style="color: teal">上面都是一些概念性的介绍，跟后文求解最小二乘解无关</span>。回到<span style="color: green">示例 3</span>的这一问题本身，我们可以根据表 3-1 的信息，得到

\f[\left\{\begin{align}
\phi_0(t_0)a_0+\phi_1(t_0)a_1&=f(t_0)\\
\phi_0(t_1)a_0+\phi_1(t_1)a_1&=f(t_1)\\
\phi_0(t_2)a_0+\phi_1(t_2)a_1&=f(t_2)\\
\phi_0(t_3)a_0+\phi_1(t_3)a_1&=f(t_3)
\end{align}\right.\tag{3-10a}\f]

即

\f[\left\{\begin{align}a_0+a_1&=0\\a_0+2a_1&=2\\a_0+3a_1&=1\\a_0+4a_1&=3\end{align}\right.\tag{3-10b}\f]

这就转化为了一个超定线性方程组 \f$A\boldsymbol a=\boldsymbol f\f$，求解可直接使用公式 \f$\text{(2-7)}\f$，一般的，其系数矩阵可以表示为

\f[A=\begin{bmatrix}
\phi_0(t_0)&\phi_1(t_0)&\cdots&\phi_{n-1}(t_0)\\
\phi_0(t_1)&\phi_1(t_1)&\cdots&\phi_{n-1}(t_1)\\
\vdots&\vdots&&\vdots\\
\phi_0(t_{s-1})&\phi_1(t_{s-1})&\cdots&\phi_{n-1}(t_{s-1})
\end{bmatrix}\qquad\boldsymbol f=\begin{bmatrix}
f(t_0)\\f(t_1)\\\vdots\\f(t_{s-1})\end{bmatrix}\tag{3-11}\f]

------

后文将给出另外一种描述最小二乘法的做法，这种做法有别于上面构造向量与子空间垂直的方式，通过对误差平方和直接求其最小值来得到最小二乘解（两种方式结果均能推导出公式 \f$\text{(2-7)}\f$，但出发点不同）。此外，要介绍的这个解法也同样适用于整个线性空间（包括欧式空间、多项式空间等线性空间）。

### 4. 法方程求解最小二乘法

相关类 rm::CurveFitter

后文会以多项式空间为例，从 \f$\delta^2\f$ 的极值（最小值）入手，通过对该误差平方和求导数的方式获得该最优解，这也是数值分析教材中普遍采用的做法。在本小节后，会使用这一方法求解<span style="color: green">**示例 3**</span>的问题。

回顾公式 \f$\text{(2-3b)}\f$：\f$\sum\limits_{j=1}^na_{ij}x_j\f$ 的部分，这是对应于欧式空间 \f$\mathbb R^n\f$ 的误差平方和的写法，这表示生成的位于<span style="color: red">列空间</span>中的向量在基下的第 \f$i\f$ 个分量（坐标）。对于多项式空间 \f$\mathbb R{[t]}_n\f$，这部分的写法为 \f$a_0+a_1t+\cdots+a_{n-1}t^{n-1}=\sum\limits_{j=0}^{n-1}a_jt^j\f$。那么对于包含 \f$s\f$ 个已知点的集合（每个点包含 \f$t_i\f$ 和 \f$f(t_i)\f$ 两部分），使用 \f$\mathbb R{[t]}_n\f$ 即 \f$\sum\limits_{j=0}^{n-1}a_jt^j=\sum\limits_{j=0}^{n-1}a_j\phi_j(t_i)\f$ 的多项式来拟合这些点集，其最小二乘解设为 \f$f^*(t)\f$，此时的误差平方和的最小值可以表示成

\f[\delta_\min^2=\sum_{i=0}^{s-1}\left[f(t_i)-f^*(t_i)\right]^2=\min\sum_{i=0}^{s-1}\left[f(t_i)-\sum\limits_{j=0}^{n-1}a_j\phi_j(t_i)\right]^2\tag{4-1}\f]

要求解 \f$a_0,a_1,\cdots,a_{n-1}\f$，这相当于求多元函数

\f[F(a_0,a_1,\cdots,a_{n-1})=\sum_{i=0}^{s-1}\left[f(t_i)-\sum_{j=0}^{n-1}a_j\phi_j(t_i)\right]^2\tag{4-2}\f]

的极小值点。按照求极值的必要条件，可以对上述多元函数求偏导，令其为 \f$0\f$ 有

\f[\frac{\partial F}{\partial a_k}=2\sum_{i=0}^{s-1}\left[f(t_i)-\sum_{j=0}^{n-1}a_j\phi_j(t_i)\right][-\phi_k(t_i)]=0\qquad(k=0,1,\cdots,n-1),\f]

整理为

\f[\sum_{j=0}^{n-1}\left[\sum_{i=0}^{s-1}\phi_k(t_i)\phi_j(t_i)\right]a_j=\sum_{i=0}^{s-1}\phi_k(t_i)f(t_i)\qquad(k=0,1,\cdots,n-1)\tag{4-3}\f]

不同的 2 个数字向量的内积定义为 \f$(\boldsymbol\alpha,\boldsymbol\beta)=\sum\limits_{i=0}^sa_ib_i\f$，同样的，多项式不同的 2 个分量（例如 \f$\phi_1(t)=t\f$ 和 \f$\phi_2(t)=t^2\f$ 就是不同的分量）之间的内积可以定义为 \f$(\phi_p(t),\phi_q(t))=\sum\limits_{i=0}^{s-1}\phi_p(t_i)\phi_q(t_i)\f$，简记为 \f$(\phi_p,\phi_q)\f$。则有

\f[\left\{\begin{align}
\sum_{i=0}^{s-1}\phi_k(t_i)\phi_j(t_i)&=(\phi_k,\phi_j)\\
\sum_{i=0}^{s-1}\phi_k(t_i)f(t_i)&=(\phi_k,f)\equiv d_k\\
\end{align}\right.\tag{4-4}\f]

于是公式 \f$\text{(4-3)}\f$ 可以写成

\f[\sum_{j=0}^{n-1}(\phi_k,\phi_j)a_j=d_k\qquad(k=0,1,\cdots,n-1)\tag{4-5a}\f]

或者展开写为

\f[(\phi_k,\phi_0)a_0+(\phi_k,\phi_1)a_1+\cdots+(\phi_k,\phi_{n-1})a_{n-1}=d_k\qquad(k=0,1,\cdots,n-1)\tag{4-5b}\f]

对每一个 \f$k\f$ 值，可以一并写成如下形式

\f[\left\{\begin{matrix}
(\phi_0,\phi_0)a_0&+&(\phi_0,\phi_1)a_1&+&\cdots&+&(\phi_0,\phi_{n-1})a_{n-1}&=&d_0\\
(\phi_1,\phi_0)a_0&+&(\phi_1,\phi_1)a_1&+&\cdots&+&(\phi_1,\phi_{n-1})a_{n-1}&=&d_1\\
\vdots&&\vdots&&&&\vdots&=&\vdots\\
(\phi_{n-1},\phi_0)a_0&+&(\phi_{n-1},\phi_1)a_1&+&\cdots&+&(\phi_{n-1},\phi_{n-1})a_{n-1}&=&d_{n-1}\\
\end{matrix}\right.\tag{4-5c}\f]

上式称为 \f$a_0,a_1,\cdots,a_{n-1}\f$ 的<span style="color: red">法方程（组）</span>，是 \f$n\f$ 阶线性方程组，其系数矩阵是

\f[G=\begin{bmatrix}
(\phi_0,\phi_0)&(\phi_0,\phi_1)&\cdots&(\phi_0,\phi_{n-1})\\
(\phi_1,\phi_0)&(\phi_1,\phi_1)&\cdots&(\phi_1,\phi_{n-1})\\
\vdots&\vdots&&\vdots\\
(\phi_{n-1},\phi_0)&(\phi_{n-1},\phi_1)&\cdots&(\phi_{n-1},\phi_{n-1})\\
\end{bmatrix}\tag{4-6}\f]

对于 \f$(\phi_p,\phi_q)\f$，依照公式\f$\text{(4-4)}\f$，可以写成矩阵的表示方式，即

\f[\begin{align}(\phi_p,\phi_q)&=\sum_{i=0}^{s-1}\phi_p(t_i)\phi_q(t_i)\\&=[\phi_p(t_0),\phi_p(t_1),\cdots,\phi_p(t_{s-1})]
\begin{bmatrix}\phi_q(t_0)\\\phi_q(t_1)\\\vdots\\\phi_q(t_{s-1})\end{bmatrix}\end{align}\tag{4-7}\f]

因此对法方程系数矩阵 \f$G\f$ 的第 \f$k\ (k=0,1,\cdots,n-1)\f$ 行，有

\f[[(\phi_k,\phi_0),(\phi_k,\phi_1),\cdots,(\phi_k,\phi_{n-1})]\\=[\phi_k(t_0),\phi_k(t_1),\cdots,\phi_k(t_{s-1})]
\begin{bmatrix}\phi_0(t_0)&\phi_1(t_0)&\cdots&\phi_{n-1}(t_0)\\\phi_0(t_1)&\phi_1(t_1)&\cdots&\phi_{n-1}(t_1)\\\vdots&\vdots
&&\vdots\\\phi_0(t_{s-1})&\phi_1(t_{s-1})&\cdots&\phi_{n-1}(t_{s-1})\end{bmatrix}\tag{4-8}\f]

将 \f$k=0,1,\cdots,n-1\f$ 的行向量拼起来，得到

\f[\begin{align}G&=\begin{bmatrix}
(\phi_0,\phi_0)&(\phi_0,\phi_1)&\cdots&(\phi_0,\phi_{n-1})\\
(\phi_1,\phi_0)&(\phi_1,\phi_1)&\cdots&(\phi_1,\phi_{n-1})\\
\vdots&\vdots&&\vdots\\
(\phi_{n-1},\phi_0)&(\phi_{n-1},\phi_1)&\cdots&(\phi_{n-1},\phi_{n-1})\\
\end{bmatrix}\\&=\begin{bmatrix}
\phi_0(t_0)&\phi_0(t_1)&\cdots&\phi_0(t_{s-1})\\
\phi_1(t_0)&\phi_1(t_1)&\cdots&\phi_1(t_{s-1})\\
\vdots&\vdots&&\vdots\\
\phi_{n-1}(t_0)&\phi_{n-1}(t_1)&\cdots&\phi_{n-1}(t_{s-1})\\
\end{bmatrix}\begin{bmatrix}
\phi_0(t_0)&\phi_1(t_0)&\cdots&\phi_{n-1}(t_0)\\
\phi_0(t_1)&\phi_1(t_1)&\cdots&\phi_{n-1}(t_1)\\
\vdots&\vdots&&\vdots\\
\phi_0(t_{s-1})&\phi_1(t_{s-1})&\cdots&\phi_{n-1}(t_{s-1})
\end{bmatrix}\end{align}\tag{4-9}\f]

将公式 \f$\text{(3-11)}\f$ 中 \f$A\f$ 的矩阵表示方式代入 \f$\text{(4-9)}\f$ 可以得到 \f$G=A^TA\f$，并且右端项 \f$\boldsymbol d=A^T\boldsymbol f\f$，代入公式 \f$\text{(4-5)}\f$ 可以得到

\f[A^TA\boldsymbol a=A^T\boldsymbol f\tag{4-10}\f]

即

\f[\boxed{\hat{\boldsymbol a}=\left(A^TA\right)^{-1}A^T\boldsymbol f}\tag{4-11}\f]

这与公式 \f$\text{(2-7)}\f$ 完全一致。

此外，对于形如 \f$G\boldsymbol a=\boldsymbol d\ (G=A^TA)\f$ 的方程组，我们在求解的时候可以使用平方根法，即 Cholesky 方法进行求解，OpenCV 中对应的枚举类型为 `cv::DECOMP_CHOLESKY`。

对于<span style="color: green">**示例 3**</span>，可以依次计算出

\f[\begin{align}
(\phi_0,\phi_0)&=\sum_{i=0}^3\phi_0(t_i)\phi_0(t_i)=\sum_{i=0}^31\times1=4\\
(\phi_1,\phi_0)=(\phi_0,\phi_1)&=\sum_{i=0}^3\phi_0(t_i)\phi_1(t_i)=\sum_{i=0}^31\times t_i=1+2+3+4=10\\
(\phi_1,\phi_1)&=\sum_{i=0}^3\phi_1(t_i)\phi_1(t_i)=\sum_{i=0}^3t_i\times t_i=1+4+9+16=30\\
d_0=(\phi_0,f)&=\sum_{i=0}^3\phi_0(t_i)f(t_i)=\sum_{i=0}^31\times f(t_i)=0+2+1+3=6\\
d_1=(\phi_1,f)&=\sum_{i=0}^3\phi_1(t_i)f(t_i)=\sum_{i=0}^3t_i\times f(t_i)\\&=0+2\times2+3\times1+4\times3=19\\
\end{align}\tag{4-12}\f]

得到

\f[\begin{bmatrix}4&10\\10&30\end{bmatrix}\hat{\boldsymbol a}=
\begin{bmatrix}6\\19\end{bmatrix}\tag{4-13}\f]

解得：\f$\left\{\begin{align}a_0&=-0.5\\a_1&=0.8\end{align}\right.\f$，即拟合曲线为 \f$y=-0.5+0.8t\f$，在图像中展示，如下。

<center>

![图 4-1 拟合曲线](lsq/eg3.png)

</center>

### 5. 部署使用

OpenCV 中提供了最小二乘法求解的函数 `cv::solve()`，并设置有 2 个相关的成员函数，其函数原型如下。此外可参考 [cv::solve](https://docs.opencv.org/4.x/d2/de8/group__core__array.html#ga12b43690dbd31fed96f213eefead2373) 的 OpenCV 手册。

```cpp
bool cv::solve(
    InputArray src1,
    InputArray src2,
    OutputArray dst,
    int flags = DECOMP_LU 
);
```

---

有关递推最小二乘法的介绍可参考 @ref tutorial_extra_spi_rune_predictor 。

#### 参考书籍

\cite tongji14
\cite luo13
\cite zheng08

多维无约束最优化方法 {#tutorial_modules_fminunc}
============

@author 赵曦
@date 2024/05/06
@version 1.0
@brief 包含最速下降、共轭梯度的介绍

@prev_tutorial{tutorial_modules_fminbnd}

@next_tutorial{tutorial_modules_ew_topsis}

@tableofcontents

------

\f[
\def\red#1{\color{red}{#1}}
\def\teal#1{\color{teal}{#1}}
\def\green#1{\color{green}{#1}}
\def\transparent#1{\color{transparent}{#1}}
\def\orange#1{\color{orange}{#1}}
\def\fml#1{\text{(#1)}}
\f]

### 1. 最速下降法

沿函数的负梯度方向，函数值下降最多，因此，对于存在导数的连续目标函数，最速下降法是一种简单而有效的优化方法。可以将目标函数的负梯度方向作为寻优方向，即

\f[\boldsymbol s_k=-\frac{\nabla f(\boldsymbol x_k)}{\|\nabla f(\boldsymbol x_k)\|}\tag{1-1}\f]

因此当前点为 \f$\boldsymbol x_k\f$ 时，下一个点的表达式为

\f[\boldsymbol x_{k+1}=\boldsymbol x_k+\alpha_k\boldsymbol s_k=\boldsymbol x_k-\alpha_k\frac{\nabla f(\boldsymbol x_k)}{\|\nabla f(\boldsymbol x_k)\|}\tag{1-2}\f]

对于每轮得到的一个新的负梯度方向，再利用 @ref tutorial_modules_fminbnd 求解 \f$\alpha_k\f$。最速下降法的迭代步骤如下：

1. 选取初始点 \f$\boldsymbol x_0\f$，设置判断收敛的正数 \f$\epsilon\f$；
2. 令 \f$k=0\f$；
3. 计算 \f$-\nabla f(\boldsymbol x_k)\f$；
4. 按 \f$\fml{1-2}\f$ 计算 \f$\boldsymbol s_k\f$，若 \f$\|\boldsymbol s_k\|<\epsilon\f$，则停止迭代，\f$\boldsymbol x_k\f$ 为最优解，否则进行下一步；
5. 进行一维搜索，求解 \f$\alpha_k\f$，使 \f[f(\boldsymbol x_k+\alpha_k\boldsymbol s_k)=\min_{\alpha>0}f(\boldsymbol x_k+\alpha\boldsymbol s_k)\tag{1-3}\f]
6. 计算 \f$\boldsymbol x_{k+1}=\boldsymbol x_k+\alpha_k\boldsymbol s_k\f$，令 \f$k=k+1\f$，返回第 3 步。

最速下降法对于一般的函数而言，在远离极值点时函数值下降得很快，最速下降法队椭圆类函数十分有效，可以很快搜索到接近极值点。但是当距离极值点较近时，特别是存在脊线的目标函数，收敛过程可能会十分缓慢，如图 1-1 所示。

<center>

![图 1-1 存在脊线的目标函数](fminunc/fig1-1.png)

</center>

### 2. 共轭梯度法

#### 2.1 共轭方向

同心椭圆族曲线的两平行切线有这样的特性：通过两平行线与椭圆的切点作连线，该直线通过该椭圆族的中心，如图 2-1 所示。因为该连线的方向与两平行线是共轭方向，所以利用这一特性寻优称为共轭方向法。

<center>

![图 2-1 平行的同心椭圆族的切点连线过其中心](fminunc/fig2-1.png)

</center>

如果有一组 \f$n\f$ 个非零向量组 \f$\boldsymbol s_1,\boldsymbol s_2,\dots,\boldsymbol s_n\in\boldsymbol E^n\f$，且这个向量组中的任意两个向量关于 \f$n\f$ 阶实对称正定矩阵 \f$\boldsymbol A\f$ 满足式 \f[\boldsymbol s_i^T\boldsymbol A\boldsymbol s_j=0,\quad i,j=1,2,\dots,n\ 且\ i\ne j\tag{2-1}\f] 则称

- 向量组 \f$\boldsymbol s_1,\boldsymbol s_2,\dots,\boldsymbol s_n\f$ 是关于矩阵 \f$\boldsymbol A\f$ 共轭的；
- \f$\boldsymbol s_i\f$ 和 \f$\boldsymbol s_j\f$ 是实对称正定矩阵 \f$\boldsymbol A\f$ 的共轭方向。

有这一个特殊情况，当矩阵 \f$\boldsymbol A\f$ 是单位矩阵时，向量的共轭就相当于向量的正交。共轭方向相当于将原来的非正椭圆函数通过矩阵 \f$\boldsymbol A\f$ 变换为正圆函数，而共轭方向 \f$\boldsymbol s_1\f$ 和 \f$\boldsymbol s_2\f$ 则是变换后的垂直方向 \f$\boldsymbol p_1\f$ 和 \f$\boldsymbol p_2\f$，如图 2-2 所示。

<center>

![图 2-2 共轭与正交对比](fminunc/fig2-2.png)

</center>

#### 2.2 共轭梯度方向的构造

在极值点 \f$x^*\f$ 附近，目标函数可以近似为二次型函数，即 \f[f(\boldsymbol x)\approx c+\boldsymbol b^T\boldsymbol x+\frac12\boldsymbol x^T\boldsymbol A\boldsymbol x\tag{2-2}\f]

1. 从 \f$\boldsymbol x_k\f$ 点出发，沿负梯度 \f$\boldsymbol s_k=-\nabla f(\boldsymbol x_k)\f$ 方向寻优，得到新优化点 \f$\boldsymbol x_{k+1}\f$。再按下式构造与 \f$\boldsymbol s_k\f$ 共轭的方向 \f$\boldsymbol s_{k+1}\f$：\f[\boldsymbol s_{k+1}=-\nabla f(\boldsymbol x_{k+1})+\beta_k\boldsymbol s_k\tag{2-3}\f] 在公式 \f$\fml{2-3}\f$ 中，\f$\beta_k\f$ 按下式计算时，可满足共轭条件 \f$\boldsymbol s_{k+1}^T\boldsymbol A\boldsymbol s_k=0\f$：\f[\beta_k=\frac{\|\nabla f(\boldsymbol x_{k+1})\|^2}{\|\nabla f(\boldsymbol x_k)\|^2}\tag{2-4}\f]
2. 沿着 \f$\boldsymbol s_{k+1}\f$ 方向寻优，直至求出极值 \f$\boldsymbol x^*\f$。

上面只是对目标函数为二次型函数的情况求得了构成共轭方向的系数 \f$\beta_k\f$，对于一般的目标函数，有 \f[\beta_k=\frac{\|\nabla f(\boldsymbol x_{k+1})\|^2-[\nabla f(\boldsymbol x_{k+1})]^T\nabla f(\boldsymbol x_k)}{\|\nabla f(\boldsymbol x_k)\|^2}\tag{2-5}\f]

从而类似式 \f$\fml{2-3}\f$ 有 \f[\boldsymbol s_{k+1}=-\nabla f(\boldsymbol x_{k+1})+\beta_k\boldsymbol s_k\f]

#### 2.3 迭代步骤

1. 选取初始点 \f$\boldsymbol x_0\f$，设置判断收敛的正数 \f$\epsilon\f$；
2. 令 \f$k=0\f$，\f$\boldsymbol s_0=-\nabla f(\boldsymbol x_0)\f$；
3. 进行一维搜索，求解 \f$\alpha_k\f$，使 \f$f(\boldsymbol x_k+\alpha_k\boldsymbol s_k)=\min_{\alpha>0}f(\boldsymbol x_k+\alpha\boldsymbol s_k)\f$
4. 计算 \f$\nabla f(\boldsymbol x_{k+1})\f$，并令 \f$\boldsymbol x_{k+1}=\boldsymbol x_k+\alpha_k\boldsymbol s_k\f$；
5. 若 \f$\|\nabla f(\boldsymbol x_{k+1})\|<\epsilon\f$，则停止迭代，\f$\boldsymbol x_{k+1}\f$ 为最优解，否则按 \f$\fml{2-5}\f$ 计算 \f$\beta_k\f$，令 \f[\boldsymbol s_{k+1}=-\nabla f(\boldsymbol x_{k+1})+\beta_k\boldsymbol s_k\f]
6. 令 \f$k=k+1\f$，返回第 3 步。

#### 2.4 如何使用

首先定义一个目标函数，例如：

```cpp
static inline double quadratic(const std::valarray<double> &x) {
    const auto &x1 = x[0], &x2 = x[1];
    return 60 - 10 * x1 - 4 * x2 + x1 * x1 + x2 * x2 - x1 * x2;
}
```

然后调用 `rm::fminunc` 函数：

```cpp
auto [x, fval] = rm::fminunc(quadratic, {0, 0});
```

汇总起来，共轭梯度法的使用实例如下：

```cpp
#include <cstdio>

#include <rmvl/algorithm/numcal.hpp>

int main() {
    // 定义二次函数
    auto quadratic = [](const std::valarray<double> &x) {
        const auto &x1 = x[0], &x2 = x[1];
        return 60 - 10 * x1 - 4 * x2 + x1 * x1 + x2 * x2 - x1 * x2;
    };

    // 求解无约束多维最优化问题，默认使用共轭梯度法
    auto [x, fval] = rm::fminunc(quadratic, {0, 0});
    printf("min[f(x,y)] = f(%f, %f) = %f\n", x[0], x[1], fval);
}
```

运行结果如下：

```
min[f(x,y)] = f(8.0000, 6.0000) = 8.0000
```

非线性最小二乘 {#tutorial_modules_lsqnonlin}
============

@author 赵曦
@date 2024/05/24
@version 1.0
@brief 涉及 **Gauss-Newton 迭代** 与 **LM** 非线性最小二乘求解算法

@prev_tutorial{tutorial_modules_least_square}

@next_tutorial{tutorial_modules_func_iteration}

@tableofcontents

------

\f[
\def\red#1{\color{red}{#1}}
\def\teal#1{\color{teal}{#1}}
\def\green#1{\color{green}{#1}}
\def\transparent#1{\color{transparent}{#1}}
\def\orange#1{\color{orange}{#1}}
\def\Var{\mathrm{Var}}
\def\Cov{\mathrm{Cov}}
\def\tr{\mathrm{tr}}
\def\fml#1{\text{(#1)}}
\def\ptl#1#2{\frac{\partial#1}{\partial#2}}
\f]

### 1. Gauss-Newton 迭代

#### 1.1 算法原理

数据处理中，最常见的一种函数形式是 \f[f(\pmb x)=\frac12\sum_{i=1}^m\varphi_i^2(\pmb x)\tag{1-1}\f]

如果 \f$f(x)\f$ 的极小点 \f$x^*\f$ 满足 \f$f(x^*)< e\f$（\f$e\f$ 是预先给定的精度），那么可以认为 \f$x^*\f$ 是
方程组 \f[\varphi_i(x_1,x_2,\cdots,x_n)=0,\quad i=1,2,\cdots,m(m\le n)\f] 的解。

对式 \f$\fml{1-1}\f$，可以用一阶导数运算来代替牛顿法中的二阶导数矩阵的求逆运算。因为

\f[\begin{align}\nabla f(\pmb x)&=\begin{bmatrix}
\sum\limits_{i=1}^m\ptl{\varphi_i(\pmb x)}{x_1}\varphi_i(\pmb x)\\
\sum\limits_{i=1}^m\ptl{\varphi_i(\pmb x)}{x_2}\varphi_i(\pmb x)\\
\vdots\\\sum\limits_{i=1}^m\ptl{\varphi_i(\pmb x)}{x_n}\varphi_i(\pmb x)\end{bmatrix}=\begin{bmatrix}
\ptl{\varphi_1(\pmb x)}{x_1}&\ptl{\varphi_2(\pmb x)}{x_1}&\cdots&\ptl{\varphi_m(\pmb x)}{x_1}\\
\ptl{\varphi_1(\pmb x)}{x_2}&\ptl{\varphi_2(\pmb x)}{x_2}&\cdots&\ptl{\varphi_m(\pmb x)}{x_2}\\
\vdots&\vdots&\ddots&\vdots\\
\ptl{\varphi_1(\pmb x)}{x_n}&\ptl{\varphi_2(\pmb x)}{x_n}&\cdots&\ptl{\varphi_m(\pmb x)}{x_n}
\end{bmatrix}\begin{bmatrix}
\varphi_1(\pmb x)\\\varphi_2(\pmb x)\\\vdots\\\varphi_m(\pmb x)\end{bmatrix}\\
&=\pmb J^T(\pmb x)\pmb\varphi(\pmb x)\tag{1-2}\end{align}\f]

式中，\f$\pmb\varphi(\pmb x)\f$ 为 \f$m\f$ 维的函数向量，即 \f$\pmb\varphi(\pmb x)=[\varphi_1(x),\varphi_2(x),\cdots,\varphi_m(x)]^T\f$；\f$\pmb J(\pmb x)\f$ 为函数 \f$\pmb\varphi(\pmb x)\ (i=1,2,\cdots,m)\f$ 一阶偏导数组成的矩阵，即

\f[\pmb J(\pmb x)=\begin{bmatrix}
\ptl{\varphi_1(\pmb x)}{x_1}&\ptl{\varphi_1(\pmb x)}{x_2}&\cdots&\ptl{\varphi_1(\pmb x)}{x_n}\\
\ptl{\varphi_2(\pmb x)}{x_1}&\ptl{\varphi_2(\pmb x)}{x_2}&\cdots&\ptl{\varphi_2(\pmb x)}{x_n}\\
\vdots&\vdots&\ddots&\vdots\\
\ptl{\varphi_m(\pmb x)}{x_1}&\ptl{\varphi_m(\pmb x)}{x_2}&\cdots&\ptl{\varphi_m(\pmb x)}{x_n}
\end{bmatrix}\tag{1-3}\f]

特别当 \f$\pmb\varphi(\pmb x)\f$ 是 \f$\pmb x\f$ 的线性函数时，式 \f$\fml{1-2}\f$ 中的 \f$\pmb J(\pmb x)\f$ 是常系数矩阵。这时式 \f$\fml{1-1}\f$ 的海赛矩阵可以写成 \f[\pmb H=\pmb J^T\pmb J\tag{1-4}\f]

这样，关于 \f$\pmb\varphi(\pmb x)\f$ 是 \f$\pmb x\f$ 的线性函数时的最小二乘法的迭代公式可以写为 \f[\pmb x_{k+1}=\pmb x_k-\pmb H^{-1}(\pmb x_k)\pmb J^T(\pmb x_k)\pmb\varphi(\pmb x_k)\tag{1-5}\f]

当 \f$\pmb\varphi(\pmb x)\f$ 不是 \f$\pmb x\f$ 的线性函数时，也可以近似将式 \f$\fml{1-4}\f$ 视为函数 \f$f(\pmb x)\f$ 的海赛矩阵，即 \f[\pmb H\approx\left[\pmb J(\pmb x_k)\right]^T\pmb J(\pmb x_k)\tag{1-6}\f]

所以，关于 \f$\pmb\varphi(\pmb x)\f$ 不是 \f$\pmb x\f$ 的线性函数时的最小二乘法的迭代公式也可以写为 \f$\fml{1-5}\f$ 的形式。

#### 1.2 迭代步骤

1. 选择一初始点 \f$\pmb x_0=(x_{1,0},x_{2,0},\cdots,x_{n,0})^T\f$；
2. 算出 \f[\Delta\pmb x_0=-\pmb H_0^{-1}\pmb J_0^T\pmb\varphi(\pmb x_k)\tag{1-6}\f]
3. 令 \f$\pmb x_1\f$ 为函数 \f$f(\pmb x)\f$ 的极小点的第 1 次近似，则有 \f[\pmb x_1=\pmb x_0+\Delta\pmb x_0\tag{1-7}\f]
4. 以 \f$\pmb x_1\f$ 代替前面的 \f$\pmb x_0\f$，\f$\Delta\pmb x_1\f$ 代替 \f$\Delta\pmb x_0\f$，重复上述计算过程，直到 \f[\|\pmb\varphi(\pmb x_k)\|<\epsilon'\tag{1-8a}\f] 或 \f[\|\nabla f(\pmb x_k)\|<\epsilon''\tag{1-8b}\f] 为止。\f$\epsilon'\f$ 和 \f$\epsilon''\f$ 是预先给定的精度。

#### 1.3 改进

上述高斯－牛顿最小二乘法利用了目标函数在极小值处近似为自变量各分量的平方和的特点，用 \f$\pmb J^T\pmb J\f$ 近似代替牛顿法中 \f$f(\pmb x)\f$ 的二阶导数矩阵，大大节省了计算量。但是它对初始点 \f$\pmb x_0\f$ 的要求比较严格，如果初始点 \f$\pmb x_0\f$ 与极小点 \f$\pmb x^*\f$ 相距很远，这个算法往往失败。原因是

1. 上述算法基于线性逼近，但在 \f$\pmb x_0\f$ 远离极小点时，这种线性逼近无效；
2. \f$\pmb J_0^T\pmb J_0\f$ 的最大特征值与最小特征值的比很大，致使解 \f$\Delta\pmb x_0\f$ 变得无意义。

为此采取下述改进的办法。在求出 \f$\pmb x_k\f$ 的校正量 \f$\Delta\pmb x_k\f$ 后，不把 \f$\pmb x_k+\Delta\pmb x_k\f$ 作为第 \f$k+1\f$ 次近似点，而是将 \f$\Delta\pmb x_k\f$ 作为下一步的一维方向搜索。求 \f$\alpha_k\f$，使\f[f(\pmb x_k+\alpha_k\pmb s_k)=\min_{\alpha>0}f(\pmb x_k+\alpha\pmb s_k)\f]然后令\f[\pmb x_{k+1}=\pmb x_k+\alpha_k\pmb s_k\f]以 \f$\pmb x_{k+1}\f$ 代替 \f$\pmb x_k\f$ 重复上述计算过程，直到 \f$\|\pmb\varphi(\pmb x_k)\|<\epsilon'\f$ 或 \f$\|\nabla f(\pmb x_k)\|<\epsilon"\f$ 为止。

#### 1.4 如何使用

RMVL 提供了改进的 Gauss-Newton 迭代算法，可参考 `rm::lsqnonlin` 函数。例如，我们需要拟合一个正弦函数\f[y=A\sin(\omega t+\varphi_0)+b\f]其中，\f$A,\omega,\varphi_0,b\f$ 是待拟合的参数，不妨统一写为 \f$\pmb x=(A,\omega,\varphi_0,b)\f$，也就是说我们需要拟合的函数是\f[\green y=x_1\sin(x_2\green t+x_3)+x_4\f]其中 \f$t\f$ 和 \f$y\f$ 是可以观测到的数据，我们需要通过观测的数据来拟合 \f$\pmb x\f$ 的值。比方说，下面的 `obtain` 函数就可以观测每一帧的数据。

@add_toggle_cpp

```cpp
double obtain();
```

@end_toggle

@add_toggle_python
    
```python
def obtain(): ...
```

@end_toggle

例如经过了 20 帧的数据采集，我们得到了一个长度为 `20` 的队列，即

@add_toggle_cpp

```cpp
std::deque<double> datas;

/* code */
datas.push_front(obtain());
if (datas.size() == 20)
    datas.pop_back();
/* code */
```

@end_toggle

@add_toggle_python

```python
datas = []

# code
datas.insert(0, obtain())
if len(datas) == 20:
    datas.pop()
# code
```

@end_toggle

准备好数据后，可以使用下面的代码来拟合正弦函数。

@add_toggle_cpp

```cpp
rm::FuncNds lsq_sine(datas.size());
for (std::size_t i = 0; i < datas.size(); ++i)
    lsq_sine.push_back([=](const std::vector<double> &x) {
        return x[0] * std::sin(x[1] * i + x[2]) + x[3] - datas[i];
    });

// 拟合正弦函数，初始值为 (1, 0.02, 0, 1.09)
auto x = rm::lsqnonlin(lsq_sine, {1, 0.02, 0, 1.09}); // 默认采用 Gauss-Newton 算法
```

@end_toggle

@add_toggle_python

```python
lsq_sine = []

for i, data in enumerate(datas):
    lsq_sine.append(
        lambda x, i=i, data=data: x[0] * np.sin(x[1] * i + x[2]) + x[3] - data
    )

# 拟合正弦函数，初始值为 (1, 0.02, 0, 1.09)
x = rm.lsqnonlin(lsq_sine, [1, 0.02, 0, 1.09]) # 默认采用 Gauss-Newton 算法
```

@end_toggle

### 2. Levenberg–Marquardt 算法

#### 2.1 算法原理

普通的 Gauss-Newton 迭代，在初始值附近做了一阶线性化处理，而当初始值与极小值相差较远时，曲线的非线性特性会导致迭代失败。为了解决这个问题，Levenberg–Marquardt 算法在 Gauss-Newton 迭代的基础上，引入了一个参数 \f$\lambda\f$，使得迭代公式变为 \f[\pmb x_{k+1}=\pmb x_k-\left(\pmb J^T\pmb J+\red{\lambda\pmb I}\right)^{-1}\pmb J^T\pmb\varphi(\pmb x_k)\tag{2-1}\f]

\f$I\f$ 是单位矩阵，\f$\lambda\f$ 是一个非负数。如果 \f$\lambda\f$ 取值较大时，\f$\lambda I\f$ 占主要地位，此时的 LM 算法更接近一阶梯度下降法，说明此时距离最终解还比较远，用一阶近似更合适。反之，如果 \f$\lambda\f$ 取值较小时，\f$\pmb H=\pmb J^T\pmb J\f$ 占主要地位，说明此时距离最终解距离较近，用二阶近似模型比较合适，可以避免梯度下降的<u>震荡</u>，容易快速收敛到极值点。因此参数 \f$\lambda\f$ 不仅影响到迭代的方向还影响到迭代步长的大小。

令初值为 \f$\pmb x_0\f$，可以设置 \f$\lambda\f$ 的初值 \f$\lambda_0\f$ 为

\f[\begin{align}\pmb A_0&=\pmb J^T(\pmb x_0)\pmb J(\pmb x_0)\\
\lambda_0&=\tau\max_i\left\{a_{ii}^{(0)}\right\}\end{align}\tag{2-2}\f]

其中，\f$\tau\f$ 可以自己指定，\f$a_{ii}\f$ 表示矩阵 \f$\pmb A\f$ 对角线元素。此外，\f$\lambda\f$ 需要在迭代过程中不断调整，以保证迭代的收敛性。一般会判断近似的模型与实际函数之间的差异，可以使用下面的公式来判断

\f[\rho_k=\frac{f(\pmb x_k+\Delta\pmb x_k)-f(\pmb x_k)}{\pmb J(\pmb x_k)\Delta\pmb x_k}\tag{2-3}\f]

#### 2.2 迭代步骤

1. 选择一初始点 \f$\pmb x_0=(x_{1,0},x_{2,0},\cdots,x_{n,0})^T\f$，按照式 \f$\fml{2-2}\f$ 计算 \f$\lambda_0\f$；
2. 对于第 \f$k\f$ 次迭代，根据式 \f$\fml{2-1}\f$ 计算 \f$\Delta\pmb x_k\f$，并计算 \f$\rho_k\f$；
3. 如果
   - \f$\rho_k\le0.25\f$，应减小 \f$\lambda_k\f$，即 \f[\lambda_{k+1}=\frac{\lambda_k}2\f]
   - \f$0.25<\rho_k\le0.75\f$，保持 \f$\lambda_k\f$ 不变；
   - \f$\rho_k>0.75\f$，增大 \f$\lambda_k\f$，即\f[\lambda_{k+1}=2\lambda_k\f]
   - 如果 \f$\rho_k\le0\f$，这时不应该更新 \f$\pmb x_k\f$，即\f[\pmb x_{k+1}=\pmb x_k\f]并且和上面 \f$\rho_k\le0.25\f$ 的情况一样，减小 \f$\lambda_k\f$，反之，在 \f$\rho_k>0\f$ 的情况下，更新 \f$\pmb x_k\f$，即\f[\pmb x_{k+1}=\pmb x_k\f]

#### 2.3 如何使用

还是上面的例子，我们可以使用下面的代码来拟合正弦函数。

@add_toggle_cpp

```cpp
// 拟合正弦函数，初始值为 (1, 0.02, 0, 1.09)

rm::OptimalOptions options;
options.lsq_mode = rm::LsqMode::LM; // 使用 LM 算法
options.max_iter = 2000;            // 最大迭代次数可以设置高一点，以保证收敛
auto x = rm::lsqnonlin(lsq_sine, {1, 0.02, 0, 1.09}, options);
```

@end_toggle

@add_toggle_python

```python
# 拟合正弦函数，初始值为 (1, 0.02, 0, 1.09)

options = rm.OptimalOptions()
options.lsq_mode = rm.LsqMode.LM # 使用 LM 算法
options.max_iter = 2000          # 最大迭代次数可以设置高一点，以保证收敛
x = rm.lsqnonlin(lsq_sine, [1, 0.02, 0, 1.09], options)
```

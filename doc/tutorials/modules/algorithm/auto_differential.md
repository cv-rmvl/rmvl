自动求导、数值微分 {#tutorial_modules_auto_differential}
============

@author 赵曦
@date 2024/05/06
@version 1.0
@brief 包含中心差商以及 Richardson 外推原理的介绍

@prev_tutorial{tutorial_modules_runge_kutta}

@next_tutorial{tutorial_modules_fminbnd}

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

### 1. 基于 Taylor 公式的数值微分

利用以下两个 2 阶 Taylor 公式：

\f[\begin{align}f(x+h)&=f(x)+f'(x)h+\frac{f''(x)}2h^2+\frac{f'''(x)}{3!}h^3\tag{1-1a}\\
f(x-h)&=f(x)-f'(x)h+\frac{f''(x)}2h^2-\frac{f'''(x)}{3!}h^3\tag{1-1b}\end{align}\f]

可以得到两个数值微分公式及其对应的截断误差（余项）\f$R\f$：

\f[\begin{align}f'(x)&\approx\frac{f(x+h)-f(x)}h,\qquad R=-\frac{f''(\xi)}2h+o(h^2)\tag{1-2a}\\
f'(x)&\approx\frac{f(x)-f(x-h)}h,\qquad R=\frac{f''(\eta)}2h+o(h^2)\tag{1-2b}\end{align}\f]

其中\f$\fml{1-2a}\f$称为<span style="color: red">前向差商</span>，\f$\fml{1-2b}\f$称为<span style="color: red">后向差商</span>。由于截断误差的存在，差商法的精度不高，因此我们可以考虑使用更高阶的差商法。我们将\f$\fml{1-1a}\f$和\f$\fml{1-1b}\f$相减，得到：

\f[f(x+h)-f(x-h)=2f'(x)h+\frac2{3!}f'''(\xi)h^3+o(h^3)\tag{1-3a}\f]

整理得到

\f[f'(x)=\frac{f(x+h)-f(x-h)}{2h},\qquad R=-\frac{f'''(\xi)}{3!}h^2+o(h^2)\tag{1-3b}\f]

如果导数\f$f'(x)\f$用\f$\fml{1-3b}\f$计算，那么截断误差为\f$o(h^2)\f$，比\f$\fml{1-2a}\f$和\f$\fml{1-2b}\f$的截断误差\f$O(h)\f$更小。这种方法称为<span style="color: red">中心差商</span>。

### 2. Richardson 外推原理

#### 2.1 公式推导

将公式\f$\fml{1-3b}\f$扩展，可以得到更完全的写法：

\f[f'(x)=\green{\frac{f(x+h)-f(x-h)}{2h}}-\left[\frac1{3!}f^{(3)}(x)h^2+\frac1{5!}f^{(5)}(x)h^4+\frac1{7!}f^{(7)}(x)h^6+\cdots\right]\tag{2-1}\f]

一般的，可以记作

\f[L=\green{\varphi(h)}+\left[a_2h^2+a_4h^4+a_6h^6+\cdots\right]\tag{2-2}\f]

正如上一节所说，如果\f$L\f$用\f$\varphi(h)\f$来近似表示，则截断误差是按\f$h^2\f$展开的幂级数，误差阶为\f$o\left(h^2\right)\f$。

事实上，对于公式\f$\fml{2-2}\f$，求解\f$L\f$的过程还可以继续向前推进（外推），不妨以\f$\frac h2\f$替换\f$\fml{2-2}\f$中的\f$h\f$，可得

\f[L=\varphi\left(\frac h2\right)+\left[a_2\frac{h^2}4+a_4\frac{h^4}{16}+a_6\frac{h^6}{64}+\cdots\right]\tag{2-3}\f]

由\f$4\times\fml{2-3}-\fml{2-2}\f$可以得到

\f[L=\left[\frac43\varphi\left(\frac h2\right)-\frac13\varphi(h)\right]-\left[a_4\frac{h^4}4+5a_6\frac{h^6}{16}+\cdots\right]\tag{2-4}\f]

上式表达了外推过程的第 1 步，说明\f$\varphi(h)\f$和\f$\varphi\left(\frac h2\right)\f$的一个线性组合提供了导数即\f$L\f$的新的计算公式，其<span style="color: red">精度提高</span>至了\f$o\left(h^4\right)\f$

继续进行第 2 步，令

\f[\psi(h)=\frac43\varphi\left(\frac h2\right)-\frac13\varphi(h)=\frac1{4^1-1}\left[4^1\varphi\left(\frac h2\right)-\varphi(h)\right]\tag{2-5}\f]

那么\f$\fml{2-4}\f$可以改写成

\f[L=\psi(h)+\left[b_4h^4+b_6h^6+\cdots\right]\tag{2-6}\f]

与第 1 步的外推过程类似，以\f$\frac h2\f$替换\f$\fml{2-6}\f$中的\f$h\f$，可得

\f[L=\psi\left(\frac h2\right)+\left[b_4\frac{h^4}{16}+b_6\frac{h^6}{64}+\cdots\right]\tag{2-7}\f]

由\f$4^2\times\fml{2-7}-\fml{2-6}\f$可以得到

\f[L=\left[\frac{16}{15}\psi\left(\frac h2\right)-\frac1{15}\psi(h)\right]-\left[b_6\frac{h^6}{20}+\cdots\right]\tag{2-8}\f]

此式表达了外推过程的第 2 步，说明\f$\psi(h)\f$和\f$\psi\left(\frac h2\right)\f$的一个线性组合提供了导数即\f$L\f$的新的计算公式，其<span style="color: red">精度提高</span>至了\f$o\left(h^6\right)\f$

如果我们令

\f[\theta(h)=\frac{16}{15}\psi\left(\frac h2\right)-\frac1{15}\psi(h)\tag{2-9}\f]

不难猜出，新的导数计算公式为

\f[L\approx\frac{64}{63}\theta\left(\frac h2\right)-\frac1{63}\theta(h)\tag{2-10}\f]

读者可以自行验证，按照这个思路可以进一步外推，可执行任意多步得到精度不断提高的新公式，这就是 <span style="color: red">Richardson 外推原理</span>。

#### 2.2 外推算法总结

令\f$\varphi(h)=\frac{f(x+h)-f(x-h)}{2h}\f$，外推\f$M\f$步，则外推算法的步骤如下

① 选取一个适当的\f$h\f$值，计算\f$M+1\f$个数，记为\f$T(*,*)\f$

\f[T(n,0)=\varphi\left(\frac h{2^n}\right),\qquad n=0,1,2,\cdots,M\tag{2-11}\f]

② 按公式计算

\f[T(n,k)=\frac1{4^k-1}\left[4^kT(n,k-1)-T(n-1,k-1)\right],\qquad\left\{\begin{align}
k&=1,2,\cdots,M\\n&=k,k+1,\cdots,M
\end{align}\right.\tag{2-12}\f]

按照以上步骤计算得到的\f$T(n,k)\f$，满足等式

\f[L=T(n,k)+o(h^{2k+2})\qquad(h\to0)\tag{2-13}\f]

即\f$T(n,k)\f$具备\f$2k+2\f$阶的精度。

### 3. 如何使用

RMVL 中提供了一元函数以及多元函数的微分工具，求解一元函数导数时可使用 rm::derivative ，求解多元函数梯度是需要使用 rm::grad 。

以下展示了使用自动求导、数值微分的例子。

#### 3.1 创建项目

1. 添加源文件 `main.cpp`
   ```cpp
   #include <cstdio>
   #include <rmvl/algorithm/numcal.hpp>
   
   // 自定义函数 f(x)=x²+4x-3
   inline double quadratic(double x) { return x * x + 4 * x - 3; }

   int main()
   {
       double dydx = rm::derivative(quadratic, 1);
       printf("f'(1) = %f\n", dydx);
   }
   ```

2. 添加 `CMakeLists.txt`
   ```cmake
   cmake_minimum_required(VERSION 3.10)
   project(DerivativeDemo)
   find_package(RMVL COMPONENTS algorithm REQUIRED)
   add_executable(demo main.cpp)
   target_link_libraries(demo PRIVATE ${RMVL_LIBS})
   ```

#### 3.2 构建、运行

在项目根目录打开终端，输入

```bash
mkdir build
cd build
cmake ..
make -j2
./demo
```

可以看到运行结果

```
f'(1) = 6
```

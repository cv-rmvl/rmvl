一维最优化方法 {#tutorial_modules_fminbnd}
============

@author 赵曦
@date 2024/05/06
@version 1.0
@brief 包含区间搜索以及黄金分割法的介绍

@prev_tutorial{tutorial_modules_auto_differential}

@next_tutorial{tutorial_modules_fminunc}

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

### 1. 区间搜索

在使用一维优化方法搜索目标函数的极小值点时，首先要确定搜索区间\f$[a,b]\f$，这个搜索区间要包含目标函数的极小值点，而且是单峰区间，即在这个区间内，目标函数只有一个极小值点。

在单峰区间内，函数值具有“高—低—高”的特点。根据这一特点，可以采用进退法来找到搜索区间。

进退法分为

- 初始探察确定进退
- 前进或者后退寻查

两步，其步骤如下

<center>

![图 1-1 向前一步](fminbnd_forward.png)

![图 1-2 向前两步](fminbnd_forward_2.png)

</center>

1. 选择一个初始点\f$\alpha_1\f$和一个初始步长\f$h\f$。
2. 如图 1-1 所示，计算点\f$\alpha_1\f$和点\f$\alpha_1+h\f$对应的函数值\f$f(\alpha_1)\f$和\f$f(\alpha_1+h)\f$，令\f[f_1=f(\alpha_1),\quad f_2=f(\alpha_1+h)\f]
3. 比较\f$f_1\f$和\f$f_2\f$，若\f$f_1>f_2\f$，则执行前进运算，将步长加大\f$k\f$倍（如加大 2 倍），取新点\f$\alpha_1+3h\f$，如图 1-2 所示，计算其函数值，并令\f[f_1=f(\alpha_1+h),\quad f_2=f(\alpha_1+3h)\f]若\f$f_1< f_2\f$，则初始搜索区间端点为\f[a=\alpha_1,\quad b=\alpha_1+3h\f]若\f$f_1=f_2\f$，则初始搜索区间端点为\f[a=\alpha_1+h,\quad b=\alpha_1+3h\f]若\f$f_1>f_2\f$，则要继续做前进运算，且步长再加大两倍，取第 4 个点\f$\alpha_1+7h\f$，再比较第 3 和第 4 个点处的函数值……如此反复循环，直到在连续的 3 个点的函数值出现“两头大、中间小”的情况为止。
4. 如果在 **步骤 3** 中出现\f$f_1< f_2\f$的情况，则执行后退运算，将步长变为负值，取新点\f$\alpha_1-h\f$，计算函数值，令\f[f_1=f(\alpha_1-h),\quad f_2=f(\alpha_1)\f]若\f$f_1>f_2\f$，则初始搜索区间端点为\f[a=\alpha_1-h,\quad b=\alpha_1+h\f]若\f$f_1=f_2\f$，则初始搜索区间端点为\f[a=\alpha_1-h,\quad b=\alpha_1\f]若\f$f_1< f_2\f$，则要继续做后退运算，且步长再加大两倍，取第 4 个点\f$\alpha_1-3h\f$，再比较第 3 和第 4 个点处的函数值……如此反复循环，直到在连续的 3 个点的函数值出现“两头大、中间小”的情况为止。

<span style="color: green">**示例**</span>

试用进退法确定目标函数\f$f(\alpha)=\alpha^2-5\alpha+8\f$的一维优化初始搜索区间\f$[a,b]\f$。设初始点\f$\alpha_1=0\f$，初始步长\f$h=1\f$

**解：** 由初始点\f$\alpha_1=0\f$，初始步长\f$h=1\f$，得\f[f_1=f(\alpha_1)=8,\quad f_2=f(\alpha_1+h)=4\f]因为\f$f_1>f_2\f$，所以执行前进运算，将步长加大 2 倍，取新点\f$\alpha_1+3h=3\f$，令\f[f_1=f(\alpha_1+h)=4,\quad f_2=f(\alpha_1+3h)=2\f]因为\f$f_1>f_2\f$，应继续做前进运算，且步长再加大 2 倍，取第 4 个点\f$\alpha_1+7h=7\f$。令\f[f_1=f(\alpha_1+3h)=2,\quad f_2=f(\alpha_1+7h)=22\f]此时\f$f_1< f_2\f$，在连续的 3 个点\f$\alpha_1+h\f$，\f$\alpha_1+3h\f$，\f$\alpha_1+7h\f$的函数值出现了“两头大、中间小”的情况，则初始搜索区间端点为\f[a=\alpha_1+h=1,\quad b=\alpha_1+7h=7\f]

### 2. 黄金分割法

#### 2.1 区间消去

黄金分割法是利用区间消去法的原理，通过不断缩小单峰区间长度，即每次迭代都消去一部分不含极小值点的区间，使搜索区间不断缩小，从而逐渐逼近目标函数极小值点的一种优化方法。黄金分割法是直接寻优法，通过直接比较区间上点的函数值的大小来判断区间的取舍。这种方法具有计算简单、收敛速度快等优点。

如图1-2所示，在已确定的单峰区间[a,b]内任取\f$\alpha_1\f$，\f$\alpha_2\f$两点，计算并比较两点处的函数值\f$f(\alpha_1)\f$和\f$f(\alpha_2)\f$,可能出现3种情况：

1. \f$f(\alpha_1)< f(\alpha_2)\f$，因为函数是单峰的，所以极小值点必定位于点\f$\alpha_2\f$的左侧，即\f$\alpha^*\in[a,\alpha_2]\f$，搜索区间可以缩小为\f$[a,\alpha_2]\f$，如图 2-1(a) 所示；
2. \f$f(\alpha_1)>f(\alpha_2)\f$，极小值点必定位于点\f$\alpha_1\f$的右侧，即\f$\alpha^*\in[\alpha_1,b]\f$，搜索区间可以缩小为\f$[\alpha_1,b]\f$，如图 2-1(b) 所示；
3. \f$f(\alpha_1)=f(\alpha_2)\f$，极小值点必定位于点\f$\alpha_1\f$和\f$\alpha_2\f$之间，即\f$\alpha^*\in[\alpha_1,\alpha_2]\f$，搜索区间可缩小为\f$[\alpha_1,\alpha_2]\f$，如图 2-1(c) 所示。

<center>

![图 2-1 搜索区间缩小示意图](region_reduce.png)

</center>

根据上述方法，可在新搜索区间里再取两个新点比较函数值来继续缩小区间，但这样做效率较低，应该考虑利用已经计算过的区间内剩下的那个点。对于以上的 (1)、(2) 两种情况，可以在新搜索区间内取一点\f$x_3\f$和该区间内剩下的那个点（第 (1) 种情况的\f$\alpha_1\f$点或第 (2) 种情况的\f$\alpha_2\f$点）进行函数值的比较来继续缩短搜索区间。而第 (3) 种情况则要取两个新点进行比较，为统一起见，将前面 3 种情况综合为以下两种情况: 

1. 若\f$f(\alpha_1)< f(\alpha_2)\f$，则将搜索区间缩小为\f$[a,\alpha_2]\f$
2. 若\f$f(\alpha_2)\ge f(\alpha_2)\f$，则将搜索区间缩小为\f$[\alpha_1,b]\f$。

#### 2.2 黄金分割法原理

黄金分割法就是基于上述原理来选择区间内计算点的位置的，它有以下要求：
1. 点\f$\alpha_1\f$和\f$\alpha_2\f$相对区间\f$[a,b]\f$的边界要对称布置，即区间\f$[a,\alpha_1)\f$的大小与区间\f$(\alpha_2,b]\f$的大小相等；
2. 每次计算一个新点，要求保留的区间长度\f$l\f$与原区间长度\f$L\f$之比等于被消去的区间长度\f$L-l\f$与保留区间长度\f$l\f$之比，即要求下式成立：\f[\frac lL=\frac{L-l}l\tag{2-1}\f]

令\f[\lambda=\frac lL\f]将上式代入式\f$\fml{2-1}\f$，有\f[\lambda^2+\lambda-1=0\tag{2-2}\f]求解式\f$\fml{2-2}\f$，得\f[\lambda=\frac{\sqrt5-1}2\approx0.618\f]

该方法保证每次迭代都以同一比率缩小区间，缩短率为\f$0.618\f$，故黄金分割法又称为\f$0.618\f$法。保留的区间长度为整个区间长度的\f$0.618\f$倍，消去的区间长度为整个区间长度的\f$0.382\f$倍。

黄金分割法的计算步骤如下：

<center>

![图 2-2 黄金分割法原理](fminbnd_principle.png)

</center>

1. 在\f$[a,b]\f$内取两点\f$\alpha_1\f$，\f$\alpha_2\f$，如图 2-2(a) 所示，使\f[\begin{align}\alpha_1&=a+0.382(b-a)\\\alpha_2&=a+0.618(b-a)\end{align}\f]令\f$f_1=f(\alpha_1)\f$，\f$f_2=f(\alpha_2)\f$。
2. 比较\f$f_1\f$和\f$f_2\f$。当\f$f_1< f_2\f$时，消去区间\f$(\alpha_2,b]\f$。做置换\f$b=\alpha_2\f$，\f$\alpha_2=\alpha_1\f$并另取新点\f$\alpha_1=a+0.382(b-a)\f$，如图 2-2(b) 所示，令\f$f_1=f(\alpha_1)\f$。当\f$f_1\ge f_2\f$时，消去区间\f$[a,\alpha_1)\f$。做置换\f$a=\alpha_1\f$，\f$\alpha_1=\alpha_2\f$，\f$f_1=f_2\f$，并另取新点\f$\alpha_2=a+0.618(b-a)\f$，如图 2-2(c) 所示，令 \f$f_2=f(\alpha_2)\f$。
3. 检查终止条件。若\f$b-a\le\epsilon\f$，则输出最优解\f$\alpha^*=\frac12(a+b)\f$和最优值\f$f(\alpha^*)\f$；否则转第 (2) 步。

### 3. 如何使用

RMVL 中提供了一维寻优的函数 rm::fminbnd ，以下展示了一维寻优的例子。

#### 3.1 创建项目

1. 添加源文件 `main.cpp`
   ```cpp
   #include <cstdio>
   #include <rmvl/core/numcal.hpp>
   
   // 自定义函数 f(x)=x²+4x-3
   inline double quadratic(double x) { return x * x + 4 * x - 3; }

   int main()
   {
       // 确定搜索区间
       auto [x1, x2] = rm::region(quadratic, 0);
       // 添加优化选项（容许误差和最大迭代次数）
       rm::OptimalOptions options;
       options.max_iter = 100; // 最大迭代次数是 100，不加以设置默认是 1000
       options.tol = 1e-4;     // 容许误差是 1e-4，不加以设置默认是 1e-6
       // 一维寻优
       auto [x, fval] = rm::fminbnd(quadratic, x1, x2, options);
       printf("x    = %f\n", x);
       printf("fval = %f\n", fval);
   }
   ```

2. 添加 `CMakeLists.txt`
   ```cmake
   cmake_minimum_required(VERSION 3.10)
   project(FminbndDemo)
   find_package(RMVL COMPONENTS core REQUIRED)
   add_executable(demo main.cpp)
   target_link_libraries(demo PRIVATE rmvl_core)
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
x    = -2.000000
fval = -7.000000
```
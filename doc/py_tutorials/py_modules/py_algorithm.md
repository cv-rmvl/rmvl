适用于 Python 的 algorithm 模块 {#tutorial_py_algorithm}
=======================

@author 赵曦
@date 2024/08/05
@version 1.0
@brief RMVL-Python algorithm 模块的使用示例

@prev_tutorial{tutorial_py_core}
@next_tutorial{tutorial_py_opcua}

@tableofcontents

------

## 数值计算与最优化使用示例

```python
import rm
import numpy as np

# 多项式插值
interpolate = rm.Interpolator([-1, 0, 2], [1, 0, 4])
val = interpolate(1)
print("f(1) = {:.2f}".format(val))

# 非线性方程求解
nl = rm.NonlinearSolver(lambda x: x**2 - 3 * x + 2)
print("x = ({:.3f}, {:.3f})".format(nl(-5), nl(5)))

# 2 阶 2 级 Runge-Kutta
dot_x1 = lambda t, x: 2 * x[1] + t
dot_x2 = lambda t, x: -x[0] - 3 * x[1]
rk2 = rm.RungeKutta2([dot_x1, dot_x2])
rk2.init(0, [1, -1])
x1, x2 = rk2.solve(0.01, 100)[-1]
print("calc x1 = {:.4f}, calc x2 = {:.4f}".format(x1, x2))
rx1 = 3.0 / 4.0 * np.exp(-2) + 2 * np.exp(-1) + 3.0 / 2.0 - 7.0 / 4.0
rx2 = -3.0 / 4.0 * np.exp(-2) - np.exp(-1) - 1.0 / 2.0 + 3.0 / 4.0
print("real x1 = {:.4f}, real x2 = {:.4f}".format(rx1, rx2))

# 一维寻优
foo1 = lambda x: x**2 - 3*x + 2
x, fval = rm.fminbnd(foo1, -4, 4)
print("x = {:.2f}, f(x) = {:.2f}".format(x, fval))

# 多维无约束寻优
quadratic = lambda x: 60 - 10 * x[0] - 4 * x[1] + x[0] * x[0] + x[1] * x[1] - x[0] * x[1]
x, fval = rm.fminunc(quadratic, [1, 1])
print("x = ({:.2f}, {:.2f}), f(x) = {:.2f}".format(x[0], x[1], fval))

# 多维有约束寻优
ceq = lambda x: x[0] + x[1] - 10
x, fval = rm.fmincon(quadratic, [0, 0], [], [ceq])
print("x = ({:.2f}, {:.2f}), f(x) = {:.2f}".format(x[0], x[1], fval))
```
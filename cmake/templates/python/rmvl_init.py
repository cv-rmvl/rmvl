"""
Python bindings for the RMVL @RMVL_VERSION@
===

DO NOT EDIT OR RUN THIS FILE!!!

---

You can use RMVL Python bindings by importing the module

>>> import rm

For example, to create a multidimensional unconstrained optimization method, you can use the following code after `import rm`:

>>> foo = lambda x: 60 - 10 * x[0] - 4 * x[1] + x[0] * x[0] + x[1] * x[1] - x[0] * x[1]
>>> x, fval = rm.fminunc(foo, [1, 1])
>>> print("x = ({:.3f}, {:.3f}), f(x) = {:.3f}".format(x[0], x[1], fval))
>>> # Output: x = (8.000, 6.000), f(x) = 8.000
"""
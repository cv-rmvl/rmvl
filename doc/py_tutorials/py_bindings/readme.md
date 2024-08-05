RMVL-Python 绑定功能 {#tutorial_py_table_of_contents_bindings}
============

## 目标

了解：

- RMVL-Python 绑定是如何生成的
- 如何在配置 RMVL-Python 绑定功能

## RMVL-Python 绑定是如何生成的

在 RMVL 中，所有算法都是用 C++ 实现的，但这些算法可以在 Python 的语言环境中使用。这一效果是通过在 CMake 配置阶段绑定生成器实现。这些生成器在 C++ 和 Python 之间架起了一座桥梁，使用户能够从 Python 调用 C++ 函数。

## 如何在配置 RMVL-Python 绑定功能

RMVL 使用 CMake 进行构建，要启用 RMVL-Python 绑定功能，需要在 CMake 配置阶段设置 `BUILD_PYTHON` 为 `ON`。这样 RMVL 就会在构建时生成 Python 绑定。

```bash
cd build
cmake -DBUILD_PYTHON=ON ..
```

RMVL-Python 绑定功能 {#tutorial_py_table_of_contents_bindings}
============

## 目标

了解：

- RMVL-Python 绑定是如何生成的
- 如何在配置 RMVL-Python 绑定功能

## RMVL-Python 绑定是如何生成的

在 RMVL 中，所有算法都是用 C++ 实现的，但这些算法可以在 Python 的语言环境中使用。这一效果是通过在 CMake 配置阶段绑定生成器实现。这些生成器在 C++ 和 Python 之间架起了一座桥梁，使用户能够从 Python 调用 C++ 函数。RMVL 使用 Pybind11 作为生成器，相比于其他 Python 生成器，Pybind11 对 C++11 乃至部分 C++17 语言标准的支持较好。此外，提供了适用于 CMake 的[使用示例](https://github.com/pybind/cmake_example)。但是，Pybind11 包装代码的代码量较大，通过手动编写 Pybind11 的包装函数将 RMVL 中的指定的函数或类扩展到 Python 是一项耗时的任务，不过，包装代码与源代码之间具有较多相似之处，比较适合自动转化。因此，RMVL 提供了若干宏以及自动转化的脚本完成了这项工作。

## 如何在配置 RMVL-Python 绑定功能

### 安装依赖

首先，需要安装 Python 3 和 Pybind11。在 **Debian** 以及 **Ubuntu** 上可以通过 APT 包管理工具安装

```bash
sudo apt install python3-dev pybind11-dev
```

也可以通过 `pip` 安装 Pybind11

```bash
pip install pybind11
```

在 **Windows** 上，需要提前准备好 Python 3 的环境，Pybind11 就可以直接通过 `pip` 进行安装

```bash
pip install pybind11
```

### 启用 RMVL-Python 绑定功能

RMVL 使用 CMake 进行构建，要启用 RMVL-Python 绑定功能，需要在 CMake 配置阶段设置 `BUILD_PYTHON` 为 `ON`。这样 RMVL 就会在构建时生成 Python 绑定。

```bash
cd build
cmake -DBUILD_PYTHON=ON ..
```

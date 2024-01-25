RMVL 的编译、配置选项 {#tutorial_configuration_options}
============

@prev_tutorial{tutorial_install_overview}
@next_tutorial{tutorial_install}

@tableofcontents

------

## 1. 介绍

配置选项可以通过几种不同的方式设置：

- 命令行：`cmake -Doption=value ...`
- 初始化缓存文件：`cmake -C my_options.txt ...`
- 通过GUI进行交互：`cmake-gui ..`

大多数选项可以在 RMVL 的 cmake 脚本中找到：`rmvl/CMakeLists.txt`。

可以使用 CMake 工具打印所有可用选项：

```shell
cd build
# 初始化配置
cmake ..
# 打印所有选项
cmake -L ..
# 打印所有选项并显示帮助信息
cmake -LH ..
# 打印包含高级变量的所有选项
cmake -LA ..
```

最常见以及最有用的选项一般均以 `BUILD_`、`WITH_`、`RMVL_`、`ENABLE_` 开头。

## 2. 通用选项

### 2.1 调试构建

`CMAKE_BUILD_TYPE` 选项可用于启用调试构建。生成的二进制文件将包含调试符号，并且大多数编译器优化将被关闭。

在某些平台上（例如 Linux），必须在配置阶段设置构建类型：

```shell
cmake -DCMAKE_BUILD_TYPE=Debug ..
cmake --build .
```

在其他平台上，不同类型的构建可以在同一个构建目录下生成（例如Visual Studio，XCode）：

```shell
cmake <options> ..
cmake --build . --config Debug
```

@see
- [Build type in CMake documents](https://cmake.org/cmake/help/latest/variable/CMAKE_BUILD_TYPE.html) 
- [Build type in C++ manual](https://gcc.gnu.org/onlinedocs/libstdc++/manual/using_macros.html) 

### 2.2 静态构建与动态构建

`BUILD_SHARED_LIBS` 选项控制是否生成动态（`*.dll`，`*.so`，`*.dylib`）或静态（`*.a`，`*.lib`）库。默认值取决于目标平台，在大多数情况下它是 `OFF`。

示例:

```shell
cmake -DBUILD_SHARED_LIBS=ON ..
```

@see
- [Static library in Wiki](https://en.wikipedia.org/wiki/Static_library)

`ENABLE_PIC` 启用或禁用 **位置无关代码** 的生成，在构建动态库或打算链接到动态库的静态库时，必须启用此选项，该选项默认值为`ON`。

@see
- [PIC in Wiki](https://en.wikipedia.org/wiki/Position-independent_code)

### 2.3 构建测试

有两种测试：准确性（`rmvl_*_test`）和性能（`rmvl_*_perf`），默认情况下禁用测试。

对应的 CMake 选项:

```shell
cmake \
  -DBUILD_TESTS=ON \
  -DBUILD_PERF_TESTS=ON \
  ..
```

### 2.4 构建有限的模块集

每个模块都是 modules 目录的一个子目录。可以禁用一个模块：

```shell
cmake -DBUILD_rmvl_armor_detector=OFF ..
```

## 3. 功能特性

有许多可选的依赖关系和特性可以打开或关闭，CMake 有一个特殊的选项，允许打印所有可用的配置参数：

```shell
cmake -LH ..
```

此外，CMake 在运行后也会通过一系列的 `message()` 命令输出到终端或 GUI 界面，可在此界面查看部分配置情况。
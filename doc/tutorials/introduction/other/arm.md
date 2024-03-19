为基于 ARM 的 Linux 系统配置交叉编译{#tutorial_other_arm}
============

@prev_tutorial{tutorial_use}
@next_tutorial{tutorial_document}

@tableofcontents

------

### 交叉编译应具备的先决条件

* Linux 主机

* ![img](https://img.shields.io/badge/CMake-3.16+-green)

* ![img](https://img.shields.io/badge/OpenCV_for_ARM-4.2.0+-red) [点击此处下载最新发行版](https://github.com/opencv/opencv/releases/latest)

* ARM 的交叉编译工具：gcc，libstdc++.so 等内容，根据目标平台的不同，您需要选择 `gneabi` 或 `gneabihf` 工具。

`gneabi` 安装命令如下

```bash
sudo apt install gcc-arm-linux-gneabi
```

`gneabihf` 安装命令如下

```bash
sudo apt install gcc-arm-linux-gneabihf
```

### 使用提供的工具链构建 RMVL

在 RMVL 目录下创建 `build` 文件夹，运行以下命令

```bash
cd build
cmake -DCMAKE_TOOLCHAIN_FILE=../platforms/linux/arm-gnueabi.toolchain.cmake ..
```

上文代码中的 CMake 配置也可使用 `cmake-gui` 或者 `ccmake` 工具，例如使用 `ccmake` 则需要改写为

```bash
ccmake -DCMAKE_TOOLCHAIN_FILE=../platforms/linux/arm-gnueabi.toolchain.cmake ..
```

运行构建 `make`

```bash
make -j4
```

### 自定义工具链构建 RMVL

如果交叉编译器是通过编译内核源码获取到的，在使用这类指定的交叉编译器进行编译时，自定义工具链是个有效的方法。

工具链文件的一般写法如下

```cmake
# toolchain.cmake

set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR aarch64)

set(CMAKE_C_COMPILER "/path/to/xxx-gcc")
set(CMAKE_CXX_COMPILER "/path/to/xxx-g++")

set(CMAKE_FIND_ROOT_PATH "/path/to/")
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
```

然后进入 RMVL 项目根目录，在终端中输入

```bash
cmake -DCMAKE_TOOLCHAIN_FILE=/path/to/toolchain.cmake ..
```

同样，CMake 配置也可使用 `cmake-gui` 或者 `ccmake` 工具。

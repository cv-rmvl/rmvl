为基于 ARM 的 Linux 系统配置交叉编译{#tutorial_other_arm}
============

@prev_tutorial{tutorial_use}
@next_tutorial{tutorial_document}

@tableofcontents

------

### 交叉编译应具备的先决条件

* Linux 主机

* ![img](https://img.shields.io/badge/CMake-3.19+-green)

* ![img](https://img.shields.io/badge/OpenCV_for_ARM-4.2.0+-red) [点击此处下载最新发行版](https://github.com/opencv/opencv/releases/latest)

* ARM 的交叉编译工具：gcc，libstdc++.so 等内容，根据目标平台的不同，您需要选择 `gneabi` 或 `gneabihf` 工具。`gneabi` 安装命令如下
  ```shell
  sudo apt install gcc-arm-linux-gneabi
  ```
  `gneabihf` 安装命令如下
  ```shell
  sudo apt install gcc-arm-linux-gneabihf
  ```

### 构建 RMVL

1. 在 RMVL 目录下创建 `build` 文件夹，运行以下命令
   ```shell
   cd build
   cmake [<some optional parameters>] -DCMAKE_TOOLCHAIN_FILE=../platforms/linux/arm-gnueabi.toolchain.cmake ..
   ```

2. 运行构建 `make`
   ```shell
   make -j4
   ```
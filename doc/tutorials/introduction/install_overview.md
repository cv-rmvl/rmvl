视觉库安装总览 {#tutorial_install_overview}
============

@next_tutorial{tutorial_configuration_options}

@tableofcontents

------

#### 源码安装

- RMVL 唯一的一种安装方式：从源代码开始编译

- 前往 https://github.com/cv-rmvl/rmvl/releases 并下载 "Release" 存档

- 也可直接 clone RMVL 的远程存储库，这样您就可以体验 RMVL 的最新功能（可能不稳定）。
  ```shell
  git clone https://github.com/cv-rmvl/rmvl.git
  ```

#### CMake 架构

- RMVL 采用 [CMake](https://cmake.org) 构建管理系统进行配置和构建。版本号最低要求为 3.16。

- RMVL 设置丰富的编译选项，可供您在 CMake 配置期间根据您的环境配置，或者预期功能选择您想构建的部分。

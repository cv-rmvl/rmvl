## RMVL: RoboMaster Vision Library

**RoboMaster 视觉库**

![image](https://img.shields.io/badge/OpenCV-4.5.0+-red) ![image](https://img.shields.io/badge/CMake-3.19+-blue)

RMVL 是面向 RoboMaster 赛事的视觉库，旨在打造适用范围最广、使用最简洁、架构最统一、功能最强大的视觉库，并且以此建立和谐的纯技术向的 RoboMaster 视觉开源社区。

### 项目资源

* 说明文档:（暂无）亦可自行构建说明文档，具体构建方式如下

  + 安装 Doxygen 依赖

    ```shell
    sudo apt install doxygen doxygen-gui graphviz
    ```

  + 生成说明文档

    ```shell
    mkdir build
    cd build
    cmake -DBUILD_DOCS=ON ..
    make doxygen
    ```

  + 在浏览器中打开位于 build 下的 `html/index.html`

    ```shell
    firefox html/index.html
    ```

* 工单: <https://github.com/RoboMaster-Vision/RMVL/issues>

### 贡献

* 在发起合并请求之前，请先阅读[贡献指南](https://github.com/RoboMaster-Vision/RMVL/wiki/How_to_contribute)。

构建并安装 RMVL {#tutorial_install}
============

@brief RMVL 的依赖安装、项目配置、编译安装

@prev_tutorial{tutorial_configuration_options}
@next_tutorial{tutorial_use}

@tableofcontents

------

### 1. 安装依赖

@note 带有 `(*)` 的表示必须安装

#### 1.1 OpenCV (*)

OpenCV @cite opencv 是 RMVL 必需的依赖库，如果没有找到 OpenCV 库，RMVL 将无法构建，下面介绍 OpenCV 的 2 种安装方法。
 
@add_toggle{快速安装}

Linux 发行版常用的镜像站中一般都添加了 OpenCV 的软件源，可以很方便的通过 `apt` 包管理工具安装，例如 Ubuntu

- `18.04` 对应的 OpenCV 版本是 `3.2.0`
  
  <span style="color: red">此版本过低</span>，无法通过 CMake 的 `find_package` 方式找到 OpenCV，若是在这个 Ubuntu 的发行版，请手动编译安装 4.0 或以上版本的 OpenCV

- `20.04` 对应的 OpenCV 版本是 `4.2.0`

- `22.04` 对应的 OpenCV 版本是 `4.5.4`

可以输入以下命令行进行安装

```shell
sudo apt install libopencv-dev
```

@end_toggle

@add_toggle{编译安装}

**安装 OpenCV 依赖**

```shell
sudo apt install build-essential
sudo apt install libgtk2.0-dev libavcodec-dev libavformat-dev libjpeg-dev libswscale-dev libtiff5-dev pkg-config
```

**下载并解压缩 OpenCV**

```shell
wget https://codeload.github.com/opencv/opencv/tar.gz/refs/tags/4.7.0
tar -xvf 4.7.0
```

**构建 OpenCV**

```shell
cd opencv-4.7.0
mkdir build && cd build
cmake \
  -DBUILD_EXAMPLES=OFF \
  -DBUILD_PERF_TESTS=OFF \
  -DBUILD_TESTS=OFF \
  -DBUILD_opencv_python3=OFF \
  -DBUILD_JAVA=OFF \
  -DBUILD_opencv_js=OFF \
  -DBUILD_opencv_gapi=OFF \
  -DOPENCV_ENABLE_NONFREE=ON \
  -DENABLE_FAST_MATH=ON \
  -DWITH_GSTREAMER=ON \
  -DCMAKE_BUILD_TYPE=Release \
  ..

make -j8 && sudo make install
```

@end_toggle

@see
- [OpenCV documents](https://docs.opencv.org/4.x/)

#### 1.2 Eigen3 (*)

使用 `apt` 包管理工具进行安装

```shell
sudo apt install libeigen3-dev
```

#### 1.3 硬件设备 SDK

<table class="markdownTable">
<tr class="markdownTableHead">
  <th class="markdownTableHeadCenter">设备</th>
  <th class="markdownTableHeadCenter">品牌</th>
  <th class="markdownTableHeadCenter">CMake 包 <code>find_package(..)</code></th>
  <th class="markdownTableHeadCenter">SDK 下载地址（点击即可下载）</th></tr>
<tr class="markdownTableRowOdd">
  <td class="markdownTableBodyCenter" rowspan="3">相机</td>
  <td class="markdownTableBodyCenter">MindVision</td>
  <td class="markdownTableBodyCenter">MvSDK</td>
  <td class="markdownTableBodyCenter">
    <a href="https://www.mindvision.com.cn/uploadfiles/SDK/linuxSDK_V2.1.0.37.tar.gz">For all arch</a>
  </td></tr>
<tr class="markdownTableRowEven">
  <td class="markdownTableBodyCenter">HikVision</td>
  <td class="markdownTableBodyCenter">HikSDK</td>
  <td class="markdownTableBodyCenter">
    <a href="https://www.hikrobotics.com/cn2/source/support/software/MVS_STD_GML_V2.1.2_221208.zip">For all arch</a>
  </td></tr>
<tr class="markdownTableRowOdd">
  <td class="markdownTableBodyCenter">OPT</td>
  <td class="markdownTableBodyCenter">OPTCameraSDK</td>
  <td class="markdownTableBodyCenter">
    <a href="https://vision.scutbot.cn/files/OPTCameraDemo_Ver3.1_Linux_x86_Build20220429.run">For all arch</a>
  </td></tr>
<tr class="markdownTableRowEven">
  <td class="markdownTableBodyCenter">光源控制器</td>
  <td class="markdownTableBodyCenter">OPT</td>
  <td class="markdownTableBodyCenter">OPTLightCtrl</td>
  <td class="markdownTableBodyCenter">
    <a href="https://vision.scutbot.cn/files/opt-controller-amd64.deb">For amd64</a>
  </td></tr>
</table>
@note 以上与相机相关的 SDK 在进行二次封装得到的库都需要链接到 OpenCV。

#### 1.4 onnxruntime

onnxruntime 库是目前数字识别所依赖的第三方库，如果有需要开启此功能，则需要安装 onnxruntime

- 获取压缩包
  ```shell
  curl -SL https://github.com/microsoft/onnxruntime/releases/download/v1.12.0/onnxruntime-linux-x64-1.12.0.tgz -o onnxruntime-linux-x64-1.12.0.tgz
  ```
- 解压
  ```shell
  tar -xvf onnxruntime-linux-x64-1.12.0.tgz
  ```
- 安装（复制头文件与库文件）
  ```shell
  sudo mkdir /usr/local/include/onnxruntime
  sudo cp onnxruntime-linux-x64-1.12.0/include/* /usr/local/include/onnxruntime
  sudo cp -r onnxruntime-linux-x64-1.12.0/lib /usr/local
  ```
- 移除中间文件
  ```shell
  rm -r onnxruntime-linux-x64-1.12.0 && rm onnxruntime-linux-x64-1.12.0.tgz
  ```

### 2. 配置 RMVL 项目

进入编译空间，没有 `build` 文件夹请先创建

```shell
cd build
```

请继续在终端中输入以下内容

```shell
cmake ..
```

或者使用图形用户界面（GUI）来配置 RMVL

```shell
cmake-gui ..
```

@note
若需要启用单元测试，请输入
```shell
cmake -D BUILD_TESTS=ON ..
```

### 3. 构建 RMVL 并进行单元测试

#### 3.1 编译安装

编译 RMVL（这里开启 8 个线程进行编译，可灵活设置）

```shell
make -j8
```

@note 若启用了单元测试，可以运行 RMVL 单元测试的测试用例
```shell
ctest
```

在构建完成之后，可以通过 `make install` 来安装 RMVL 的头文件、库文件、和 CMake 配置文件

```shell
sudo make install
```

同样可以选择线程数来加速安装过程

```shell
sudo make install -j4
```

#### 3.2 检查安装结果

在任意一个地方打开终端，输入

```shell
rmvl_version
```

如果显示了对应的版本号，则安装成功。同时，也可查看构建时的配置情况

```shell
rmvl_version -v
```

------

若想为 RMVL 编写测试 demo，请继续阅读下一篇： @ref tutorial_use 。

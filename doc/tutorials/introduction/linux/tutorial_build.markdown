视觉库的配置与构建 {#tutorial_build}
============

@prev_tutorial{tutorial_configuration_options}
@next_tutorial{tutorial_install}

@tableofcontents

------

### 1. 安装依赖

@note 带有 `*` 的表示必须安装

#### 1.1 OpenCV*

OpenCV 是 RMVL 必需的依赖库，如果没有找到 OpenCV 库，RMVL 将无法构建，下面介绍 OpenCV 的安装方法。

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

@see
- [OpenCV documents](https://docs.opencv.org/4.x/)

#### 1.2 Eigen3*

```shell
# use apt to obtain Eigen3
sudo apt install libeigen3-dev
```

#### 1.3 相机 SDK（可选）

| 相机厂商 | 编译选项 | CMake 包 `find_package(..)` | SDK 下载地址（点击即可下载） |
| :-----: | :----: | :-----------: | :--------: |
| MindVision | `BUILD_rmvl_mv_video_capture` | MvSDK | [RMVL_mv](https://www.mindvision.com.cn/uploadfiles/SDK/linuxSDK_V2.1.0.37.tar.gz) |
| HikVision | `BUILD_rmvl_hik_video_capture` | HikSDK | [RMVL_hik](https://www.hikrobotics.com/cn2/source/support/software/MVS_STD_GML_V2.1.2_221208.zip) |

以上 SDK 在进行二次封装得到的相机库都需要链接到 OpenCV。

#### 1.4 onnxruntime

onnxruntime 库是目前数字识别所依赖的第三方库，如果有需要开启此功能，则需要执行以下命令。

```shell
# 获取压缩包
curl -SL https://github.com/microsoft/onnxruntime/releases/download/v1.12.0/onnxruntime-linux-x64-1.12.0.tgz -o onnxruntime-linux-x64-1.12.0.tgz
# 解压
tar -xvf onnxruntime-linux-x64-1.12.0.tgz
# 安装（复制头文件与库文件）
sudo mkdir /usr/local/include/onnxruntime
sudo cp onnxruntime-linux-x64-1.12.0/include/* /usr/local/include/onnxruntime
sudo cp -r onnxruntime-linux-x64-1.12.0/lib /usr/local
# 移除中间文件
rm -r onnxruntime-linux-x64-1.12.0 && rm onnxruntime-linux-x64-1.12.0.tgz
```

### 2. 配置 RMVL 项目

#### 2.1 进入编译空间

```shell
cd build
```

#### 2.2 请继续在终端中输入以下内容

```shell
# Open the tests
cmake -D BUILD_TESTS=ON ..
```

或者使用图形用户界面（GUI）来配置 RMVL

```shell
cmake-gui ..
```

### 3. 构建 RMVL

```shell
make -j8
```

### 4. 运行 RMVL 的测试用例

```shell
ctest
```
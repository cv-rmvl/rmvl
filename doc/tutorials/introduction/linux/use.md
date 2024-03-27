通过 gcc 和 CMake 使用 RMVL {#tutorial_use}
============

@brief RMVL 部署示例（涉及 extra 模块）

@prev_tutorial{tutorial_install}
@next_tutorial{tutorial_other_arm}

@tableofcontents

------

**注意**

- <span style="color: red">必须确保您已经成功安装了 RMVL</span>。如果你不熟悉 CMake，请移步到 CMake [教程](https://cmake.org/cmake/help/latest) 
- 本示例使用到了 rm::ArmorDetector 模块，需要提前安装 onnxruntime 的 C++ 开发包，可参考 @ref install_onnxruntime 的内容，若未安装，请安装后再重新编译 RMVL。

### 详细步骤

#### 1. 创建项目

创建新的文件夹：`rmvl_deploy_test`，将其链接至 RMVL

```shell
mkdir rmvl_deploy_test build
cd rmvl_deploy_test
touch main.cpp CMakeLists.txt
```

#### 2. 编写测试文件

**编写 main.cpp**

将以下内容写至 `main.cpp` 中

```cpp
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>

#include <rmvl/detector.hpp>

int main(int argc, char *argv[])
{
    cv::Mat src = cv::Mat::zeros(cv::Size(1280, 1024), CV_8UC3);

    cv::line(src, cv::Point(450, 380), cv::Point(440, 470), cv::Scalar(0, 0, 255), 18);
    cv::line(src, cv::Point(650, 400), cv::Point(640, 490), cv::Scalar(0, 0, 255), 18);
    cv::line(src, cv::Point(850, 370), cv::Point(870, 460), cv::Scalar(0, 0, 255), 18);

    rm::detector::ptr detector = rm::ArmorDetector::make_detector();
    std::vector<rm::group::ptr> groups;

    auto info = detector->detect(groups, src, rm::RED, rm::GyroData{}, cv::getTickCount());

    INFO_("size of armors = %ld", info.combos.size());
    for (auto p_combo : info.combos)
    {
        const auto &corners = p_combo->getCorners();
        cv::line(src, corners[0], corners[2], cv::Scalar(0, 255, 0), 2);
        cv::line(src, corners[1], corners[3], cv::Scalar(0, 255, 0), 2);
    }

    cv::namedWindow("rmvl_deploy_test: src", cv::WINDOW_NORMAL);
    cv::resizeWindow("rmvl_deploy_test: src", cv::Size(640, 512));
    cv::imshow("rmvl_deploy_test: src", src);

    HIGHLIGHT_("RMVL build Successfully!\n\n\t\t-------- "
               "press any key to exit this program. --------\n");
    cv::waitKey(0);

    return 0;
}
```

**编写 CMakeLists.txt**

将以下内容写至 `CMakeLists.txt` 中
```cmake
cmake_minimum_required(VERSION 3.16)

project(rmvl_deploy)

find_package(RMVL REQUIRED)

add_executable(demo main.cpp)

target_link_libraries(
    demo
    PRIVATE ${RMVL_LIBS}
)
```

#### 3. 构建项目

在 `rmvl_deploy_test` 的顶层文件夹中打开终端，输入以下命令

```shell
cd build
cmake ..
make
```

#### 4. 运行

继续输入以下命令

```shell
./demo
```

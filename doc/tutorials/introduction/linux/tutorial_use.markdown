通过 gcc 和 CMake 使用视觉库 {#tutorial_use}
============

@prev_tutorial{tutorial_install}
@next_tutorial{tutorial_other_arm}

@tableofcontents

------

@note 必须确保您已经成功安装了 RMVL

如果你不熟悉 CMake，请移步到 CMake [教程](https://cmake.org/cmake/help/latest) 

### 详细步骤

#### 1. 创建项目

创建新的文件夹：`rmvl_deploy_test`，将其链接至 RMVL

```shell
mkdir rmvl_deploy_test build
cd rmvl_deploy_test
touch main.cpp CMakeLists.txt
```

#### 2. 编写测试文件

##### 2.1 编写 main.cpp

将以下内容写至 `main.cpp` 中
```cpp
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>

#include <rmvl/detector.hpp>

using namespace std;
using namespace cv;

int main(int argc, char *argv[])
{
    Mat src = Mat::zeros(Size(1280, 1024), CV_8UC3);

    line(src, Point(450, 380), Point(440, 470), Scalar(0, 0, 255), 18);
    line(src, Point(650, 400), Point(640, 490), Scalar(0, 0, 255), 18);
    line(src, Point(850, 370), Point(870, 460), Scalar(0, 0, 255), 18);

    detect_ptr detector = ArmorDetector::make_detector();
    vector<group_ptr> groups;

    detector->detect(groups, src, RED, GyroData(), getTickCount());
    auto p_combos = detector.combos;

    INFO_("size of armors = %ld", p_combos.size());
    for (auto &p_combo : p_combos)
    {
        auto corners = p_combo->getCorners();
        line(src, corners[0], corners[2], Scalar(0, 255, 0), 2);
        line(src, corners[1], corners[3], Scalar(0, 255, 0), 2);
    }

    namedWindow("rmvl_deploy_test: src", WINDOW_NORMAL);
    resizeWindow("rmvl_deploy_test: src", Size(640, 512));
    imshow("rmvl_deploy_test: src", src);

    HIGHLIGHT_("RMVL build Successfully!\n\n\t\t-------- "
               "press any key to exit this program. --------\n");
    waitKey(0);

    return 0;
}
```

##### 2.2 编写 CMakeLists.txt

将以下内容写至 `CMakeLists.txt` 中
```cmake
cmake_minimum_required(VERSION 3.19)
project(rmvl_deploy)
find_package(RMVL REQUIRED)
add_executable(demo main.cpp)
target_include_directories(
    demo
    PUBLIC ${RMVL_INCLUDE_DIRS}
)
target_link_libraries(
    demo
    ${RMVL_LIBS}
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

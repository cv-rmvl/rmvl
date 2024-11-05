导出数据、发出操控指令{#tutorial_extra_upper_write_data}
============

@author 赵曦
@date 2023/10/04

@prev_tutorial{tutorial_extra_upper_process}

@tableofcontents

------

### 1. 显示图像、打印调试信息

视觉程序日常调试必不可少的就是显示图像，例如 OpenCV 中提供了 `cv::imshow` 函数来显示图像。在执行完程序处理的全部流程后，可以对当前帧捕获到或计算出的信息进行观察， rm::DetectInfo ， rm::CompensateInfo ， rm::PredictInfo ， rm::DecideInfo 包含了各自模块得到的全部信息。例如 rm::DetectInfo 中提供了原图、二值图、所有 ROI、渲染图等图像信息，可以使用 `cv::imshow()` 显示到屏幕； rm::CompensateInfo 中提供了补偿的角度偏移信息，可以使用 `printf` 或 `std::cout` 输出到终端。

### 2. 角点写入文件

@ref core_io 提供了 rm::readCorners 和 rm::writeCorners 函数，可以从指定的 YAML 文件读取角点信息至 `std::vector<std::vector<cv::Point>>` 表示的角点集 `corners` 中，或将 `corners` 写入到指定的 YAML 文件中。

参考以下将角点写入文件的示例代码

```cpp
// 获取识别模块得到的所有组合体
auto combos = detect_info.combos;
std::vector<std::vector<cv::Point2f>> corners;
corners.reserve(combos.size());

for (auto p_combo : combos)
    corners.emplace_back(p_combo->corners());

/* 当前循环到第 idx 轮 */

rm::writeCorners("corners.yml", idx, corners);
```

### 3. 发出操控指令至下位机

`XxxInfo` 类的数据信息不能直接发送至下位机，给下位机发送的数据一般是控制电机的若干个速度分量或偏移量，这一步可以由 rm::DecideInfo 中存储的信息导出，通信方式各异，这里不多赘述。

------

至此，顶层模块的所有内容已介绍完毕，下面开始[创建](https://github.com/new)自己的顶层项目吧！

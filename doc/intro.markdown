引言与介绍 {#intro}
============

@tableofcontents

RMVL 提供了简化视觉伺服控制操作的功能，并包括智能射击机器人的基本功能模块，包括：步兵 Infantry、英雄 Hero、哨兵 Sentry 和无人机 Drone 。该代码库易于维护，所有的功能模块都已解耦，包括由特征`feature`、特征组合`combo`、追踪器`tracker`和序列组`group`组成的 **数据组件** ，由检测和识别模块`detector`、补偿模块`compensator`、目标预测模块`predictor`和决策模块`decider`组成的 **功能模块** ，还有已经打包了许多厂商的工业相机底层驱动程序的相机模块，以及与单片机进行通信的串口通信模块。此外，还有一些实用程序可以缩短开发周期，如图像处理`imgproc`、线程池库`thread_pool`、机器学习与深度学习支持库`ml`等。

RMVL 具有模块化结构，这意味着该软件包包含了多个共享或静态库，有以下内容可用。

主要模块 {#main_modules}
-----

- @ref core (**core**) —— 包含主要的工具库、数据读写、宏定义、版本管理等内容，以及 rm::Exception 异常管理模块就在此处定义。

- @ref camera (**camera**) —— 目前包含基于 MindVision 迈德威视工业相机、HikRobot 海康机器人工业相机两种相机厂商 SDK 的二次开发工具库，点击[此处](@ref tutorial_build)可以安装以上两种相机的 SDK。

- @ref ml (**ml**)，@ref rmath (**rmath**)

- ... 以及其他包含在 `modules` 文件夹中的模块。

自瞄模块 {#autoaim_modules}
-----

### 数据组件 {#data_components}

- @ref feature (**feature**) —— RMVL 最基本的数据组件，代表图像中的一个封闭曲线（轮廓、简单封闭图形）。开发中，轮廓通常可以使用 **OpenCV** 中的 `cv::findContours` 函数来获取，简单封闭图形可通过 **OpenCV** 中的 `imgproc` 模块提供的各类函数接口来进行获取，例如最小外接矩形 `cv::minAreaRect`、最小包络三角 `cv::minEnclosingTriangle` 等，提取到的对象 `cv::RotatedRect` 或一个 `cv::OutputArray` 等内容可由开发者自行转化成 `feature` 有用的信息（一般是通过 `feature` 的构造函数完成）。

- @ref combo (**combo**) —— 这种类型的组件由一系列特征组成，并使用 `std::vector<feature_ptr>` 来存储它们，特征的数量通常不会太多，并且这类特征在物理空间中通常是刚性的，即特征之间大致具备尺度不变的特点。开发中，一般会使用若干特征以及附带信息作为 `combo` 的构造函数。

- @ref tracker (**tracker**) —— `tracker` 是在时间上包含了许多相同物理特性的 `combo`，从而形成一个 `combo` 的时间序列，在 RMVL 中，则通过使用 `std::deque<combo_ptr>` 来表示这个时间序列。在功能上，`tracker` 不仅表示了不同时间下相同的 `combo` 的相关信息，还能处理在某个时间点上获取到不正确的 `combo` 的异常情况。

- @ref group (**group**) —— 如果多个追踪器在物理空间上具有一定的相关性，比如共享轴旋转、刚性连接等属性，它们可以一起形成一个序列组 `group`，从而能够表示更加高级的物理信息。序列组使用 `std::vector<tracker_ptr>` 来存储这些追踪器。

### 功能模块 {#function_modules}

- @ref detector (**detector**) —— 识别、检测器是功能模块中最重要的部分，也是视觉图像处理的第一步。它负责对输入图像进行识别并加以处理，提取出目标轮廓、特征点等信息，并结合已知的部分数据组件，按顺序依次构建各种新的数据组件。
  <center>
    <a href="https://imgse.com/i/xOgdat" target="_blank">
      <img src="https://s1.ax1x.com/2022/11/05/xOgdat.md.png" alt="xOgdat.png" border="0" />
    </a>
  </center>
  识别得到的各种图像以及提取到的特征和组合体均保存至识别模块信息 `rm::DetectInfo` 中。

- @ref compensator (**compensator**) —— 补偿器通常是功能模块中的第一个用于修正数据组件的算法模块，主要负责修正弹道下坠的影响。计算得到的子弹飞行时间 `tof` 以及补偿增量 `compensation` 均保存至补偿模块信息 `rm::CompensateInfo` 中。

- @ref predictor (**predictor**) —— 对于智能射击机器人而言，必须要引入目标预测环节，使得弹丸能够准确击中敌方目标。一般而言，每一个预测模块会针对特定的数据组件（派生的 `group` 对象）进行 `Kt`、`B`、`Bs` 3 种预测量类型的计算，每种预测量类型都包含 9 种预测对象，每个预测对象之间对于 3 种预测量来说都是线性关系，例如在 @ref gyro_predictor 中对于 `ANG_Y` 预测对象的 3 种预测量都是单独计算的，这也便于后续的 **决策模块** 能够自由组合这些预测量。同样，计算得到的各类预测量均被保存至预测模块信息 `rm::PredictInfo` 中。

- @ref decider (**decider**) —— 在经过识别、补偿和预测 3 个步骤后，需要使用特定的策略来获得当前时刻的最优目标，结合前三个步骤的信息（包含识别模块信息 `DetectInfo` 、补偿模块信息 `CompensateInfo` 、预测模块信息 `PredictInfo` ）导出到决策模块信息 `rm::DecideInfo` 中。

该文档的后续章节描述了每个模块的功能。但首先，请确保彻底了解库中使用的常用 API 概念。

API 概念
------------

### 统一的接口

RMVL中的每个功能模块都设计有一个抽象类和一系列派生类，以严格限制暴露的接口并实现接口的统一性。这种方法有助于顶层项目统一管理功能模块中特定的方法，顶层项目开发人员可以专注于模块之间的逻辑关系处理。例如，您可以使用同一接口实现不同的功能。

```cpp
#include <rmvl/detector.hpp>
/* ... */
std::vector<detect_ptr> detectors;
detectors.emplace_back(ArmorDetector::make_detector());
detectors.emplace_back(TopArmorDetector::make_detector());
detectors.emplace_back(RuneDetector::make_detector());

std::vector<group_ptr> groups;
/* ... */
for (auto &p_detector : detectors)
{
    auto info = p_detector->detect(groups, src, color, gyro_data, tick);
    /* ... */
}
/* ... */
```

### 低耦合逻辑

RMVL 中功能模块之间的耦合度非常低，它们不会相互影响或依赖。例如，基于装甲的自动瞄准模块可以通过以下代码实现，它们之间共享的数据组件是对象是一个序列组列表：`groups`。

```cpp
#include <rmvl/detector.hpp>
#include <rmvl/compensator.hpp>
#include <rmvl/predictor.hpp>
#include <rmvl/decider.hpp>
/* ... */
detect_ptr p_detector = ArmorDetector::make_detector();
compensate_ptr p_compensator = GravityCompensator::make_compensator();
predict_ptr p_predictor = ArmorPredictor::make_predictor();
decide_ptr p_decider = ArmorDecider::make_decider();
vector<group_ptr> groups;
/* ... */
auto detect_info = p_detector->detect(groups, src, BLUE, GyroData(), getTickCount());
auto compensate_info = p_compensator->compensate(groups, {gyro_data.yaw, gyro_data.pitch}, shoot_speed);
auto predict_info = p_predictor->predict(groups);
auto decide_info = p_decider->decide(groups);
/* ... */
```

例如，对一个数据组件进行识别的结果并不影响预测的逻辑。换句话说，在对象识别过程中，异常和不合格的对象不应该被交给预测、补偿和决策模块进行后续处理，而应由识别检测模块本身解决。

### 参数管理

不同的机器人具有不同的属性，它们对功能需求、运动响应和通信延迟都有不同的影响。RMVL 为所有这些参数提供了默认值，以确保正确的运行。但在实际部署时，但这些参数通常不是最有效的、最正确的，因此像机器人这样的顶层项目需要手动加载这些参数以最大化性能。

到目前为止，RMVL 的所有参数文件都是通过 **YAML** 文件读取和写入运行时加载的，参数规范文件本身由 `*.para` 文件所规定，包含了该参数的类型、标识符、默认值以及注释。这个文件在 CMake 架构期间会生成一系列对应的 `*.cpp` 和 `*.h/*.hpp`，这个功能由一系列 **CMake** 语法定义，例如 `rmvl_generate_para`。RMVL 参数模块在命名空间 `para` 中提供了统一的参数加载接口 `load()`，并限制只能将参数传递给已实例化的 `xxxParam` 对象。请参考以下代码：

```cpp
#include <rmvlpara/loader.hpp>

#include <rmvlpara/combo/armor.h>
#include <rmvlpara/camera.hpp>

/* code */

std::string prefix_str = "../etc/";

para::load(para::armor_param, prefix_str + "armor_param.yml");
para::load(para::camera_param, prefix_str + "camera_param.yml");
```

这段代码中，`rmvlpara/loader.hpp` 头文件被包含，并使用`load()`函数来加载参数。以加载`armor_param`和`camera_param`为例，假设这些参数分别对应于装甲板和相机的参数。

### 异常处理

RMVL 使用异常 (exception) 来触发错误，当输入数据出现格式错误、范围错误、内存错误等异常情况，它会返回一个特殊的错误代码。

异常可以是 rm::Exception 的实例化对象，或者其派生对象。此外 rm::Exception 是 `std::exception` 的派生类，因此可以优雅的使用标准 C++ 库的各类组件来处理 RMVL 抛出的异常。

RMVL 中的异常通常是使用 `RMVL_Error(code, msg)` 宏或类似 `printf` 的 `RMVL_Error_(code, fmt, args...)` 变体进行触发的，或者可以使用 `RMVL_Assert(cond)` 宏检查条件并在不满足条件时触发异常。对于性能要求高的代码，有一个 `RMVL_DbgAssert(cond)` ，它只保留在 Debug 配置中。由于自动内存管理，所有中间缓冲区自动释放的情况下突然出现错误，您只需添加 `try` 语句来捕获异常。

```cpp
try
{
  ... // 调用 RMVL 函数
}
catch (const rm::Exception& e)
{
  const char* err_msg = e.what();
  printf("exception caught: %s", err_msg);
}
```

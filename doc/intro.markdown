引言与介绍 {#intro}
============

@tableofcontents

### 写在前面

#### 前世今生{#YAT}

RMVL 起源于 SRVL（SCUT Robotlab Vision Library——华南理工大学机器人实验室视觉库）。SRVL 由华南理工大学华南虎战队视觉组成员共同研发，于实验室团队内部自建的 Gitea 服务完成代码托管，组员在此之上进行功能开发和维护。SRVL 主要用于参加 [RoboMaster 系列赛事](https://www.robomaster.com)。该项目在 RM2021 赛季内部技术交流活动后，即 2021 年 8 月正式启动，总共经过 3 届队员传承、更迭，依次开发了 1.x ~ 4.x 共 4 个系列的版本。

- **1.x** —— *2021.10* 发布<span style="color: green">（未开源）</span>，主要针对 RoboMaster 2021 赛季步兵、英雄机器人代码混乱的情况进行设计，初步尝试了使用抽象工厂设计模式来设计代码架构，届时，SRVL 开发者和使用者的不同需求需要在 SRVL 编译过程中修改编译选项。
- **2.x** —— *2022.01* 发布<span style="color: green">（未开源）</span>，使用抽象工厂设计模式后出现了维护困难的情况，包括但不限于多个 @ref function_modules 共同组合，导致产生非常多的派生工厂，此版本针对这一弊端移除了原先所有的设计模式，各功能模块仅单独存在，不再另外设置组合或其他强依赖关系。此外， **2.x** 相较于 **1.x** 添加了全新的内容： @ref group 。
- **3.x** —— *2022.08* 发布<span style="color: green">（未开源）</span>，架构、功能进一步完善。在开发上，添加了 CI/CD 自动化构建测试工具，使用 GoogleTest 为已有的组件、模块添加了单元测试，使用 benchmark 为部分功能添加了性能基准测试；在功能上，移除原先所有的 `group` 组件，重定义并完善了 `group` 组件所应当具备的功能，后续在此系列代码基础上，完成了 **RM2023 版整车状态估计** ；在使用上，顶层模块与视觉库完全分离，涉及到各个功能开启或关闭的逻辑功能将设置在顶层模块，这一部分内容由用户自行实现。此外，该版本完善了 CMake 的项目构建方式，并且可通过 `make install/uninstall` 完成编译安装、卸载。
- **4.x** —— *2023.09* 发布<span style="color: green">（已开源）</span>，并登陆 [Github](https://github.com/cv-rmvl/rmvl) 平台，更名 RMVL 以追求更加广泛的使用场景。并发布了 **RMVL 1.x** 系列版本，该系列彻底形成了面向对象迭代器设计模式与责任链设计模式相互结合的代码架构，同时将原先的 CI/CD 流水线迁移至 Github Actions。 **4.x** 版本在设计之初主要为了简化数据组件的开发，移除了不属于 @ref data_components 管理的，但在 @ref function_modules 中被设置的信息。此外还为各个 @ref function_modules 加入了 `XxxInfo` 的信息类。该版本首次加入命名空间 `rm`，避免了与其他库命名冲突的情况。此外，该版本极大程度简化并统一了参数模块的定义方式，将原先繁琐的参数定义、加载的功能使用自定义的参数文件 `*.para` 以及一组 CMake 函数 `rmvl_generate_para` 自动完成 C++ 代码生成与文档注解生成，为了更进一步简化参数模块的设计，RMVL 提供了 Visual Studio Code 的插件，为 `*.para` 文件提供语法高亮、代码提示、悬浮提示与代码块，并为 RMVL 中的部分函数与宏提供了代码提示、悬浮提示与代码块。
- **RMVL 2.x** —— *2024.09* 发布<span style="color: green">（已开源）</span>，在功能上，该系列首次加入了 Python 支持，可参考 @ref tutorials_python ，此外还为 Windows 用户提供了 MSVC 编译器的支持，使用 CPack 工具，为 Windows 以及 Debian 系 Linux 发行版制作安装包。在架构上，新增 @ref algorithm ，将原先 @ref core 中的各类算法迁移至该模块。新增 @ref io ，提供跨平台的 I/O 协程设施，以及同步、异步的串口通信、进程间通信等内容，并提供了异步的 HTTP Web 后端框架。

#### 使用对象与基本情况

RMVL 为硬件设备二次开发、网络通信、串口通信以及运动、控制、视觉算法提供了相应的支持库，为工业、日常环境下某些特征的识别、追踪等提供了完整的流程，同时也可为 RoboMaster 参赛者提供有关装甲板识别与运动追踪、能量机关识别与运动追踪等自动瞄准的完整内容，具体可提供的内容如下

- 机器人在一些经典的、特有的工作环境下的功能需求，涉及到运动追踪、视觉识别等内容
- 控制、通信、数据结构的基础工具库
- 涉及 RM 的步兵机器人、英雄机器人、哨兵机器人和空中机器人 4 类射击型机器人的绝大部分功能
- 涉及 RM 其余兵种的部分识别算法功能

该代码库易于维护，说明文档丰富且齐全， @ref extra_modules 中所有的模块均已解耦，包括由特征 `feature`、特征组合 `combo`、追踪器 `tracker` 和序列组 `group` 组成的 **数据组件** ，由检测和识别模块 `detector`、补偿模块 `compensator`、目标预测模块 `predictor` 和决策模块 `decider` 组成的 **功能模块** ，这些模块分布在项目的 <span style="color: red">extra</span> 文件夹中。此外如上文所说，RMVL 还涉及到其余有关数据结构与算法、硬件设备二次开发库等内容，这些内容作为主要模块分布在项目的 <span style="color: red">modules</span> 文件夹中。

------------

RMVL 具有模块化结构，这意味着该软件包包含了多个共享或静态库，有以下内容可用。

### 主要模块 {#main_modules}

- @ref core (**core**) —— 包含主要的工具库、数据读写、宏定义、版本管理等内容，以及 rm::Exception 异常管理模块就在此处定义。

- @ref camera (**camera**) —— 目前包含 3 种相机厂商 SDK 的二次开发工具库，具体可参考下表。

  <div class="full_width_table">

  表 1: 相机厂商 SDK 二次开发工具库<br>
  |         厂商          | 简称 |
  | :-------------------: | :--: |
  |    奥普特机器视觉     | OPT  |
  | 海康机器人 *HIKROBOT* | Hik  |
  | 迈德威视 *Mindvision* |  Mv  |
  
  </div>

  点击[此处](@ref tutorial_install)可以安装以上相机的 SDK
  
- @ref ml (**ml**)， @ref opcua (**opcua**)

- ... 以及其他包含在 `modules` 文件夹中的模块。

### 扩展模块 {#extra_modules}

#### 数据组件 {#data_components}

@ref feature (**feature**) —— RMVL 最基本的用于存储的数据结构，代表图像中的一个封闭曲线（轮廓、简单封闭图形）。开发中，轮廓通常可以使用 **OpenCV** @cite opencv_library 中的 `cv::findContours` 函数来获取，简单封闭图形可通过 **OpenCV** 中的 `imgproc` 模块提供的各类函数接口来进行获取，例如最小外接矩形 `cv::minAreaRect`、最小包络三角 `cv::minEnclosingTriangle` 等，提取到的对象 `cv::RotatedRect` 或一个 `cv::OutputArray` 等内容可由开发者自行转化成 `feature` 有用的信息（一般是通过 `feature` 的构造函数完成）。此外 RMVL 提供了默认 `feature`，即 rm::DefaultFeature ，用于表示无轮廓的信息，一般是一个角点。

@ref combo (**combo**) —— 这种类型的组件由一系列特征组成，并使用 `std::vector<feature::ptr>` 来存储它们，特征的数量通常不会太多，并且这类特征在物理空间中通常是刚性的，即特征之间大致具备尺度不变的特点。开发中，一般会使用若干特征以及附带信息用于构造 `combo` 的派生对象。此外 RMVL 提供了默认 `combo`，即 rm::DefaultCombo ，用于表示单独的无关联的 `feature`。

@ref tracker (**tracker**) —— `tracker` 是在时间上包含了许多相同物理特性的 `combo`，从而形成一个 `combo` 的时间序列，在 RMVL 中，则通过使用 `std::deque<combo::ptr>` 来表示这个时间序列。在功能上，`tracker` 不仅表示了不同时间下相同的 `combo` 的相关信息，还能处理在某个时间点上获取到不正确的 `combo` 的异常情况。此外 RMVL 提供了默认 `tracker`，即 rm::DefaultTracker ，用于表示无需时间序列信息的一组 `combo`。

@ref group (**group**) —— 如果多个追踪器在物理空间上具有一定的相关性，比如共享轴旋转、刚性连接等属性，它们可以一起形成一个序列组 `group`，从而能够表示更加高级的物理信息。序列组使用 `std::vector<tracker::ptr>` 来存储这些追踪器。此外 RMVL 提供了默认 `group`，即 rm::DefaultGroup ，用于表示若干无相关性的一组 `tracker`，即退化成了 `std::vector<tracker::ptr>`。

#### 功能模块 {#function_modules}

@ref detector (**detector**) —— 识别、检测器是功能模块中最重要的部分，也是视觉图像处理的第一步。它负责对输入图像进行识别并加以处理，提取出目标轮廓、特征点等信息，并结合已知的部分数据组件，按顺序依次构建各种新的数据组件。识别得到的各种图像以及提取到的特征和组合体均保存至识别模块信息 `rm::DetectInfo` 中。

![图 1 装甲识别模块](extra/detector.jpg)

@ref compensator (**compensator**) —— 补偿器通常是功能模块中的第一个用于修正数据组件的算法模块，<span style="color: green">主要在 RoboMaster 赛事中使用</span>。补偿器主要负责修正弹道下坠的影响。计算得到的子弹飞行时间 `tof` 以及补偿增量 `compensation` 均保存至补偿模块信息 `rm::CompensateInfo` 中。

@ref predictor (**predictor**) —— 对于需要考虑目标追踪或存在较长通讯延迟的机器人而言，必须要引入目标预测环节，使得伺服机构能够恒定追踪、捕获目标，<span style="color: green">在 RoboMaster 赛事中一般用于保证弹丸能够准确击中敌方目标</span>。一般而言，每一个预测模块会针对特定的数据组件（派生的 `group` 对象）进行 2~3 种预测量类型的计算，包括动态响应预测量 `Kt`、静态响应预测量 `B`、射击延迟预测量 `Bs`，每种预测量类型都包含 9 种预测对象，每个预测对象之间对于 3 种预测量类型来说都是线性关系，例如在 @ref gyro_predictor 中对于 `ANG_Y` 预测对象的 3 种预测量都是单独计算的，这也便于后续的 **决策模块** 能够自由组合这些预测量。同样，计算得到的各类预测量均被保存至预测模块信息 `rm::PredictInfo` 中。
  
<center>

![图 2 能量机关预测模块](extra/predictor.png)

</center>

@ref decider (**decider**) —— 在经过识别、补偿和预测 3 个步骤后，需要使用特定的策略来获得当前时刻的最优目标，结合前三个步骤的信息（包含识别模块信息 `DetectInfo` 、补偿模块信息 `CompensateInfo` 、预测模块信息 `PredictInfo` ）导出到决策模块信息 `rm::DecideInfo` 中。

![图 3 整车状态决策模块](extra/decider.jpg)

该文档的后续章节描述了每个模块的功能。但首先，请确保了解 RMVL 的设计理念以及 API 概念。

---

### 设计理念 {#intro_design_principle}

#### 统一接口 {#intro_uniform_interface}

RMVL 中 **extra** 模块的每个功能都设计有一个抽象类和一系列派生类，以严格限制暴露的接口并实现接口的统一性。这种方法有助于顶层项目统一管理功能模块中特定的方法，顶层项目开发人员可以专注于模块之间的逻辑关系处理。例如，您可以使用同一接口实现不同的功能。

```cpp
#include <rmvl/detector.hpp>
/* ... */
std::vector<rm::detector::ptr> detectors;
detectors.emplace_back(rm::GyroDetector::make_detector(4));
detectors.emplace_back(rm::TagDetector::make_detector());
detectors.emplace_back(rm::RuneDetector::make_detector());

std::vector<rm::group::ptr> groups;
/* ... */
for (auto &p_detector : detectors) {
    auto info = p_detector->detect(groups, src, color, imu_data, tick);
    /* ... */
}
/* ... */
```

#### 低耦合逻辑 {#intro_low_coupled_logic}

RMVL 的 **extra** 模块中的各个功能模块之间的耦合度非常低，它们在代码上不会设置强依赖关系。例如，自动瞄准的功能可以通过以下代码实现，它们之间共享的数据组件是一个序列组列表：`groups`。

```cpp
#include <rmvl/detector.hpp>
#include <rmvl/compensator.hpp>
#include <rmvl/predictor.hpp>
#include <rmvl/decider.hpp>
/* ... */
rm::detector::ptr p_detector = rm::ArmorDetector::make_detector();
rm::compensator::ptr p_compensator = rm::GravityCompensator::make_compensator();
rm::predictor::ptr p_predictor = rm::ArmorPredictor::make_predictor();
rm::decider::ptr p_decider = rm::TranslationDecider::make_decider();
std::vector<rm::group::ptr> groups;
/* ... */
auto detect_info = p_detector->detect(groups, src, BLUE, rm::ImuData(), Timer::now());
auto compensate_info = p_compensator->compensate(groups, {imu_data.yaw, imu_data.pitch}, shoot_speed);
auto predict_info = p_predictor->predict(groups);
auto decide_info = p_decider->decide(groups);
/* ... */
```

例如，对一个数据组件进行识别的结果并不影响预测的逻辑。换句话说，如果要开发一个 RMVL 视觉功能，在对象识别过程中，异常和不合格的对象不应该被交给预测、补偿和决策模块进行后续处理，而应由识别检测模块本身解决。

---

### API 概念 {#api_concepts}

#### 命名空间 rm {#intro_namespace_rm}

在 RMVL 中所有的类和函数都放置在 `rm` 命名空间中。因此，在您的代码中访问这些功能时，请使用 `rm::` 限定符或 `using namespace rm;` 的指令

```cpp
#include <rmvl/camera.hpp>

rm::MvCamera mv(rm::CameraConfig{});
```

或使用

```cpp
#include <rmvl/camera.hpp>

using namespace rm;

MvCamera mv(CameraConfig{});
```

第二种方式引入了整个命名空间，容易造成访问冲突，在不影响可读性的情况下建议使用第一种方案。RMVL 中的嵌套命名空间较少，并且一般很少直接使用，不会对可读性造成很大的影响。

#### 参数管理 {#intro_parameters_manager}

不同的机器人具有不同的属性，它们对功能需求、运动响应和通信延迟都有不同的影响。RMVL 为所有这些参数提供了默认值，以确保正确的运行。但在实际部署时，但这些参数通常不是最有效的、最正确的，因此像机器人这样的顶层项目需要手动加载这些参数以最大化性能。

**定义**

参数规范文件本身由 `*.para` 文件所规定，包含了该参数的类型、标识符、默认值以及注释。这个文件在 CMake 运行期间会生成一系列对应的 `*.cpp` 和 `*.h (*.hpp)`，这个功能由一系列 **CMake** 语法定义，例如 `rmvl_generate_para`。

**加载**

RMVL 的所有参数文件都是通过 **YAML** 文件的读取和写入完成运行时的参数加载的，每个参数类中均提供了一致的接口 `read()` 和 `write()` 用于从 YAML 文件中读取、写入参数，可参考以下代码：

```cpp
#include <rmvlpara/combo/armor.h>
#include <rmvlpara/camera/camera.h>

/* code */

std::string prefix_str = "../etc/";

if (!rm::para::armor_param.read(prefix_str + "armor_param.yml"))
    printf("Failed to load the param: \"armor_param\".");
if (!rm::para::camera_param.read(prefix_str + "camera_param.yml"))
    printf("Failed to load the param: \"camera_param\".");
```

这段代码中，`armor` 和 `camera` 参数类的头文件被包含，并使用成员方法 `read()` 来加载参数。

#### 异常处理 {#intro_error_handle}

RMVL 使用异常 (exception) 来触发错误，当输入数据出现格式错误、范围错误、内存错误等异常情况，它会返回一个特殊的错误代码。

异常可以是 rm::Exception 的实例化对象，或者其派生对象。此外 rm::Exception 是 `std::exception` 的派生类，因此可以优雅的使用标准 C++ 库的各类组件来处理 RMVL 抛出的异常。

RMVL 中的异常通常是使用 `RMVL_Error(code, msg)` 宏或类似 `printf` 的 `RMVL_Error_(code, fmt, args...)` 变体进行触发的，或者可以使用 `RMVL_Assert(cond)` 宏检查条件并在不满足条件时触发异常。对于性能要求高的代码，有一个 `RMVL_DbgAssert(cond)` ，它只保留在 Debug 配置中。由于自动内存管理，所有中间缓冲区自动释放的情况下突然出现错误，您只需添加 `try` 语句来捕获异常。

```cpp
try {
    ... // 调用 RMVL 函数
} catch (const rm::Exception& e) {
    const char* err_msg = e.what();
    printf("exception caught: %s", err_msg);
}
```

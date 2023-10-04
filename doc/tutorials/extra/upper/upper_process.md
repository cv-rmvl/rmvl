责任链模式下的程序处理{#tutorial_extra_upper_process}
============

@author 赵曦
@date 2023/10/04

@prev_tutorial{tutorial_extra_upper_read_data}

@next_tutorial{tutorial_extra_upper_write_data}

@tableofcontents

------

### 1. 何为责任链模式

#### 1.1 常规用法 {#process_common}

[责任链模式](https://refactoring.guru/design-patterns/chain-of-responsibility)（Chain of Responsibility Pattern）是一种行为型设计模式，其目的是将请求的发送者和接收者解耦，并使多个对象都有机会处理该请求。在责任链模式中，请求沿着一个链路传递，直到有一个对象能够处理它为止。

在该模式中，通常有一个抽象的处理器（Handler）作为基类，定义了处理请求的接口和链中的下一个处理器的引用。每个具体处理器（Concrete Handler）继承自抽象处理器，实现了处理请求的方法。具体处理器决定是否处理请求，如果可以处理，则进行处理；如果不能处理，则将请求传递给链中的下一个处理器。

以下是责任链模式的关键角色：

- Handler（处理器）：定义了处理请求的接口和链中的下一个处理器的引用。
- ConcreteHandler（具体处理器）：继承自Handler，实现了处理请求的方法，决定是否处理请求，如果可以处理则进行处理，否则将请求传递给下一个处理器。

应用责任链模式的主要优点是：

- 降低了发送者和接收者之间的耦合，发送者无需知道哪个具体处理器能够处理请求，只需要将请求发送给第一个处理器即可。
- 可以动态调整处理链的顺序或新增、删除处理器，灵活性较高。

适用于以下情况：

- 当有多个对象可以处理一个请求，但具体哪个对象将处理请求在运行时才能确定。
- 需要将请求的发送者和接收者解耦，避免直接关联。

**示例**

假设有一个问题反馈系统，用户可以提交问题，并由多个处理器来处理这些问题。首先，定义一个抽象的处理器（Handler）类：

```cpp

#include <iostream>

class Handler
{
protected:
    Handler* _next;

public:
    Handler() : _next(nullptr) {}

    inline void setNext(Handler* next) { _next = next; }

    virtual void handleRequest(const std::string& request) = 0;
};
```

然后，创建两个具体的处理器 `ConcreteHandler`：`BugHandler` 和 `FeatureHandler`。

- `BugHandler` 用于处理与软件缺陷相关的问题
- `FeatureHandler` 用于处理用户新功能请求

```cpp
class BugHandler : public Handler
{
public:
    void handleRequest(const std::string& request) override
    {
        if (request == "Bug")
            std::cout << "BugHandler: Handling the bug report." << std::endl;
        else if (_next != nullptr)
            _next->handleRequest(request);
    }
};

class FeatureHandler : public Handler
{
public:
    void handleRequest(const std::string& request) override
    {
        if (request == "Feature")
            std::cout << "FeatureHandler: Handling the feature request." << std::endl;
        else if (_next != nullptr)
            _next->handleRequest(request);
    }
};
```

现在可以创建责任链并测试它。在下面的示例中，首先将 `BugHandler` 设置为责任链的第一个处理器，然后将 `FeatureHandler` 作为下一个处理器。

```cpp
int main()
{
    Handler* bugHandler = new BugHandler();
    Handler* featureHandler = new FeatureHandler();

    bugHandler->setNext(featureHandler);

    bugHandler->handleRequest("Bug");
    bugHandler->handleRequest("Feature");
    bugHandler->handleRequest("Enhancement");
  
    delete bugHandler;
    delete featureHandler;

    return 0;
}
```

输出结果将是：

```
BugHandler: Handling the bug report.
FeatureHandler: Handling the feature request.
```

在这个例子中，如果请求是 `"Bug"`，则 `BugHandler` 将处理它；如果请求是 `"Feature"`，则 `FeatureHandler` 将处理它；如果请求是其他类型（例如 `"Enhancement"`），则请求将传递给下一个处理器。这样，责任链模式允许多个对象有机会处理请求，并且可以动态调整节点顺序或新增、删除节点。

#### 1.2 RMVL 修改后的用法 {#process_in_rmvl}

@ref process_common 中要定义指向下一个处理器的抽象指针，并且所有派生处理器均继承自一个基类，RMVL 功能模块不采用这种做法，因为

- 可能需要运行多个功能模块，并且功能模块之间可能存在各种各样的组合
- 每个功能模块负责的内容以及依赖的目标各不相同

因此，常规用法中的<span style="color: red">请求</span>在 RMVL 功能模块中表示为<span style="color: red">数据组件</span>，各功能模块的返回值 `XxxInfo` 则可以作为下一个功能模块的入参。可以继续参考此流程图

![upper-base](upper_base.png)

程序处理III（预测）的入参包含了补偿模块的返回值信息 `rm::CompensateInfo`，程序处理IV（决策）的入参包含了前 3 个模块的返回值 `rm::DetectInfo`、`rm::CompensateInfo`、`rm::PredictInfo`。

### 2. 具体使用方法

下面给出一个简单的例子

```cpp
bool run()
{
    /* 读取数据 raw_data */

    /* 提取通信得到的控制信息 */
    auto flag = raw_data.flag;
    auto color = raw_data.color;

    /* 转换数据 */
    const auto &[detect_flag, compensate_flag, predict_flag, decide_flag, camera_flag] = flag_map[flag];

    // 视觉处理，这里采用 LUT 方式控制逻辑分支
    cv::Mat src;
    if (!capture_map[camera_flag]->read(src))
    {
        // 不能正常打开相机
        WARNING_("相机打开异常");
        return false;
    }

    // 程序处理 I: 识别
    DetectInfo detect_info{};
    try
    {
        detect_info = detector_map[detect_flag]->detect(groups, src, color, data, getTickCount());
    }
    catch (const rm::Exception &e)
    {
        ERROR_("Occurred an exception! %s", e.err.c_str());
        groups.clear();
        /* 发送默认数据 */
        return false;
    }

    // 程序处理 II: 补偿
    auto compensate_info = compensator_map[compensate_flag]->compensate(groups, shoot_speed, CompensateType::UNKNOWN);
    // 预测
    auto predict_info = predictor_map[predict_flag]->predict(groups, compensate_info.tof);
    // 决策
    auto decide_info = decider_map[decide_flag]->decide(groups, detect_info, compensate_info, predict_info);

    /* 导出数据 */

    return true;
}
```

**注意**

- 模块内部均设置了异常抛出的功能，当传入了错误的数据会抛出相应的异常，可参考 @ref RMVLErrorCode 查看异常的类型。顶层模块需要妥善处理这些异常，例如使用 `try-catch` 语句来捕获异常，设置默认处理或者直接退出程序；
- **发送默认数据** 与 **导出数据** 将在下一篇说明文档中介绍。

------

在完成程序处理后，需要完成数据的导出功能，包括

- 记录数据到文件
- 显示图像、打印信息到屏幕
- 发送控制指令到下位机

等需求，请参阅 @ref tutorial_extra_upper_write_data 一文。

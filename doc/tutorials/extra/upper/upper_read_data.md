读取（默认）数据以控制逻辑分支{#tutorial_extra_upper_read_data}
============

@author 赵曦
@date 2023/10/04

@prev_tutorial{tutorial_extra_upper_init}

@next_tutorial{tutorial_extra_upper_process}

@tableofcontents

------

### 1 前言

有时程序需要同时具备很多逻辑功能，这些功能需要在运行时根据用户或者其他客户端下发的指令进行切换。本教程给出了简单的逻辑控制以及功能切换的方式。

### 2 逻辑控制

**switch-case**

最简单的方式是使用 `switch-case` 语句，在每一次得到与上一次不同的信号后直接选择对应的功能模块重新加载，这种清空能保证每个功能模块在每次运行的时候只有一个子模块生效。

**LUT**

如果想同时加载并使用多个子模块，最优雅的方式是使用查找表 —— LUT (Look-Up Table) ，维护一个映射表（一般是散列表）即可完成功能切换、参数加载。

下文均介绍 **LUT** 法的配置。

#### 2.1 映射配置

```cpp
struct ControlMode
{
    std::string detector_flag;    // 识别模块模式
    std::string compensator_flag; // 补偿模块模式
    std::string predictor_flag;   // 预测模块模式
    std::string decider_flag;     // 决策模块模式

    /* code */
};

// 控制信号映射表
std::unordered_map<uint8_t, ControlMode> flag_map;
// 识别模块映射表
std::unordered_map<std::string, rm::detector::ptr> detector_map;

/********** 下面的 xxxInit() 为初始化代码，均可置于构造函数中 **********/

// flag_map 初始化
void controlInit()
{
    flag_map[0] = {"armor_detector",
                   "gravity_compensator",
                   "planar_predictor",
                   "translation_decider"};
    flag_map[1] = {"rune_detector",
                   "gravity_compensator",
                   "spi_rune_predictor",
                   "rune_decider"};
    flag_map[2] = {"gyro_detector",
                   "gyro_compensator",
                   "gyro_predictor",
                   "gyro_decider"};
}
// detector_map 初始化
void detectorInit()
{
    detector_map["armor_detector"] = rm::ArmorDetector::make_detector();
    detector_map["rune_detector"] = rm::RuneDetector::make_detector();
    detector_map["gyro_detector"] = rm::GyroDetector::make_detector();
    detector_map["tag_detector"] = rm::TagDetector::make_detector();
}
```

#### 2.2 注意事项

##### 要点 1 {#map_tip1}

上面示例中 `flag_map` 的初始化是固定的值，因此要修改信息必须修改源码，因此可以使用运行时读取 YAML 文件的方式，使用 OpenCV 的 [cv::FileStorage](https://docs.opencv.org/4.x/da/d56/classcv_1_1FileStorage.html) 可以完成 YAML 文件的读取，例如

```cpp
/* main.cpp */

cv::FileStorage fs("flag_config.yml", cv::FileStorage::READ);
auto root = fs.root();
for (auto it : root)
{
    ControlMode val;
    int id{};
    it["id"] >> id;
    it["detector_flag"] >> val.detector_flag;
    it["compensator_flag"] >> val.compensator_flag;
    it["predictor_flag"] >> val.predictor_flag;
    it["decider_flag"] >> val.decider_flag;
    flag_map[static_cast<uint8_t>(id)] = val;
}
```

对应的 `flag_config.yml` 文件如下

```yaml
%YAML:1.0
---

- id: 0
  detector_flag: "armor_detector"
  compensator_flag: "gravity_compensator"
  predictor_flag: "planar_predictor"
  decider_flag: "translation_decider"
- id: 1
  detector_flag: "rune_detector"
  compensator_flag: "gravity_compensator"
  predictor_flag: "spi_rune_predictor"
  decider_flag: "rune_decider"
- id: 2
  detector_flag: "gyro_detector"
  compensator_flag: "gyro_compensator"
  predictor_flag: "gyro_predictor"
  decider_flag: "gyro_decider"
```

##### 要点 2 {#map_tip2}

还可以在 `ControlMode` 中添加其他有用的模式信息，例如可以添加有关感知设备的信息

```cpp
std::string camera_mode; // 相机模式
std::string light_mode;  // 光源模式
```

同样，该信息也可以添加到 @ref map_tip1 的 YAML 文件中。

##### 要点 3 {#map_tip3}

上面的例子使用 `std::string` 作为功能模块散列表的 Key 值，在不影响可读性的情况下也可以使用 `uint8_t` 或 `enum (enum class)`，在 cppreference 的 [std::hash](https://zh.cppreference.com/w/cpp/utility/hash) 中提到标准库对所有（有作用域或无作用域）枚举类型提供了特化，因此可以采用枚举类型作为散列表的 Key，例如

```cpp
enum class DetectorFlag
{
    ARMOR_DETECTOR,
    RUNE_DETECTOR,
    GYRO_DETECTOR
};

struct ControlMode
{
    DetectorFlag detector_flag; // 识别模块模式

    /* code */
};

/* code */

// 识别模块映射表
std::unordered_map<DetectorFlag, rm::detector::ptr> detector_map;
```

若在这种方法下，配合 @ref map_tip1 的 YAML 文件使用，则无法在 YAML 文件中直观的看出对应的模式，因此要用到 YAML 文件进行模式加载的情况下，使用 `std::string` 代替枚举类型或整型是个不错的选择。

#### 2.3 部署使用

获取模式字符串（若采用 @ref map_tip3 的方式则是模式枚举）可直接使用[结构化绑定](https://zh.cppreference.com/w/cpp/language/structured_binding)进行获取。

```cpp
/* 先前已获得（更新）的模式信息 flag */
const auto &[detect_str, compensate_str, predict_str, decide_str] = flag_map[flag];
```

在使用上直接访问对应的映射表即可，由于所有功能模块提供的 `ptr` 别名均代表其自身的非共享指针，若需要使用形如以下代码的表述，那么程序非良构或导致所有权转移，具体细节可参考[非共享指针 std::unique_ptr](https://zh.cppreference.com/w/cpp/memory/unique_ptr)手册。

```cpp
/* 使用上文定义的 detect_str */
auto p1 = detector_map[detect_str];            // 复制构造被弃置，非良构
auto p2 = std::move(detector_map[detect_str]); // 语法正确，但映射表原先识别模块的指针被置空
```

下面给出两种访问时的做法

##### 方法 1

直接使用，但后续每次访问功能模块指针的时候都需要在散列表 `detector_map` 中寻找。

```cpp
/* 使用上文定义的 detect_str */
auto detect_info = detector_map[detect_str]->detect(/* code */);
/* code */
```

##### 方法 2

定义指针常量，以保证指针指向不会被修改，这种做法可操作性更强。

```cpp
rm::detector *const p_detector = detector_map[detect_str].get();
auto detect_info = p_detector->detect(/* code */);
/* code */
```

### 3 功能切换方式

#### 3.1 轮询判断

这是最常用并且最方便的方法，这种做法适合于不断循环执行处理的场合下。在每一次循环的程序处理结束后，在下一次循环开始时，先接收通信传输得到的控制信号，并与上一次得到的信号做判断，若不相同，则清空所有数据组件。若在 **逻辑控制** 中选择了 **switch-case** 的方式，那么在清空数据组件后可直接为相应的功能模块重新初始化，若选择了 **LUT** 的方式，则无需重新初始化，直接根据新的控制信号指定功能模块即可。

#### 3.2 外部中断、回调函数

还有一种做法是利用外部中断或者回调函数（软件中断）的方式完成功能的切换，以 Termios 串口通信的结构体为例，在设置 `c_iflag` 的时候，可以打开 `BRKINT` 功能，即接收到 `BREAK` 信号时产生中断信号。

```cpp
xxx.c_iflag |= BRKINT;
```

然而，RMVL 提供的串口通信库不支持此操作，因此该方法可以适用于<span style="color: red">其他通信方式</span>或者其他不同于 rm::SerialPort 的自定义协议的串口通信。

这里的其他通信方式有其他类型，比如当客户端发起某个请求时，服务器会进入某个回调函数，因此可以在回调函数中完成功能模块的初始化设置或者切换。例如

- ROS / ROS2 中可注册回调函数，在 `ros::spin()` 或 `rclcpp::spin()` 调用后可阻塞当前线程，在有消息到达订阅节点时，会自动触发回调函数的执行。
- OPC UA 中在服务器中注册了一个 Method Node 即方法节点，在客户端中 `call` 该方法节点，则服务器会处理该方法节点的回调函数。

### 4 合理使用默认数据

有时会遇到某次通信收到数据异常的情况，或根本没有通信，在这种情况下可以为收到的信息（这里以控制信息 `uint8_t flag;` 来表示）使用默认数据。

```cpp
uint8_t read()
{
    if (!is_init)
    {
        flag = 0; // 用 0 为 flag 赋值，作为 flag 的默认值
                  // 此处默认值可参考 rm::para 使用参数进行赋值
        /* code */
        last_flag = flag;
        is_init = true;
    }

    /* 读取数据 raw_datas */
    bool read_success = /* 形如 read(raw_datas) 的代码 */;
    if (read_success)
    {
        flag = raw_datas.flag; // 从通信协议结构体中获取
        last_flag = flag;
    }
    return last_flag;
}
```

上面代码实现了默认值的设置，在初次读取数据异常时将启用该默认值，在某次读取数据异常时将采用上次数据。

此外，还可以加入

- 数据校验
- 读取失败次数过多断言失败或抛出异常（即使用 `assert` 或 `RMVL_Error` 宏）

等功能。

------

本文主要介绍了读取传入数据或默认数据来控制逻辑分支的方法，对于具体分支中程序处理的部分请参见 @ref tutorial_extra_upper_process 。

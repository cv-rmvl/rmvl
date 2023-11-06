硬件设备与软件模块的初始化{#tutorial_extra_upper_init}
============

@author 赵曦
@date 2023/10/02

@prev_tutorial{tutorial_extra_upper_base}

@next_tutorial{tutorial_extra_upper_read_data}

@tableofcontents

------

### 概述

软件模块一般在使用时仅涉及到 4 个功能模块，即 @ref detector ， @ref compensator ， @ref predictor ，和 @ref decider 。此外，用户仍然可以基于 RMVL 提供的抽象类数据组件（如 rm::feature ）完成适用于个人项目中派生数据组件的接口设计，在这种情况下，初始化的内容可能需要考虑到这些数据组件，在本文中不多介绍。

而硬件设备主要有感知设备和通信设备，目前 RMVL 包含的感知设备涉及到

@add_checkbox_y
相机设备
@end_checkbox
@add_checkbox_y
光源控制器
@end_checkbox
@add_checkbox_n
激光雷达
@end_checkbox

通信设备涉及到

@add_checkbox_y
数据链路层
@end_checkbox
@add_checkbox_n
运输层/应用层
@end_checkbox

### 软件模块 {#init_software}

实际上，各个功能模块在运行时初始化的主要依据正是通信传输的控制信息，这里仅介绍功能模块在运行时初始化的最一般写法。

4 个功能模块均提供了对应的 `make_xxx` 静态工厂函数，可直接使用此函数完成初始化操作，在 @ref api_concepts 的 **低耦合逻辑** 中涉及到了以下写法，其中最后一个序列组列表则是贯穿于所有功能模块的数据组件集合。

```cpp
// 创建 detector
rm::detector::ptr p_detector = rm::ArmorDetector::make_detector();
// 创建 compensator
rm::compensator::ptr p_compensator = rm::GravityCompensator::make_compensator();
// 创建 predictor
rm::predictor::ptr p_predictor = rm::ArmorPredictor::make_predictor();
// 创建 decider
rm::decider::ptr p_decider = rm::TranslationDecider::make_decider();
// 默认初始化 group 列表
std::vector<rm::group::ptr> groups;
```

此外，要注意某些静态工厂函数<span style="color: red">可能需要传入参数</span>，这些参数一般就是对应功能类的构造函数的入参。具体的功能模块在使用上均可参考该类对应的说明文档。

### 感知设备 {#init_perception}

@add_toggle{相机设备}

相机模块可参考手册 @ref tutorial_modules_camera ，此处不多赘述，以 rm::HikCamera 为例，直接使用

```cpp
// 创建并初始化相机
auto p_capture = rm::HikCamera::make_capture(rm::GRAB_CONTINUOUS, rm::RETRIEVE_CV);
```

即可完成相机的初始化。

@end_toggle

@add_toggle{光源控制器}

@todo
本小节暂无

@end_toggle

@add_toggle{激光雷达}

@todo
本小节暂无

@end_toggle

### 通信设备 {#init_communication}

@add_toggle{数据链路层}

串行接口通信，使用方法见 @ref tutorial_modules_serial

**示例**

```cpp
/* code */

#pragma pack(1)
// 发送协议
struct SendStruct
{
    uint8_t test;
    /* code */
};
#pragma pack()

// 接收协议
struct ReceiveStruct
{
    uint8_t test;
    /* code */
} __attribute__((packed));

/* code */

int main()
{
    // 创建并初始化串口通信
    auto port = rm::SerialPort("ttyACM1", B115200);

    /* code */
}

/* code */
```

@note
- 初始化硬件设备同时要定义好串口通信协议
- 串口通信协议可参考 @ref serialport_protocol 小节

@end_toggle

@add_toggle{运输层/应用层}

@todo
本小节暂无

@end_toggle

------

涉及到运行时切换模块的功能，可以对上述 @ref init_software 中的写法进行拓展，请继续阅读下一篇 @ref tutorial_extra_upper_read_data 以了解数据信息是如何控制程序分支的。

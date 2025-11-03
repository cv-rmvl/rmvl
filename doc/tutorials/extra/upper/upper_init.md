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

而硬件设备主要有感知设备 SDK 和通信设备相关层级支持（及相关通信中间件），目前 RMVL 包含的感知设备 SDK 涉及到

@add_checkbox_y
相机设备
@end_checkbox
@add_checkbox_y
光源控制器
@end_checkbox
@add_checkbox_n
激光雷达
@end_checkbox

通信设备相关层级支持涉及到

@add_checkbox_y
进程间通信
@end_checkbox
@add_checkbox_y
串口通信
@end_checkbox
@add_checkbox_y
以 Socket 为核心的传输层
@end_checkbox
@add_checkbox_y
以 HTTP 为核心的应用层及后端框架
@end_checkbox

通信中间件涉及到

@add_checkbox_y
OPC UA
@end_checkbox
@add_checkbox_n
MQTT
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

<div class="tabbed">

- <b class="tab-title">相机设备</b>

  相机模块可参考手册 @ref tutorial_modules_camera ，此处不多赘述，以 rm::HikCamera 为例，直接使用

  ```cpp
  // 创建并初始化相机
  auto p_capture = rm::HikCamera::make_capture(rm::CameraConfig::create(rm::GrabMode::Continuous, rm::RetrieveMode::OpenCV));
  ```

  即可完成相机的初始化。

- <b class="tab-title">光源控制器</b>

  @ref tutorial_modules_light 一文中指引了有关 SDK 的安装，可参考其相关类完成开发工作。

- <b class="tab-title">激光雷达</b>

  @todo
  本小节暂无

</div>

### 通信设备 {#init_communication} 

<div class="tabbed">

- <b class="tab-title">串口通信</b>

  串行接口通信，使用方法见 @ref tutorial_modules_serial

  **示例**

  ```cpp
  /* code */

  #pragma pack(1)
  // 发送协议
  struct SendStruct {
      uint8_t test;
      /* code */
  };
  #pragma pack()

  /*
      不使用 #pragma pack 预处理器宏实现取消内存对齐，也可以使用
      C++11 提供的关键字 alignas，例如下面的接收协议
  */

  // 接收协议
  struct alignas(1) ReceiveStruct {
      uint8_t test;
      /* code */
  };

  /* code */

  int main() {
      // 创建并初始化串口通信
      auto port = rm::SerialPort("/dev/ttyACM0", BaudRate::BR_115200, SerialReadMode::NONBLOCK);

      /* code */
  }

  /* code */
  ```

  @note
  - 取消内存对齐可参考 [alignas](https://zh.cppreference.com/w/cpp/language/alignas) 和 [pragma pack](https://zh.cppreference.com/w/cpp/preprocessor/impl#.23pragma_pack)
  - 初始化硬件设备同时要定义好串口通信协议
  - 串口通信协议可参考 @ref serial_protocol 小节

- <b class="tab-title">其余层级支持</b>

  - 进程间通信，使用方法见 @ref tutorial_modules_ipc
    - 跨平台的同步支持包括 rm::PipeServer 和 rm::PipeClient ，对应的异步协程支持包括 rm::async::PipeServer 和 rm::async::PipeClient
    - Linux 下基于 Unix FIFO 实现的 rm::MqServer 和 rm::MqClient
  - 以 Socket 为核心的传输层支持，使用方法见 @ref tutorial_modules_socket
  - 以 HTTP 为核心的应用层及后端框架支持，使用方法见 @ref tutorial_modules_netapp
    - 基础工具包括 rm::Request 和 rm::Response
    - 同步请求工具包括 rm::requests::get 和 rm::requests::post 等，对应的异步请求工具包括 rm::async::requests::get 和 rm::async::requests::post 等
    - 后端框架包括 rm::async::Webapp

- <b class="tab-title">通信中间件</b>

  - OPC UA，使用方法见 @ref tutorial_modules_opcua
  - MQTT，使用方法见 @ref tutorial_modules_mqtt

</div>

------

涉及到运行时切换模块的功能，可以对上述 @ref init_software 中的写法进行拓展，请继续阅读下一篇 @ref tutorial_extra_upper_read_data 以了解数据信息是如何控制程序分支的。

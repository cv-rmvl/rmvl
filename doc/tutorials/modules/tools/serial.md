串口通信模块 {#tutorial_modules_serial}
============

@author 赵曦
@date 2025/09/25
@version 2.0
@brief 串口通信模块使用教程

@prev_tutorial{tutorial_modules_ipc}

@next_tutorial{tutorial_modules_socket}

@tableofcontents

------

相关类 rm::SerialPort 及 rm::async::SerialPort

### 1. 简介

串口通信是一种通过串行数据传输进行通信的方式。它使用串行接口将数据以逐位的方式进行传输，常用于连接计算机与外部设备、嵌入式系统之间的数据传输。

串口通信一般涉及两个主要概念： **串口** 和 **波特率** 。

- 串口：指的是通信中的物理接口，常见的有 `RS-232`、`RS-485`、`UART` 等标准。每个串口都有相应的引脚用于发送和接收数据以及控制信号。
- 波特率：也称作数据传输速率，指的是每秒钟传输的位数。波特率决定了传输速度的快慢，通常使用常见的波特率如 9600、115200 等。

### 2. 同步模式使用

#### 2.1 初始化

rm::SerialPort 的构造函数原型如下

```cpp
SerialPort(std::string_view device, BaudRate baud_rate, SerialReadMode read_mode = {});
```

<div class="full_width_table">

|    参数     |                                   含义                                    |
| :---------: | :-----------------------------------------------------------------------: |
|  `device`   | 设备名，在 Windows 上一般是 `COMx`，Linux 上一般为 `ttyUSBx` 或 `ttyACMx` |
| `baud_rate` |                                  波特率                                   |
| `read_mode` |      读取模式，表示阻塞与非阻塞， rm::async::SerialPort 不提供该参数      |

</div>

#### 2.2 数据 I/O

rm::SerialPort 提供了极其方便的串口读取、写入的接口。

###### 读取函数原型

```cpp
template <typename AggregateT>
bool read(uint8_t head, uint8_t tail, AggregateT &data); // (1) 指定头尾帧的读取结构化数据
```

```cpp
template <typename AggregateT>
bool read(AggregateT &data);                             // (2) 读取结构化数据
```

```cpp
bool read(std::string &data);                            // (3) 读取字符串数据
```

```cpp
template <typename AggregateOrStringT>
SerialPort &operator>>(AggregateOrStringT &data);        // (4) 串口读取操作符重载
```

###### 写入函数原型

```cpp
template <typename AggregateT>
bool write(const AggregateT &data);                      // (5) 写入结构化数据
```

```cpp
bool write(std::string_view data);                       // (6) 写入字符串数据
```

```cpp
template <typename AggregateOrStringT>
SerialPort &operator<<(const AggregateOrStringT &data);  // (7) 串口写入操作符重载
```

@param data 读取、写入的数据
@param head 帧头
@param tail 帧尾

**注解**

<div style="margin-left: 40px;">(3) 和 (5) 带有 Python 接口</div>

#### 2.3 链路层协议 {#serial_protocol}

**写数据（视觉端 → 电控端）**

SerialPort 的通信协议一般就是指数据链路层的协议，采用封装成帧的方式，由于终端设备输出缓冲区可在写入数据前被清空，因此 rm::SerialPort 的 `write` 方法不设置任何帧头帧尾。

**读数据（电控端 → 视觉端）**

而输入缓冲区则会持续收到来自下位机的数据，因此 rm::SerialPort 的 `read` 方法提供了两个 `uint8_t` 类型帧头帧尾的传入参数，下位机发送的数据需要在有效数据包前后设置两个 `uint8_t` 的校验位，帧头和帧尾具体的数据可自行设置。

需要注意的是，在读取数据前需要留意

- 协议（结构体）内容
- 内存对齐规则，关于内存对齐可使用 `#pragma pack` 宏或使用 C++ 标准的 `alignas` 关键字
- 头尾帧是否一致

#### 2.4 用法

```cpp
#include <rmvl/io/serial.hpp>

using namespace rm;

struct Data {
    uint8_t head[4]{};
    float x{};
    float y{};
};

int main() {
    // 创建串口对象，以非阻塞模式打开 /dev/ttyUSB0，设置波特率为 115200
    auto serial = SerialPort("/dev/ttyUSB0", BaudRate::BR_115200, SerialReadMode::NONBLOCK);

    // 1. 字符串读写
    std::string str{};
    bool success = serial.read(str);
    success = serial.write("test string");

    // 2. 聚合体读写
    Data data{};
    success = serial.read(data);
    Data send_data = {1, 2, 3, 4, 1.1f, 2.2f};
    success = serial.write(send_data);

    // 3. 带帧头帧尾的聚合体读取
    // 适合于同步非阻塞模式下大量数据的读取处理
    success = serial.read(0x44, 0x55, data);
}
```

### 3. 异步模式使用

rm::async::SerialPort 需要配合 rm::async::IOContext 使用，目前，异步串口仅提供了字符串读写的功能

```cpp
SerialReadAwaiter read();

SerialWriteAwaiter write(std::string_view);
```

其中 SerialReadAwaiter 为串口读等待器，相比于 rm::async::AsyncReadAwaiter 增加了读缓冲区清理的功能。同理，SerialWriteAwaiter 增加了写缓冲区清理的功能。用法如下：

```cpp
#include <rmvl/io/serial.hpp>

using namespace rm;

async::Task<> session(async::SerialPort serial) {
    // 仅支持字符串读写
    std::string str = co_await serial.read();
    bool success = co_await serial.write("test string");
}

int main() {
    async::IOContext io_context{};
    async::SerialPort serial(io_context, "/dev/ttyUSB0", BaudRate::BR_115200);

    co_spawn(io_context, session, std::move(serial));

    io_context.run();
}
```

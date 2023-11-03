串口通信模块 {#tutorial_modules_serial}
============

@author 赵曦
@date 2023/06/22

@prev_tutorial{tutorial_modules_aggregate_reflect}

@next_tutorial{tutorial_modules_kalman}

@tableofcontents

------

相关类 rm::SerialPort

### 简介

串口通信是一种通过串行数据传输进行通信的方式。它使用串行接口将数据以逐位的方式进行传输，常用于连接计算机与外部设备、嵌入式系统之间的数据传输。

串口通信一般涉及两个主要概念： **串口** 和 **波特率** 。

- 串口：指的是通信中的物理接口，常见的有`RS-232`、`RS-485`、`UART`等标准。每个串口都有相应的引脚用于发送和接收数据以及控制信号。
- 波特率：也称作数据传输速率，指的是每秒钟传输的位数。波特率决定了传输速度的快慢，通常使用常见的波特率如9600、115200等。

在 [Unix](https://unix.org/) 中，Termios 是一个用于串口通信的 API ，它提供了对串口终端设备进行配置和控制的功能。

Termios 接口允许开发人员通过编程来设置串口终端的各种参数，如波特率、数据位数、校验位、流控制等。通过使用 Termios 接口，可以实现对串口通信的灵活控制，以满足各种应用需求。

主要的 Termios 函数包括

|              函数              |                           意义                            |
| :----------------------------: | :-------------------------------------------------------: |
|         `tcgetattr()`          | 获取当前的终端属性（设置），将属性保存到 Termios 结构体中 |
|         `tcsetattr()`          |  设置终端属性，通过提供 Termios 结构体中的属性值进行设置  |
| `cfsetispeed(), cfsetospeed()` |                设置串口的输入和输出波特率                 |
|          `tcflush()`           |                   刷新输入或输出缓冲区                    |
|          `tcdrain()`           |             等待所有输出数据完成发送后再返回              |
|           `tcflow()`           |                       控制数据流向                        |

### 使用{#serialport_how_to_use}

#### 初始化{#serialport_init}

rm::SerialPort 的构造函数原型如下

```cpp
explicit SerialPort(const std::string &device = {}, int baud_rate = B115200);
```

|    参数     |                             含义                             |
| :---------: | :----------------------------------------------------------: |
|  `device`   | 设备名，如果为空，则会自动搜寻名为 `ttyUSBx` 或 `ttyACMx` 的串口文件，如果不为空，则直接打开指定的设备 |
| `baud_rate` | 波特率，默认为 `B115200`，所有可用的波特率都被标准化为 `B` 开头的宏定义 |

#### 数据 I/O {#serialport_io}

rm::SerialPort 提供了极其方便的串口读取、写入的接口。

| 功能 |  函数   |                        功能                        |
| :--: | :-----: | :------------------------------------------------: |
| 读取 | `read`  | 指定头尾帧，读取缓冲区中最新的能够组成结构体的数据 |
| 写入 | `write` |          传入待写入的结构体完成数据的写入          |

- 读取函数原型
  ```cpp
  template <typename Tp>
  inline bool read(unsigned char head_flag, unsigned char tail_flag, Tp &data)
  ```
- 写入函数原型
  ```cpp
  template <typename Tp>
  inline bool write(Tp &data_struct);
  ```

#### 链路层协议 {#serialport_protocol}

SerialPort 的通信协议一般就是指数据链路层的协议，采用封装成帧的方式，由于终端设备输出缓冲区可在写入数据前被清空，因此 rm::SerialPort 不设置任何帧头帧尾，而输入缓冲区则会持续收到来自下位机的数据，因此 rm::SerialPort 的 `read` 方法提供了帧头帧尾的传入参数。

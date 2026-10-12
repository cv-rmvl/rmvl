Linux 外设 IO {#tutorial_modules_peripheral}
=============

@prev_tutorial{tutorial_modules_serial}

@next_tutorial{tutorial_modules_socket}

RMVL 的 @ref io_peripheral 提供 Linux 应用层 GPIO、I²C 和 SocketCAN 接口，不包含内核驱动、设备树配置或外设寄存器协议。接口仅在 Linux 可用，同步接口要求 C++17，`rm::async` 协程接口要求 C++20。

## 1 执行模式与错误处理

| 操作 | 同步阻塞 | 同步非阻塞 | IOContext 协程 |
| --- | --- | --- | --- |
| GPIO 电平 `read()/write()` | ioctl | 不提供 | 不提供 |
| GPIO 边沿 `wait()` | 等待事件 | 无事件返回 `std::nullopt` | epoll 等待事件 |
| I²C `transfer()` | I2C_RDWR 组合事务 | 不提供 | 不提供 |
| CAN `read()/write()` | 等待可读或可写 | 无数据返回 `std::nullopt`，暂不可写返回 `false` | epoll 等待可读或可写 |

`rm::GpioReadMode` 控制 GPIO 边沿等待，`rm::CanReadMode` 控制 CAN 收发，均提供 `BLOCK` 和 `NONBLOCK` 两种模式。GPIO 电平 ioctl 和 I²C 事务可能在驱动中阻塞，不提供对应的非阻塞模式。

参数错误抛出 `std::invalid_argument`，系统调用错误抛出 `std::system_error`。移动后的对象不再持有设备。设备打开、线路申请和配置仍然是同步操作，建议在进入事件循环前完成。

异步接口只使用设备原生的非阻塞 IO 与 epoll 就绪通知，不使用工作线程调度。GPIO 电平 ioctl 和 I²C 事务仅提供同步阻塞接口，请勿在需要保持响应的 `IOContext` 事件循环中调用。同一 I²C 对象的事务串行执行，多个调用的先后顺序不保证，驱动超时由内核/适配器决定。

异步对象及其 `IOContext` 必须存活到任务结束，同一 GPIO 请求只能有一个边沿读取者，同一 CAN socket 不应有多个并发读任务或多个并发写任务。CAN 可以同时等待读和写。

## 2 GPIO

rm::Gpio 使用 GPIO 字符设备 v2 ABI，要求运行内核支持该 ABI（Linux 5.10 起）。构建环境的 Linux 头文件过旧时，构造请求会报 `EOPNOTSUPP`。`offsets` 是 `/dev/gpiochipN` 内的线路偏移，不是板上排针编号。

```cpp
#include <rmvl/io/gpio.hpp>

rm::GpioOptions output;
output.direction = rm::GpioDirection::Output;
output.initial_values = 1;
rm::Gpio led("/dev/gpiochip0", {17}, output);
led.write(0);
```

同一请求可申请 1 至 64 条线路，电平位图的 bit i 对应 `offsets[i]`，不是 GPIO 的偏移值。线路是独占申请的，已被内核驱动或其他进程使用的线路不能再次申请。上下拉、消抖等能力取决于驱动，不支持时保留内核错误。

```cpp
rm::GpioOptions input;
input.edge = rm::GpioEdge::Both;
rm::Gpio button("/dev/gpiochip0", {18}, input, rm::GpioReadMode::NONBLOCK);
if (auto event = button.wait()) {
    // event->offset 为 18，timestamp_ns 使用 CLOCK_MONOTONIC
}
```

`read()` 是即时取样，不等待电平变化。`wait()` 才是边沿事件等待，必须在构造时启用边沿检测。启用 `active_low` 后，电平和边沿均为逻辑值，例如逻辑上升沿对应物理下降沿。事件序号可用于识别事件丢失，时间戳不能直接当作系统日历时间。

## 3 I²C

rm::I2c 使用 `/dev/i2c-N` 和 `I2C_RDWR`，适配器须支持 `I2C_FUNC_I2C`。每个 rm::I2cMessage 指定不含读写位的从机地址、方向、地址宽度和数据，读消息需要预先确定缓冲区长度。

```cpp
#include <rmvl/io/i2c.hpp>

rm::I2c bus("/dev/i2c-1");
// 下面的寄存器编号、地址与返回长度只是示意，应按外设手册调整。
auto messages = bus.transfer({
    {0x48, false, false, {0x10}}, // 写寄存器地址
    {0x48, true,  false, {0, 0}} // repeated START 后读取 2 字节
});
auto value = messages[1].data;
```

一次 `transfer()` 是一笔组合事务，各段通过 repeated START 连接，末尾发送 STOP。调用失败时不会自动重试，避免重复写入产生外设副作用。一次可提交 1 至 42 段消息，每段最多 8192 字节，适配器还可能有更严格的限制。

寄存器地址的宽度、字节序、读写顺序和转换等待时间属于设备协议，需要调用方实现。被内核驱动管理的设备应优先使用该驱动暴露的应用层接口，不要通过 i2c-dev 并行操作同一外设。

## 4 CAN 与 CAN FD

rm::Can 使用已配置的 `can0`、`vcan0` 等网络接口。接口启动、仲裁速率、CAN FD 数据速率由系统配置，RMVL 不修改这些配置。

```cpp
#include <rmvl/io/can.hpp>

rm::Can can("can0", rm::CanReadMode::NONBLOCK, true);
rm::CanFrame frame;
frame.id = 0x123;
frame.size = 2;
frame.data[0] = 0x10;
frame.data[1] = 0x20;
bool submitted = can.write(frame);
if (auto received = can.read()) {
    // 处理 received->id 和 received->data
}
```

`id` 不含 SocketCAN 标志位，标准帧最大 11 位，扩展帧最大 29 位。经典 CAN 数据最长 8 字节，CAN FD 最长 64 字节。发送 CAN FD 时必须先启用 FD 接收/发送能力并设置 `frame.fd = true`，CAN FD 不支持远程请求帧。CAN FD 大于 8 字节时，长度必须为 12、16、20、24、32、48 或 64 字节。

`write()` 成功只表示帧已经提交给内核，不代表对端收到或确认。错误帧默认不接收，可通过 `setErrorMask()` 配置，接收到的错误帧不能发送。`setFilters()` 使用 SocketCAN 原始 ID 和掩码格式，可包含 EFF/RTR 标志位，空过滤列表关闭普通数据帧接收。保留 SocketCAN 默认本地回环行为，不接收同一 socket 自己发送的帧。

## 5 协程示例

```cpp
#include <rmvl/io/can.hpp>
#include <rmvl/io/gpio.hpp>

rm::async::Task<> receive(rm::async::Can &can) {
    for (;;) {
        auto frame = co_await can.read();
        // 处理收到的帧
    }
}

rm::async::Task<> watch(rm::async::Gpio &gpio) {
    for (;;) {
        auto event = co_await gpio.wait();
        // 处理边沿事件
    }
}

int main() {
    rm::async::IOContext context;
    rm::async::Can can(context, "can0");
    rm::GpioOptions options;
    options.edge = rm::GpioEdge::Both;
    rm::async::Gpio gpio(context, "/dev/gpiochip0", {18}, options);
    context.spawn(receive, std::ref(can));
    context.spawn(watch, std::ref(gpio));
    context.run();
}
```

部署时需要设备节点权限或相应网络接口权限。普通 PC 是否可用取决于硬件、内核驱动和接线，设备节点存在不表示板上有可访问的物理引脚。

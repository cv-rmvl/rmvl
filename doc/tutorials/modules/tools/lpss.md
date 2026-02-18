轻量发布订阅服务 —— LPSS {#tutorial_modules_lpss}
============

@author 赵曦
@date 2025/11/25
@version 1.0
@brief 模拟 ROS 2/DDS 的以数据为中心的轻量级发布订阅服务

@prev_tutorial{tutorial_modules_mqtt}

@next_tutorial{tutorial_modules_camera}

@tableofcontents

------

相关模块： @ref lpss ， @ref tutorial_table_of_content_rmvlmsg

## 1 机制

### 1.1 简介

Lightweight Pub/Sub Service (LPSS)，即轻量发布订阅服务，通过模仿 ROS 2(DDS) 的去中心化设计，以数据为核心，使用二进制直接存储的序列化与反序列化方式，提供两层的服务发现机制，建立起数据输出端（发布者）与输入端（订阅者）之间的 UDPv4 单播或 SHM 共享内存的实时通信。

### 1.2 服务发现

LPSS 标准提供

- NDP 节点发现协议，用于发现网络中的节点
- EDP 通信端点发现协议，用于发现节点上的发布者和订阅者

两层的服务发现机制，确保发布者和订阅者能够在去中心化的网络环境中找到彼此，并进行高效的数据交换。

#### 1.2.1 节点发现协议

即 Node Discovery Protocol(NDP)， @ref lpss 提供的 RNDP 是节点发现协议的 RMVL 实现，NDP 标准的数据包格式如下所示：

> NDP 包含 \f$14\f$ 字节的 Header 头部信息和 \f$6n+1+m\f$ 字节的 Data 数据信息，Header 头部信息用于标识数据包类型，Data 数据信息用于存储节点的详细信息。
>
> 1. 其中，Header 头部信息的第 \f$0\sim3\f$ 字节为 NDP 标识符，<u>可自行定义</u>，建议使用 ASCII 字符来表示，第 \f$4\sim7\f$ 字节为 GUID 的低 4 字节主 MAC 地址，选取依据为：有线网卡 > 无线网卡 > 虚拟网卡，若系统中存在多个同类型的网卡，则选取第一个启用的网卡；第 \f$8\sim9\f$ 字节为 GUID 的 PID 部分；第 \f$10\sim11\f$ 字节为 GUID 的 Entity ID 部分，属于无效字段，一般可以设置为 `0`；第 \f$12\f$ 字节为单字节无符号整数表示的 LocatorNum，用于标记 Data 数据信息的段数；第 \f$13\f$ 字节为单字节无符号整数表示的 HBT，单位为秒，用于提示本节点的最大心跳包超时时间。
> 2. 后续的 Data 数据信息部分，每 \f$6\f$ 字节表示一个节点的信息，第 \f$0\sim1\f$ 字节为 Locator 的 Port 部分，表示 EDP 通信端点的端口号，采用大端序存储；第 \f$2\sim5\f$ 字节为 Locator 的 Addr 部分，表示 EDP 通信端点的 IPv4 地址。每个节点可以包含多个 Locator 信息段，具体数量由 Header 头部信息中的 LocatorNum 字段决定，并且由实际的网卡数量所限制。$6n$ 字节之后的 \f$1\f$ 字节为节点名称字符串的长度 NodeNameSize，记作 \f$m\f$，后续的 \f$m\f$ 字节为节点名称字符串 NodeName，采用 UTF-8 编码存储。

此外，实现方需使用 UDPv4 多播的方式发送 NDP 数据包，且多播地址为 `239.255.0.5`，多播端口为 `7500 + <LPSS_DOMAIN_ID>`，其中 `LPSS_DOMAIN_ID` 是一个单字节无符号整数。标准还规定每个节点应当周期性地发送心跳包，以维持其在网络中的可见性，心跳包的发送周期应当小于等于 \f$\frac{\text{HBT}}2\f$ 的值。

实现方需完成生命周期管理，需要不短于每隔 1s 的频率检查所有的已发现的节点在上次收到的心跳包的时间戳与当前时间间隔是否超过 HBT，若超过则删除。

RNDP 同样满足以上 NDP 标准，具体的信息格式如下所示：

<div class="full_width_table">
<table class="markdownTable">
<tr class="markdownTableHead">
  <th class="markdownTableHeadCenter">0 byte</th>
  <th class="markdownTableHeadCenter">1 byte</th>
  <th class="markdownTableHeadCenter">2 byte</th>
  <th class="markdownTableHeadCenter">3 byte</th>
  <th class="markdownTableHeadCenter">4 byte</th>
  <th class="markdownTableHeadCenter">5 byte</th>
  <th class="markdownTableHeadCenter">6 byte</th>
  <th class="markdownTableHeadCenter">7 byte</th>
</tr>
<tr class="markdownTableRowOdd">
  <td class="markdownTableBodyCenter"><code>'N'</code></td>
  <td class="markdownTableBodyCenter"><code>'D'</code></td>
  <td class="markdownTableBodyCenter"><code>'0'</code></td>
  <td class="markdownTableBodyCenter"><code>'1'</code></td>
  <td class="markdownTableBodyCenter" colspan="4">GUID MAC</td>
</tr>
<tr class="markdownTableRowEven">
  <th class="markdownTableHeadCenter">8 byte</th>
  <th class="markdownTableHeadCenter">9 byte</th>
  <th class="markdownTableHeadCenter">10 byte</th>
  <th class="markdownTableHeadCenter">11 byte</th>
  <th class="markdownTableHeadCenter">12 byte</th>
  <th class="markdownTableHeadCenter">13 byte</th>
  <th class="markdownTableHeadCenter">14 byte</th>
  <th class="markdownTableHeadCenter">15 byte</th>
</tr>
<tr class="markdownTableRowOdd">
  <td class="markdownTableBodyCenter" colspan="2">GUID PID</td>
  <td class="markdownTableBodyCenter" colspan="2">GUID Entity ID</td>
  <td class="markdownTableBodyCenter">LocatorNum</td>
  <td class="markdownTableBodyCenter">HBT</td>
  <td class="markdownTableBodyCenter" colspan="2">Port 0</td>
</tr>
<tr class="markdownTableRowEven">
  <th class="markdownTableHeadCenter">16 byte</th>
  <th class="markdownTableHeadCenter">17 byte</th>
  <th class="markdownTableHeadCenter">18 byte</th>
  <th class="markdownTableHeadCenter">19 byte</th>
  <th class="markdownTableHeadCenter">20 byte</th>
  <th class="markdownTableHeadCenter">21 byte</th>
  <th class="markdownTableHeadCenter">22 byte</th>
  <th class="markdownTableHeadCenter">23 byte</th>
</tr>
<tr class="markdownTableRowOdd">
  <td class="markdownTableBodyCenter" colspan="4">Addr 0</td>
  <td class="markdownTableBodyCenter" colspan="2">Port 1</td>
  <td class="markdownTableBodyCenter" colspan="2">Addr 1</td>
</tr>
<tr class="markdownTableRowEven">
  <th class="markdownTableHeadCenter">24 byte</th>
  <th class="markdownTableHeadCenter">25 byte</th>
  <th class="markdownTableHeadCenter">26 byte</th>
  <th class="markdownTableHeadCenter">...</th>
</tr>
<tr class="markdownTableRowOdd">
  <td class="markdownTableBodyCenter" colspan="2">Addr 1</td>
  <td class="markdownTableBodyCenter">NodeNameSize</td>
  <td class="markdownTableBodyCenter" colspan="...">NodeName</td>
</tr>
</table>
</div>

#### 1.2.2 通信端点发现协议

即 Endpoint Discovery Protocol(EDP)， @ref lpss 提供的 REDP 是通信端点发现协议的 RMVL 实现，EDP 标准的数据包格式如下所示：

> EDP 包含 \f$14\f$ 字节的 Header 头部信息和 \f$2+n\f$ 字节的 Data 数据信息，Header 头部信息用于标识数据包、承载 GUID、状态标志以及话题大小。Data 数据信息用于存储 MTP 阶段实际通信时 UDPv4 通道的端口号和话题名称，其中话题名称也作为 SHM 通道的共享内存名称。
>
> 1. 其中，Header 头部信息的第 \f$0\sim3\f$ 字节为 EDP 标识符，<u>可自行定义</u>，建议使用 ASCII 字符来表示，第 \f$4\sim7\f$ 字节为 GUID 的低 4 字节主 MAC 地址，选取依据同 NDP；第 \f$8\sim9\f$ 字节为 GUID 的 PID 部分；第 \f$10\sim11\f$ 字节为 GUID 的 Entity ID 部分，每个节点中的发布者、订阅者将有不同的 Entity ID；第 \f$12\f$ 字节为单字节无符号整数表示的 Status 状态标志，用于标记本次操作属于添加、移除发布者、订阅者，第 \f$13\f$ 字节为单字节无符号整数表示的 TopicSize，表示话题名称的字节大小。
> 2. 后续的 Data 数据信息部分，第 \f$0\sim1\f$ 字节为 Locator 的 Port 部分，表示实际通信时 UDPv4 通道的端口号，采用大端序存储；第 \f$2\sim(2+\text{TopicSize})\f$ 字节为 TopicName 部分，表示话题名称的字符串内容，采用 UTF-8 编码存储。

当触发以下事件时，节点将通过 UDPv4 单播的方式发送 EDP 数据包，目标地址为对应节点的 Locator 列表中的 Addr 和 Port：

1. 发现新节点时
   - 遍历本地的发布者，向新节点依次发送 Add 和 Writer 状态的 EDP 数据包，EDP 报文中 Port 可不作设置；
   - 遍历本地的订阅者，向新节点依次发送 Add 和 Reader 状态的 EDP 数据包；
2. 创建发布者时，遍历本地已发现的节点，向每个节点依次发送 Add 和 Writer 状态的 EDP 数据包，EDP 报文中 Port 可不作设置；
3. 创建订阅者时，遍历本地已发现的节点，向每个节点依次发送 Add 和 Reader 状态的 EDP 数据包；
4. 节点销毁时
   - 遍历已发现的节点，向每个节点依次发送 Remove 和 Writer 状态的 EDP 数据包，EDP 报文中 Port 可不作设置；
   - 遍历已发现的节点，向每个节点依次发送 Remove 和 Reader 状态的 EDP 数据包，EDP 报文中 Port 可不作设置；

REDP 同样满足以上 EDP 标准，具体的信息格式如下所示：

<div class="full_width_table">
<table class="markdownTable">
<tr class="markdownTableHead">
  <th class="markdownTableHeadCenter">0 byte</th>
  <th class="markdownTableHeadCenter">1 byte</th>
  <th class="markdownTableHeadCenter">2 byte</th>
  <th class="markdownTableHeadCenter">3 byte</th>
  <th class="markdownTableHeadCenter">4 byte</th>
  <th class="markdownTableHeadCenter">5 byte</th>
  <th class="markdownTableHeadCenter">6 byte</th>
  <th class="markdownTableHeadCenter">7 byte</th>
</tr>
<tr class="markdownTableRowOdd">
  <td class="markdownTableBodyCenter"><code>'E'</code></td>
  <td class="markdownTableBodyCenter"><code>'D'</code></td>
  <td class="markdownTableBodyCenter"><code>'0'</code></td>
  <td class="markdownTableBodyCenter"><code>'1'</code></td>
  <td class="markdownTableBodyCenter" colspan="4">GUID MAC</td>
</tr>
<tr class="markdownTableRowEven">
  <th class="markdownTableHeadCenter">8 byte</th>
  <th class="markdownTableHeadCenter">9 byte</th>
  <th class="markdownTableHeadCenter">10 byte</th>
  <th class="markdownTableHeadCenter">11 byte</th>
  <th class="markdownTableHeadCenter">12 byte</th>
  <th class="markdownTableHeadCenter">13 byte</th>
  <th class="markdownTableHeadCenter">14 byte</th>
  <th class="markdownTableHeadCenter">15 byte</th>
</tr>
<tr class="markdownTableRowOdd">
  <td class="markdownTableBodyCenter" colspan="2">GUID PID</td>
  <td class="markdownTableBodyCenter" colspan="2">GUID Entity ID</td>
  <td class="markdownTableBodyCenter">Status</td>
  <td class="markdownTableBodyCenter" colspan="2">Port</td>
  <td class="markdownTableBodyCenter">TopicSize</td>
</tr>
<tr class="markdownTableRowEven">
  <th class="markdownTableHeadCenter">16 byte</th>
  <th class="markdownTableHeadCenter">...</th>
  <th class="markdownTableHeadCenter">16 + TopicSize byte</th>
  <th class="markdownTableHeadCenter">17 + TopicSize byte</th>
  <th class="markdownTableHeadCenter">...</th>
</tr>
<tr class="markdownTableRowOdd">
  <td class="markdownTableBodyCenter" colspan="2">Topic</td>
  <td class="markdownTableBodyCenter">MsgtypeSize</td>
  <td class="markdownTableBodyCenter" colspan="...">Msgtype</td>
</tr>
</table>
</div>

### 1.3 数据传输

#### 1.3.1 话题消息传输协议

即 Message Transfer Protocol， @ref lpss 提供的 RMTP 是话题消息传输协议的 RMVL 实现，MTP 标准的数据包格式如下所示：

> 一个完整的 MTP 数据包含 \f$6+M+N\f$ 字节的 Header 头部信息和剩下的 Payload 负载信息，Header 头部信息用于标识数据包，以及提供话题、消息类型相关信息，Payload 负载用于存储后续实际通信时的数据内容。
>
> 1. 其中，Header 头部信息的第 \f$0\sim3\f$ 字节为 MTP 标识符，<u>可自行定义</u>，建议使用 ASCII 字符来表示，第 \f$4\f$ 字节为话题字符串长度 TopicSize，记作 \f$M\f$；第 \f$5\sim4+M\f$ 字节为话题名称字符串；第 \f$5+M\f$ 字节为消息类型字符串长度 TypeSize，记作 \f$N\f$，第 \f$6+M\sim5+M+N\f$ 字节为消息类型字符串。
> 2. 后续的 Payload 负载信息部分，由序列化与反序列化的具体协议提供支持。

RMTP 同样满足以上 MTP 标准，具体的信息格式如下所示：

<div class="full_width_table">
<table class="markdownTable">
<tr class="markdownTableHead">
  <th class="markdownTableHeadCenter">0 byte</th>
  <th class="markdownTableHeadCenter">1 byte</th>
  <th class="markdownTableHeadCenter">2 byte</th>
  <th class="markdownTableHeadCenter">3 byte</th>
  <th class="markdownTableHeadCenter">4 byte</th>
  <th class="markdownTableHeadCenter">5 byte</th>
  <th class="markdownTableHeadCenter">...</th>
  <th class="markdownTableHeadCenter">4+M byte</th>
</tr>
<tr class="markdownTableRowOdd">
  <td class="markdownTableBodyCenter"><code>'M'</code></td>
  <td class="markdownTableBodyCenter"><code>'T'</code></td>
  <td class="markdownTableBodyCenter"><code>'0'</code></td>
  <td class="markdownTableBodyCenter"><code>'1'</code></td>
  <td class="markdownTableBodyCenter">TopicSize</td>
  <td class="markdownTableBodyCenter" colspan="3">Topic</td>
</tr>
<tr class="markdownTableRowEven">
  <th class="markdownTableHeadCenter">5+M byte</th>
  <th class="markdownTableHeadCenter">6+M byte</th>
  <th class="markdownTableHeadCenter">...</th>
  <th class="markdownTableHeadCenter">5+M+N byte</th>
  <th class="markdownTableHeadCenter">...</th>
</tr>
<tr class="markdownTableRowOdd">
  <td class="markdownTableBodyCenter">TypeSize</td>
  <td class="markdownTableBodyCenter" colspan="3">Type</td>
  <td class="markdownTableBodyCenter">Payload</td>
</tr>
</table>
</div>

#### 1.3.2 序列化与反序列化

MTP 标准使用二进制直接序列化 / 反序列化的方式，不区分端序（这会降低一部分兼容性，但在主流架构以及 OS 上均一致），因此数据在发布者与订阅者之间的传输效率非常高。此外 RMVL 提供了消息类型的自动代码生成工具，用户可以通过定义消息类型的 `*.msg` 文件，使用 RMVL 提供的代码生成工具生成对应的 C++ 代码文件，从而简化消息类型的创建过程。

RMVL 内置了一些常用的消息类型，用户可以直接使用这些消息类型，而无需自行定义和生成代码。同时，RMVL 提供了 `rmvl_generate_msg` 的 CMake 函数，可以辅助用户完成自定义消息类型的代码生成过程，详情可参考 @ref tutorial_table_of_content_rmvlmsg 。

## 2 同步模式使用示例

LPSS 提供了简单易用的发布者与订阅者接口，用户可以方便地创建发布者与订阅者，实现节点间的数据通信。每个节点内部维护了众多线程，以下示例展示了如何使用 LPSS 创建发布者与订阅者。

### 2.1 创建简单的发布者与订阅者

#### 2.1.1 发布者示例

下面的示例展示了如何创建一个发布者，该发布者每隔 100 毫秒发布一次包含递增计数值的字符串消息。

```cpp
// RMVL 内置的第三方 fmt 库，用于格式化字符串，不需要 fmt 库的不用包含该头文件
#include <fmt/format.h>
// LPSS 节点功能的头文件
#include <rmvl/lpss/node.hpp>
// 内置的 std 分组的 String 消息类型头文件
#include <rmvlmsg/std/string.hpp>

using namespace std::chrono_literals; // 使用时间字面量，如 100ms、5s 等
using namespace rm;                   // 使用 RMVL 提供的命名空间

int main() {
    // 创建 LPSS 节点
    auto nd = lpss::Node("pub_node");
    // 创建发布者，发布 String 类型的消息到 /topic 话题
    auto publisher = nd.createPublisher<msg::String>("/topic");

    // 准备消息类型
    auto msg = msg::String();
    uint16_t count{};

    // 循环发布消息
    while (true) {
        // sleep 是为了模拟程序可能执行的其他功能
        std::this_thread::sleep_for(100ms);
        // 设置消息内容，这里使用的是 RMVL 内置的第三方 fmt 库，也可以使用别的方式进行设置
        msg.data = fmt::format("Times: {}", count++);
        // 发布消息，这一步会完成消息的序列化和传输
        publisher.publish(msg);
    }
    return 0;
}
```

#### 2.1.2 订阅者示例

下面展示了如何创建一个订阅者，该订阅者订阅 `/topic` 话题的字符串消息，并在收到消息时打印消息内容。

```cpp
#include <fmt/format.h>
#include <rmvl/lpss/node.hpp>
#include <rmvlmsg/std/string.hpp>

using namespace std::chrono_literals;
using namespace rm;

int main() {
    // 创建 LPSS 节点
    auto nd = lpss::Node("sub_node");
    // 创建订阅者，订阅 /topic 话题的 String 类型的消息
    auto subscriber = nd.createSubscriber<msg::String>(
        "/topic", [](const msg::String &msg) {
            // 收到消息后的回调函数，这里使用的是 lambda 表达式，内部仅简单地打印收到的消息内容
            fmt::println("I heard: '{}'", msg.data);
        });

    while (true) {
        // 下面的 sleep 只是为了保持程序运行，可以在这里执行其他功能
        std::this_thread::sleep_for(1s);
    }
}
```

@warning 订阅者的回调函数是在 LPSS 内部的线程中执行的，如果用户定义了多个订阅者，这些订阅者的回调函数可能会在不同的线程中并发执行。因此，用户在编写回调函数时<u><i><b>需要注意线程安全问题</b></i></u>，避免在回调函数中使用非线程安全的资源，或者使用适当的同步机制来保护共享资源，如果想避免这一问题，可以考虑使用下文异步模式的发布订阅服务。

## 3 异步模式使用示例

LPSS 同样支持异步模式的发布订阅服务，均定义在 `::rm::lpss::async` 命名空间中。内部使用 coroutine + epoll/IOCP 的方式创建发布者与订阅者，实现更加灵活的数据通信，但需要 C++20 的支持，详情请参见 @ref tutorial_modules_coro 。下面的示例展示了如何使用异步模式创建发布者与订阅者。

### 3.1 异步发布者示例

异步模式的 LPSS 节点增加了定时器功能，可以方便地实现周期性任务。下面的示例展示了如何创建一个异步发布者，该发布者每隔 20 毫秒发布一次包含递增计数值的字符串消息。

```cpp
#include <fmt/format.h>

#include <rmvl/lpss/node.hpp>
#include <rmvlmsg/std/string.hpp>

using namespace rm;
using namespace std::chrono_literals;

class MyPublisher : public lpss::async::Node {
public:
    MyPublisher(std::string_view name) : lpss::async::Node(name) {
        _pub = this->createPublisher<msg::String>("/topic");
        _timer = this->createTimer(20ms, [this]() {
            auto msg = msg::String{};
            msg.data = fmt::format("Async Times: {}", _count++);
            _pub->publish(msg);
        });
    }
  
private:
    uint16_t _count{};
    lpss::async::Publisher<msg::String>::ptr _pub{};
    lpss::async::Timer::ptr _timer{};
};

int main() {
    auto node = MyPublisher("async_pub_node");
    node.spin();
    return 0;
}
```

### 3.2 异步订阅者示例

下面展示了如何创建一个异步订阅者，该订阅者订阅 `/topic` 话题的字符串消息，并在收到消息时打印消息内容。

```cpp
#include <fmt/format.h>

#include <rmvl/lpss/node.hpp>
#include <rmvlmsg/std/string.hpp>

using namespace rm;

class MySubscriber : public lpss::async::Node {
public:
    MySubscriber(std::string_view name) : lpss::async::Node(name) {
        _sub = this->createSubscriber<msg::String>("/topic", [](const msg::String &msg) {
            fmt::println("Async I heard: '{}'", msg.data);
        });
    }

private:
    lpss::async::Subscriber<msg::String>::ptr _sub{};
};

int main() {
    auto node = MySubscriber("async_sub_node");
    node.spin();
    return 0;
}
```

## 4 创建自定义消息类型

除了使用 RMVL 内置的消息类型外，用户还可以自定义消息类型，并使用这些自定义的消息类型创建发布者与订阅者。下面的示例展示了如何定义一个自定义的消息类型，并使用该消息类型创建异步的发布者与订阅者。

### 4.1 创建项目结构

首先可以创建一个项目，假设项目名称为 `demo`，并在其中创建

- `custom_msg` 目录，用于存放自定义的消息类型文件，以及自动生成的代码；
- `src` 目录，用于存放发布者与订阅者的源代码文件；
- `CMakeLists.txt` 文件，用于配置项目的构建过程。

### 4.2 文件内容

基本内容：

<div class="tabbed">

- <b class="tab-title">CMakeLists.txt</b>

  <div class="fragment">
  <div class="line"><span class="keyword">cmake_minimum_required</span>(VERSION 3.16)</div>
  <div class="line"><span class="keyword">project</span>(LPSSDemo)</div>
  <div class="line"></div>
  <div class="line"><span class="keyword">set</span>(CMAKE_CXX_STANDARD 17)</div>
  <div class="line"><span class="keyword">set</span>(CMAKE_CXX_STANDARD_REQUIRED ON)</div>
  <div class="line"></div>
  <div class="line"><span class="comment"># 查找 RMVL 包，其中包含了自动代码生成的功能</span></div>
  <div class="line"><span class="keyword">find_package</span>(RMVL REQUIRED)</div>
  <div class="line"></div>
  <div class="line"><span class="comment"># 添加子目录</span></div>
  <div class="line"><span class="keyword">add_subdirectory</span>(custom_msg)</div>
  <div class="line"></div>
  <div class="line"><span class="comment"># 添加发布者和订阅者的可执行文件</span></div>
  <div class="line"><span class="keywordflow">foreach</span>(m pub sub)</div>
  <div class="line">&nbsp;&nbsp;<span class="keyword">rmvl_add_exe</span>(${m}</div>
  <div class="line">&nbsp;&nbsp;&nbsp;&nbsp;<span class="keyword">SOURCES</span> src/${m}.cpp</div>
  <div class="line">&nbsp;&nbsp;&nbsp;&nbsp;<span class="keyword">DEPENDS</span> lpss custom_msg</div>
  <div class="line">&nbsp;&nbsp;)</div>
  <div class="line"><span class="keywordflow">endforeach</span>()</div>
  </div>

- <b class="tab-title">src/pub.cpp</b>

  在 `src` 目录下创建 `pub.cpp` 文件，实现异步发布者：

  ```cpp
  #include <rmvl/lpss.hpp>

  #include "rmvlmsg/my_custom_msg.hpp"

  using namespace std::chrono_literals;
  using namespace rm;

  class CustomMsgPublisher : public lpss::async::Node {
  public:
      CustomMsgPublisher(std::string_view name) : lpss::async::Node(name) {
          _pub = this->createPublisher<msg::MyCustomMsg>("/my_custom_topic");
          _timer = this->createTimer(50ms, [this]() {
              msg::MyCustomMsg msg;
              msg.id = _count++;
              msg.name = "Message_" + std::to_string(msg.id);
              msg.is_active = (msg.id % 2 == 0);
              _pub->publish(msg);
          });
      }

  private:
      int32_t _count{};
      lpss::async::Publisher<msg::MyCustomMsg>::ptr _pub{};
      lpss::async::Timer::ptr _timer{};
  };

  int main() {
      auto nd = CustomMsgPublisher("custom_msg_pub_node");
      nd.spin();
  }
  ```

- <b class="tab-title">src/sub.cpp</b>

  在 `src` 目录下创建 `sub.cpp` 文件，实现异步订阅者：

  ```cpp
  #include <fmt/format.h>
  #include <rmvl/lpss.hpp>

  #include "rmvlmsg/my_custom_msg.hpp"

  using namespace std::chrono_literals;
  using namespace rm;

  class CustomMsgSubscriber : public lpss::async::Node {
  public:
      CustomMsgSubscriber(std::string_view name) : lpss::async::Node(name) {
          _sub = this->createSubscriber<msg::MyCustomMsg>(
              "/my_custom_topic", [](const msg::MyCustomMsg &msg) {
                  fmt::println("ID: {}, Name: {}, Active: {}", msg.id, msg.name, msg.is_active);
              });
      }

  private:
      lpss::async::Subscriber<msg::MyCustomMsg>::ptr _sub{};
  };

  int main() {
      auto nd = CustomMsgSubscriber("custom_msg_sub_node");
      nd.spin();
  }
  ```

</div>

然后在 `custom_msg` 目录下创建

- `msg` 目录，用于存放自定义的消息类型定义文件；
- `CMakeLists.txt` 文件，配置消息类型的代码生成。

<div class="tabbed">

- <b class="tab-title">MyCustomMsg.msg</b>

  在 `msg` 目录下创建 `MyCustomMsg.msg` 文件，定义自定义的消息类型：

  <div class="fragment">
  <div class="line"><span class="comment"># MyCustomMsg.msg</span></div>
  <div class="line"></div>
  <div class="line"><span class="keywordtype">int32</span> id</div>
  <div class="line"><span class="keywordtype">string</span> name</div>
  <div class="line"><span class="keywordtype">bool</span> is_active</div>
  </div>

- <b class="tab-title">CMakeLists.txt</b>

  <div class="fragment">
  <div class="line"><span class="comment"># 生成自定义消息类型的代码</span></div>
  <div class="line"><span class="keyword">rmvl_generate_msg</span>(MyCustomMsg)</div>
  <div class="line"><span class="comment"># 添加模块</span></div>
  <div class="line"><span class="keyword">rmvl_add_module</span>(custom_msg)</div>
  </div>

</div>

### 4.3 构建与运行

在项目根目录下运行以下命令进行构建，并运行发布者：

<div class="fragment">
<div class="line"><span class="comment"># Build</span></div>
<div class="line"><span class="keywordflow">mkdir</span> build &amp;&amp; <span class="keywordflow">cd</span> build</div>
<div class="line"><span class="keywordflow">cmake</span> ..</div>
<div class="line"><span class="keywordflow">cmake</span> <span class="comment">\-\-build</span> .</div>
<div class="line"></div>
<div class="line"><span class="comment"># Run</span></div>
<div class="line">./pub</div>
</div>

然后在另一个终端中运行订阅者，记得进入 build 目录：

<div class="fragment">
<div class="line">./sub</div>
</div>

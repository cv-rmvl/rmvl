传输层设施 —— Socket {#tutorial_modules_socket}
============

@author 赵曦
@date 2025/09/25
@version 1.0

@prev_tutorial{tutorial_modules_serial}

@next_tutorial{tutorial_modules_netapp}

@tableofcontents

------

## 1 Socket 基础

Socket 是网络通信的基本操作单元，提供了应用层与传输层之间的接口。Socket 可以分为多种类型，最常见的有流式套接字（TCP）和数据报套接字（UDP）。RMVL 提供了跨平台的 Socket 实现，并提供了协程支持，Windows 下基于 Winsock2 实现，Linux 下基于 BSD Socket 实现。

## 2 用法

相关类：

- 同步：
  - 流式 rm::StreamSocket 以及 rm::Acceptor 和 rm::Connector
  - 数据报式 rm::DgramSocket 以及 rm::Sender 和 rm::Listener

- 异步：
  - 流式 rm::async::StreamSocket 以及 rm::async::Acceptor 和 rm::async::Connector
  - 数据报式 rm::async::DgramSocket 以及 rm::async::Sender 和 rm::async::Listener

一般工程使用中，对于流式 Socket 推荐客户端使用<span style="color: red">同步阻塞</span>方式进行通信，服务端使用<span style="color: red">异步非阻塞</span>方式进行通信，以兼顾高并发性能。对于数据报式 Socket，则根据具体需求选择同步或异步方式，例如在多播场景下，使用同步阻塞的 Listener 和 Sender 即可实现绝大部分功能。

### 2.1 同步阻塞

下面展示了流式 Socket 的同步阻塞用法。

**服务端**

```cpp
#include <rmvl/io/socket.hpp>

using namespace rm;

int main() {
    Acceptor acceptor(Endpoint(ip::tcp::v4(), 8080));

    // 阻塞的等待客户端连接
    auto socket = acceptor.accept();

    // 读取数据
    std::string request = socket.read();
    if (request.empty()) {
        printf("Read failed\n");
        return -1;
    }
    printf("Received: %s\n", request.c_str());

    // 发送响应
    bool success = socket.write("response");
    if (!success) {
        printf("Write failed\n");
        return -1;
    }
}
```

**客户端**

```cpp
#include <rmvl/io/socket.hpp>

using namespace rm;

int main() {
    Connector connector(Endpoint(ip::tcp::v4(), 8080), "127.0.0.1");
    auto socket = connector.connect();

    // 发送请求
    bool success = socket.write("request");
    if (!success) {
        printf("Write failed\n");
        return -1;
    }

    // 读取响应
    std::string response = socket.read();
    if (response.empty()) {
        printf("Read failed\n");
        return -1;
    }
    printf("Received: %s\n", response.c_str());
}
```

### 2.2 异步非阻塞

下面展示了流式 Socket 的异步非阻塞用法。

**服务端**

```cpp
#include <rmvl/io/socket.hpp>

using namespace rm;

async::Task<> session(async::StreamSocket socket) {
    // 读取数据
    std::string request = co_await socket.read();
    if (request.empty()) {
        printf("Read failed\n");
        co_return;
    }
    printf("Received: %s\n", request.c_str());

    // 发送响应
    bool success = co_await socket.write("response");
    if (!success) {
        printf("Write failed\n");
        co_return;
    }
}

int main() {
    async::IOContext io_context{};
    async::Acceptor acceptor(io_context, Endpoint(ip::tcp::v4(), 8080));

    // 异步等待客户端连接
    co_spawn(io_context, [&]() -> async::Task<> {
        while (true) {
            auto socket = co_await acceptor.accept();
            // 为每个连接启动一个协程任务
            co_spawn(io_context, session, std::move(socket));
        }
    });

    // 此处可以使用 co_spawn 注册其他 I/O 任务

    io_context.run();
}
```

**客户端**

```cpp
#include <rmvl/io/socket.hpp>

using namespace rm;

async::Task<> session(async::Connector &connector) {
    auto socket = co_await connector.connect();

    // 发送请求
    bool success = co_await socket.write("request");
    if (!success) {
        printf("Write failed\n");
        co_return;
    }

    // 读取响应
     std::string response = co_await socket.read();
    if (response.empty()) {
        printf("Read failed\n");
        co_return;
    }
    printf("Received: %s\n", response.c_str());
}

int main() {
    async::IOContext io_context{};
    async::Connector connector(io_context, Endpoint(ip::tcp::v4(), 8080), "127.0.0.1");

    co_spawn(io_context, session, std::ref(connector));

    // 此处可以使用 co_spawn 注册其他 I/O 任务

    io_context.run();
}
```

### 2.3 Socket 控制选项

Socket 类提供了一些控制选项，用于配置 Socket 的行为，例如设置监听方的多播选项等

```cpp
auto reader = rm::Listener(ip::udp::v4(), 8080);
// 加入多播组
reader.setOption(ip::multicast::JoinGroup("224.0.0.1"));
// 启用多播环回
reader.setOption(ip::multicast::Loopback(true));
```

以及发送方的多播选项

```cpp
auto writer = rm::Sender(ip::udp::v4());
// 设置多播接口
writer.setOption(ip::multicast::Interface("192.168.1.100"));
```

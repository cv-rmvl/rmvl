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

除此之外，Socket 还提供了 Unix Domain Socket（Unix 域套接字）支持，允许在同一台机器上的不同进程之间进行高效的通信，使用方法可参考 @ref tutorial_modules_ipc 。

## 2 用法

相关类：

- 同步： rm::Socket, rm::Acceptor 及 rm::Connector
- 异步： rm::async::Socket, rm::async::Acceptor 及 rm::async::Connector

一般工程使用中，推荐客户端使用<span style="color: red">同步阻塞</span>方式进行通信，服务端使用<span style="color: red">异步非阻塞</span>方式进行通信，以兼顾高并发性能。

### 2.1 同步阻塞

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

**服务端**

```cpp
#include <rmvl/io/socket.hpp>

using namespace rm;

async::Task<> session(async::Socket socket) {
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
            async::Socket socket = co_await acceptor.accept();
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
    
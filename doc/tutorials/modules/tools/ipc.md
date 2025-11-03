进程间通信设施 —— IPC {#tutorial_modules_ipc}
============

@author 赵曦
@date 2025/09/25
@version 1.0

@prev_tutorial{tutorial_modules_coro}

@next_tutorial{tutorial_modules_serial}

@tableofcontents

------

## 1 命名管道

相关类：

- 同步： rm::PipeServer, rm::PipeClient
- 异步： rm::async::PipeServer, rm::async::PipeClient

管道是一种半双工的通信方式，允许一个进程与另一个进程进行通信。命名管道（Named Pipe）是一种特殊类型的管道，它具有名称，可以在不同的进程之间进行通信。

RMVL 提供了跨平台的命名管道实现，并提供了协程支持，Windows 下基于 Windows 命名管道，Linux 下基于 Unix FIFO 实现。

### 1.1 同步阻塞

**服务端**

<div class="tabbed">

- <b class="tab-title">C++</b>

  ```cpp
  #include <rmvl/io/ipc.hpp>

  using namespace rm;

  int main() {
      PipeServer server("mypipe");

      bool success = server.write("message");
      if (!success) {
          printf("Write failed\n");
          return -1;
      }

      std::string msg = server.read();
      if (msg.empty()) {
          printf("Read failed\n");
          return -1;
      }

      printf("Received: %s\n", msg.c_str());
  }
  ```

- <b class="tab-title">Python</b>

  ```python
  import rm

  server = rm.PipeServer("mypipe")

  success = server.write("message")
  if not success:
      print("Write failed")
      return -1

  msg = server.read()
  if not msg:
      print("Read failed")
      return -1

  print(f"Received: {msg}")
  ```

</div>

**客户端**

<div class="tabbed">

- <b class="tab-title">C++</b>

  ```cpp
  #include <rmvl/io/ipc.hpp>

  using namespace rm;

  int main() {
      PipeClient client("mypipe");

      std::string msg = client.read();
      if (msg.empty()) {
          printf("Read failed\n");
          return -1;
      }
      printf("Received: %s\n", msg.c_str());

      bool success = client.write("received: " + msg);
      if (!success) {
          printf("Write failed\n");
          return -1;
      }
  }
  ```

- <b class="tab-title">Python</b>

  ```python
  import rm

  client = rm.PipeClient("mypipe")

  msg = client.read()
  if not msg:
      print("Read failed")
      return -1
  print(f"Received: {msg}")

  success = client.write("received: " + msg)
  if not success:
      print("Write failed")
      return -1
  ```

</div>

### 1.2 异步非阻塞

**服务端**

```cpp
#include <rmvl/io/ipc.hpp>

using namespace rm;

async::Task<> session(async::PipeServer server) {
    bool success = co_await server.write("message");
    if (!success) {
        printf("Write failed\n");
        co_return;
    }

    std::string msg = co_await server.read();
    if (msg.empty()) {
        printf("Read failed\n");
        co_return;
    }

    printf("Received: %s\n", msg.c_str());
}

int main() {
    async::IOContext io_context{};
    async::PipeServer server("mypipe");

    co_spawn(io_context, session, std::move(server));
    
    // 此处可以使用 co_spawn 注册其他 I/O 任务

    io_context.run();
    return 0;
}
```

**客户端**

```cpp
#include <rmvl/io/ipc.hpp>

using namespace rm;

async::Task<> session(async::PipeClient client) {
    std::string msg = co_await client.read();
    if (msg.empty()) {
        printf("Read failed\n");
        co_return;
    }
    printf("Received: %s\n", msg.c_str());

    bool success = co_await client.write("received: " + msg);
    if (!success) {
        printf("Write failed\n");
        co_return;
    }
}

int main() {
    async::IOContext io_context{};
    async::PipeClient client("mypipe");

    co_spawn(io_context, session, std::move(client));

    // 此处可以使用 co_spawn 注册其他 I/O 任务

    io_context.run();
    return 0;
}
```

## 2 消息队列（仅 Linux 可用）

相关类： rm::MqServer 及 rm::MqClient

**服务端**

```cpp
#include <rmvl/io/ipc.hpp>

using namespace rm;

int main() {
    MqServer server("/myqueue");

    bool success = server.send("message");
    if (!success) {
        printf("Send failed\n");
        return -1;
    }

    std::string msg = server.receive();
    if (msg.empty()) {
        printf("Receive failed\n");
        return -1;
    }

    printf("Received: %s\n", msg.c_str());
}
```

**客户端**

```cpp
#include <rmvl/io/ipc.hpp>

using namespace rm;

int main() {
    MqClient client("/myqueue");

    std::string msg = client.receive();
    if (msg.empty()) {
        printf("Receive failed\n");
        return -1;
    }

    printf("Received: %s\n", msg.c_str());

    bool success = client.send("received: " + msg);
    if (!success) {
        printf("Send failed\n");
        return -1;
    }
}
```

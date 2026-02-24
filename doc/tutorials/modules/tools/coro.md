基于异步 I/O 的协程设施 {#tutorial_modules_coro}
============

@author 赵曦
@date 2025/09/25
@version 1.0
@brief Epoll / IOCP 基本介绍与协程设施

@prev_tutorial{tutorial_modules_aggregate_reflect}

@next_tutorial{tutorial_modules_ipc}

@tableofcontents

------

## 1 异步 I/O

### 1.1 Reactor 模式

Reactor 模式讲究在单个线程中管理多个连接，以高效处理大量并发连接，其核心思想是通过事件驱动机制来处理 I/O 操作，即当某个 I/O 事件（如数据可读、可写等）发生时，操作系统会通知应用程序，应用程序再去处理相应的 I/O 操作。对于协程而言，Reactor 模式的实现通常涉及以下几个关键步骤：

- 协程调度器：负责管理和调度协程的执行，维护一个事件循环，当就绪队列执行完后，进入 I/O 多路复用等待状态；
- 协程挂起：注册 I/O 事件，并将协程挂起，等待事件发生；
- 事件通知：当 I/O 事件发生时，操作系统唤醒阻塞的 I/O 多路复用等待；
- 协程恢复：调度器将就绪的协程重新加入就绪队列，执行实际的 I/O 操作，并取出操作结果。

Reactor 模式常见的实现有 Linux 下的 select、poll、epoll 以及 Mac OS 下的 kqueue。关于 select、poll 和 epoll 的区别，主要有以下内容：

<div class="full_width_table">

|          | select                      | poll                        | epoll                       |
| :----------: | :-------------------------: | :-------------------------: | :-------------------------: |
| 存储结构     | 使用固定大小的数组存储文件描述符 | 使用链表存储文件描述符 | 使用内核空间的红黑树存储文件描述符 |
| 通知机制     | 线性扫描数组，效率较低     | 线性扫描链表，效率较低     | 事件通知模型（事件驱动），效率高     |
| 性能         | 适用于连接数较少的场景       | 适用于连接数较少的场景       | 适用于高并发连接的场景     |
| 可扩展性     | 受限于文件描述符数量（通常为 1024） | 不受文件描述符数量限制 | 不受文件描述符数量限制，支持大量连接 |

</div>

### 1.2 Proactor 模式

Proactor 模式则是通过操作系统提供的异步 I/O 接口来实现，即应用程序可以直接发起异步 I/O 操作，操作系统会在后台完成 I/O 操作，并在操作完成后通知应用程序。 对于协程而言，Proactor 模式的实现通常涉及以下几个关键步骤：

- 协程调度器：负责管理和调度协程的执行，维护一个事件循环，当就绪队列执行完后，进入等待状态；
- 协程挂起：发起异步 I/O 操作，并将协程挂起，等待操作完成；
- 事件通知：当异步 I/O 操作完成时，操作系统唤醒阻塞的等待；
- 协程恢复：调度器将就绪的协程重新加入就绪队列，直接取出操作结果。

Proactor 模式常见的实现有 Linux 下的 io_uring 以及 Windows 下的 IOCP

## 2 用法

RMVL 目前提供了跨平台的异步 I/O 协程设施

- Linux 下基于 epoll 实现 Reactor 模式的协程调度器
- Windows 下基于 IOCP 实现 Proactor 模式的协程调度器

未来可能会基于 Linux 下的 io_uring 实现 Proactor 模式的协程调度器，并且进一步支持 Mac OS。

### 2.1 协程调度器

相关类：
- rm::async::IOContext —— 以系统异步 I/O 为内核的调度器
- rm::async::Task - 可等待的协程任务

rm::async::IOContext 是基于异步 I/O 的协程调度器，负责管理和调度协程的执行。内部维护了 BasicTask 任务基类，由维护了 rm::async::Task 模板的 TaskWrapper 派生类实现类型擦除的功能。部分成员变量的细节如下：

- `ready` 就绪队列，存储所有可以执行的协程任务；
- `unfinish` 未完成哈希表，存储所有被挂起但未完成的协程任务，其中 `Key` 为任务协程句柄，`Value` 为 BasicTask 任务基类。

当 `run` 方法被执行后，存在如下调度流程：

1. 优先处理 `ready` 中的协程任务，调用其 `resume` 方法唤醒，交还控制权给对应的协程；
2. 当再次被挂起，如果该任务未完成，则加入 `unfinish`；
3. 当就绪队列为空，则进入等待状态，等待 I/O 事件的发生，例如 epoll 使用 `epoll_wait`，IOCP 使用 `GetQueuedCompletionStatusEx` 实现阻塞；
4. 当 I/O 事件发生时，唤醒阻塞的等待，并将存在于 `unfinish` 的就绪协程重新加入 `ready`，以等待后续唤醒，若不在则直接唤醒。

需要注意的是，由于 C++ Coroutine 是无栈协程，所有协程任务都要求是可重入的，因此，协程任务要么具有静态存储期，要么由 `ready` 与 `unfinish` 进行生命周期管理（多次在二者之间移动），使用 `co_spawn` 函数或者 `spawn` 成员函数创建的协程，均会被 `ready` 与 `unfinish` 进行管理，而使用 `co_await` 直接等待的协程，一般具有静态存储期，可以无需特殊管理。<span style="color: red">当然，生命周期的管理对于用户是无感的。</span>

一个基本的使用示例如下：

```cpp
#include <cstdio>

#include <rmvl/io/async.hpp>

using namespace rm;

int main() {
    async::IOContext io_context{};

    co_spawn(io_context, []() -> async::Task<> {
        printf("Test\n");
        co_return;
    });

    io_context.run();
}
```

### 2.2 通用等待器

相关类：
- rm::async::AsyncIOAwaiter —— 异步等待器基类
- rm::async::AsyncReadAwaiter, rm::async::AsyncWriteAwaiter —— 通用读写等待器

@ref io 提供了两种通用等待器，分别是 rm::async::AsyncReadAwaiter 和 rm::async::AsyncWriteAwaiter 。它们的主要作用是将 I/O 操作封装成协程的挂起、恢复操作，使得用户可以使用 `co_await` 关键字来异步的等待 I/O 操作的完成。

例如 rm::async::PipeServer 和 rm::async::PipeClient 均提供了异步 `read` 和 `write` 方法，返回对应的等待器实例，可以直接使用 `co_await` 关键字等待 I/O 操作的完成。在协程任务中，可以使用如下代码完成异步的读写操作：

```cpp
async::Task<> session(async::PipeClient cli) {
    bool success = co_await cli.write(data); // 异步写数据
    std::string data = co_await cli.read();  // 异步读数据
}
```

而此处的 `session` 协程函数则可以通过 `co_spawn` 来生成协程任务。

### 2.3 异步定时器

相关类： rm::async::Timer —— 异步定时器

@ref io 在

- Linux 平台下，提供了 `timerfd` 即基于文件描述符的异步定时器，配合 epoll 实现异步定时
- Windows 平台下，提供了 `ThreadpoolTimer` 即 Windows 线程池提供的异步定时器。它通过定时回调，间接与 IOCP 结合，以实现基于事件触发的异步定时。

一个简易的使用示例如下：

```cpp
#include <csignal>
#include <cstdio>
#include <thread>

#include <rmvl/io/async.hpp>

using namespace rm;
using namespace std::chrono_literals;

// 创建执行上下文
static async::IOContext io_context;

int main() {
    signal(SIGINT, [](int) {
        io_context.stop();
    });

    // 启动一个协程任务
    co_spawn(io_context, []() -> async::Task<> {
        async::Timer timer(io_context);
        for (int i = 0; i < 5; ++i) {
            printf("Task 1: 1s\n");
            co_await timer.sleep_for(1000); // 每 1s 打印一次
        }
    });

    // 启动另一个协程任务
    co_spawn(io_context, []() -> async::Task<> {
        async::Timer timer(io_context);
        for (int i = 0; i < 5; ++i) {
            printf("Task 2: 500ms\n");
            co_await timer.sleep_for(500); // 每 500ms 打印一次
        }
    });

    // 运行调度器
    io_context.run();

    return 0;
}
```

这段代码中，创建了 `async::IOContext` 的异步 I/O 协程调度器，通过 `co_spawn` 函数启动了两个协程任务，分别每 1 秒和每 500 毫秒打印一次信息。调度器通过 `io_context.run()` 启动，直到捕获到 Ctrl + C 信号后停止。代码的运行结果为：

```
Task 1: 1s
Task 2: 500ms
Task 2: 500ms
Task 1: 1s
Task 2: 500ms
Task 2: 500ms
Task 1: 1s
Task 2: 500ms
Task 1: 1s
Task 1: 1s
```

## 3 Echo Server 示例 {#echo_server}

```cpp
#include <rmvl/io/socket.hpp>

using namespace rm;

async::Task<> session(async::StreamSocket socket) {
    std::string str = co_await socket.read();
    if (str.empty()) {
        printf("Read failed\n");
        co_return;
    }
    bool success = co_await socket.write(str);
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
            co_spawn(io_context, session, std::move(socket));
        }
    });

    io_context.run();
    return 0;
}
```

可以使用 TCP 连接工具（如 `netcat`）进行测试：

```bash
nc localhost 8080
```
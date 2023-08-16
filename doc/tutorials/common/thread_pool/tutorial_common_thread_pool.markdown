线程与线程池管理 {#tutorial_common_thread_pool}
============

@author RoboMaster Vision Community
@date 2022/02/09

@prev_tutorial{tutorial_common_singleton}

@next_tutorial{tutorial_common_camera}

@tableofcontents

------

相关类 rm::ThreadPool, rm::PriorityThreadPool

## 1. 引言

自从线程支持库 `std::thread` 被添加到 C++11 标准中以来，C++ 摆脱了不支持并发编程的历史。然而，与其他编程语言相比，C++ 的多线程支持程度较低,像线程池等概念在标准库中没有实现。现在，在 RMVL 中已经添加了用于线程池的库，以便于开发异步编程。

## 2. 操作原则

### 2.1 基本概念

##### 互斥锁 Mutex

互斥锁（Mutexes）用于确保共享数据操作的完整性。每个对象对应一个称为互斥锁的标记，用于确保在任何给定时间只有一个线程可以访问该对象。当多个线程访问同一块内存时，首先访问内存的线程会对该空间进行锁定，后续的线程由于互斥锁而被阻塞。

可参考:

- <a href="https://en.cppreference.com/w/cpp/thread/mutex" target="_blank">
    Document of std::mutex
  </a>
- <a href="https://zh.cppreference.com/w/cpp/thread/mutex" target="_blank">
    std::mutex 中文文档
  </a>

##### 条件变量 Condition variable

条件变量（Condition variable）是使用在线程之间共享的全局变量进行同步的另一种机制。它主要由两个动作组成：一个线程会 **阻塞** 以等待条件变量满足某一条件，而另一个线程则使条件变量满足条件，即发送信号表示条件已满足。

可参考:

- <a href="https://en.cppreference.com/w/cpp/thread/condition_variable" target="_blank">
    Document of std::condition_variable
  </a>
- <a href="https://zh.cppreference.com/w/cpp/thread/condition_variable" target="_blank">
    std::condition_variable 中文文档
  </a>

##### 任务队列 Task queue

任务队列（Task queue）用于存储按顺序执行的任务。每个任务通常是一个没有参数和返回值的 `void(*)(void)` 函数。通常，我们使用 `push()` 方法将函数指针推入任务队列中，线程池中的线程在执行完任务后会自动从队列中弹出以执行下一个任务。

### 2.2 流程图

RMVL 中线程池的简化工作流程图如下

<center>
  <a href="https://imgse.com/i/xqJxcF" target="_blank">
    <img src="https://s1.ax1x.com/2022/11/03/xqJxcF.png" width=500 height=521/>
  </a>
</center>

## 3. C++ 语言细节

如上所述，任务通常是无参数、无返回的函数。

函数原型：

```cpp
template <typename _Task>
void addTask(_Task &&task);
```

我们可以使用 `lambda` 表达式或 `bind` 表达式来构造这个函数，例如

```cpp
tp.addTask(bind(
    [](vector<int> &m)
    {
        for (auto &element : m)
            element += 10;
    }, ref(mat)));
```

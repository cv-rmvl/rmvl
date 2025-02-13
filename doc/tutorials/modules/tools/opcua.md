工业自动化通信协议 OPC UA {#tutorial_modules_opcua}
============

@author 赵曦
@date 2024/12/31
@version 2.1
@brief OPC UA 和 open62541 库简介

@prev_tutorial{tutorial_modules_serial}

@next_tutorial{tutorial_modules_camera}

@tableofcontents

------

相关模块： @ref opcua

## 1. 简介 {#tutorial_opcua_intro}

### 1.1 OPC UA 是什么 {#tutorial_opcua_intro_what}

[OPC UA](https://opcfoundation.org/about/opc-technologies/opc-ua/)（全称为 Open Platform Communications Unified Architecture）是一种用于工业和物联网（IoT）应用的开放通信协议和架构。它提供了一种统一的框架，用于在不同设备和系统之间实现数据传输、通信和集成。

OPC UA 的设计目标是建立一种通用的、独立于厂商和平台的通信标准，以实现互操作性和集成性。它提供了一套标准的服务和功能，使不同类型的设备和系统能够相互通信和交换数据，其特点包括：

<div class="full_width_table">

|   特点   |                             介绍                             |
| :------: | :----------------------------------------------------------: |
|  兼容性  | OPC UA 不依赖于特定的硬件、操作系统或网络协议，可以在不同的平台上运行，并与其他通信标准集成 |
|  安全性  | OPC UA 提供了强大的安全机制，包括身份验证、加密和访问控制，以确保数据和通信的机密性和完整性 |
|  可扩展  | OPC UA 支持灵活的数据模型和信息建模，可以适应不同应用领域和需求的变化 |
| 信息建模 | OPC UA 使用统一的信息模型，将数据和功能以标准化的方式表示和描述，使不同系统之间的数据交换更加简化和一致 |
|  可靠性  | OPC UA 提供了可靠的通信机制，包括消息确认、重试和错误处理，以确保数据的可靠传输 |

</div>

### 1.2 地址空间 {#tutorial_opcua_intro_address_space}

在 OPC UA 中，所有的数据都被组织成一个地址空间，地址空间中的每一个元素都被称为一个节点。每个节点都有一个唯一的节点号，在 @ref opcua 中表示为 rm::NodeId 。

<center>

![图 1-1 OPC UA 地址空间模型](opcua.svg)

</center>

**对象类型节点 rm::ObjectType**

提供对象的定义，即对象的抽象，与类相当，且子类可以继承父类的特征，方便模型的扩充。该节点包括对象的各种数据类型，数据的语义，以及控制方式。OPC UA 命名空间 `0` 中规定了多个基础的对象类型节点。如使用最广的 BaseObjectType（在 RMVL 中表示为 `rm::nodeBaseObjectType`），所有对象类型节点都需要继承该节点再进行扩充。在对具体设备建模的过程中，应该将设备组成的各部分分解为不同的对象分别建模，再用引用节点将各部分按照实际设备中的关系相关联，从而得到完整设备的对象类型节点。

**对象节点 rm::Object**

将对象类型实例化即可得到对象节点，该节点是设备在数字空间的映射。所有对设备数据的访问都能在该模型中访问到对应的数据节点。所有对 设备的控制都转换为方法节点的触发。设备产生的消息在节点对象中将触发对应的事件。

**引用类型节点 ReferenceType**

引用类型描述了引用的语义，而引用用于定义引用两端的节点之间的关系。最常用的引用类型如 Organizes（在 RMVL 中表示为 `rm::nodeOrganizes`），表示节点之间的层级关系，如同文件夹与文件夹内的文件，数据层级复杂的设备，需要通过多种引用类型对设备信息节点之间的关系进行描述。

**数据类型节点 rm::DataType**

数据类型节点描述了变量节点中变量的数据类型。在 OPC UA 信息模型在命名空间 `0` 中定义了多种内置的数据类型，包括整型、浮点型、 字符串等多个类型，能对变量的数据进行准确的描述。也可以自定义数据类型，比如描述二维坐标的 `2DPoint` 等类型，获得更符合数据本身的描述。@note 注意：此类节点并不能提供具体的数据构成，只是提供了数据类型的一个描述，因此 RMVL 中的 @ref opcua 仅提供内置数据类型。若计划提供数据的构成，比如包含的数据长度等信息，请使用变量类型节点 rm::VariableType 。

**变量类型节点 rm::VariableType**

该节点提供了对变量节点的定义，是设备中各种数据的抽象。常用引用中的 HasTypeDefinition 引用节点连接数据类型节点，对数据类型进行描述（在 RMVL 中表示为 `rm::nodeHasTypeDefinition`）。用 HasProperty 引用节点对数据的语义进行描述（在 RMVL 中表示为 `rm::nodeHasProperty`）。也可以使用自定义的数据类型节点对变量的数据进行描述，具有灵活性。

**变量节点 rm::Variable 及 rm::DataSourceVariable**

该节点是变量类型节点的实例，也是使用的最多的节点。客户端访问设备数据有以下 3 种方式。

<div class="full_width_table">

|    访问方式    |                             介绍                             |                             备注                             |
| :------------: | :----------------------------------------------------------: | :----------------------------------------------------------: |
|    直接读写    | 将设备多模态数据写入对应的变量节点，然后客户端读取对应节点内保存的数值 | 如果客户端要获取设备最新的值，需要一直手动去触发对设备数据源的读取请求 |
|     值回调     | 客户端发起 **IO** 请求后，服务器在 **读取前** 和 **写入后** 分别调用对应的回调函数 |     可以利用此功能在需要访问数据的时候才让服务器更新数据     |
| 数据源变量节点 | 客户端的读取请求直接重定向到设备的数据源中，即客户端直接从数据源获取数据，变量节点不存储数据 | 缩减了数据先写入变量节点再进行读取的过程，但多个客户端连接访问同一数据时会增大服务器与设备之间的传输负载 |

</div>

@note 前两种访问方式在 @ref opcua 中通过 rm::Variable 实现，第三种数据源变量节点在 @ref opcua 中通过 rm::DataSourceVariable 实现。

**方法节点 rm::Method**

方法节点是对设备控制方法在数字模型中的映射。方法节点可以通过服务器或客户端进行调用，然后将会对设备的控制器发送指令，使得设备执行对应的操作。常见的方法节点有：触发视觉采集、电机反转、设备初始化等。

**视图节点 rm::View**

视图节点可将地址空间中感兴趣的节点提取出来，作为一个子集，视图节点作为该子集的入口，方便客户端浏览。

## 2. 服务器/客户端 {#tutorial_opcua_server_client}

基于服务器/客户端的方式是 OPC UA 最基本的一种通信方式，上文的地址空间在服务器/客户端通信的过程中完全展现出来。下面列举一些 opcua 模块中常用的服务器与客户端通信的内容。

### 2.1 初始化

**服务器**

@add_toggle_cpp

**方案一**

使用 `spin` 作为事件循环，并提供全局变量 `p_server` 用于在信号处理函数中关闭服务器。

```cpp
// server.cpp
#include <csignal>
#include <rmvl/opcua/server.hpp>

rm::Server *p_server{nullptr};

void onStop(int)
{
    if (p_server)
        p_server->shutdown();
}

int main()
{
    // 注册信号处理函数
    signal(SIGINT, onStop);

    // 创建 OPC UA 服务器，端口为 4840
    rm::Server srv(4840);
    p_server = &srv;
    // 服务器运行
    srv.spin();
}
```

**方案二**

使用多线程，主线程负责关闭服务器。

```cpp
// server.cpp
#include <thread>
#include <rmvl/opcua/server.hpp>

int main()
{
    // 创建 OPC UA 服务器，端口为 4840
    rm::Server srv(4840);
    // 服务器运行
    std::thread t(&rm::Server::spin, &srv);

    /* other code */

    srv.shutdown();
    t.join();
}
```

**方案三**

使用 `spinOnce` 执行单次事件循环，需要自定义主循环，在某个条件下退出循环，并关闭服务器（此时可不必显式调用 `shutdown`）

```cpp
// server.cpp
#include <csignal>
#include <rmvl/opcua/server.hpp>

bool stop = false;

int main()
{
    signal(SIGINT, [](int) { stop = true; });

    // 创建 OPC UA 服务器，端口为 4840
    rm::Server srv(4840);
    // 服务器运行
    while (!stop)
    {
        /* other code */
        srv.spinOnce();
    }
    srv.shutdown(); // 可省略，因为在 rm::Server::~Server 析构函数中将自动调用 shutdown
                    // 但是上文的方案一和方案二中需要手动将循环标志 _running 设置为 false，这一步是在 shutdown 中完成的，因此需要显式调用 shutdown
}
```

@end_toggle

@add_toggle_python

Python 的信号处理机制依赖于 Python 解释器的 GIL，因此在 Python 中无法通过常规方式结束 `spin`（除非直接结束进程），但是可以使用 `spinOnce` 执行单次事件循环，因此需要自定义主循环。

```python
# server.py
from signal import signal, SIGINT
import rm

stop = False

def onStop(sig, frame):
    global stop
    stop = True

signal(SIGINT, onStop)

# 创建 OPC UA 服务器，端口为 4840
srv = rm.Server(4840)
# 服务器运行
while not stop:
    # other code
    srv.spinOnce()
```

@end_toggle

**客户端**

@add_toggle_cpp

```cpp
// client.cpp
#include <rmvl/opcua/client>

int main()
{
    // 创建 OPC UA 客户端，连接到 127.0.0.1:4840
    rm::Client cli("opc.tcp://127.0.0.1:4840");

    /* other code */
}
```

@end_toggle

@add_toggle_python

```python
# client.py
import rm

# 创建 OPC UA 客户端，连接到
cli = rm.Client("opc.tcp://127.0.0.1:4840")
```

@end_toggle

### 2.2 变量

在上文 @ref tutorial_opcua_intro_address_space 中介绍了变量的 3 种访问方式，这里使用最简单的直接读写的方式。首先在服务器中添加变量节点，后文均采用 `while + spinOnce` 的方式处理异步事件。

@add_toggle_cpp

```cpp
// server.cpp
#include <csignal>
#include <thread>
#include <rmvl/opcua/server.hpp>

static bool stop = false;

int main()
{
    signal(SIGINT, [](int) { stop = true; });

    rm::Server srv(4840);

    // 定义 double 型变量，如果要强制使用 3.14 定义 float 型变量，
    // 可以使用 rm::Variable num = float(3.14);
    rm::Variable num = 3.14;
    // 浏览名 BrowseName
    num.browse_name = "number";
    // 显示名 DisplayName
    num.display_name = "Number";
    // 描述
    num.description = "数字";
    // 添加到服务器的默认位置（默认被添加至 ObjectsFolder 下）
    srv.addVariableNode(num);

    while (!stop)
    {
        srv.spinOnce();
    }
    // srv.shutdown(); 可省略，后文不再赘述
}
```

@end_toggle

@add_toggle_python

```python
# server.py
from signal import signal, SIGINT
import rm

stop = False

def onStop(sig, frame):
    global stop
    stop = True

signal(SIGINT, onStop)

"""
RMVL-Python 的 opcua 模块仅支持

- int, float, str, bool
- list[int], list[float]

类型的变量
"""

svr = rm.Server(4840)
# 定义 float 型变量
num = rm.Variable(3.14)
# 浏览名 BrowseName
num.browse_name = "number"
# 显示名 DisplayName
num.display_name = "Number"
# 描述
num.description = "数字"
# 添加到服务器的默认位置（默认被添加至 ObjectsFolder 下）
svr.addVariableNode(num)

while not stop:
    svr.spinOnce()
# srv.shutdown() 可省略，后文不再赘述
```

@end_toggle

然后在客户端中直接读取变量节点。

@add_toggle_cpp

```cpp
// client.cpp
#include <rmvl/opcua/client.hpp>

int main()
{
    rm::Client cli("opc.tcp://127.0.0.1:4840");

    // 使用管道运算符 "|" 进行路径搜索，寻找待读取的变量
    auto node = rm::nodeObjectsFolder | cli.node("number");
    // 或者可以直接使用 find 方法，寻找 rm::nodeObjectsFolder 下命名空间为 1 的节点（更推荐！）
    // auto node = cli.find("number");
    // 读取变量
    rm::Variable target = cli.read(node);
    // 判断是否为空
    if (target.empty())
    {
        ERROR_("Failed to read the variable.");
        return 0;
    }
    // 使用静态成员函数将 target 转化为目标格式，并打印
    printf("%f\n", rm::Variable::cast<double>(target));
    // 或者直接使用 target 的成员函数进行转化
    // printf("%f\n", target.cast<double>());
}
```

@end_toggle

@add_toggle_python

```python
# client.py
import rm

cli = rm.Client("opc.tcp://127.0.0.1:4840")
# 路径搜索，寻找待读取的变量
node = cli.find("number")
# 读取变量
target = cli.read(node)
# 判断是否为空
if target.empty():
    print("Failed to read the variable.")
    exit(0)
# 使用静态成员函数将 target 转化为目标格式，并打印
print(target.double())
```

@end_toggle

### 2.3 方法

在服务器中添加两数之和的方法节点，供客户端调用。

@add_toggle_cpp

```cpp
// server.cpp
#include <csignal>
#include <rmvl/opcua/server.hpp>

using namespace std::chrono_literals;

static bool stop = false;

int main()
{
    signal(SIGINT, [](int) { stop = true; });

    rm::Server srv(4840);

    // 定义方法，初始化或设置 rm::Method::func 成员必须使用以下兼容形式的可调用对象
    // std::function<pair<bool, rm::Variables>(const rm::NodeId &, const rm::Variables &)>
    // 其中 rm::Variables 是 std::vector<rm::Variable> 的别名
    rm::Method method = [](const rm::NodeId &, const rm::Variables &iargs) {
        int num1 = iargs[0], num2 = iargs[1];
        rm::Variables oargs = {num1 + num2};
        return std::make_pair(true, oargs);
    };
    method.browse_name = "add";
    method.display_name = "Add";
    method.description = "两数之和";
    // 定义函数传入参数 iargs 的类型说明
    method.iargs = {{"Number 1", rm::tpInt32},
                    {"Number 2", rm::tpInt32}};
    // 也可以使用 create 工厂函数创建参数，后文不再赘述
    // method.iargs = {rm::Argument::create("Number 1", rm::tpInt32), 
    //                 rm::Argument::create("Number 2", rm::tpInt32)};

    // 定义函数返回值 oargs 的类型说明
    method.oargs = {{"Sum", rm::tpInt32}};

    /*
        1. 数据类型也可使用在 open62541 中定义的 UA_TYPES_ 作为前缀的宏，如 rm::tpInt32 可使用 UA_TYPES_INT32 宏
        2. {"Number 1", rm::tpInt32} 的部分是 rm::Argument 的聚合类，表示方法的参数
        3. 允许有多个返回值，即 oargs 的长度允许 > 1
    */

    // 方法节点添加至服务器
    srv.addMethodNode(method);
    
    while (!stop)
    {
        srv.spinOnce();
    }
}
```

@end_toggle

@add_toggle_python

```python
# server.py
from signal import signal, SIGINT
import rm

stop = False

def onStop(sig, frame):
    global stop
    stop = True

signal(SIGINT, onStop)

svr = rm.Server(4840)

# 定义方法，初始化或设置 rm.Method.func 成员必须使用以下形式的可调用对象
# Callable[[NodeId, Variables], tuple[bool, Variables]]
# 其中 Variables 是 list[Variable] 的别名
def add(nd, iargs):
    num1, num2 = iargs
    oarg = rm.Variable(num1.int() + num2.int())
    return True, [oarg]


method = rm.Method(add)
method.browse_name = "add"
method.display_name = "Add"
method.description = "两数之和"
# 定义函数传入参数 iargs 的类型说明
iarg1 = rm.Argument.create("Number 1", rm.tp_int)
iarg2 = rm.Argument.create("Number 2", rm.tp_int)
method.iargs = [iarg1, iarg2]
# 定义函数返回值 oargs 的类型说明
oarg = rm.Argument.create("Sum", rm.tp_int)
method.oargs = [oarg]

"""
允许有多个返回值，即 oargs 的长度允许 > 1
"""

# 方法节点添加至服务器
svr.addMethodNode(method)

while not stop:
    svr.spinOnce()
```

@end_toggle

在客户端调用指定方法。

@add_toggle_cpp

```cpp
// client.cpp
#include <rmvl/opcua/client.hpp>

int main()
{
    rm::Client cli("opc.tcp://127.0.0.1:4840");

    // 设置输入参数，1 和 2 是 Int32 类型的，因此可以直接隐式构造
    rm::Variables iargs = {1, 2};
    // 调用方法，判断调用是否成功，并存储结果
    auto [res, oargs] = cli.call("add", iargs);
    if (!res)
        ERROR_("Failed to call the method");
    else
        printf("retval = %d\n", oargs.front().cast<int>());
}
```

此外，还可以使用 `Client::callx` 方法的变参模板直接从底层数据进行调用，例如

```cpp
// client.cpp
#include <rmvl/opcua/client.hpp>

int main()
{
    rm::Client cli("opc.tcp://127.0.0.1:4840");

    auto [res, oargs] = cli.callx("add", 1, 2);
    if (!res)
        ERROR_("Failed to call the method");
    else
        printf("retval = %d\n", oargs.front().cast<int>());
}
```

@end_toggle

@add_toggle_python

```python
# client.py
import rm

cli = rm.Client("opc.tcp://127.0.0.0:4840")

# 设置输入参数
iargs = [rm.Variable(1), rm.Variable(2)]
# 调用方法，判断调用是否成功，并存储结果
res, oargs = cli.call("add", iargs)
if not res:
    print("Failed to call the method")
else:
    print(f"retval = {oargs[0].int()}")
```

@end_toggle

### 2.4 对象

在服务器中添加对象节点：

```
A
├── B1
│   ├── C1: 3.14
│   └── C2: 666
└── B2
    └── C3: "xyz"
```

@add_toggle_cpp

```cpp
// server.cpp
#include <csignal>
#include <rmvl/opcua/server.hpp>

using namespace std::chrono_literals;

static bool stop = false;

int main()
{
    signal(SIGINT, [](int) { stop = true; });

    rm::Server srv(4840);
    // 准备对象节点数据 A
    rm::Object a;
    a.browse_name = a.description = a.display_name = "A";
    // 添加对象节点 A 至服务器
    auto node_a = srv.addObjectNode(a);
    // 准备对象节点数据 B1
    rm::Object b1;
    b1.browse_name = b1.description = b1.display_name = "B1";
    // 准备 B1 的变量节点 C1
    rm::Variable c1 = 3.14;
    c1.browse_name = c1.description = c1.display_name = "C1";
    b1.add(c1);
    // 准备 B1 的变量节点 C2
    rm::Variable c2 = 666;
    c2.browse_name = c2.description = c2.display_name = "C2";
    b1.add(c2);
    // 添加对象节点 B1 至服务器
    srv.addObjectNode(b1, node_a);
    // 准备对象节点数据 B2
    rm::Object b2;
    b2.browse_name = b2.description = b2.display_name = "B2";
    // 准备 B2 的变量节点 C3
    rm::Variable c3 = "xyz";
    c3.browse_name = c3.description = c3.display_name = "C3";
    b2.add(c3);
    // 添加对象节点 B2 至服务器
    srv.addObjectNode(b2, node_a);

    while (!stop)
        srv.spinOnce();
}
```

@end_toggle

@add_toggle_python

```python
# server.py

from signal import signal, SIGINT
import rm

stop = False

def onStop(sig, frame):
    global stop
    stop = True

signal(SIGINT, onStop)

svr = rm.Server(4840)
# 准备对象节点数据 A
a = rm.Object()
a.browse_name = a.description = a.display_name = "A"
# 添加对象节点 A 至服务器
node_a = svr.addObjectNode(a)
# 准备对象节点数据 B1
b1 = rm.Object()
b1.browse_name = b1.description = b1.display_name = "B1"
# 准备 B1 的变量节点 C1
c1 = rm.Variable(3.14)
c1.browse_name = c1.description = c1.display_name = "C1"
b1.add(c1)
# 准备 B1 的变量节点 C2
c2 = rm.Variable(666)
c2.browse_name = c2.description = c2.display_name = "C2"
b1.add(c2)
# 添加对象节点 B1 至服务器
svr.addObjectNode(b1, node_a)
# 准备对象节点数据 B2
b2 = rm.Object()
b2.browse_name = b2.description = b2.display_name = "B2"
# 准备 B2 的变量节点 C3
c3 = rm.Variable("xyz")
c3.browse_name = c3.description = c3.display_name = "C3"
b2.add(c3)
# 添加对象节点 B2 至服务器
svr.addObjectNode(b2, node_a)

while not stop:
    svr.spinOnce()
```

@end_toggle

在客户端寻找 `C2` 和 `C3` 并打印。

@add_toggle_cpp

```cpp
// client.cpp
#include <iostream>
#include <rmvl/opcua/client.hpp>

int main()
{
    rm::Client cli("opc.tcp://127.0.0.1:4840");

    // 路径搜索寻找 C2
    auto node_c2 = rm::nodeObjectsFolder | cli.node("A") | cli.node("B1") | cli.node("C2");
    // 也可以直接使用 find 方法，寻找 rm::nodeObjectsFolder 下的节点（更推荐！）
    // auto node_c2 = cli.find("A/B1/C2");
    rm::Variable c2 = cli.read(node_c2);
    std::cout << c2.cast<int>() << std::endl;
    // 路径搜索寻找 C3
    auto node_c3 = cli.find("A/B2/C3");
    rm::Variable c3 = cli.read(node_c3);
    std::cout << c3.cast<std::string>() << std::endl;
}
```

@end_toggle

@add_toggle_python

```python
# client.py
import rm

cli = rm.Client("opc.tcp://127.0.0.1:4840")

# 路径搜索寻找 C2
node_c2 = cli.find("A/B1/C2")
c2 = cli.read(node_c2)
print(c2.int())
# 路径搜索寻找 C3
node_c3 = cli.find("A/B2/C3")
c3 = cli.read(node_c3)
print(c3.str())
```

@end_toggle

### 2.5 视图

在 `nodeObjectsFolder` 中先添加 `A/num1`、`num2` 2 个变量节点，并将 `num1` 和 `num2` 加入视图，下面的示例演示在 **服务器** 中创建并添加视图节点。若要在客户端中进行此操作，创建并添加视图节点的步骤基本一致，这里不做展示。需要注意的是，在客户端中创建并添加视图节点，需要提前在服务器中加入对应的（变量、方法、对象……）节点

@add_toggle_cpp

```cpp
// server.cpp
#include <csignal>
#include <rmvl/opcua/server.hpp>

using namespace std::chrono_literals;

static bool stop = false;

int main()
{
    signal(SIGINT, [](int) { stop = true; });

    rm::Server srv(4840);
    // 准备对象节点数据 A
    rm::Object a;
    a.browse_name = a.description = a.display_name = "A";
    // 创建 num1 变量节点
    rm::Variable num1 = 1;
    num1.browse_name = "num1";
    num1.display_name = "num1";
    num1.description = "num1";
    a.add(num1);
    auto node_a = srv.addObjectNode(a);
    auto node_num1 = srv.find("A/num1");
    // 这里稍微展示一下，使用宏来创建 num2，这里也可以使用上文的方式创建 :)
    uaCreateVariable(num2, 2);
    auto node_num2 = srv.addVariableNode(num2);

    // 创建视图
    rm::View num_view;
    // 添加节点至视图（这里使用的是变量节点的 NodeId，实际上其他节点也是允许的）
    num_view.add(node_num1, node_num2);
    // 添加至服务器
    srv.addViewNode(num_view);
    
    while (!stop)
        srv.spinOnce();
}
```

@end_toggle

@add_toggle_python

```python
# server.py

from signal import signal, SIGINT
import rm

stop = False

def onStop(sig, frame):
    global stop
    stop = True

signal(SIGINT, onStop)

svr = rm.Server(4840)
# 准备对象节点数据 A
a = rm.Object()
a.browse_name = a.description = a.display_name = "A"
# 创建 num1
num1 = rm.Variable(1)
num1.browse_name = num1.description = num1.display_name = "num1"
a.add(num1)
node_a = svr.addObjectNode(a)
node_num1 = svr.find("A/num1")
# 创建 num2
num2 = rm.Variable(2)
num2.browse_name = num2.description = num2.display_name = "num2"
node_num2 = svr.addVariableNode(num2)

# 创建视图
num_view = rm.View()
# 添加节点至视图
num_view.add(node_num1, node_num2)
# 添加至服务器
svr.addViewNode(num_view)

while not stop:
    svr.spinOnce()
```

@end_toggle

### 2.6 监视

OPC UA 支持变量节点和事件的监视，下面分别以变量节点和事件的监视为例。

#### 2.6.1 变量监视

首先在服务器中添加待监视的变量节点

@add_toggle_cpp

```cpp
// server.cpp
#include <csignal>
#include <rmvl/opcua/server.hpp>

using namespace std::chrono_literals;

static bool stop = false;

int main()
{
    signal(SIGINT, [](int) { stop = true; });

    rm::Server srv(4840);

    // 定义 int 型变量
    rm::Variable num = 100;
    num.browse_name = "number";
    num.display_name = "Number";
    num.description = "数字";
    // 添加到服务器的默认位置
    srv.addVariableNode(num);

    while (!stop)
        srv.spinOnce();
}
```

@end_toggle

@add_toggle_python

```python
# server.py

from signal import signal, SIGINT
import rm

stop = False

def onStop(sig, frame):
    global stop
    stop = True

signal(SIGINT, onStop)

svr = rm.Server(4840)
# 定义 int 型变量
num = rm.Variable(100)
num.browse_name = "number"
num.display_name = "Number"
num.description = "数字"
# 添加到服务器的默认位置
svr.addVariableNode(num)

while not stop:
    svr.spinOnce()
```

@end_toggle

在客户端 1 中修改变量节点的数据

@add_toggle_cpp

```cpp
// client_1.cpp
#include <rmvl/opcua/client.hpp>
#include <rmvl/core/timer.hpp>

using namespace std::chrono_literals;

int main()
{
    rm::Client cli("opc.tcp://127.0.0.1:4840");
    auto node = cli.find("number");
    for (int i = 0; i < 100; ++i)
    {
        TImer::sleep_for(1000);
        // 写入数据，i + 200 隐式构造成了 rm::Variable
        bool success = cli.write(node, i + 200);
        if (!success)
            ERROR_("Failed to write data to the variable.");
    }
}
```

@end_toggle

@add_toggle_python

```python
# client_1.py

import rm
import time

cli = rm.Client("opc.tcp://127.0.0.1:4840")
node = cli.find("number")
for i in range(100):
    time.sleep(1)
    # 写入数据
    success = cli.write(node, rm.Variable(i + 200))
    if not success:
        print("Failed to write data to the variable.")
```

@end_toggle

然后，在客户端 2 中监视变量节点

@add_toggle_cpp

```cpp
// client_2.cpp
#include <rmvl/opcua/client.hpp>

int main()
{
    rm::Client cli("opc.tcp://127.0.0.1:4840");
    auto node = cli.find("number");
    // 监视变量
    auto on_change = [](rm::ClientView, const rm::Variable &value) {
        int receive_data = value;
        printf("Data (n=number) was changed to: %d\n", receive_data);
    };
    cli.monitor(node, on_change, 5);
    // 线程阻塞
    cli.spin();
}
```

@end_toggle

@add_toggle_python

```python
# client_2.py

from signal import signal, SIGINT
import rm

stop = False

def onStop(sig, frame):
    global stop
    stop = True

cli = rm.Client("opc.tcp://127.0.0.1:4840")
node = cli.find("number")

# 监视变量
def on_change(view, value):
    receive_data = value.int()
    print(f"Data (n=number) was changed to: {receive_data}")

cli.monitor(node, on_change, 5)

while not stop:
    cli.spinOnce()
```

@end_toggle

#### 2.6.2 事件监视

事件需要服务器自主触发，在实际应用中，事件的触发可以是设备状态的变化、设备的报警等，例如客户端通过调用方法节点，服务端修改自身状态机状态，任务执行完毕，状态再次发生改变后将触发事件，可以实现<span style="color: green">同步非阻塞</span>向<span style="color: green">同步阻塞</span>的转换。例如服务器用于 **异步的控制设备启动、关闭** ，客户端通过调用方法节点控制设备启动。

首先在服务器中添加

- 对应的方法节点用于发出启动、关闭指令；
- 实际发出启动、关闭指令的<span style="color: red">伪代码</span>。

如下所示。

@add_toggle_cpp

```cpp
// server.cpp
#include <csignal>

#include <rmvl/opcua/server.hpp>
#include <thread>

using namespace std::chrono_literals;

static bool stop = false;

// OPC UA 状态
enum class OPCUAState
{
    NONE,  // 无状态
    START, // 设备启动中...
    STOP,  // 设备关闭中...
};

int main()
{
    // OPC UA 状态
    OPCUAState mode{};

    // 消息事件类型
    rm::EventType msg_type_info;
    msg_type_info.browse_name = "msg_type";
    msg_type_info.display_name = "MsgType";
    msg_type_info.description = "任务执行完成时触发的事件";
    msg_type_info.add("Result", 0);
    auto msg_info = rm::Event::makeFrom(msg_type_info);

    // 启动设备
    rm::Method start_info = [&](const rm::NodeId &, const rm::Variables &) -> std::pair<bool, rm::Variables> {
        if (mode != OPCUAState::NONE)
            return {false, {}};
        mode = OPCUAState::START;
        return {true, {}};
    };
    start_info.browse_name = "start";
    start_info.display_name = "Start";
    start_info.description = "启动设备";

    // 关闭设备
    rm::Method stop_info = [&](const rm::NodeId &, const rm::Variables &) -> std::pair<bool, rm::Variables> {
        if (mode != OPCUAState::NONE)
            return {false, {}};
        mode = OPCUAState::STOP;
        return {true, {}};
    };
    stop_info.browse_name = "stop";
    stop_info.display_name = "Stop";
    stop_info.description = "关闭设备";

    // 服务器
    signal(SIGINT, [](int) { stop = true; });
    rm::Server srv(4840);
    srv.addEventTypeNode(msg_type_info);
    srv.addMethodNode(start_info);

    while (!stop)
    {
        srv.spinOnce();
        if (mode == OPCUAState::START)
        {
            // 实际发出 Start 指令

            /* code */

            if (true) // 'true' 应改为状态确定发生变更的判断条件
            {
                msg_info.message = "Msg_Start";
                msg_info["Result"] = 0;
                srv.triggerEvent(msg_info);
                mode = OPCUAState::NONE; // 恢复 OPC UA 状态
            }
        }
        else if (mode == OPCUAState::STOP)
        {
            // 实际发出 Stop 指令

            /* code */

            if (true) // 'true' 应改为状态确定发生变更的判断条件
            {
                msg_info.message = "Msg_Stop";
                msg_info["Result"] = 0;
                srv.triggerEvent(msg_info);
                mode = OPCUAState::NONE; // 恢复 OPC UA 状态
            }
        }
    }
}
```

@end_toggle

@add_toggle_python

```python
# server.py
from signal import signal, SIGINT
from enum import Enum
import rm

stop = False

# OPC UA 状态
class OPCUAState(Enum):
    NONE = 0  # 无状态
    START = 1 # 设备启动中...
    STOP = 2  # 设备关闭中...

mode = OPCUAState.NONE

# 事件类型
msg_type_info = rm.EventType()
msg_type_info.browse_name = "msg_type"
msg_type_info.display_name = "MsgType"
msg_type_info.description = "任务执行完成时触发的事件"
msg_type_info.add("Result", 0)
msg_info = rm.Event.makeFrom(msg_type_info)

# 启动设备
def start_cb(sv, iargs):
    global mode
    if mode != OPCUAState.NONE:
        return False, {}
    mode = OPCUAState.START
    return True, {}

start_info = rm.Method(start_cb)
start_info.browse_name = "start"
start_info.display_name = "Start"
start_info.description = "启动设备"

# 关闭设备
def stop_cb(nd, iargs):
    global mode
    if mode != OPCUAState.NONE:
        return False, {}
    mode = OPCUAState.STOP
    return True, {}

stop_info = rm.Method(stop_cb)
stop_info.browse_name = "stop"
stop_info.display_name = "Stop"
stop_info.description = "关闭设备"

# 服务器
def onStop(sig, frame):
    global stop
    stop = True

signal(SIGINT, onStop)

svr = rm.Server(4840)
svr.addEventTypeNode(msg_type_info)
svr.addMethodNode(start_info)

while not stop:
    svr.spinOnce()
    if mode == OPCUAState.START:
        # 实际发出 Start 指令
        """
        code
        """

        if True: # 'True' 应改为状态确定发生变更的判断条件
            msg_info.message = "Msg_Start"
            msg_info["Result"] = 0
            svr.triggerEvent(msg_info)
            mode = OPCUAState.NONE # 恢复 OPC UA 状态
    elif mode == OPCUAState.STOP:
        # 实际发出 Stop 指令
        """
        code
        """

        if True: # 'True' 应改为状态确定发生变更的判断条件
            msg_info.message = "Msg_Stop"
            msg_info["Result"] = 0
            svr.triggerEvent(msg_info)
            mode = OPCUAState.NONE # 恢复 OPC UA 状态
```

@end_toggle

正常情况下，客户端调用方法节点会立刻返回，如以下代码

@add_toggle_cpp

```cpp
// client_old.cpp
#include <rmvl/opcua/client.hpp>

int main()
{
    rm::Client cli("opc.tcp://127.0.0.1:4840");
    auto node = cli.find("start");
    auto [res, oargs] = cli.call(node, {});
    if (!res) // res 只表示方法节点是否调用成功，而非任务执行结果
        ERROR_("Failed to call the method");
}
```

@end_toggle

@add_toggle_python

```python
# client_old.py
import rm

cli = rm.Client("opc.tcp://127.0.0.1:4840")
node = cli.find("start")
res, oargs = cli.call(node, [])
if not res: # res 只表示方法节点是否调用成功，而非任务执行结果
    print("Failed to call the method")
```

@end_toggle

此时返回的结果表示该方法节点是否调用成功，并非任务执行结果，而真实的执行结果在多个事件循环后才能得到。不过服务器在任务执行完成后会触发事件，客户端可以通过监视事件来获取任务执行结果，如以下代码。

@add_toggle_cpp

```cpp
// client_new.cpp
#include <rmvl/opcua/client.hpp>

class OpcUaController
{
public:
    OpcUaController(std::string_view addr) : _cli(addr) {
        // 监视事件
        _cli.monitor({"Message", "Result"}, [this](rm::ClientView, const rm::Variables &vals) {
            if (vals[0] == "Msg_Start")
                _start_res = (vals[1] == 0);
            else if (vals[0] == "Msg_Stop")
                _stop_res = (vals[1] == 0);
        });
    }

    // 同步阻塞的 start 函数
    bool start()
    {
        _start_res.reset();
        auto [res, oargs] = _cli.call("Start", {});
        if (!res)
        {
            printf("Failed to call start\n");
            return false;
        }
        while (!_start_res.has_value())
            _cli.spinOnce();
        return _start_res.value();
    }

    // 同步阻塞的 stop 函数
    bool stop()
    {
        _stop_res.reset();
        auto [res, oargs] = _cli.call("Stop", {});
        if (!res)
        {
            printf("Failed to call stop\n");
            return false;
        }
        while (!_stop_res.has_value())
            _cli.spinOnce();
        return _stop_res.value();
    }

private:
    rm::Client _cli;

    std::optional<bool> _start_res{};
    std::optional<bool> _stop_res{};
};

int main()
{
    OpcUaController uactl("opc.tcp://127.0.0.1:4840");
    // 启动设备
    bool val = uactl.start();
    printf("Start result: %d\n", val);

    /* code */

    // 关闭设备
    val = uactl.stop();
    printf("Stop result: %d\n", val);
}
```

@end_toggle

@add_toggle_python

```python
# client_new.py
import rm

class OpcUaController:
    def __init__(self, addr):
        self.__cli = rm.Client(addr)
        self.__start_res = None
        self.__stop_res = None
        # 监视事件
        self.__cli.monitor(["Message", "Result"], self.on_event)

    def on_event(self, view, vals):
        if vals[0] == "Msg_Start":
            self.__start_res = vals[1] == 0
        elif vals[0] == "Msg_Stop":
            self.__stop_res = vals[1] == 0

    # 同步阻塞的 start 函数
    def start(self):
        res, oargs = self.__cli.call("Start", [])
        if not res:
            print("Failed to call start")
            return False
        while self.__start_res is None:
            self.__cli.spinOnce()
        return self.__start_res

    # 同步阻塞的 stop 函数
    def stop(self):
        res, oargs = self.__cli.call("Stop", [])
        if not res:
            print("Failed to call stop")
            return False
        while self.__stop_res is None:
            self.__cli.spinOnce()
        return self.__stop_res

uactl = OpcUaController("opc.tcp://127.0.0.1:4840")
# 启动设备
val = uactl.start()
print(f"Start result: {val}")

"""
code
"""

# 关闭设备
val = uactl.stop()
print(f"Stop result: {val}")
```

@end_toggle

### 2.7 定时

@ref opcua 为服务器和客户端均提供了循环定时器，用于周期性执行任务。下面的示例演示在 **服务器** 中创建并添加定时器。

@add_toggle_cpp

```cpp
// server.cpp
#include <csignal>
#include <rmvl/opcua/server.hpp>

bool stop = false;

int main()
{
    signal(SIGINT, [](int) { stop = true; });

    rm::Server srv(4840);

    int times{};
    // 创建定时器，每 1s 执行一次
    rm::ServerTimer timer(srv, 1000, [&](rm::ServerView) {
        // 定时器回调函数
        printf("Timer callback, times = %d\n", times);
        times++;
    });

    while (!stop)
        srv.spinOnce();
}
```

@end_toggle

@add_toggle_python

```python
# server.py

from signal import signal, SIGINT
import rm

stop = False

def onStop(sig, frame):
    global stop
    stop = True

signal(SIGINT, onStop)

svr = rm.Server(4840)

times = 0
# 创建定时器，每 1s 执行一次
def timer_callback(view):
    global times
    print(f"Timer callback, times = {times}")
    times += 1

timer = rm.ServerTimer(svr, 1000, timer_callback)

while not stop:
    svr.spinOnce()
```

@end_toggle

## 3. 发布/订阅 {#tutorial_opcua_pub_sub}

这是一段来自 [open62541 手册](https://www.open62541.org)中有关 PubSub 的介绍。

> 在 PubSub 中，参与的 OPC UA 应用程序扮演发布者和订阅者的角色。发布者是数据的来源，而订阅者则使用该数据。PubSub 中的通信是基于消息的。发布者将消息发送到面向消息的中间件，而不知道可能有哪些订阅者（如果有）。同样，订阅者表达对特定类型数据的兴趣，并处理包含此数据的消息，而不知道有哪些发布者。
> 
> 面向消息的中间件是支持在分布式系统之间发送和接收消息的软件或硬件基础设施。OPC UA PubSub 支持两种不同的面向消息的中间件变体，即 **无代理形式** 和 **基于代理的形式** 。在无代理形式中，面向消息的中间件是能够路由基于数据报的消息的网络基础设施。订阅者和发布者使用 UDP 等数据报协议。在基于代理的形式中，消息中间件的核心组件是消息代理。订阅者和发布者使用 AMQP 或 MQTT 等标准消息传递协议与代理进行通信。
>
> 这使得 PubSub 适合需要位置独立性和（或）可扩展性的应用程序。
>
> OPC UA 的发布/订阅（PubSub）扩展可实现快速高效的通信。PubSub 扩展与协议无关，可与基于代理的协议（如 MQTT 和 AMQP）或无代理实现（如 UDP 多播）一起使用。
>
> PubSub 的配置模型使用以下组件
> ```cpp
> typedef enum  {
>     UA_PUBSUB_COMPONENT_CONNECTION,
>     UA_PUBSUB_COMPONENT_WRITERGROUP,
>     UA_PUBSUB_COMPONENT_DATASETWRITER,
>     UA_PUBSUB_COMPONENT_READERGROUP,
>     UA_PUBSUB_COMPONENT_DATASETREADER
> } UA_PubSubComponentEnumType;
> ```
>
> open62541 PubSub API 使用以下方案
> - 为所需的 PubSub 元素创建配置
> - 调用 `add[element]` 函数并传入配置
> - `add[element]` 函数返回内部创建的元素的唯一 `UA_NodeId`

有关 API 使用的更多详细信息，请查看 [PubSub 教程](https://www.open62541.org/doc/master/pubsub.html)。

### 3.1 无代理 Pub/Sub

RMVL 提供了基于 `UDP` 传输协议的 Broker-less 即无代理的发布订阅机制，目前支持 `UADP` 的消息映射方式，对应的枚举类型是 `TransportID::UDP_UADP`。

需要留意的是，OPC UA 的发布订阅模型仍然是建立在 @ref tutorial_opcua_server_client 模型之上的，此外 @ref opcua 的 PubSub 在实现上是继承于 rm::Server 的，因此，RMVL 的发布订阅模型在使用时具备服务器的所有功能，初始化、释放资源等操作与服务器完全一致。

**创建发布者**

@add_toggle_cpp

```cpp
// publisher.cpp
#include <csignal>
#include <rmvl/opcua/publisher.hpp>

using namespace std::chrono_literals;

static bool stop = false;

int main()
{
    signal(SIGINT, [](int) { stop = true; });

    // 创建 OPC UA 发布者，端口为 4840
    rm::Publisher<rm::TransportID::UDP_UADP> pub("DemoNumberPub", "opc.udp://224.0.0.22:4840");

    // 添加变量节点至发布者自身的服务器中
    rm::Variable num = 3.14;
    num.browse_name = "number";
    num.display_name = "Number";
    num.description = "数字";
    auto num_node = pub.addVariableNode(num);
    // 准备待发布的数据
    std::vector<rm::PublishedDataSet> pds_list;
    pds_list.emplace_back("Number 1", num_node);

    // 发布数据
    pub.publish(pds_list, 50);

    while (!stop)
    {
        /* other code */
        
        /* 例如 num_node 所对应的值可以直接在这里修改 */
        
        pub.spinOnce();
    }
}
```

@end_toggle

@add_toggle_python

```python
# publisher.py

from signal import signal, SIGINT
import rm

stop = False

def onStop(sig, frame):
    global stop
    stop = True

signal(SIGINT, onStop)

# 创建 OPC UA 发布者，端口为 4840
pub = rm.Publisher(rm.TransportID.UDP_UADP, "DemoNumberPub", "opc.udp://224.0.0.22:4840")

# 添加变量节点至发布者自身的服务器中
num = rm.Variable(3.14)
num.browse_name = "number"
num.display_name = "Number"
num.description = "数字"
num_node = pub.addVariableNode(num)
# 准备待发布的数据
pds_list = [rm.PublishedDataSet("Number 1", num_node)]

# 发布数据
pub.publish(pds_list, 50)

while not stop:
    # other code
    # 例如 num_node 所对应的值可以直接在这里修改
    pub.spinOnce()
```

@end_toggle

**创建订阅者**

@add_toggle_cpp

```cpp
// subscriber.cpp
#include <csignal>
#include <rmvl/opcua/subscriber.hpp>

using namespace std::chrono_literals;

static bool stop = false;

int main()
{
    signal(SIGINT, [](int) { stop = true; });

    // 创建 OPC UA 订阅者
    rm::Subscriber<rm::TransportID::UDP_UADP> sub("DemoNumberSub", "opc.udp://224.0.0.22:4840", 4841);

    // 准备需要订阅的数据
    rm::FieldMetaData meta_data{"Number 1", rm::tpDouble, -1};

    /* 也可以通过创建变量对 meta_data 进行初始化，例如以下代码
    rm::Variable num = 1.0; // 这个 1.0 只是代表是个 Double 类型的数据 
    num.browse_name = "Number 1";
    auto meta_data = rm::FieldMetaData::makeFrom(num);
    */
    
    // 订阅数据，第 2 个参数传入的是 std::vector 类型的数据，单个数据请使用初始化列表
    auto nodes = sub.subscribe("DemoNumberPub", {meta_data});
    // 订阅接收的数据均存放在订阅者自身的服务器中，请使用服务器端变量的写操作进行访问
    // 订阅返回值是一个 NodeId 列表，存放订阅接收的数据的 NodeId
    
    while (!stop)
    {
        // 读取订阅的已更新的数据
        auto sub_val = sub.read(nodes.front());
        std::printf("Sub value [1] = %f\n", sub_val.cast<double>());
        
        /* other code */
        
        sub.spinOnce();
    }
}
```

@end_toggle

@add_toggle_python

```python
# subscriber.py

from signal import signal, SIGINT
import rm

stop = False

def onStop(sig, frame):
    global stop
    stop = True

signal(SIGINT, onStop)

# 创建 OPC UA 订阅者
sub = rm.Subscriber(rm.TransportID.UDP_UADP, "DemoNumberSub", "opc.udp://224.0.0.22:4840", 4841)

# 准备需要订阅的数据
meta_data = rm.FieldMetaData("Number 1", rm.tp_float, -1)

"""
也可以通过创建变量对 meta_data 进行初始化，例如以下代码
num = rm.Variable(1.0) # 这个 1.0 只是代表是个 Double 类型的数据
num.browse_name = "Number 1"
meta_data = rm.FieldMetaData.makeFrom(num)
"""

# 订阅数据，第 2 个参数传入的是 list 类型的数据，单个数据请使用列表
nodes = sub.subscribe("DemoNumberPub", [meta_data])

while not stop:
    # 读取订阅的已更新的数据
    sub_val = sub.read(nodes[0])
    print(f"Sub value [1] = {sub_val.float()}")
    
    # other code
    
    sub.spinOnce()
```

@end_toggle

### 3.2 有代理 Pub/Sub

@warning RMVL 目前暂不支持有代理的发布订阅机制。

## 4. 使用技巧

以下是 @ref opcua 的使用技巧。

### 4.1 参数加载 {#opcua_parameters}

@ref opcua 中提供了以下几个运行时可调节参数

|    类型    |       参数名        | 默认值 |                             注释                             |
| :--------: | :-----------------: | :----: | :----------------------------------------------------------: |
|   `bool`   |     SERVER_WAIT     | false  |         单次处理网络事件时，允许服务器等待最多 50ms          |
| `uint32_t` |   CONNECT_TIMEOUT   | 30000  |           请求连接时，判定为超时的时间，单位 (ms)            |
| `uint32_t` | CLIENT_WAIT_TIMEOUT |   10   |               服务器超时响应的时间，单位 (ms)                |
|  `double`  |  SAMPLING_INTERVAL  |   2    |             服务器监视变量的采样速度，单位 (ms)              |
|  `double`  | PUBLISHING_INTERVAL |   2    | 服务器尝试发布数据变更的期望时间间隔，若数据未变更则不会发布，单位 (ms) |
| `uint32_t` |   LIFETIME_COUNT    |  100   | 在没有发布任何消息的情况下，订阅请求所期望的能够保持活动状态的最大发布周期数 |
| `uint32_t` | MAX_KEEPALIVE_COUNT |   50   | 在没有任何通知的情况下，订阅请求所期望的服务器应该发送的最大 “保活” 消息数 |
| `uint32_t` |  MAX_NOTIFICATIONS  |  100   | 服务器应该发送的期望的最大通知数（通知是服务器向客户端报告订阅的变化的方式） |
| `uint8_t`  |      PRIORITY       |   0    |                       订阅请求的优先级                       |

具体调节方式可参考引言中的 @ref intro_parameters_manager 部分。

### 4.2 从 XML 配置 OPC UA {#opcua_nodeset_compiler}

#### 4.2.1 安装 UaModeler

可使用 UaModeler 等软件进行可视化信息模型的建立，构建后可以导出为一个 `*.xml` 文件，首先先安装 UaModeler。

**Windows EXE**

- Windows 下可点击[此处](https://pan.baidu.com/s/1pK0gYf-yQjUoFQ-Ie7qB1w)安装官方版本的 UaModeler 软件。

**Python**

- 如果有 Python 环境，也可以使用开源的 UaModeler 库，功能与官方软件基本一致。使用之前需要安装 `pip3` Python 包管理工具，安装好包管理工具后，可使用以下命令行安装 UaModeler
  ```bash
  pip3 install opcua-modeler

  # Linux 下可以执行以下命令行运行 UaModeler
  opcua-modeler
  ```

具体安装细节可参考 [opcua-modeler on Github](https://github.com/FreeOpcUa/opcua-modeler) 的 README。

#### 4.2.2 可视化配置 OPC UA

对于项目创建或导出等内容，此处不做过多介绍，可参考[此博客](https://wanghao1314.blog.csdn.net/article/details/104092781)了解上述内容。

@note
- 一般的，定义对象、变量、方法等内容均按照在代码中的顺序进行定义即可，但需要注意的是，添加了方法节点后，还需要在代码中设置该方法节点执行的回调函数，可参见 `rm::Server::setMethodNodeCallBack`。
- `NamespaceArray` 的 `[1]` 的字符串需要更改为 `urn:open62541.server.application`

#### 4.2.3 生成 \*.c/\*.h 文件

@note 以下生成 C/C++ 文件的介绍来自 [open62541 nodeset-compiler](https://www.open62541.org/doc/master/nodeset_compiler.html#getting-started)。

进入 `<path-to-open62541>/tools/nodeset_compiler` 文件夹，执行以下命令行

```bash
# 获取 Opc.Ua.NodeSet2.xml 文件
wget https://files.opcfoundation.org/schemas/UA/1.05/Opc.Ua.NodeSet2.xml
# 将刚刚生成的 XML 文件移动至当前文件夹中，并重命名为 xxx.xml
mv <path-to-xml> ./xxx.xml
# 执行 nodeset_compiler
python3 ./nodeset_compiler.py \
  --types-array=UA_TYPES \
  --existing Opc.Ua.NodeSet2.xml \
  --xml xxx.xml \
  myNodeSet # myNodeSet 是要生成的文件名，包含 myNodeSet.h 和 myNodeSet.c，请自行设置
```

### 4.3 不占有所有权的 C/S 视图

`rm::Server` 使用 RAII 进行设计，一个对象占有了服务器的所有权和生命周期，当对象析构时，会自动停止并结束服务器。使用 `rm::ServerView` 来获取不占有所有权的服务器视图，并进行变量读写、路径搜索的操作。

@add_toggle_cpp

```cpp
// server.cpp

#include <csignal>
#include <rmvl/opcua/server.hpp>

void modify(rm::ServerView sv, int val)
{
    auto node = sv.find("num");
    sv.write(node, val);
}

int main()
{
    signal(SIGINT, [](int) { stop = true; });

    rm::Server srv(4840);

    // 定义 int 型变量
    rm::Variable num_info = 42;
    num_info.browse_name = "num";
    num_info.display_name = "Num";
    num_info.description = "数字";
    // 添加到服务器的默认位置
    srv.addVariableNode(num_info);

    /* code */

    // 修改变量值
    modify(srv, 100);

    while (!stop)
        srv.spinOnce();
}
```

@end_toggle

@add_toggle_python

```python
# server.py

from signal import signal, SIGINT
import rm

def modify(sv, val):
    node = sv.find("num")
    sv.write(node, val)

def onStop(sig, frame):
    global stop
    stop = True

signal(SIGINT, onStop)

svr = rm.Server(4840)

# 定义 int 型变量
num_info = rm.Variable(42)
num_info.browse_name = "num"
num_info.display_name = "Num"
num_info.description = "数字"
# 添加到服务器的默认位置
svr.addVariableNode(num_info)

# 修改变量值
modify(svr, 100)

while not stop:
    svr.spinOnce()
```
@end_toggle

同样的，客户端也可以使用 `rm::ClientView` 来获取不占有所有权的客户端视图，进行变量读写、路径搜索的操作，此处不再赘述。

## 5. 参考内容

@cite FreeOpcUa22 UaModeler · FreeOpcUa/opcua-modeler · Github
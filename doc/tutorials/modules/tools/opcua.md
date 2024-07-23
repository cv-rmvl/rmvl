工业自动化通信协议 OPC UA {#tutorial_modules_opcua}
============

@author 赵曦
@date 2023/11/24
@version 2.0
@brief OPC UA 和 open62541 库简介

@prev_tutorial{tutorial_modules_serial}

@next_tutorial{tutorial_modules_camera}

@tableofcontents

------

相关模块： @ref opcua

## 1. 简介

### 1.1 OPC UA 是什么

[OPC UA](https://opcfoundation.org/about/opc-technologies/opc-ua/)（全称为 Open Platform Communications Unified Architecture）是一种用于工业和物联网（IoT）应用的开放通信协议和架构。它提供了一种统一的框架，用于在不同设备和系统之间实现数据传输、通信和集成。

OPC UA 的设计目标是建立一种通用的、独立于厂商和平台的通信标准，以实现互操作性和集成性。它提供了一套标准的服务和功能，使不同类型的设备和系统能够相互通信和交换数据，其特点包括：

|   特点   |                             介绍                             |
| :------: | :----------------------------------------------------------: |
|  兼容性  | OPC UA 不依赖于特定的硬件、操作系统或网络协议，可以在不同的平台上运行，并与其他通信标准集成 |
|  安全性  | OPC UA 提供了强大的安全机制，包括身份验证、加密和访问控制，以确保数据和通信的机密性和完整性 |
|  可扩展  | OPC UA 支持灵活的数据模型和信息建模，可以适应不同应用领域和需求的变化 |
| 信息建模 | OPC UA 使用统一的信息模型，将数据和功能以标准化的方式表示和描述，使不同系统之间的数据交换更加简化和一致 |
|  可靠性  | OPC UA 提供了可靠的通信机制，包括消息确认、重试和错误处理，以确保数据的可靠传输 |

### 1.2 地址空间

在 OPC UA 中，所有的数据都被组织成一个地址空间，地址空间中的每一个元素都被称为一个节点。每个节点都有一个唯一的节点号，在 @ref opcua 中表示为 rm::NodeId 。

<center>

![图 1-1 OPC UA 地址空间模型](opcua.svg)

</center>

1. 对象类型节点 rm::ObjectType ：提供对象的定义，即对象的抽象，与类相当，且子类可以继承父类的特征，方便模型的扩充。该节点包括对象的各种数据类型，数据的语义，以及控制方式。OPC UA 命名空间 `0` 中规定了多个基础的对象类型节点。如使用最广的 BaseObjectType（在 RMVL 中表示为 `rm::nodeBaseObjectType`），所有对象类型节点都需要继承该节点再进行扩充。在对具体设备建模的过程中，应该将设备组成的各部分分解为不同的对象分别建模，再用引用节点将各部分按照实际设备中的关系相关联，从而得到完整设备的对象类型节点。

2. 对象节点 rm::Object ：将对象类型实例化即可得到对象节点，该节点是设备在数字空间的映射。所有对设备数据的访问都能在该模型中访问到对应的数据节点。所有对 设备的控制都转换为方法节点的触发。设备产生的消息在节点对象中将触发对应的事件。

3. 引用类型节点 **ReferenceType** ：引用类型描述了引用的语义，而引用用于定义引用两端的节点之间的关系。最常用的引用类型如 Organizes（在 RMVL 中表示为 `rm::nodeOrganizes`），表示节点之间的层级关系，如同文件夹与文件夹内的文件，数据层级复杂的设备，需要通过多种引用类型对设备信息节点之间的关系进行描述。

4. 数据类型节点 rm::DataType ：数据类型节点描述了变量节点中变量的数据类型。在 OPC UA 信息模型在命名空间 `0` 中定义了多种内置的数据类型，包括整型、浮点型、 字符串等多个类型，能对变量的数据进行准确的描述。也可以自定义数据类型，比如描述二维坐标的 `2DPoint` 等类型，获得更符合数据本身的描述。@note 注意：此类节点并不能提供具体的数据构成，只是提供了数据类型的一个描述，因此 RMVL 中的 @ref opcua 仅提供内置数据类型。若计划提供数据的构成，比如包含的数据长度等信息，请使用变量类型节点 rm::VariableType 。

5. 变量类型节点 rm::VariableType ：该节点提供了对变量节点的定义，是设备中各种数据的抽象。常用引用中的 HasTypeDefinition 引用节点连接数据类型节点，对数据类型进行描述（在 RMVL 中表示为 `rm::nodeHasTypeDefinition`）。用 HasProperty 引用节点对数据的语义进行描述（在 RMVL 中表示为 `rm::nodeHasProperty`）。也可以使用自定义的数据类型节点对变量的数据进行描述，具有灵活性。

6. 变量节点 rm::Variable ：该节点是变量类型节点的实例，也是使用的最多的节点。客户端访问设备数据有以下 3 种方式。
   |    访问方式    |                             介绍                             |                             备注                             |
   | :------------: | :----------------------------------------------------------: | :----------------------------------------------------------: |
   |    直接读写    | 将设备多模态数据写入对应的变量节点，然后客户端读取对应节点内保存的数值 | 如果客户端要获取设备最新的值，需要一直手动去触发对设备数据源的读取请求 |
   |     值回调     | 客户端发起 **IO** 请求后，服务器在 **读取前** 和 **写入后** 分别调用对应的回调函数 |     可以利用此功能在需要访问数据的时候才让服务器更新数据     |
   | 数据源变量节点 | 客户端的读取请求直接重定向到设备的数据源中，即客户端直接从数据源获取数据，变量节点不存储数据 | 缩减了数据先写入变量节点再进行读取的过程，但多个客户端连接访问同一数据时会增大服务器与设备之间的传输负载 |

7. 方法节点 rm::Method ：方法节点是对设备控制方法在数字模型中的映射。方法节点可以通过服务器或客户端进行调用，然后将会对设备的控制器发送指令，使得设备执行对应的操作。常见的方法节点有：触发视觉采集、电机反转、设备初始化等。

8. 视图节点 rm::View ：视图节点可将地址空间中感兴趣的节点提取出来，作为一个子集，视图节点作为该子集的入口，方便客户端浏览。

## 2. 服务器/客户端 {#opcua_server_client}

基于服务器/客户端的方式是 OPC UA 最基本的一种通信方式，上文的地址空间在服务器/客户端通信的过程中完全展现出来。下面列举一些 opcua 模块中常用的服务器与客户端的成员方法。

### 2.1 初始化

**服务器**

```cpp
// server.cpp
#include <rmvl/opcua/server.hpp>

int main()
{
    // 创建 OPC UA 服务器，端口为 4840
    rm::Server srv(4840);
    // 服务器运行
    srv.start();

    /* other code */

    // 线程阻塞，直到调用了 srv.stop()，线程才会继续执行。
    srv.join();
}
```

**客户端**

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

### 2.2 变量

在上文介绍了变量的 3 种访问方式，这里使用最简单的直接读写的方式。首先在服务器中添加变量节点。

```cpp
// server.cpp
#include <rmvl/opcua/server.hpp>

int main()
{
    rm::Server srv(4840);
    srv.start();

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

    srv.join();
}
```

然后在客户端中直接读取变量节点。

```cpp
// client.cpp
#include <rmvl/opcua/client.hpp>

int main()
{
    rm::Client cli("opc.tcp://127.0.0.1:4840");

    // 使用管道运算符 "|" 进行路径搜索，寻找待读取的变量
    auto node = rm::nodeObjectsFolder | cli.find("number");
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
}
```

### 2.3 方法

在服务器中添加两数之和的方法节点，供客户端调用。

```cpp
// server.cpp
#include <rmvl/opcua/server.hpp>

int main()
{
    rm::Server srv(4840);
    srv.start();

    // 定义方法
    rm::Method method;
    method.browse_name = "add";
    method.display_name = "Add";
    method.description = "两数之和";
    // 定义函数传入参数 iargs 的类型
    method.iargs = {{"Number 1", UA_TYPES_INT32}, {"Number 2", UA_TYPES_INT32}};
    // 定义函数返回值 oargs 的类型
    method.oargs = {{"Sum", UA_TYPES_INT32}};

    /*
        1. 数据类型均使用在 open62541 中定义的 UA_TYPES_ 作为前缀的宏
        2. {"Number 1", UA_TYPES_INT32} 的部分是 rm::Argument 的聚合类，表示方法的参数
        3. 允许有多个返回值，即 oargs 的长度允许 > 1
    */

    // 方法的函数指针，无捕获列表的 lambda 表达式可发生向函数指针的隐式转换，因此可以使用 "=" 完成赋值
    method.func = [](UA_Server *, const UA_NodeId *, void *, const UA_NodeId *, void *, const UA_NodeId *,
                     void *, size_t, const UA_Variant *input, size_t, UA_Variant *output) -> UA_StatusCode {
        int32_t num1 = *reinterpret_cast<int *>(input[0].data);
        int32_t num2 = *reinterpret_cast<int *>(input[1].data);
        int32_t retval = num1 + num2;
        return UA_Variant_setScalarCopy(output, &retval, &UA_TYPES[UA_TYPES_INT32]);
    };
    // 方法节点添加至服务器
    server.addMethodNode(method);

    srv.join();
}
```

在客户端调用指定方法。

```cpp
// client.cpp
#include <rmvl/opcua/client.hpp>

int main()
{
    rm::Client cli("opc.tcp://127.0.0.1:4840");

    // 设置输入参数，1 和 2 是 Int32 类型的，因此可以直接隐式构造
    std::vector<rm::Variable> input = {1, 2};
    // 设置输出参数，用来存储结果
    std::vector<rm::Variable> output;
    // 调用方法，判断调用是否成功
    if (!cli.call("add", input, output))
    {
        ERROR_("Failed to call the method");
        return 0;
    }
    // 输出结果
    printf("retval = %d\n", rm::Variable::cast<int>(output.front()));
}
```

### 2.4 对象

在服务器中添加对象节点：

- A
  - B1
    - C1: `3.14`
    - C2: `666`
  - B2
    - C3: `"xyz"`

```cpp
// server.cpp
#include <rmvl/opcua/server.hpp>

int main()
{
    rm::Server srv(4840);
    srv.start();
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

    srv.join();
}
```

在客户端寻找 `C2` 和 `C3` 并打印。

```cpp
// client.cpp
#include <iostream>
#include <rmvl/opcua/client.hpp>

int main()
{
    rm::Client cli("opc.tcp://127.0.0.1:4840");

    // 路径搜索寻找 C2
    auto node_c2 = rm::nodeObjectsFolder | cli.find("A") | cli.find("B1") | cli.find("C2");
    rm::Variable c2;
    cli.read(node_c2, c2);
    std::cout << rm::Variable::cast<int>(c2) << std::endl;
    // 路径搜索寻找 C3
    auto node_c3 = rm::nodeObjectsFolder | cli.find("A") | cli.find("B2") | cli.find("C3");
    rm::Variable c3;
    cli.read(node_c3, c3);
    std::cout << rm::Variable::cast<const char *>(c3) << std::endl;
}
```

### 2.5 视图

在 `nodeObjectsFolder` 中先添加 `A/num1`、`num2` 2 个变量节点，并将 `num1` 和 `num2` 加入视图，下面的示例演示在 **服务器** 中创建并添加视图节点。若要在客户端中进行此操作，创建并添加视图节点的步骤基本一致，这里不做展示。需要注意的是，在客户端中创建并添加视图节点，需要提前在服务器中加入对应的（变量、方法、对象……）节点

```cpp
// server.cpp
#include <rmvl/opcua/server.hpp>

int main()
{
    rm::Server srv(4840);
    srv.start();
    // 准备对象节点数据 A
    rm::Object a;
    a.browse_name = a.description = a.display_name = "A";
    // 这里使用宏来创建 num1
    uaCreateVariable(num1, 1);
    a.add(num1);
    auto node_a = srv.addObjectNode(a);
    auto node_num1 = node_a | srv.find("num1");
    // 这里使用宏来创建 num2
    uaCreateVariable(num2, 2);
    auto node_num2 = srv.addVariableNode(num2);

    // 创建视图
    rm::View num_view;
    // 添加节点至视图（这里使用的是变量节点的 NodeId，实际上其他节点也是允许的）
    num_view.add(node_num1, node_num2);
    // 添加至服务器
    srv.addViewNode(num_view);
    srv.join();
}
```

### 2.6 监视

在服务器中添加待监视的变量节点

```cpp
// server.cpp
#include <rmvl/opcua/server.hpp>

int main()
{
    rm::Server srv(4840);
    srv.start();

    // 定义 int 型变量
    rm::Variable num = 100;
    num.browse_name = "number";
    num.display_name = "Number";
    num.description = "数字";
    // 添加到服务器的默认位置
    srv.addVariableNode(num);

    srv.join();
}
```

在客户端 1 中修改变量节点的数据

```cpp
// client_1.cpp
#include <rmvl/opcua/client.hpp>

using namespace std::chrono_literals;

int main()
{
    rm::Client cli("opc.tcp://127.0.0.1:4840");
    auto node = rm::nodeObjectsFolder | cli.find("number");
    for (int i = 0; i < 100; ++i)
    {
        std::this_thread::sleep_for(1s);
        // 写入数据，i + 200 隐式构造成了 rm::Variable
        bool success = cli.write(node, i + 200);
        if (!success)
            ERROR_("Failed to write data to the variable.");
    }
}
```

在客户端 2 中监视变量节点

```cpp
// client_2.cpp
#include <rmvl/opcua/client.hpp>

void onChange(UA_Client *, UA_UInt32, void *, UA_UInt32, void *, UA_DataValue *value)
{
    int receive_data = *reinterpret_cast<int *>(value->value.data);
    printf("Data (n=number) was changed to: %d\n", receive_data);
}

int main()
{
    rm::Client cli("opc.tcp://127.0.0.1:4840");
    auto node = rm::nodeObjectsFolder | cli.find("number");
    // 监视变量，这里的 onChange 同样可以写成无捕获列表的 lambda 表达式，因为存在隐式转换
    client.monitor(node, onChange, 5);
    // 线程阻塞
    client.spin();
}
```

## 3. 发布/订阅 {#opcua_pub_sub}

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

需要留意的是，OPC UA 的发布订阅模型仍然是建立在 @ref opcua_server_client 模型之上的，此外 @ref opcua 的 PubSub 在实现上是继承于 rm::Server 的，因此，RMVL 的发布订阅模型在使用时具备服务器的所有功能，初始化、释放资源等操作与服务器完全一致。

**创建发布者**

```cpp
// publisher.cpp
#include <rmvl/opcua/publisher.hpp>

int main()
{
    // 创建 OPC UA 发布者，端口为 4840
    rm::Publisher<rm::TransportID::UDP_UADP> pub("DemoNumberPub", "opc.udp://224.0.0.22:4840");
    // 添加变量节点至发布者自身的服务器中
    rm::Variable num = 3.14;
    num.browse_name = "number";
    num.display_name = "Number";
    num.description = "数字";
    auto num_node = pub.addVariableNode(num);
    // 发布者的服务器运行
    pub.start();
    // 准备待发布的数据
    std::vector<PublishedDataSet> pds_list;
    pds_list.emplace_back("Number 1", num_node);

    // 发布数据
    pub.publish(pds_list, 50);

    /* other code */
    /* 例如 num_node 所对应的值可以直接在这里修改 */

    // 线程阻塞，直到调用了 pub.stop()，线程才会继续执行。
    pub.join();
}
```

**创建订阅者**

```cpp
// subscriber.cpp
#include <rmvl/opcua/subscriber.hpp>

int main()
{
    // 创建 OPC UA 订阅者
    rm::Subscriber<rm::TransportID::UDP_UADP> sub("DemoNumberSub", "opc.udp://224.0.0.22:4840", 4841);
    // 订阅者的服务器运行
    sub.start();

    // 准备需要订阅的数据
    // 这里只订阅 1 个，如果订阅多个请使用 std::vector
    rm::FieldMetaData meta_data("Number 1", UA_TYPES_DOUBLE, UA_VALUERANK_SCALAR);

    /* 也可以通过创建变量对 meta_data 进行初始化，例如以下代码
    rm::Variable num = 1.0; // 这个 1.0 只是代表是个 Double 类型的数据 
    num.browse_name = "Number 1";
    rm::FieldMetaData meta_data = num;
    */
    
    // 订阅数据，第 2 个参数传入的是 std::vector 类型的数据，单个数据请使用初始化列表
    auto nodes = sub.subscribe("DemoNumberPub", {meta_data});
    // 订阅接收的数据均存放在订阅者自身的服务器中，请使用服务器端变量的写操作进行访问
    // 订阅返回值是一个 NodeId 列表，存放订阅接收的数据的 NodeId

    // 读取订阅的已更新的数据
    auto sub_val = sub.read(nodes.front());
    std::printf("Sub value [1] = %f\n", sub_val.cast<double>());
    
    // 线程阻塞，直到调用了 sub.stop()，线程才会继续执行。
    sub.join();
}
```

### 3.2 有代理 Pub/Sub

@warning RMVL 目前暂不支持有代理的发布订阅机制。

## 4. 使用技巧

以下是 @ref opcua 的使用技巧。

### 4.1 参数加载 {#opcua_parameters}

@ref opcua 中提供了以下几个运行时可调节参数

|    类型    |       参数名        | 默认值 |                             注释                             |
| :--------: | :-----------------: | :----: | :----------------------------------------------------------: |
| `uint32_t` |    SPIN_TIMEOUT     |   10   |               服务器超时响应的时间，单位 (ms)                |
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

#### 4.2.2 可视化配置 OPC UA 信息模型

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

### 4.3 不占有所有权的服务器视图

`rm::Server` 使用 RAII 进行设计，一个对象占有了服务器的所有权和生命周期，当对象析构时，会自动停止并结束服务器。使用 `rm::ServerView` 来获取不占有所有权的服务器视图，并进行变量读写、路径搜索的操作，下面用服务器视图的单元测试作为示例。

```cpp
rm::Method method;
method.browse_name = "plus";
method.display_name = "Input + Number";
method.description = "输入值加数";
method.func = [](UA_Server *p_server, const UA_NodeId *, void *, const UA_NodeId *, void *, const UA_NodeId *,
                 void *, size_t, const UA_Variant *inputs, size_t, UA_Variant *) -> UA_StatusCode {
    rm::ServerView sv = p_server;
    auto num_node = nodeObjectsFolder | sv.find("num");
    int num = sv.read(num_node).cast<int>();
    rm::Variable dst = *reinterpret_cast<int *>(inputs->data) + num;
    sv.write(num_node, dst);
    return UA_STATUSCODE_GOOD;
};
method.iargs = {{"input", UA_TYPES_INT32, 1, "输入值"}};
srv.addMethodNode(method);
```

---

## 5. 引用

@cite ua-modeler UaModeler · FreeOpcUa/opcua-modeler · Github
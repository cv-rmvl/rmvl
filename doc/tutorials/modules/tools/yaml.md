YAML 数据读写 {#tutorial_modules_yaml}
============

@prev_tutorial{tutorial_modules_aggregate_reflect}

@next_tutorial{tutorial_modules_coro}

@tableofcontents

------

### 1. 概述

core 模块提供了基于 rapidyaml 的 YAML 树模型，接口位于 `rm::yaml` 命名空间中。它适合读取配置、机器人描述和其他结构化数据，不会向用户暴露 rapidyaml 的节点或字符串视图类型。

`rm::yaml::Node` 是共享底层 YAML 树所有权的轻量句柄。保存子节点时无需同时保存根节点，但同一棵树上的并发写入或读写并发需要由调用方同步。

直接包含对应头文件即可使用：

```cpp
#include <rmvl/core/yaml.hpp>
```

也可以包含 core 模块汇总头文件：

```cpp
#include <rmvl/core.hpp>
```

### 2. 解析与读取

`rm::yaml::parse()` 从内存文本解析 YAML，`rm::yaml::load()` 从文件加载。两者都返回 `rm::yaml::Result`，解析或文件错误不会导致进程终止。

```cpp
auto result = rm::yaml::parse(R"(
camera:
  name: front
  width: 1280
  enabled: true
labels: [robot, armor]
)");

if (!result) {
    fmt::print("YAML error at {}:{}: {}\n",
               result.error.line,
               result.error.column,
               result.error.message);
    return;
}

auto name = (*result)["camera"]["name"].as<std::string>();
auto width = (*result)["camera"]["width"].valueOr(640);
auto labels = (*result)["labels"].as<std::vector<std::string>>();
```

`operator[]` 查找失败时返回无效节点。`as<T>()` 转换失败时返回 `std::nullopt`，`valueOr()` 可直接提供默认值。整数转换会检查格式和目标类型的取值范围。

### 3. 构建与保存

可以从映射、序列或标量根节点开始构建 YAML：

```cpp
auto root = rm::yaml::Node::createMap();
root.set("name", "demo");
root.set("ports", std::vector<int>{8000, 8001});

auto network = root.ensure("network");
network.set("host", "127.0.0.1");
network.set("enabled", true);

const auto text = rm::yaml::dump(root);

rm::yaml::Error error;
if (!rm::yaml::save("config.yml", root, error))
    fmt::print("failed to save YAML: {}\n", error.message);
```

映射节点保持 YAML 树中的展示顺序。类型转换同时支持以 `std::string` 为键的 `std::map` 和 `std::unordered_map`：前者适合需要稳定排序输出的配置，后者适合不关心顺序的运行时查找。

### 4. 自定义类型

为用户类型提供 `yaml_encode()` 和 `yaml_decode()` 即可通过 ADL 接入，无需继承基类或注册运行时类型：

```cpp
struct Point {
    double x{};
    double y{};
};

bool yaml_encode(rm::yaml::Node &node, const Point &point) {
    node.makeMap();
    return node.set("x", point.x) && node.set("y", point.y);
}

bool yaml_decode(const rm::yaml::Node &node, Point &point) {
    return node["x"].read(point.x) && node["y"].read(point.y);
}
```

之后自定义类型可以与内置类型一样使用：

```cpp
root.set("origin", Point{1.5, 2.0});
auto origin = root["origin"].as<Point>();
```

当前支持字符串、布尔值、整数、浮点数、枚举、`std::optional`、`std::vector`、`std::array` 以及字符串键映射。需要访问 YAML tag、anchor 或样式等 rapidyaml 底层特性时，应直接使用 rapidyaml。

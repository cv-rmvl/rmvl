TOML 配置加载 {#tutorial_modules_toml}
============

@prev_tutorial{tutorial_modules_yaml}

@next_tutorial{tutorial_modules_coro}

@tableofcontents

------

### 1. 概述

core 模块提供 TOML 1.0 配置解析，包含 `<rmvl/core/toml.hpp>` 或 `<rmvl/core.hpp>` 即可使用。

`rm::toml::load()` 从文件加载，`rm::toml::loads()` 从 UTF-8 内存文本加载，两者直接返回只读的 `rm::toml::Node`。成功时根节点始终为 Table，空输入得到空 Table，加载失败返回无效节点。

```cpp
#include <fmt/format.h>
#include <rmvl/core/toml.hpp>

auto config = rm::toml::loads(R"(
[server]
host = "127.0.0.1"
port = 8080
enabled = false
)");

if (auto server = config["server"]) {
    auto host = server["host"].valueOr<std::string>("localhost");
    auto port = server["port"].valueOr<uint16_t>(8080);
    fmt::print("{}:{}\n", host, port);
}
```

### 2. 节点访问与类型转换

`valid()` 和显式布尔转换只判断节点是否有效。因此 `enabled = false` 对应的节点仍然满足 `if (node)`，实际的布尔值通过 `as<bool>()` 读取。

- `node["name"]` 查询表的字面键，`node[index]` 查询数组元素，类型不符或查询失败返回无效节点。
- `node["a.b"]` 查询包含点的单个键，访问嵌套表使用 `node["a"]["b"]`。
- `contains()` 判断键是否存在，`keys()` 返回表的键名，键的顺序不作保证。
- `read(value)` 返回转换是否成功，失败保持输出对象不变。
- `as<T>()` 转换失败返回 `std::nullopt`，`valueOr<T>()` 返回指定的默认值。

转换支持字符串、布尔、整数、浮点、枚举、日期时间，以及 `optional`、`vector`、`array` 和以 `std::string` 为键的 `map/unordered_map`。严格区分整数、浮点和字符串，整数转换检查目标类型范围。缺失节点可读取为空 `optional`，TOML 自身没有 null 值。

自定义类型在其命名空间提供 `bool toml_decode(const rm::toml::Node &, T &)`，即可使用上述读取接口。转换在临时对象上完成，失败不会改写调用方原有对象。

### 3. 日期时间与所有权

日期、时间和日期时间分别对应 `rm::toml::Date`、`Time`、`DateTime`。`Time` 的小数秒保留至纳秒，超出部分截断。`DateTime::offsetMinutes` 无值表示本地日期时间，0 表示 UTC，其他值表示以分钟为单位的 UTC 偏移。

`Node` 拷贝共享底层节点树，不复制节点树。子节点可以脱离根句柄保存，键名视图在底层节点树最后一个句柄销毁前有效。节点树只读，可以在多个线程中并发读取。

### 4. 错误处理

加载失败时返回无效节点，可通过 `valid()` 或显式布尔转换判断。需要错误详情时传入可选的 `rm::toml::Error *` 输出参数，成功时会清空已有错误。

`ErrorCode::Io` 表示文件路径无效或文件打开、读取失败，`ErrorCode::Parse` 表示编码、语法或定义冲突错误。解析错误的行列号从 1 开始，文件错误的行列号为 0。

```cpp
rm::toml::Error error;
auto config = rm::toml::load("config.toml", &error);
if (!config) {
    fmt::print(stderr, "TOML error at {}:{}: {}\n",
               error.line, error.column, error.message);
    return;
}

auto port = config["server"]["port"].valueOr<uint16_t>(8080);
fmt::print("port: {}\n", port);
```

只关心是否成功时，可以省略错误输出参数：

```cpp
if (auto config = rm::toml::loads("port = 8080")) {
    fmt::print("port: {}\n", config["port"].valueOr(8080));
}
```

后端使用不抛解析异常的模式，文件和语法错误通过返回值处理。接口仍会使用标准容器和动态内存，不承诺 `noexcept`，不拦截内存分配失败等标准库异常。

当前接口用于配置加载，不提供修改、序列化或保留原始注释的接口。解析后端是内置的 toml++ v3.4.0，以必选的 tomlplusplus 接口库接入 CMake，并随 RMVL 安装和导出。下游项目只需链接 RMVL core，依赖自动传递。

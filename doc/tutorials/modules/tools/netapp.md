网络、应用层设施 {#tutorial_modules_netapp}
============

@author 赵曦
@date 2025/09/25
@version 1.0

@prev_tutorial{tutorial_modules_socket}

@next_tutorial{tutorial_modules_opcua}

@tableofcontents

------

相关类
- 请求、响应结构： rm::Request 及 rm::Response
- 请求工具： rm::requests::request 及 rm::async::requests::request
- 后端框架： rm::async::Webapp

### 1 HTTP 数据结构

相关类： rm::Request 及 rm::Response

两个类均提供了

- `generate` 成员函数，用于从请求/响应结构生成 HTTP 格式的字符串，在调用此方法后可直接通过 Socket 发送数据
- `parse` 静态成员函数，用于从 HTTP 格式的字符串解析出请求/响应结构，在接收到 Socket 原始数据后可调用此方法进行解析

rm::Response 还提供了多种简化生成响应的成员函数，如 `send`、`json`、`redirect` 等。

### 2 HTTP 请求工具

相关函数：

- 同步请求： rm::requests::request 及其简化版本 rm::requests::get, rm::requests::post 等
- 异步请求： rm::async::requests::request 及其简化版本 rm::async::requests::get, rm::async::requests::post 等

设计理念参考 Python 的 requests 库，提供了极其简洁易用的接口，这里给出一个简单的 GET 请求示例：

```cpp
#include <rmvl/io/webapp.hpp>

using namespace rm;

int main() {
    // 发送 GET 请求
    auto response = requests::get("http://example.com");
    if (response) {
        printf("Response: %s\n", response->body().c_str());
    }
}
```

异步请求的使用方法类似，只需将 `requests` 替换为 `async::requests`，并在协程中使用 `co_await` 关键字等待结果，这里不再赘述。

### 3 Web 后端框架

RMVL 提供了 HTTP 的后端应用框架 rm::async::Webapp 。这是 @ref io 的集大成者，设计理念参考 JavaScript 的 Express 框架，这里给出一个简单的 Web 服务器示例：

```cpp
// demo.cpp
#include <iostream>

#include "rmvl/io/netapp.hpp"

using namespace rm;

int main(int argc, char *argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <port>" << std::endl;
        return 1;
    }

    uint16_t port = static_cast<uint16_t>(std::atoi(argv[1]));

    async::IOContext io_context{};
    async::Webapp app(io_context);

    app.use(cors());

    app.get("/", [](const Request &req, Response &res) {
        res.send("<html><body><h1>Hello, World!</h1></body></html>");
    });

    app.get("/test", [](const Request &req, Response &res) {
        res.redirect("/");
    });

    app.get("/str", [](const Request &req, Response &res) {
        res.send("only string");
    });

    app.get("/api", [](const Request &req, Response &res) {
        res.json("{ \"key\": \"value\", \"message\": \"This is a test API.\" }");
    });

    app.get("/api/name/:name", [](const Request &req, Response &res) {
        auto name = req.params.at("name");
        res.json("{ \"greeting\": \"Hello, " + name + "!\" }");
    });

    app.listen(port, [&]() {
        std::cout << "Server is running on port " << port << std::endl;
    });

    co_spawn(io_context, &async::Webapp::spin, &app);
    io_context.run();
}
```

可以简单使用以下命令行进行编译

```bash
g++ demo.cpp -std=c++20 -I /usr/local/include/RMVL -l rmvl_io -l rmvl_core -o demo
```

<div class="fragment">
<div class="line"><span class="comment"># 如果使用的是 Windows 系统，可能需要额外链接 ws2_32 库</span></div>
</div>

编译后，在终端输入

<div class="fragment">
<div class="line"><span class="keywordflow">./demo</span> 8080</div>
</div>

即可创建 Web 后端服务，并且监听 8080 端口，可以使用 `curl` 工具或浏览器访问 `http://localhost:8080/` 来测试服务器。

<div class="fragment">
<div class="line"><span class="keywordflow">curl</span>
    <span class="comment">-s</span>
    <span class="stringliteral">"http://localhost:8080"</span>
</div>
</div>

会得到如下输出：

```html
<html><body><h1>Hello, World!</h1></body></html>
```

同时，可以配合 `jq` 工具测试 JSON API：

<div class="fragment">
<div class="line"><span class="keywordflow">curl</span>
    <span class="comment">-s</span>
    <span class="stringliteral">"http://localhost:8080/api"</span> |
    <span class="keywordflow">jq</span>
</div>
</div>

会得到如下输出：

<div class="fragment">
<div class="line">{</div>
<div class="line">&nbsp;&nbsp;<span class="keywordtype">"key"</span>:
    <span class="stringliteral">"value"</span>
</div>
<div class="line">&nbsp;&nbsp;<span class="keywordtype">"message"</span>:
    <span class="stringliteral">"This is a test API."</span>
</div>
<div class="line">}</div>
</div>

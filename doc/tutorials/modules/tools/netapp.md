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
- 后端应用： rm::async::Webapp
- HTTP/HTTPS 服务器： rm::async::HttpServer 及 rm::async::HttpsServer

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
#include <rmvl/io/netapp.hpp>

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

RMVL 提供了后端应用框架 rm::async::Webapp ，以及负责传输层监听的 rm::async::HttpServer 和 rm::async::HttpsServer 。设计参考 Express JS，具体来说，Webapp 负责路由与中间件，服务器对象负责 HTTP 或 HTTPS 监听，同一个 Webapp 可以交给不同类型的服务器。

#### 3.1 HTTP 服务器

rm::async::Webapp 和 rm::Router 提供了 GET、POST、DELETE 等相关方法用于定义路由，并且 rm::async::Webapp 可以使用 `use` 方法挂载中间件或挂载由 rm::Router 定义的子路由。

```cpp
// demo.cpp
#include <rmvl/io/netapp.hpp>

using namespace rm;

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s <port>\n", argv[0]);
        return 1;
    }

    uint16_t port = static_cast<uint16_t>(std::atoi(argv[1]));

    async::IOContext io_context{};

    // 创建后端应用
    async::Webapp app(io_context);
    async::HttpServer server(app);

    // 定义路由：以 GET 方法响应根路径请求
    app.get("/", [](const Request &req, Response &res) {
        res.send("<html><body><h1>Hello, World!</h1></body></html>");
    });
    // 定义路由：重定向示例
    app.get("/test", [](const Request &req, Response &res) {
        res.redirect("/");
    });

    // 创建子路由
    auto api_router = Router{};

    // 定义子路由：以 GET 方法响应根路径请求
    api_router.get("/", [](const Request &req, Response &res) {
        res.json({
            {"key", "value"},
            {"message", "This is a test API."},
        });
    });
    // 定义子路由：以 GET 方法响应 /name/:name 路径请求
    api_router.get("/name/:name", [](const Request &req, Response &res) {
        auto name = req.params.at("name");
        res.json({
            {"greeting", "Hello, " + name + "!"},
        });
    });

    // 使用 CORS 中间件允许跨域请求
    app.use(cors());

    // 挂载路由至 /api 路径
    app.use("/api", api_router);

    // 监听指定端口
    server.listen(port, [&]() {
        printf("Server is running on port %d\n", port);
    });

    // 生成协程任务并执行
    co_spawn(io_context, &async::HttpServer::spin, &server);
    io_context.run();
}
```

可以简单使用以下命令行进行编译

<div class="fragment">
<div class="line"><span class="keywordflow">g++</span> demo.cpp <span class="comment">-std=c++20</span> <span class="comment">-I</span> /usr/local/include/RMVL <span class="comment">-l</span> rmvl_io <span class="comment">-l</span> rmvl_core <span class="comment">-o</span> demo</div>
<div class="line"><span class="comment"># 如果使用的是 Windows 系统，可能需要额外链接 ws2_32 库</span></div>
</div>

@note 当然，也可以自行编写 CMakeLists.txt 文件，采用 CMake 构建系统进行构建，具体的 CMakeLists.txt 内容可参考如下内容：
<div class="fragment">
<div class="line"><span class="keyword">cmake_minimum_required</span>(VERSION 3.16)</div>
<div class="line"><span class="keyword">project</span>(Demo)</div>
<div class="line"></div>
<div class="line"><span class="keyword">find_package</span>(RMVL REQUIRED)</div>
<div class="line"></div>
<div class="line"><span class="keyword">rmvl_add_exe</span>(</div>
<div class="line">&nbsp;&nbsp;demo</div>
<div class="line">&nbsp;&nbsp;<span class="v">SOURCES</span> demo.cpp</div>
<div class="line">&nbsp;&nbsp;<span class="v">DEPENDS</span> io</div>
<div class="line"><span class="keyword"></span>)</div>
</div>

编译后，在终端输入

<div class="fragment">
<div class="line"><span class="keywordflow">./demo</span> 8080</div>
</div>

即可创建 Web 后端服务，并且监听 8080 端口，可以使用 `curl` 工具或浏览器访问 `http://localhost:8080/` 来测试服务器。

<div class="fragment">
<div class="line"><span class="keywordflow">curl</span> <span class="comment">-s</span> <span class="stringliteral">"http://localhost:8080"</span></div>
</div>

会得到如下输出：

<div class="fragment">
<div class="line"><span class="comment">&lt;</span><span class="keywordtype">html</span><span class="comment">&gt;</span><span class="comment">&lt;</span><span class="keywordtype">body</span><span class="comment">&gt;</span><span class="comment">&lt;</span><span class="keywordtype">h1</span><span class="comment">&gt;</span>Hello, World!<span class="comment">&lt;</span><span class="keywordtype">/h1</span><span class="comment">&gt;</span><span class="comment">&lt;</span><span class="keywordtype">/body</span><span class="comment">&gt;</span><span class="comment">&lt;</span><span class="keywordtype">/html</span><span class="comment">&gt;</span></div>
</div>

@note rm::async::Webapp 虽然也提供了 `listen` 方法，但该方法仅用于快速测试，实际生产环境推荐使用 rm::async::HttpServer 或 rm::async::HttpsServer 进行监听。

#### 3.2 HTTPS 服务器

HTTPS 与 HTTP 使用相同的 Webapp，只需要将 rm::async::HttpServer 替换为 rm::async::HttpsServer ，并提供服务端的 rm::SSLContext 即可，示例代码如下：

```cpp
async::IOContext io_context{};
async::Webapp app(io_context);

app.get("/", [](const Request &, Response &res) {
    res.send("Hello, HTTPS!");
});

SSLContext ssl_context = SSLContext::server();
if (!ssl_context.load_cert("server.crt", "server.key")) {
    printf("%s\n", ssl_context.lasterr().c_str());
    return 1;
}

async::HttpsServer server(app, ssl_context);
server.listen(8443, [] {
    printf("HTTPS server is listening on 8443\n");
});

co_spawn(io_context, &async::HttpsServer::spin, &server);
io_context.run();
```

SSLContext 必须比 HttpsServer 和所有活动 TLS 连接存活得更久。证书自动续期后，需要重新加载 SSLContext 或重启/重载服务进程，才能让新连接使用更新后的证书。 使用命令行直接编译 HTTPS 程序时，还需要链接 OpenSSL 的 ssl 和 crypto 库；使用 CMake 链接 rmvl_io 目标时会自动传递该依赖。

测试自签名证书时可以使用：

<div class="fragment">
<div class="line"><span class="keywordflow">curl</span> <span class="comment">-k</span> <span class="stringliteral">"https://127.0.0.1:8443/"</span></div>
</div>

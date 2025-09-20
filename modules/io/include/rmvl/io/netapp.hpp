/**
 * @file netapp.hpp
 * @author zhaoxi (535394140@qq.com)
 * @brief 异步 IO 的应用层网络通信框架
 * @version 1.0
 * @date 2025-07-31
 *
 * @copyright Copyright 2025 (c), zhaoxi
 *
 */

#pragma once

#include <regex>
#include <unordered_map>
#include <vector>

#include "async.hpp"

namespace rm {

//! @addtogroup io_net
//! @{

//! URL 解析结果
struct URLParseInfo {
    std::string scheme;              //!< 协议部分，例如 `http` 或 `https`
    std::string hostname;            //!< 域名部分，例如 `example.com`
    uint16_t port{};                 //!< 端口号，默认为 80 (HTTP) 或 443 (HTTPS)
    std::string path;                //!< 路径部分，例如 `/path/to/resource`
    std::vector<std::string> querys; //!< 以 `std::vector` 存储的查询参数部分，例如 `[key=value, key2=value2]`
};

/**
 * @brief 解析 URL
 *
 * @param[in] url 统一资源定位符，例如 `http://example.com:8080/path`
 * @retval scheme 协议部分，例如 `http` 或 `https`
 * @retval hostname 域名部分，例如 `example.com`
 * @retval port 端口号，默认为 80 (HTTP) 或 443 (HTTPS)
 * @retval path 路径部分，例如 `/path/to/resource`
 * @retval querys 以 `std::vector` 存储的查询参数部分，例如 `[key=value, key2=value2]`
 */
URLParseInfo parseURL(std::string_view url);

/**
 * @brief 域名解析
 *
 * @param[in] hostname 域名
 * @retval ip IP 地址
 * @retval isv6 是否为 IPv6 地址
 */
std::tuple<std::string, bool> parseDNS(std::string_view hostname);

//! HTTP 请求方法
enum class HTTPMethod : uint8_t {
    Get,     //!< 从服务器获取资源，用于请求数据而不对数据进行更改
    Post,    //!< 向服务器发送数据，用于提交表单数据或上传文件
    Put,     //!< 向服务器发送数据，一般用于更新现有资源
    Delete,  //!< 从服务器删除指定的资源，请求中包含要删除的资源标识符
    Patch,   //!< 对资源进行部分修改，与 `Put` 类似，但 `Patch` 只更改部分数据而不是替换整个资源
    Head,    //!< 类似于 `Get`，但服务器只返回响应的头部，不返回实际数据，常用于检查资源的元数据
    Options, //!< 返回服务器支持的 HTTP 方法
    Trace,   //!< 回显服务器收到的请求，主要用于诊断
    Connect, //!< 建立一个到服务器的隧道，通常用于 HTTPS 连接
    Unknown  //!< 未知方法
};

//! HTTP 请求
struct Request {
    /**
     * @brief 解析 HTTP 请求
     *
     * @param[in] str HTTP 请求报文
     * @return 解析后的请求对象
     */
    static Request parse(std::string_view str);

    /**
     * @brief 生成 HTTP 请求报文
     *
     * @return 请求报文
     */
    std::string generate() const;

    HTTPMethod method{}; //!< 请求行：请求方法
    std::string uri{};   //!< 请求行：请求的路径

    std::unordered_map<std::string, std::string> params{}; //!< 路径参数 @note 仅在解析请求时访问有效
    std::unordered_map<std::string, std::string> query{};  //!< 查询参数

    std::string host{};            //!< 请求头：主机名
    std::string content_type{};    //!< 请求头：内容类型
    std::string accept{"*/*"};     //!< 请求头：可接受的内容类型
    std::string accept_language{}; //!< 请求头：可接受的语言
    std::string connection{};      //!< 请求头：连接类型

    std::string body{}; //!< 请求数据
};

//! HTTP 响应
struct Response {
    /**
     * @brief 解析 HTTP 响应
     *
     * @param[in] str HTTP 响应报文
     * @return 解析后的响应对象
     */
    static Response parse(std::string_view str);

    /**
     * @brief 生成 HTTP 响应报文
     *
     * @return 响应报文
     */
    std::string generate();

    /**
     * @brief 发送响应数据
     *
     * @param[in] str 响应数据
     */
    void send(std::string_view str);

    /**
     * @brief 发送文件内容作为响应
     *
     * @param[in] file 文件名
     */
    void sendFile(std::string_view file);

    /**
     * @brief 发送 JSON 格式的响应数据
     *
     * @param[in] str JSON 字符串
     */
    void json(std::string_view str);

    /**
     * @brief 发送重定向响应
     *
     * @param[in] url 重定向的 URL
     */
    void redirect(std::string_view url) { redirect(302, url); }

    /**
     * @brief 重定向响应
     *
     * @param[in] code 状态码
     * @param[in] url 重定向的 URL
     */
    void redirect(uint16_t code, std::string_view url);

    operator bool() const noexcept { return status != 0 && !message.empty(); }

    uint16_t status{};     //!< 响应行：状态码
    std::string message{}; //!< 响应行：状态消息

    std::unordered_map<std::string, std::string> heads{}; //!< 响应头

    std::string body{}; //!< 响应数据
};

//! 响应中间件类型
using ResponseMiddleware = std::function<void(Response &)>;

/**
 * @brief 跨域资源共享 CORS 中间件，为响应添加 CORS 头部信息
 * @code {.cpp}
 * // 直接操作 Response 对象
 * cors(res);
 * // 或在 Web 应用程序框架中使用
 * auto app = async::Webapp(io_context);
 * app.use(cors);
 * @endcode
 */
void cors(Response &res);

//! @} io_net

namespace requests {

//! @addtogroup io_net
//! @{

/**
 * @brief 发出同步 HTTP 请求
 *
 * @param[in] method 请求方法
 * @param[in] url 请求的 URL
 * @param[in] querys 可选的 URL 查询参数列表
 * @param[in] heads 可选的请求头列表
 * @param[in] body 可选的请求体
 * @return 响应报文
 */
Response request(HTTPMethod method, std::string_view url, const std::vector<std::string> &querys = {},
                 const std::unordered_map<std::string, std::string> &heads = {}, std::string_view body = "");

/**
 * @brief 发出同步 GET 请求
 *
 * @param[in] url 请求的 URL
 * @param[in] querys 可选的 URL 参数列表
 * @param[in] heads 可选的请求头列表
 * @return Response 响应报文
 */
inline Response get(std::string_view url, const std::vector<std::string> &querys = {},
                    const std::unordered_map<std::string, std::string> &heads = {}) {
    return request(HTTPMethod::Get, url, querys, heads);
}

/**
 * @brief 发出同步 POST 请求
 *
 * @param[in] url 请求的 URL
 * @param[in] body 请求体
 * @param[in] querys 可选的 URL 参数列表
 * @param[in] heads 可选的请求头列表
 * @return Response 响应报文
 */
inline Response post(std::string_view url, std::string_view body, const std::vector<std::string> &querys = {},
                     const std::unordered_map<std::string, std::string> &heads = {}) {
    return request(HTTPMethod::Post, url, querys, heads, body);
}

/**
 * @brief 发出同步 DELETE 请求
 *
 * @param[in] url 请求的 URL
 * @param[in] querys 可选的 URL 参数列表
 * @param[in] heads 可选的请求头列表
 * @return Response 响应报文
 */
inline Response del(std::string_view url, const std::vector<std::string> &querys = {},
                    const std::unordered_map<std::string, std::string> &heads = {}) {
    return request(HTTPMethod::Delete, url, querys, heads);
}

//! @} io_net

} // namespace requests

#if __cplusplus >= 202002L

namespace async {

namespace requests {

//! @addtogroup io_net
//! @{

/**
 * @brief 发出异步 HTTP 请求
 *
 * @param[in] io_context 异步 I/O 执行上下文
 * @param[in] method 请求方法
 * @param[in] url 请求的 URL
 * @param[in] querys 可选的 URL 参数列表
 * @param[in] heads 可选的请求头列表
 * @param[in] body 可选的请求体
 * @return 响应报文的异步任务
 */
Task<Response> request(IOContext &io_context, HTTPMethod method, std::string_view url, const std::vector<std::string> &querys = {},
                       const std::unordered_map<std::string, std::string> &heads = {}, std::string_view body = "");

/**
 * @brief 发出异步 GET 请求
 *
 * @param[in] io_context 异步 I/O 执行上下文
 * @param[in] url 请求的 URL
 * @param[in] querys 可选的 URL 参数列表
 * @param[in] heads 可选的请求头列表
 * @return rm::async::Task<Response>
 */
inline Task<Response> get(IOContext &io_context, std::string_view url, const std::vector<std::string> &querys = {},
                          const std::unordered_map<std::string, std::string> &heads = {}) {
    co_return co_await request(io_context, HTTPMethod::Get, url, querys, heads);
}

/**
 * @brief 发出异步 POST 请求
 *
 * @param[in] io_context 异步 I/O 执行上下文
 * @param[in] url 请求的 URL
 * @param[in] body 请求体
 * @param[in] querys 可选的 URL 参数列表
 * @param[in] heads 可选的请求头列表
 * @return rm::async::Task<Response>
 */
inline Task<Response> post(IOContext &io_context, std::string_view url, std::string_view body,
                           const std::vector<std::string> &querys = {}, const std::unordered_map<std::string, std::string> &heads = {}) {
    co_return co_await request(io_context, HTTPMethod::Post, url, querys, heads, body);
}

/**
 * @brief 发出异步 DELETE 请求
 *
 * @param[in] io_context 异步 I/O 执行上下文
 * @param[in] url 请求的 URL
 * @param[in] querys 可选的 URL 参数列表
 * @param[in] heads 可选的请求头列表
 * @return rm::async::Task<Response>
 */
inline Task<Response> del(IOContext &io_context, std::string_view url, const std::vector<std::string> &querys = {},
                          const std::unordered_map<std::string, std::string> &heads = {}) {
    co_return co_await request(io_context, HTTPMethod::Delete, url, querys, heads);
}

//! @} io_net

} // namespace requests

//! @addtogroup io_net
//! @{

//! Web 应用程序框架
class Webapp final {
public:
    //! 路由处理器类型
    using RouteHandler = std::function<void(const Request &, Response &)>;

    //! 路由模式匹配器
    class RoutePattern {
    public:
        /**
         * @brief 构造路由模式
         * @param[in] pattern_str 路由模式字符串，如 `"/api/:name"`
         */
        explicit RoutePattern(std::string_view pattern_str);

        /**
         * @brief 匹配路径，并提取参数
         * @param[in] path 请求路径
         * @param[out] params 输出参数，提取的路径参数
         * @return 是否匹配成功
         */
        bool match(std::string_view path, std::unordered_map<std::string, std::string> &params) const;

    private:
        std::string _pattern{};                  //!< 原始模式字符串
        std::vector<std::string> _param_names{}; //!< 参数名列表
        std::regex _matcher{};                   //!< 编译后的正则表达式
    };

    //! 路由条目：路由模式 + 处理器
    struct RouteEntry {
        RoutePattern pattern;
        RouteHandler handler;

        RouteEntry(std::string_view pattern_str, RouteHandler h)
            : pattern(pattern_str), handler(std::move(h)) {}
    };

    /**
     * @brief 创建 Web 应用程序实例
     *
     * @param[in] io_context 异步 I/O 执行上下文
     */
    Webapp(IOContext &io_context) : _ctx(io_context){};

    //! @cond
    Webapp(const Webapp &) = delete;
    Webapp &operator=(const Webapp &) = delete;
    Webapp(Webapp &&) noexcept = delete;
    Webapp &operator=(Webapp &&) noexcept = delete;
    ~Webapp();
    //! @endcond

    /**
     * @brief 使用中间件
     *
     * @param[in] mwf 中间件函数
     */
    void use(ResponseMiddleware mwf);

    /**
     * @brief 监听指定端口
     *
     * @param[in] port 监听的端口号
     * @param[in] callback 启动后调用的回调函数
     */
    void listen(uint16_t port, std::function<void()> callback = nullptr) {
        _port = port;
        _listen = std::move(callback);
    }

    /**
     * @brief Get 请求路由
     *
     * @param[in] uri 统一资源标识符，支持路径参数，如 "/api/:name"
     * @param[in] callback Get 响应回调
     */
    void get(std::string_view uri, std::function<void(const Request &, Response &)> callback) { _gets.emplace_back(uri, std::move(callback)); }

    /**
     * @brief Post 请求路由
     *
     * @param[in] uri 统一资源标识符，支持路径参数，如 "/api/:name"
     * @param[in] callback Post 响应回调
     */
    void post(std::string_view uri, std::function<void(const Request &, Response &)> callback) { _posts.emplace_back(uri, std::move(callback)); }

    /**
     * @brief Head 请求路由
     *
     * @param[in] uri 统一资源标识符，支持路径参数，如 "/api/:name"
     * @param[in] callback Head 响应回调
     */
    void head(std::string_view uri, std::function<void(const Request &, Response &)> callback) { _heads.emplace_back(uri, std::move(callback)); }

    /**
     * @brief Delete 请求路由
     *
     * @param[in] uri 统一资源标识符，支持路径参数，如 "/api/:name"
     * @param[in] callback Delete 响应回调
     */
    void del(std::string_view uri, std::function<void(const Request &, Response &)> callback) { _deletes.emplace_back(uri, std::move(callback)); }

    /**
     * @brief 启动事件循环
     *
     * @return rm::async::Task<> 异步任务
     */
    [[nodiscard]] Task<> spin();

    //! 是否正在运行
    [[nodiscard]] bool running() const noexcept { return _running; }

    //! 停止运行
    void stop() noexcept { _running = false; }

private:
    std::atomic_bool _running{false}; //!< 运行标志位

    IOContextRef _ctx;                //!< 异步 I/O 执行上下文
    uint16_t _port{};                        //!< 监听端口
    std::function<void()> _listen{};         //!< 启动后调用的回调函数
    std::vector<ResponseMiddleware> _mwfs{}; //!< 响应中间件列表

    std::vector<RouteEntry> _gets{};    //!< GET 请求路由列表
    std::vector<RouteEntry> _posts{};   //!< POST 请求路由列表
    std::vector<RouteEntry> _heads{};   //!< HEAD 请求路由列表
    std::vector<RouteEntry> _deletes{}; //!< DELETE 请求路由列表
};

//! @} io_net

} // namespace async

} // namespace rm

#endif // __cplusplus >= 202002L

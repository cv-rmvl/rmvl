/**
 * @file netapp.cpp
 * @author zhaoxi (535394140@qq.com)
 * @brief 异步 IO 的应用层网络通信框架实现
 * @version 1.0
 * @date 2025-07-31
 *
 * @copyright Copyright 2025 (c), zhaoxi
 *
 */

#ifdef _WIN32
#include <WS2tcpip.h>
#include <afunix.h>
#else
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#endif

#include <chrono>
#include <fstream>
#include <tuple>

#include "rmvl/core/str.hpp"
#include "rmvl/core/util.hpp"
#include "rmvl/core/version.hpp"

#include "rmvl/io/netapp.hpp"
#include "rmvl/io/socket.hpp"

namespace rm {

//////////////////////////////// 基本功能 //////////////////////////////////

static constexpr HTTPMethod get_method_from(std::string_view str) {
    if (str == "GET")
        return HTTPMethod::Get;
    else if (str == "POST")
        return HTTPMethod::Post;
    else if (str == "PUT")
        return HTTPMethod::Put;
    else if (str == "DELETE")
        return HTTPMethod::Delete;
    else if (str == "PATCH")
        return HTTPMethod::Patch;
    else if (str == "HEAD")
        return HTTPMethod::Head;
    else if (str == "OPTIONS")
        return HTTPMethod::Options;
    else if (str == "TRACE")
        return HTTPMethod::Trace;
    else if (str == "CONNECT")
        return HTTPMethod::Connect;
    else
        return HTTPMethod::Unknown;
}

static constexpr const char *get_str_from(HTTPMethod m) {
    constexpr const char *methods[] = {"GET", "POST", "PUT", "DELETE", "PATCH", "HEAD", "OPTIONS", "TRACE", "CONNECT", "UNKNOWN"};
    return methods[static_cast<uint8_t>(m)];
}

static std::string get_date_str(std::chrono::system_clock::time_point date) {
    std::time_t t = std::chrono::system_clock::to_time_t(date);
#ifdef _WIN32
    std::tm tm{};
    gmtime_s(&tm, &t);
#else
    std::tm tm = *std::gmtime(&t);
#endif
    char buffer[64]{};
    std::strftime(buffer, sizeof(buffer), "%a, %d %b %Y %H:%M:%S GMT", &tm);
    return buffer;
}

////////////////////////////// 请求解析、生成 //////////////////////////////

Request Request::parse(std::string_view str) {
    Request req{};
    // 解析请求行
    auto pos = str.find("\r\n");
    if (pos == std::string_view::npos)
        return req;
    auto line = str.substr(0, pos);

    auto method_end = line.find(' ');
    req.method = get_method_from(line.substr(0, method_end));
    auto uri_end = line.find(' ', method_end + 1);
    req.uri = line.substr(method_end + 1, uri_end - method_end - 1);
    // 解析查询参数
    if (auto query_start = req.uri.find('?'); query_start != std::string_view::npos) {
        auto query_part = req.uri.substr(query_start + 1);
        req.uri = req.uri.substr(0, query_start);
        auto query_items = str::split(query_part, "&");
        for (const auto &q : query_items) {
            if (q.empty())
                continue;
            auto key_value = str::split(q, "=");
            if (key_value.empty())
                continue;
            req.query[key_value[0]] = key_value.size() == 2 ? key_value[1] : std::string{};
        }
    }

    str.remove_prefix(pos + 2);
    // 解析请求头
    auto ltrim = [](std::string &s) {
        size_t i = 0;
        while (i < s.size() && (s[i] == ' ' || s[i] == '\t'))
            ++i;
        if (i)
            s.erase(0, i);
    };
    while ((pos = str.find("\r\n")) != std::string_view::npos) {
        line = str.substr(0, pos);
        str.remove_prefix(pos + 2);
        if (line.empty())
            break;
        auto req_head_strs = str::split(line, ":");
        if (req_head_strs.size() < 2)
            continue;
        // 重新拼接 value（允许 value 中再出现 ':'）
        std::string value(line.substr(req_head_strs[0].size() + 1));
        ltrim(value);
        if (req_head_strs[0] == "Host")
            req.host = value;
        else if (req_head_strs[0] == "Content-Type")
            req.content_type = value;
        else if (req_head_strs[0] == "Accept")
            req.accept = value;
        else if (req_head_strs[0] == "Accept-Language")
            req.accept_language = value;
        else if (req_head_strs[0] == "Connection")
            req.connection = value;
        else
            continue;
    }
    // 请求体
    req.body = str;
    return req;
}

std::string Request::generate() const {
    std::string str{};
    str.reserve(host.size() + content_type.size() + accept.size() + accept_language.size() + connection.size() + body.size() + 512);
    // 512: Extra space
    std::string url = uri;
    url.reserve(uri.size() + query.size() * 32); // 32: Average size for each query parameter
    if (!query.empty()) {
        url.append("?");
        std::vector<std::string> query_parts{};
        query_parts.reserve(query.size());
        for (const auto &[key, value] : query)
            query_parts.push_back(value.empty() ? key : key + "=" + value);
        url.append(str::join(query_parts, "&"));
    }

    str.append(get_str_from(method)).append(" ").append(url).append(" ").append("HTTP/1.1\r\n");
    if (!host.empty())
        str.append("Host: ").append(host).append("\r\n");
    if (!content_type.empty())
        str.append("Content-Type: ").append(content_type).append("\r\n");
    if (!accept.empty())
        str.append("Accept: ").append(accept).append("\r\n");
    if (!accept_language.empty())
        str.append("Accept-Language: ").append(accept_language).append("\r\n");
    if (!connection.empty())
        str.append("Connection: ").append(connection).append("\r\n");

    str.append("\r\n").append(body);
    return str;
}

////////////////////////////// 响应解析、生成 //////////////////////////////

using namespace std::string_literals;

Response Response::parse(std::string_view response_str) {
    Response res;

    // 解析响应行
    auto pos = response_str.find("\r\n");
    if (pos == std::string_view::npos)
        return res;
    auto line = response_str.substr(0, pos);
    response_str.remove_prefix(pos + 2);
    // HTTP Version
    auto version_end = line.find(' ');
    if (version_end == std::string_view::npos)
        return res;
    // Status Code
    auto code_start = version_end + 1;
    auto code_end = line.find(' ', code_start);
    if (code_end == std::string_view::npos)
        return res;
    auto code_str = line.substr(code_start, code_end - code_start);
    res.status = std::stoi(std::string(code_str));
    // Message
    res.message = line.substr(code_end + 1);

    // 解析响应头
    while ((pos = response_str.find("\r\n")) != std::string_view::npos) {
        line = response_str.substr(0, pos);
        response_str.remove_prefix(pos + 2);
        if (line.empty())
            break;

        // 解析头部字段
        auto colon_pos = line.find(':');
        if (colon_pos != std::string_view::npos) {
            auto key = line.substr(0, colon_pos);
            auto value = line.substr(colon_pos + 1);

            res.heads[std::string(key)] = str::strip(value);
        }
    }

    // 响应体
    res.body = response_str;
    return res;
}

std::string Response::generate() {
    std::string str{};
    str.reserve(body.size() + 1024); // 1024: Extra space for heads

    // 生成响应行
    str.append("HTTP/1.1 ").append(std::to_string(status)).append(" ").append(message).append("\r\n");

    // 生成响应头
    str.append("Server: RMVL/").append(version()).append("\r\n");
    str.append("Date: ").append(get_date_str(std::chrono::system_clock::now())).append("\r\n");
    for (const auto &[key, value] : heads)
        str.append(key).append(": ").append(value).append("\r\n");

    str.append("\r\n");

    // 生成响应体
    str.append(body);
    return str;
}

void Response::send(std::string_view str) {
    body = str;
    heads["Content-Length"] = std::to_string(body.size());
    heads["Content-Type"] = "text/html; charset=utf-8";
    status = 200;
    message = "OK";
}

void Response::sendFile(std::string_view file) {
    std::ifstream ifs(file.data());
    if (!ifs) {
        status = 404;
        message = "Not Found";
        return;
    }
    ifs.seekg(0, std::ios::end);
    auto file_size = ifs.tellg();
    ifs.seekg(0, std::ios::beg);
    body.resize(file_size);
    ifs.read(body.data(), file_size);
    heads["Content-Length"] = std::to_string(body.size());
    heads["Content-Type"] = "text/html; charset=utf-8";
    status = 200;
    message = "OK";
}

void Response::json(std::string_view str) {
    body = str;
    heads["Content-Length"] = std::to_string(body.size());
    heads["Content-Type"] = "application/json; charset=utf-8";
    status = 200;
    message = "OK";
}

void Response::redirect(uint16_t code, std::string_view url) {
    RMVL_DbgAssert(code >= 300 && code < 400);
    status = code;
    static const std::unordered_map<uint16_t, std::string_view> redirect_map =
        {{301, "Moved Permanently"}, {302, "Found"}, {303, "See Other"}, {307, "Temporary Redirect"}, {308, "Permanent Redirect"}};
    message = redirect_map.find(code) != redirect_map.end() ? redirect_map.at(code) : "Redirect";
    heads["Location"] = url;
    heads["Content-Length"] = "0";
    heads["Content-Type"] = "text/html; charset=utf-8";
    heads["Connection"] = "close";
}

void cors(Response &res) {
    res.heads["Access-Control-Allow-Origin"] = "*";
    res.heads["Access-Control-Allow-Methods"] = "GET, POST, PUT, DELETE, OPTIONS";
    res.heads["Access-Control-Allow-Headers"] = "Content-Type, Authorization";
    res.heads["Access-Control-Max-Age"] = "86400";
}

URLParseInfo parseURL(std::string_view url) {
    std::string scheme{};             // 协议部分，例如 "http" 或 "https"
    std::string hostname{};           // 主机名部分，例如 "example.com"
    uint16_t port{80};                // 默认端口
    std::string path{};               // 路径部分，例如 "/path/to/resource"
    std::vector<std::string> query{}; // 查询参数部分

    // 查找协议部分
    auto scheme_end = url.find("://");
    if (scheme_end != std::string_view::npos) {
        scheme = url.substr(0, scheme_end);
        url.remove_prefix(scheme_end + 3);
        port = (scheme == "https") ? 443 : 80;
    } else {
        scheme = "http";
        port = 80;
    }

    // 查找路径部分
    auto path_start = url.find('/');
    std::string_view host_part;
    if (path_start != std::string_view::npos) {
        host_part = url.substr(0, path_start);
        path = url.substr(path_start);
    } else {
        host_part = url;
        path = "/";
    }

    // 查找端口部分
    auto port_start = host_part.find(':');
    if (port_start != std::string_view::npos) {
        hostname = host_part.substr(0, port_start);
        auto port_str = host_part.substr(port_start + 1);
        port = static_cast<uint16_t>(std::stoi(std::string(port_str)));
    } else
        hostname = host_part;

    // 查找查询参数部分
    auto query_start = url.find('?');
    if (query_start != std::string_view::npos) {
        path = url.substr(path_start, query_start - path_start);
        auto query_str = url.substr(query_start + 1);
        query = str::split(query_str, "&");
    }

    return {scheme, hostname, port, path, query};
}

std::tuple<std::string, bool> parseDNS(std::string_view hostname) {
#ifdef _WIN32
    rm::SocketEnv::ensure_init();
#endif
    addrinfo hints{}, *result = nullptr;
    hints.ai_family = AF_UNSPEC; // 允许 IPv4 或 IPv6
    hints.ai_socktype = SOCK_STREAM;

    int status = getaddrinfo(hostname.data(), nullptr, &hints, &result);
    if (status != 0)
        RMVL_Error_(RMVL_StsError, "getaddrinfo failed: %s", gai_strerror(status));

    std::string ip_str;
    bool is_ipv6 = false;

    char ip[INET6_ADDRSTRLEN];
    if (result != nullptr) {
        void *addr;
        if (result->ai_family == AF_INET) { // IPv4
            sockaddr_in *ipv4 = reinterpret_cast<sockaddr_in *>(result->ai_addr);
            addr = &(ipv4->sin_addr);
            is_ipv6 = false;
        } else {
            // IPv6
            sockaddr_in6 *ipv6 = reinterpret_cast<sockaddr_in6 *>(result->ai_addr);
            addr = &(ipv6->sin6_addr);
            is_ipv6 = true;
        }
        inet_ntop(result->ai_family, addr, ip, INET6_ADDRSTRLEN);
        ip_str = ip;
    }
    freeaddrinfo(result);
    return {ip_str, is_ipv6};
}

// 生成 HTTP 请求字符串
static std::string _generate(HTTPMethod method, std::string_view full_path, uint16_t port, std::string_view hostname,
                             const std::unordered_map<std::string, std::string> &heads, std::string_view body) {
    // 构建 HTTP 请求
    Request req;
    req.method = method;
    req.uri = full_path;
    if (port != 80 && port != 443)
        req.host += ":" + std::to_string(port);
    // HTTP 请求头
    req.host = hostname;
    req.connection = "close";
    req.content_type = "application/json";
    for (const auto &[key, value] : heads) {
        if (key == "Host")
            req.host = value;
        else if (key == "Content-Type")
            req.content_type = value;
        else if (key == "Accept")
            req.accept = value;
        else if (key == "Accept-Language")
            req.accept_language = value;
        else if (key == "Connection")
            req.connection = value;
    }
    // 设置请求体
    req.body = body;
    return req.generate();
}

Response requests::request(HTTPMethod method, std::string_view url, const std::vector<std::string> &querys,
                           const std::unordered_map<std::string, std::string> &heads, std::string_view body) {
    Response response{};
    // 解析 URL
    auto [scheme, hostname, port, path, all_querys] = parseURL(url);
    auto [ip, isv6] = parseDNS(hostname);
    // 构建完整的路径（包含查询参数）
    std::string full_path = path;
    all_querys.insert(all_querys.end(), querys.begin(), querys.end());
    if (!all_querys.empty())
        full_path = full_path + "?" + str::join(all_querys, "&");
    // 建立 TCP 连接
    Connector connector(Endpoint(isv6 ? ip::tcp::v6() : ip::tcp::v4(), port), ip);
    auto socket = connector.connect();
    // 生成 HTTP 请求
    auto req_str = _generate(method, full_path, port, hostname, heads, body);
    // 发送 HTTP 请求
    if (!socket.write(req_str)) {
        response.status = 0;
        response.message = "Connection Error";
        return response;
    }
    // 读取 HTTP 响应
    std::string response_str = socket.read();
    if (response_str.empty()) {
        response.status = 0;
        response.message = "No Response";
        return response;
    }
    // 解析响应
    return Response::parse(response_str);
}

#if __cplusplus >= 202002L

namespace async {

Task<Response> requests::request(IOContext &io_context, HTTPMethod method, std::string_view url, const std::vector<std::string> &querys,
                                 const std::unordered_map<std::string, std::string> &heads, std::string_view body) {
    Response response{};

    // 解析 URL
    auto [scheme, hostname, port, path, all_querys] = parseURL(url);
    auto [ip, isv6] = parseDNS(hostname);
    // 构建完整的路径（包含查询参数）
    std::string full_path = path;
    all_querys.insert(all_querys.end(), querys.begin(), querys.end());
    if (!all_querys.empty())
        full_path = full_path + "?" + str::join(all_querys, "&");
    // 建立异步 TCP 连接
    rm::async::Connector connector(io_context, Endpoint(isv6 ? ip::tcp::v6() : ip::tcp::v4(), port), ip);
    auto socket = co_await connector.connect();
    // 生成 HTTP 请求
    auto req_str = _generate(method, full_path, port, hostname, heads, body);
    // 发送 HTTP 请求
    if (!co_await socket.write(req_str)) {
        response.status = 0;
        response.message = "Connection Error";
        co_return response;
    }
    // 读取 HTTP 响应
    std::string response_str = co_await socket.read();
    if (response_str.empty()) {
        response.status = 0;
        response.message = "No Response";
        co_return response;
    }
    // 解析响应
    co_return Response::parse(response_str);
}

Webapp::~Webapp() {
    if (_running)
        stop();
}

void Webapp::use(ResponseMiddleware mwf) {
    if (!mwf)
        RMVL_Error(RMVL_StsError, "Middleware function cannot be empty");
    _mwfs.push_back(mwf);
}

static void bad_request(const Request &req, Response &res) {
    char bad_request_body[256]{};
    sprintf(bad_request_body, "<!DOCTYPE html><html lang=\"en\"><head><meta charset=\"utf-8\"><title>Error</title></head><body><pre>Cannot %s %s</pre></body></html>",
            get_str_from(req.method), req.uri.data());
    res.body = bad_request_body;
}

static void _handle(const std::vector<rm::async::Webapp::RouteEntry> &entries, Request &req, Response &res) {
    for (const auto &entry : entries) {
        if (entry.pattern.match(req.uri, req.params)) {
            entry.handler(req, res);
            DEBUG_PASS_("%s %s success", get_str_from(req.method), req.uri.c_str());
            return;
        }
    }
    DEBUG_ERROR_("%s %s failed, execute bad_request", get_str_from(req.method), req.uri.c_str());
    bad_request(req, res);
}

Task<> Webapp::spin() {
    // open event loop
    auto acceptor = async::Acceptor(_ctx, Endpoint(ip::tcp::v4(), _port));

    // execute listen callback
    if (_listen)
        _listen();

    _running = true;
    while (_running) {
        auto socket = co_await acceptor.accept();
        if (socket.invalid()) {
            ERROR_("Failed to accept connection");
            continue;
        }

        auto req = Request::parse(co_await socket.read());
        auto res = Response{};

        if (req.method == HTTPMethod::Get)
            _handle(_gets, req, res);
        else if (req.method == HTTPMethod::Post)
            _handle(_posts, req, res);
        else if (req.method == HTTPMethod::Delete)
            _handle(_deletes, req, res);
        else if (req.method == HTTPMethod::Head)
            _handle(_heads, req, res);
        else
            res.json("{ \"code\": 500, \"message\": \"unknown error\" }");

        // 响应中间件处理
        for (const auto &mwf : _mwfs)
            mwf(res);

        auto res_str = res.generate();
        bool send_status = co_await socket.write(res_str);
        if (!send_status)
            printf("Failed to send response\n");
    }
}

//////////////////////////////// 路由匹配器 //////////////////////////////////
static void _updateRegexPattern(std::string &regex, std::string_view literal) {
    for (auto c : literal) {
        if (c == '/' || c == '.' || c == '*' || c == '+' || c == '?' || c == '{' || c == '}' ||
            c == '[' || c == ']' || c == '\\' || c == '|' || c == '(' || c == ')' || c == '^' || c == '$')
            regex += '\\';
        regex += c;
    }
}

Webapp::RoutePattern::RoutePattern(std::string_view pattern_str) : _pattern(pattern_str) {
    std::string regex_pattern = "^";
    size_t pos = 0;

    while (pos < pattern_str.size()) {
        size_t param_start = pattern_str.find(':', pos);

        if (param_start == std::string_view::npos) {
            _updateRegexPattern(regex_pattern, pattern_str.substr(pos));
            break;
        }

        // 添加参数前的字面量部分
        if (param_start > pos)
            _updateRegexPattern(regex_pattern, pattern_str.substr(pos, param_start - pos));

        // 找到参数名结束位置（遇到 '/' 或字符串结束）
        size_t param_end = param_start + 1;
        while (param_end < pattern_str.size() && pattern_str[param_end] != '/' && std::isalnum(pattern_str[param_end]))
            param_end++;

        // 提取参数名
        _param_names.emplace_back(pattern_str.substr(param_start + 1, param_end - param_start - 1));

        // 添加捕获组（匹配除 '/' 外的任意字符）
        regex_pattern += "([^/]+)";
        pos = param_end;
    }

    regex_pattern += "$";
    _matcher = std::regex(regex_pattern);
}

bool Webapp::RoutePattern::match(std::string_view path, std::unordered_map<std::string, std::string> &params) const {
    std::smatch matches{};
    std::string path_str(path);

    if (!std::regex_match(path_str, matches, _matcher))
        return false;
    for (size_t i = 0; i < _param_names.size() && i + 1 < matches.size(); ++i)
        params[_param_names[i]] = matches[i + 1].str();

    return true;
}

} // namespace async

#endif

} // namespace rm
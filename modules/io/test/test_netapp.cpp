/**
 * @file test_netapp.cpp
 * @author zhaoxi (535394140@qq.com)
 * @brief
 * @version 1.0
 * @date 2025-09-14
 *
 * @copyright Copyright 2025 (c), zhaoxi
 *
 */

#include <chrono>

#include <gtest/gtest.h>

#include "rmvl/io/netapp.hpp"

#if __cplusplus >= 202002L
#include <thread>
#endif

using namespace rm;

namespace rm_test {

TEST(IO_netapp, basic_parse) {
    auto [addr, is_ipv6] = parseDNS("localhost");
    if (addr == "::1")
        EXPECT_TRUE(is_ipv6);
    else {
        EXPECT_EQ(addr, "127.0.0.1");
        EXPECT_FALSE(is_ipv6);
    }

    auto [scheme, host, port, path, query] = parseURL("http://example.com:8080/path/to/resource?key=value&key2=value2");
    EXPECT_EQ(scheme, "http");
    EXPECT_EQ(host, "example.com");
    EXPECT_EQ(port, 8080);
    EXPECT_EQ(path, "/path/to/resource");
    EXPECT_EQ(query.size(), 2);
    EXPECT_EQ(query[0], "key=value");
    EXPECT_EQ(query[1], "key2=value2");
}

constexpr const char httpreq[] =
    "POST /api/test?k1=v1&k2=v2 HTTP/1.1\r\n"
    "Host: www.test.com\r\n"
    "User-Agent: rmvl\r\n"
    "Accept: */*\r\n\r\n"
    "{\"name\":\"rmvl_test\",\"message\":\"test\"}";

TEST(IO_netapp, basic_http) {
    auto req = Request::parse(httpreq);

    EXPECT_EQ(req.method, HTTPMethod::Post);
    EXPECT_EQ(req.uri, "/api/test");
    EXPECT_EQ(req.query.size(), 2);
    EXPECT_EQ(req.query["k1"], "v1");
    EXPECT_EQ(req.query["k2"], "v2");
    EXPECT_EQ(req.host, "www.test.com");
    EXPECT_EQ(req.accept, "*/*");
    EXPECT_EQ(req.body, "{\"name\":\"rmvl_test\",\"message\":\"test\"}");
}

TEST(IO_netapp, sync_request) {
    auto res = requests::get("http://www.example.com");
    EXPECT_EQ(res.status, 200);
    EXPECT_EQ(res.message, "OK");
    EXPECT_NE(res.body.find("<title>Example Domain</title>"), std::string::npos);
}

#if __cplusplus >= 202002L

TEST(IO_netapp, async_request) {
    auto io_context = async::IOContext{};

    auto task = [&]() -> async::Task<> {
        auto res = co_await async::requests::get(io_context, "http://www.example.com");
        EXPECT_EQ(res.status, 200);
        EXPECT_EQ(res.message, "OK");
        EXPECT_NE(res.body.find("<title>Example Domain</title>"), std::string::npos);

        io_context.stop();
    };

    co_spawn(io_context, task);
    io_context.run();
}

using namespace std::literals::chrono_literals;

TEST(IO_netapp, webapp) {
    async::IOContext io_context{};
    async::Webapp app(io_context);

    app.use(cors);
    app.get("/", [](const Request &, Response &res) {
        res.send("<html><body><h1>Hello, World!</h1></body></html>");
    });
    app.get("/test", [](const Request &, Response &res) {
        res.redirect("/");
    });
    app.get("/api", [](const Request &, Response &res) {
        res.json("{ \"key\": \"value\", \"message\": \"This is a test API.\" }");
    });
    app.get("/api/name/:name", [](const Request &req, Response &res) {
        auto name = req.params.at("name");
        res.json("{ \"greeting\": \"Hello, " + name + "!\" }");
    });
    app.get("/str", [](const Request &, Response &res) {
        res.send("only string");
    });
    app.post("/pstr", [](const Request &req, Response &res) {
        res.send("pstr " + req.body);
    });
    app.post("/api/data", [](const Request &req, Response &res) {
        auto data = req.body;
        res.send("{ \"received\": " + data + " }");
    });
    app.del("/del/:id", [](const Request &req, Response &res) {
        auto id = req.params.at("id");
        res.send("Deleted item with id " + id);
    });
    app.listen(10802);
    co_spawn(
        io_context, [](async::Webapp &app) -> async::Task<> {
            co_await app.spin();
        },
        std::ref(app));

    auto thrd = std::jthread([&]() {
        // Get /str
        auto res = requests::get("http://127.0.0.1:10802/str");
        EXPECT_EQ(res.status, 200);
        EXPECT_EQ(res.body, "only string");
        // Get /test 重定向至 /
        res = requests::get("http://127.0.0.1:10802/test");
        EXPECT_EQ(res.status, 302);
        EXPECT_EQ(res.heads["Location"], "/");
        // Get /api/name/rmvl
        res = requests::get("http://127.0.0.1:10802/api/name/rmvl");
        EXPECT_EQ(res.status, 200);
        EXPECT_EQ(res.body, "{ \"greeting\": \"Hello, rmvl!\" }");
        // Post /pstr
        res = requests::post("http://127.0.0.1:10802/pstr", "bonjour");
        EXPECT_EQ(res.status, 200);
        EXPECT_EQ(res.body, "pstr bonjour");
        // Delete /del/123
        res = requests::del("http://127.0.0.1:10802/del/123");
        EXPECT_EQ(res.status, 200);
        EXPECT_EQ(res.body, "Deleted item with id 123");

        app.stop();
        io_context.stop();
    });
    io_context.run();
}

#endif

} // namespace rm_test
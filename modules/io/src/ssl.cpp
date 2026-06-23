/**
 * @file ssl.cpp
 * @author zhaoxi (535394140@qq.com)
 * @brief 基于 OpenSSL 的 TLS/SSL 安全流封装
 * @version 1.0
 * @date 2026-06-21
 *
 * @copyright Copyright 2026 (c), zhaoxi
 *
 */

#include "rmvl/io/ssl.hpp"

#include "rmvl/core/util.hpp"

#ifdef HAVE_OPENSSL
#include <openssl/err.h>
#include <openssl/ssl.h>
#endif

#if __cplusplus >= 202002L
#ifndef _WIN32
#include <cerrno>
#include <cstring>
#include <sys/epoll.h>
#endif
#endif

namespace rm {

#ifdef HAVE_OPENSSL

static std::string ssl_error_string() {
    auto err = ERR_get_error();
    if (err == 0)
        return {};
    char buf[128]{};
    ERR_error_string_n(err, buf, sizeof(buf));
    return buf;
}

static void set_error(std::string &dst, std::string_view prefix) {
    auto err = ssl_error_string();
    dst.assign(prefix);
    if (!err.empty()) {
        dst.append(": ");
        dst.append(err);
    }
}

void SSLContext::free_ctx(void *ctx) noexcept { SSL_CTX_free(static_cast<SSL_CTX *>(ctx)); }

bool SSLContext::available() noexcept { return true; }

SSLContext::SSLContext(SSLMode mode) : _mode(mode) {
    OPENSSL_init_ssl(0, nullptr);
    const SSL_METHOD *method = mode == SSLMode::Client ? TLS_client_method() : TLS_server_method();
    _ctx.reset(SSL_CTX_new(method));
    if (!_ctx)
        _lasterr = ssl_error_string();
}

bool SSLContext::load_cert(std::string_view cert_file, std::string_view key_file) {
    if (!_ctx) {
        _lasterr = "invalid SSL context";
        return false;
    }
    std::string cert(cert_file);
    std::string key(key_file);
    auto *ctx = static_cast<SSL_CTX *>(_ctx.get());
    if (SSL_CTX_use_certificate_file(ctx, cert.c_str(), SSL_FILETYPE_PEM) != 1) {
        set_error(_lasterr, "failed to load certificate");
        return false;
    }
    if (SSL_CTX_use_PrivateKey_file(ctx, key.c_str(), SSL_FILETYPE_PEM) != 1) {
        set_error(_lasterr, "failed to load private key");
        return false;
    }
    if (SSL_CTX_check_private_key(ctx) != 1) {
        set_error(_lasterr, "certificate and private key do not match");
        return false;
    }
    _lasterr.clear();
    return true;
}

bool SSLContext::load_ca(std::string_view ca_file, std::string_view ca_path) {
    if (!_ctx) {
        _lasterr = "invalid SSL context";
        return false;
    }
    std::string file(ca_file);
    std::string path(ca_path);
    const char *file_ptr = file.empty() ? nullptr : file.c_str();
    const char *path_ptr = path.empty() ? nullptr : path.c_str();
    if (!file_ptr && !path_ptr) {
        _lasterr = "empty CA file and CA path";
        return false;
    }
    if (SSL_CTX_load_verify_locations(static_cast<SSL_CTX *>(_ctx.get()), file_ptr, path_ptr) != 1) {
        set_error(_lasterr, "failed to load CA");
        return false;
    }
    _lasterr.clear();
    return true;
}

void SSLContext::set_verify_mode(SSLVerifyMode mode) {
    if (!_ctx)
        return;
    int verify = mode == SSLVerifyMode::Peer ? SSL_VERIFY_PEER : SSL_VERIFY_NONE;
    SSL_CTX_set_verify(static_cast<SSL_CTX *>(_ctx.get()), verify, nullptr);
}

void SSLStream::free_ssl(void *ssl) noexcept { SSL_free(reinterpret_cast<SSL *>(ssl)); }

SSLStream::SSLStream(StreamSocket socket, SSLContext &ctx) : _ctx(ctx), _socket(std::move(socket)) {
    auto ssl = SSL_new(static_cast<SSL_CTX *>(ctx.native_handle()));
    _ssl.reset(ssl);
    if (!ssl) {
        set_error(_lasterr, "failed to create SSL object");
        return;
    }
    auto bio = BIO_new_socket(static_cast<int>(_socket.native_handle()), BIO_NOCLOSE);
    if (!bio) {
        set_error(_lasterr, "failed to create SSL socket BIO");
        _ssl.reset();
        return;
    }
    SSL_set_bio(ssl, bio, bio);
}

SSLStream::~SSLStream() { close(); }

bool SSLStream::connect(std::string_view server_name) {
    if (!_ssl) {
        _lasterr = "invalid SSL stream";
        return false;
    }
    if (!server_name.empty()) {
        std::string name(server_name);
        SSL_set_tlsext_host_name(static_cast<SSL *>(_ssl.get()), name.c_str());
    }
    if (SSL_connect(static_cast<SSL *>(_ssl.get())) != 1) {
        set_error(_lasterr, "TLS client handshake failed");
        return false;
    }
    _lasterr.clear();
    return true;
}

bool SSLStream::accept() {
    if (!_ssl) {
        _lasterr = "invalid SSL stream";
        return false;
    }
    if (SSL_accept(static_cast<SSL *>(_ssl.get())) != 1) {
        set_error(_lasterr, "TLS server handshake failed");
        return false;
    }
    _lasterr.clear();
    return true;
}

bool SSLStream::handshake(std::string_view server_name) {
    return context().mode() == SSLMode::Client ? connect(server_name) : accept();
}

std::string SSLStream::read(size_t max_size) noexcept {
    std::string buf(max_size, '\0');
    auto n = read_to(buf.data(), buf.size());
    if (n == 0)
        return {};
    buf.resize(n);
    return buf;
}

size_t SSLStream::read_to(char *buf, size_t size) noexcept {
    if (!_ssl || !buf || size == 0)
        return 0;
    int n = SSL_read(static_cast<SSL *>(_ssl.get()), buf, static_cast<int>(size));
    return n > 0 ? static_cast<size_t>(n) : 0;
}

bool SSLStream::write(std::string_view data) noexcept {
    if (!_ssl)
        return false;
    int n = SSL_write(static_cast<SSL *>(_ssl.get()), data.data(), static_cast<int>(data.size()));
    return n == static_cast<int>(data.size());
}

void SSLStream::close() noexcept {
    if (_ssl) {
        SSL_shutdown(static_cast<SSL *>(_ssl.get()));
        _ssl.reset();
    }
    _socket.close();
}

#else

void SSLContext::free_ctx(void *) noexcept {}
bool SSLContext::available() noexcept { return false; }
SSLContext::SSLContext(SSLMode mode) : _mode(mode) { _lasterr = "OpenSSL is not enabled"; }
bool SSLContext::load_cert(std::string_view, std::string_view) {
    _lasterr = "OpenSSL is not enabled";
    return false;
}
bool SSLContext::load_ca(std::string_view, std::string_view) {
    _lasterr = "OpenSSL is not enabled";
    return false;
}
void SSLContext::set_verify_mode(SSLVerifyMode) {}

void SSLStream::free_ssl(void *) noexcept {}
SSLStream::SSLStream(StreamSocket socket, SSLContext &ctx) : _ctx(ctx), _socket(std::move(socket)) { _lasterr = "OpenSSL is not enabled"; }
SSLStream::~SSLStream() { close(); }
bool SSLStream::connect(std::string_view) { return false; }
bool SSLStream::accept() { return false; }
bool SSLStream::handshake(std::string_view) { return false; }
std::string SSLStream::read(size_t) noexcept { return {}; }
size_t SSLStream::read_to(char *, size_t) noexcept { return 0; }
bool SSLStream::write(std::string_view) noexcept { return false; }
void SSLStream::close() noexcept { _socket.close(); }

#endif

#if __cplusplus >= 202002L

namespace async {

#ifdef HAVE_OPENSSL

static inline bool ssl_wait_required(int error) { return error == SSL_ERROR_WANT_READ || error == SSL_ERROR_WANT_WRITE; }

static inline bool ssl_wait_write(int error) { return error == SSL_ERROR_WANT_WRITE; }

static inline void set_async_error(std::string &dst, std::string_view prefix) { set_error(dst, prefix); }

SSLStream::SSLStream(StreamSocket socket, SSLContext &ctx) : ::rm::SSLStream(static_cast<::rm::StreamSocket &&>(socket), ctx), _ctx(socket.context()) {}

void SSLStream::SSLIOAwaiter::await_suspend(std::coroutine_handle<> handle) {
    RMVL_DbgAssert(_fd != INVALID_FD);
#ifdef _WIN32
    _ovl = std::make_unique<IocpOverlapped>(handle);
    DWORD len = 0;
    if (!_wait_write) {
        WSABUF buf{};
        buf.buf = _ovl->buf;
        buf.len = 0;
        DWORD flags = 0;
        if (WSARecv((SOCKET)_fd, &buf, 1, &len, &flags, &_ovl->ov, nullptr) == SOCKET_ERROR) {
            DWORD error = WSAGetLastError();
            if (error != ERROR_IO_PENDING)
                RMVL_Error_(RMVL_StsBadArg, "TLS wait read failed: %lu", error);
        }
    } else {
        WSABUF buf{};
        buf.buf = _ovl->buf;
        buf.len = 0;
        if (WSASend((SOCKET)_fd, &buf, 1, &len, 0, &_ovl->ov, nullptr) == SOCKET_ERROR) {
            DWORD error = WSAGetLastError();
            if (error != ERROR_IO_PENDING)
                RMVL_Error_(RMVL_StsBadArg, "TLS wait write failed: %lu", error);
        }
    }
#else
    epoll_event ev{};
    ev.events = _wait_write ? EPOLLOUT : EPOLLIN;
    ev.data.ptr = handle.address();
    if (epoll_ctl(_aioh, EPOLL_CTL_ADD, _fd, &ev) == -1)
        RMVL_Error_(RMVL_StsBadArg, "Failed to add TLS fd to epoll: %s", std::strerror(errno));
#endif
}

void SSLStream::SSLIOAwaiter::await_resume() noexcept {
#ifndef _WIN32
    if (_fd != INVALID_FD)
        epoll_ctl(_aioh, EPOLL_CTL_DEL, _fd, nullptr);
#endif
}

Task<bool> SSLStream::do_handshake(std::string_view server_name, bool client_mode) {
    auto *ssl = static_cast<SSL *>(native_handle());
    if (!ssl) {
        _lasterr = "invalid SSL stream";
        co_return false;
    }
    if (client_mode && !server_name.empty()) {
        std::string name(server_name);
        SSL_set_tlsext_host_name(ssl, name.c_str());
    }

    while (true) {
        int rc = client_mode ? SSL_connect(ssl) : SSL_accept(ssl);
        if (rc == 1) {
            _lasterr.clear();
            co_return true;
        }

        int error = SSL_get_error(ssl, rc);
        if (!ssl_wait_required(error)) {
            set_async_error(_lasterr, client_mode ? "TLS client handshake failed" : "TLS server handshake failed");
            co_return false;
        }
        co_await SSLIOAwaiter(_ctx, socket().native_handle(), ssl_wait_write(error));
    }
}

Task<bool> SSLStream::connect(std::string_view server_name) { co_return co_await do_handshake(server_name, true); }
Task<bool> SSLStream::accept() { co_return co_await do_handshake({}, false); }
Task<bool> SSLStream::handshake(std::string_view server_name) { co_return co_await do_handshake(server_name, context().mode() == SSLMode::Client); }

Task<std::string> SSLStream::read(size_t max_size) {
    std::string buf(max_size, '\0');
    auto n = co_await read_to(buf.data(), buf.size());
    if (n == 0)
        co_return std::string{};
    buf.resize(n);
    co_return buf;
}

Task<size_t> SSLStream::read_to(char *buf, size_t size) {
    auto *ssl = static_cast<SSL *>(native_handle());
    if (!ssl || !buf || size == 0)
        co_return 0U;

    while (true) {
        int rc = SSL_read(ssl, buf, static_cast<int>(size));
        if (rc > 0) {
            _lasterr.clear();
            co_return static_cast<size_t>(rc);
        }

        int error = SSL_get_error(ssl, rc);
        if (!ssl_wait_required(error)) {
            set_async_error(_lasterr, "TLS read failed");
            co_return 0U;
        }
        co_await SSLIOAwaiter(_ctx, socket().native_handle(), ssl_wait_write(error));
    }
}

Task<bool> SSLStream::write(std::string_view data) {
    auto *ssl = static_cast<SSL *>(native_handle());
    if (!ssl)
        co_return false;

    size_t offset = 0;
    while (offset < data.size()) {
        auto remain = data.substr(offset);
        int rc = SSL_write(ssl, remain.data(), static_cast<int>(remain.size()));
        if (rc > 0) {
            offset += static_cast<size_t>(rc);
            continue;
        }

        int error = SSL_get_error(ssl, rc);
        if (!ssl_wait_required(error)) {
            set_async_error(_lasterr, "TLS write failed");
            co_return false;
        }
        co_await SSLIOAwaiter(_ctx, socket().native_handle(), ssl_wait_write(error));
    }

    _lasterr.clear();
    co_return true;
}

#else

SSLStream::SSLStream(StreamSocket socket, SSLContext &ctx)
    : ::rm::SSLStream(static_cast<::rm::StreamSocket &&>(socket), ctx), _ctx(socket.context()), _lasterr("OpenSSL is not enabled") {}

void SSLStream::SSLIOAwaiter::await_suspend(std::coroutine_handle<> handle) { handle.resume(); }
void SSLStream::SSLIOAwaiter::await_resume() noexcept {}

Task<bool> SSLStream::do_handshake(std::string_view, bool) {
    _lasterr = "OpenSSL is not enabled";
    co_return false;
}

Task<bool> SSLStream::connect(std::string_view) {
    _lasterr = "OpenSSL is not enabled";
    co_return false;
}

Task<bool> SSLStream::accept() {
    _lasterr = "OpenSSL is not enabled";
    co_return false;
}

Task<bool> SSLStream::handshake(std::string_view) {
    _lasterr = "OpenSSL is not enabled";
    co_return false;
}

Task<std::string> SSLStream::read(size_t) {
    _lasterr = "OpenSSL is not enabled";
    co_return std::string{};
}

Task<size_t> SSLStream::read_to(char *, size_t) {
    _lasterr = "OpenSSL is not enabled";
    co_return 0U;
}

Task<bool> SSLStream::write(std::string_view) {
    _lasterr = "OpenSSL is not enabled";
    co_return false;
}

#endif

} // namespace async

#endif

} // namespace rm

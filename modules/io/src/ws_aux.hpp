/**
 * @file ws_aux.hpp
 * @author zhaoxi
 * @brief WebSocket 握手辅助工具 (SHA1 & Base64)
 * @note 这是一个纯头文件实现，用于避免引入 OpenSSL 依赖
 */

#pragma once

#include <array>
#include <cstdint>
#include <string>
#include <string_view>

namespace rm::async::ws_helper {

namespace detail {

class SHA1 {
public:
    constexpr SHA1() { reset(); }
    constexpr SHA1(std::string_view s) : SHA1() { update(s); }

    constexpr void update(std::string_view s) {
        for (char c : s) {
            _buffer[_buffer_idx++] = static_cast<uint8_t>(c);
            if (_buffer_idx == 64) {
                _process_block();
                _buffer_idx = 0;
            }
        }
        _byte_count += s.size();
    }

    constexpr std::array<uint8_t, 20> final() {
        // Pad the message
        _buffer[_buffer_idx++] = 0x80;
        if (_buffer_idx > 56) {
            while (_buffer_idx < 64)
                _buffer[_buffer_idx++] = 0;
            _process_block();
            _buffer_idx = 0;
        }
        while (_buffer_idx < 56)
            _buffer[_buffer_idx++] = 0;
        // Append length (bits)
        uint64_t bit_len = _byte_count * 8;
        for (int i = 7; i >= 0; --i) {
            _buffer[56 + i] = static_cast<uint8_t>(bit_len & 0xFF);
            bit_len >>= 8;
        }
        _process_block();
        std::array<uint8_t, 20> digest{};
        for (int i = 0; i < 5; ++i) {
            digest[i * 4 + 0] = static_cast<uint8_t>((_state[i] >> 24) & 0xFF);
            digest[i * 4 + 1] = static_cast<uint8_t>((_state[i] >> 16) & 0xFF);
            digest[i * 4 + 2] = static_cast<uint8_t>((_state[i] >> 8) & 0xFF);
            digest[i * 4 + 3] = static_cast<uint8_t>((_state[i] >> 0) & 0xFF);
        }
        return digest;
    }

private:
    constexpr void reset() {
        _state[0] = 0x67452301;
        _state[1] = 0xEFCDAB89;
        _state[2] = 0x98BADCFE;
        _state[3] = 0x10325476;
        _state[4] = 0xC3D2E1F0;
        _byte_count = 0;
        _buffer_idx = 0;
        for (auto &b : _buffer)
            b = 0;
    }

    static constexpr uint32_t rol(uint32_t value, std::size_t bits) {
        return (value << bits) | (value >> (32 - bits));
    }

    constexpr void _process_block() {
        uint32_t w[80]{};
        for (int i = 0; i < 16; ++i)
            w[i] = (static_cast<uint32_t>(_buffer[i * 4 + 0]) << 24) | (static_cast<uint32_t>(_buffer[i * 4 + 1]) << 16) |
                   (static_cast<uint32_t>(_buffer[i * 4 + 2]) << 8) | (static_cast<uint32_t>(_buffer[i * 4 + 3]));
        for (int i = 16; i < 80; ++i)
            w[i] = rol(w[i - 3] ^ w[i - 8] ^ w[i - 14] ^ w[i - 16], 1);

        uint32_t a = _state[0], b = _state[1], c = _state[2], d = _state[3], e = _state[4];

        for (int i = 0; i < 80; ++i) {
            uint32_t f = 0, k = 0;
            if (i < 20) {
                f = (b & c) | ((~b) & d);
                k = 0x5A827999;
            } else if (i < 40) {
                f = b ^ c ^ d;
                k = 0x6ED9EBA1;
            } else if (i < 60) {
                f = (b & c) | (b & d) | (c & d);
                k = 0x8F1BBCDC;
            } else {
                f = b ^ c ^ d;
                k = 0xCA62C1D6;
            }
            uint32_t temp = rol(a, 5) + f + e + k + w[i];
            e = d;
            d = c;
            c = rol(b, 30);
            b = a;
            a = temp;
        }

        _state[0] += a;
        _state[1] += b;
        _state[2] += c;
        _state[3] += d;
        _state[4] += e;
    }

    uint32_t _state[5]{};
    uint8_t _buffer[64]{};
    std::size_t _buffer_idx{};
    uint64_t _byte_count{};
};

inline std::string base64_encode(const std::array<uint8_t, 20> &in) {
    std::string out;
    out.reserve(((20 + 2) / 3) * 4);

    constexpr char tab[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

    int val = 0, valb = -6;
    for (uint8_t c : in) {
        val = (val << 8) + c;
        valb += 8;
        while (valb >= 0) {
            out.push_back(tab[(val >> valb) & 0x3F]);
            valb -= 6;
        }
    }
    if (valb > -6)
        out.push_back(tab[((val << 8) >> (valb + 8)) & 0x3F]);
    while (out.size() % 4)
        out.push_back('=');

    return out;
}

} // namespace detail

/**
 * @brief 生成 WebSocket 握手所需的 Accept Key
 * @param[in] client_key 客户端请求头中的 Sec-WebSocket-Key
 * @return 服务器应答的 Sec-WebSocket-Accept
 */
inline std::string generate_accept_key(std::string_view client_key) {
    constexpr std::string_view magic_guid = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";

    detail::SHA1 sha1;
    sha1.update(client_key);
    sha1.update(magic_guid);
    auto digest = sha1.final();

    return detail::base64_encode(digest);
}

} // namespace rm::async::ws_helper

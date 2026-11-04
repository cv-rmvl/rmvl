/**
 * @file toml.hpp
 * @brief TOML 1.0 解析、只读节点树和类型转换
 * @copyright Copyright 2026 (c), zhaoxi
 */
#pragma once

#include <array>
#include <cmath>
#include <cstdint>
#include <limits>
#include <map>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <vector>

#include "rmvl/core/rmvldef.hpp"

namespace rm::toml {

//! @defgroup core_toml TOML 解析模块
//! @ingroup core
//! @{

//! TOML 节点类型，Undefined 仅表示无效句柄，不是 TOML 值。
enum class NodeType : uint8_t {
    Undefined, //!< 未定义
    Table,
    Array,
    String,
    Integer,
    Floating,
    Boolean,
    Date,
    Time,
    DateTime
};

//! 本地日期
struct Date {
    int year{};
    unsigned month{}, day{};
};

//! 本地时间，小数秒保留至纳秒，超出部分截断。
struct Time {
    unsigned hour{}, minute{}, second{}, nanosecond{};
};

//! 日期时间，无偏移表示本地日期时间，偏移 0 表示 UTC。
struct DateTime {
    Date date;
    Time time;
    std::optional<int> offsetMinutes;
};

//! TOML 语法或定义冲突，行列号从 1 开始。
class RMVL_EXPORTS ParseError : public std::runtime_error {
public:
    ParseError(std::string message, std::size_t line, std::size_t column)
        : std::runtime_error(std::move(message)), line(line), column(column) {}
    const std::size_t line;
    const std::size_t column;
};

//! 文件打开或读取失败。
class RMVL_EXPORTS IoError : public std::runtime_error {
public:
    using std::runtime_error::runtime_error;
};

/**
 * @brief 共享底层文档所有权的只读节点句柄
 * @details 拷贝不复制树。子节点可独立于根句柄存活，同一文档支持并发只读访问。
 * 返回的键名视图在底层文档最后一个句柄销毁前有效。
 */
class RMVL_EXPORTS Node {
public:
    Node() noexcept = default;
    [[nodiscard]] bool valid() const noexcept { return _impl && _node != nullptr; }
    //! 只判断句柄有效性，与节点保存的布尔值无关。
    explicit operator bool() const noexcept { return valid(); }
    NodeType type() const noexcept;
    bool isTable() const noexcept { return type() == NodeType::Table; }
    bool isArray() const noexcept { return type() == NodeType::Array; }
    //! 非容器节点返回 0。
    std::size_t size() const noexcept;
    bool contains(std::string_view key) const noexcept;
    //! 查询字面键，不解释点分路径，不插入节点。
    Node operator[](std::string_view key) const noexcept;
    //! 仅适用于数组，越界或类型不符返回无效节点。
    Node operator[](std::size_t index) const noexcept;
    std::vector<std::string_view> keys() const;

    //! 严格类型转换并检查数值范围，失败时保持输出对象不变。
    template <typename T>
    bool read(T &value) const;
    template <typename T>
    std::optional<T> as() const;
    template <typename T>
    T valueOr(T fallback) const;

private:
    struct Impl;
    std::shared_ptr<const Impl> _impl;
    const void *_node{};
    Node(std::shared_ptr<const Impl> impl, const void *node) noexcept
        : _impl(std::move(impl)), _node(node) {}
    bool readValue(std::string &value) const;
    bool readValue(bool &value) const noexcept;
    bool readValue(int64_t &value) const noexcept;
    bool readValue(double &value) const noexcept;
    bool readValue(Date &value) const noexcept;
    bool readValue(Time &value) const noexcept;
    bool readValue(DateTime &value) const noexcept;
    friend Node loads(std::string_view source);
};

//! 从文件加载，成功时返回 Table 根节点。失败抛 IoError 或 ParseError。
RMVL_EXPORTS Node load(std::string_view path);
//! 从 UTF-8 文本加载，成功时返回 Table 根节点，语法错误抛 ParseError。
RMVL_EXPORTS Node loads(std::string_view source);

//! @cond
namespace detail {
template <typename>
struct optional_traits : std::false_type {};
template <typename T>
struct optional_traits<std::optional<T>> : std::true_type {
    using value_type = T;
};
template <typename>
struct vector_traits : std::false_type {};
template <typename T, typename A>
struct vector_traits<std::vector<T, A>> : std::true_type {
    using value_type = T;
};
template <typename>
struct array_traits : std::false_type {};
template <typename T, std::size_t N>
struct array_traits<std::array<T, N>> : std::true_type {
    using value_type = T;
};
template <typename>
struct map_traits : std::false_type {};
template <typename V, typename C, typename A>
struct map_traits<std::map<std::string, V, C, A>> : std::true_type {
    using value_type = V;
};
template <typename V, typename H, typename E, typename A>
struct map_traits<std::unordered_map<std::string, V, H, E, A>> : std::true_type {
    using value_type = V;
};
} // namespace detail
//! @endcond

template <typename T>
bool Node::read(T &value) const {
    using U = std::remove_cv_t<T>;
    if constexpr (std::is_same_v<U, std::string> || std::is_same_v<U, bool> ||
                  std::is_same_v<U, Date> || std::is_same_v<U, Time> || std::is_same_v<U, DateTime>) {
        return readValue(value);
    } else if constexpr (std::is_enum_v<U>) {
        std::underlying_type_t<U> raw{};
        if (!read(raw))
            return false;
        value = static_cast<U>(raw);
        return true;
    } else if constexpr (std::is_integral_v<U>) {
        static_assert(sizeof(U) <= sizeof(int64_t), "unsupported TOML integer width");
        int64_t raw{};
        if (!readValue(raw))
            return false;
        if constexpr (std::is_signed_v<U>) {
            if (raw < static_cast<int64_t>(std::numeric_limits<U>::min()) ||
                raw > static_cast<int64_t>(std::numeric_limits<U>::max()))
                return false;
        } else {
            if (raw < 0 || static_cast<uint64_t>(raw) > std::numeric_limits<U>::max())
                return false;
        }
        value = static_cast<U>(raw);
        return true;
    } else if constexpr (std::is_floating_point_v<U>) {
        double raw{};
        if (!readValue(raw))
            return false;
        if (std::isfinite(raw) && (raw < -std::numeric_limits<U>::max() || raw > std::numeric_limits<U>::max()))
            return false;
        value = static_cast<U>(raw);
        return true;
    } else if constexpr (detail::optional_traits<U>::value) {
        if (!valid()) {
            value.reset();
            return true;
        }
        typename detail::optional_traits<U>::value_type item{};
        if (!read(item))
            return false;
        value = std::move(item);
        return true;
    } else if constexpr (detail::vector_traits<U>::value) {
        if (!isArray())
            return false;
        U result;
        result.reserve(size());
        for (std::size_t i = 0; i < size(); ++i) {
            typename U::value_type item{};
            if (!(*this)[i].read(item))
                return false;
            result.push_back(std::move(item));
        }
        value = std::move(result);
        return true;
    } else if constexpr (detail::array_traits<U>::value) {
        U result{};
        if (!isArray() || size() != result.size())
            return false;
        for (std::size_t i = 0; i < size(); ++i)
            if (!(*this)[i].read(result[i]))
                return false;
        value = std::move(result);
        return true;
    } else if constexpr (detail::map_traits<U>::value) {
        if (!isTable())
            return false;
        U result;
        for (auto key : keys()) {
            typename U::mapped_type item{};
            if (!(*this)[key].read(item))
                return false;
            result.emplace(std::string(key), std::move(item));
        }
        value = std::move(result);
        return true;
    } else {
        // 用户类型在其命名空间提供 bool toml_decode(const Node &, T &)。
        U result{};
        if (!toml_decode(*this, result))
            return false;
        value = std::move(result);
        return true;
    }
}

template <typename T>
std::optional<T> Node::as() const {
    T value{};
    if (!read(value))
        return std::nullopt;
    return value;
}

template <typename T>
T Node::valueOr(T fallback) const {
    T value{};
    return read(value) ? std::move(value) : std::move(fallback);
}

//! @} core_toml
} // namespace rm::toml

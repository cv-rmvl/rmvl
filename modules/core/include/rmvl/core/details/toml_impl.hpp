/**
 * @file toml_impl.hpp
 * @brief TOML 节点类型转换的模板实现
 * @copyright Copyright 2026 (c), zhaoxi
 */

#pragma once

#include "rmvl/core/toml.hpp"

#include <array>
#include <cmath>
#include <limits>
#include <map>
#include <type_traits>
#include <unordered_map>

//! @cond

namespace rm::toml {

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

} // namespace rm::toml

//! @endcond

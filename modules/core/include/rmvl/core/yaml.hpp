/**
 * @file yaml.hpp
 * @author zhaoxi (535394140@qq.com)
 * @brief YAML 树模型、类型转换和文件读写
 * @version 1.0
 * @date 2026-08-21
 *
 * @copyright Copyright 2026 (c), zhaoxi
 */

#pragma once

#include <array>
#include <cstdint>
#include <limits>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <vector>

#include "rmvl/core/rmvldef.hpp"

namespace rm::yaml {

//! @defgroup core_yaml YAML 解析模块
//! @ingroup core
//! @{
//! @brief 提供基于 rapidyaml 的高性能 YAML 解析、树访问、类型转换和文件读写功能
//! @details Node 是共享底层 YAML 树所有权的轻量句柄。子节点可安全脱离根节点保存，但同一棵树不提供内部线程同步。

//! YAML 节点类型
enum class NodeType : uint8_t {
    Undefined, //!< 无效或不存在的节点
    Null,      //!< 空值
    Scalar,    //!< 标量
    Sequence,  //!< 序列
    Map,       //!< 映射
};

//! YAML 操作错误类型
enum class ErrorCode : uint8_t {
    None,  //!< 无错误
    Parse, //!< YAML 语法错误
    Io,    //!< 文件读写错误
};

//! YAML 操作错误
struct Error {
    ErrorCode code{ErrorCode::None}; //!< 错误类型
    std::string message{};           //!< 错误描述
    std::size_t line{};              //!< 语法错误行号，不适用时为 0
    std::size_t column{};            //!< 语法错误列号，不适用时为 0

    //! @return 是否包含错误
    explicit operator bool() const noexcept { return code != ErrorCode::None; }
};

struct Result;

/**
 * @brief YAML 树节点句柄
 * @details
 * - 拷贝 Node 仅复制共享所有权和节点标识，不复制 YAML 数据
 * - scalar()、key() 和 keys() 返回的视图会在同一棵树发生写入后失效
 * - 删除节点后，指向被删除节点及其后代的既有句柄不可继续使用
 * @note 同一棵树上的并发写入，或读写并发，需要由调用方同步。
 */
class RMVL_EXPORTS_W Node {
public:
    //! 构造无效节点
    Node() noexcept = default;

    //! 创建空映射根节点
    static Node createMap();

    //! 创建空序列根节点
    static Node createSequence();

    /**
     * @brief 创建标量根节点
     *
     * @param[in] value 标量文本
     * @return 新建的标量节点
     */
    static Node createScalar(std::string_view value);

    //! @return 节点是否有效
    bool valid() const noexcept;

    //! @return 节点类型
    NodeType type() const noexcept;

    //! @return 是否为空值节点
    bool isNull() const noexcept { return type() == NodeType::Null; }

    //! @return 是否为标量节点
    bool isScalar() const noexcept { return type() == NodeType::Scalar; }

    //! @return 是否为序列节点
    bool isSequence() const noexcept { return type() == NodeType::Sequence; }

    //! @return 是否为映射节点
    bool isMap() const noexcept { return type() == NodeType::Map; }

    //! @return 子节点数量，非容器节点返回 0
    std::size_t size() const noexcept;

    //! @return 节点键名，根节点和序列元素返回空视图
    std::string_view key() const noexcept;

    //! @return 标量文本，非标量和空值节点返回空视图
    std::string_view scalar() const noexcept;

    /**
     * @brief 判断映射是否包含指定键
     *
     * @param[in] name 键名
     * @return 是否存在对应子节点
     */
    bool contains(std::string_view name) const noexcept;

    /**
     * @brief 查找映射子节点
     *
     * @param[in] name 键名
     * @return 找到时返回子节点，否则返回无效节点
     */
    Node operator[](std::string_view name) const noexcept;

    /**
     * @brief 获取指定位置的容器子节点
     *
     * @param[in] index 从 0 开始的子节点位置
     * @return 索引有效时返回子节点，否则返回无效节点
     */
    Node operator[](std::size_t index) const noexcept;

    //! @return 映射的全部键名，非映射节点返回空数组
    std::vector<std::string_view> keys() const;

    /**
     * @brief 获取或创建映射子节点
     *
     * @param[in] name 键名
     * @return 对应子节点，当前节点无效时返回无效节点
     * @note 当前节点不是映射时，其原有内容会被替换为空映射。
     */
    Node ensure(std::string_view name);

    /**
     * @brief 向序列末尾添加空值节点
     *
     * @return 新增子节点，当前节点无效时返回无效节点
     * @note 当前节点不是序列时，其原有内容会被替换为空序列。
     */
    Node append();

    /**
     * @brief 删除映射子节点
     *
     * @param[in] name 键名
     * @return 是否找到并删除节点
     */
    bool erase(std::string_view name);

    /**
     * @brief 删除指定位置的容器子节点
     *
     * @param[in] index 从 0 开始的子节点位置
     * @return 是否找到并删除节点
     */
    bool erase(std::size_t index);

    //! 将节点替换为空值
    void setNull();

    //! 将节点替换为空映射
    void makeMap();

    //! 将节点替换为空序列
    void makeSequence();

    /**
     * @brief 将节点替换为标量
     *
     * @param[in] value 标量文本
     */
    void setScalar(std::string_view value);

    /**
     * @brief 读取并转换节点值
     *
     * @tparam T 目标类型，支持标量、枚举、optional、vector、array、以 string 为键的 map/unordered_map 和 ADL 自定义类型
     * @param[out] value 目标对象
     * @return 是否转换成功
     */
    template <typename T>
    bool read(T &value) const;

    /**
     * @brief 读取并转换节点值
     *
     * @tparam T 目标类型
     * @return 成功时返回目标值，否则返回 std::nullopt
     */
    template <typename T>
    std::optional<T> as() const;

    /**
     * @brief 读取节点值，失败时返回默认值
     *
     * @tparam T 目标类型
     * @param[in] fallback 默认值
     * @return 转换结果或默认值
     */
    template <typename T>
    T valueOr(T fallback) const;

    /**
     * @brief 写入任意受支持类型
     *
     * @tparam T 输入类型
     * @param[in] value 输入值
     * @return 是否写入成功
     */
    template <typename T>
    bool write(const T &value);

    /**
     * @brief 设置映射字段
     *
     * @tparam T 输入类型
     * @param[in] name 键名
     * @param[in] value 输入值
     * @return 是否写入成功
     */
    template <typename T>
    bool set(std::string_view name, const T &value);

    /**
     * @brief 向序列追加一个值
     *
     * @tparam T 输入类型
     * @param[in] value 输入值
     * @return 是否写入成功
     */
    template <typename T>
    bool push(const T &value);

private:
    struct Impl;
    std::shared_ptr<Impl> _impl{};
    std::size_t _id{std::numeric_limits<std::size_t>::max()};

    Node(std::shared_ptr<Impl> impl, std::size_t id) noexcept : _impl(std::move(impl)), _id(id) {}

    bool readBool(bool &value) const noexcept;
    bool readSigned(int64_t &value) const noexcept;
    bool readUnsigned(uint64_t &value) const noexcept;
    bool readFloating(double &value) const noexcept;
    bool writeBool(bool value);
    bool writeSigned(int64_t value);
    bool writeUnsigned(uint64_t value);
    bool writeFloating(double value);

    friend struct Result;
    friend Result parse(std::string_view source);
    friend Result load(std::string_view path);
    friend std::string dump(const Node &node);
};

//! YAML 解析或加载结果
struct Result {
    Node root{};   //!< 成功时的根节点
    Error error{}; //!< 失败时的错误

    //! @return 操作是否成功
    explicit operator bool() const noexcept { return root.valid() && !error; }

    //! @return 根节点的常指针
    const Node *operator->() const noexcept { return &root; }

    //! @return 根节点指针
    Node *operator->() noexcept { return &root; }

    //! @return 根节点的常引用
    const Node &operator*() const noexcept { return root; }

    //! @return 根节点引用
    Node &operator*() noexcept { return root; }
};

/**
 * @brief 从内存解析 YAML
 *
 * @param[in] source YAML 文本
 * @return 解析结果
 */
RMVL_EXPORTS_W Result parse(std::string_view source);

/**
 * @brief 从文件加载 YAML
 *
 * @param[in] path 文件路径
 * @return 加载与解析结果
 */
RMVL_EXPORTS_W Result load(std::string_view path);

/**
 * @brief 将节点及其后代序列化为 YAML
 *
 * @param[in] node 待序列化节点
 * @return YAML 文本，节点无效时返回空字符串
 */
RMVL_EXPORTS_W std::string dump(const Node &node);

/**
 * @brief 将节点保存为 YAML 文件并返回错误信息
 *
 * @param[in] path 文件路径
 * @param[in] node 待保存节点
 * @param[out] err 错误信息，保存成功时重置为空错误
 * @return 是否保存成功
 */
RMVL_EXPORTS_W bool save(std::string_view path, const Node &node, Error &err);

/**
 * @brief 将节点保存为 YAML 文件
 *
 * @param[in] path 文件路径
 * @param[in] node 待保存节点
 * @return 是否保存成功
 */
RMVL_EXPORTS_W inline bool save(std::string_view path, const Node &node) {
    Error err;
    return save(path, node, err);
}

//! @cond

namespace detail {

template <typename>
struct always_false : std::false_type {};

template <typename T>
struct is_optional : std::false_type {};
template <typename T>
struct is_optional<std::optional<T>> : std::true_type {
    using value_type = T;
};

template <typename T>
struct is_vector : std::false_type {};
template <typename T, typename A>
struct is_vector<std::vector<T, A>> : std::true_type {
    using value_type = T;
};

template <typename T>
struct is_array : std::false_type {};
template <typename T, std::size_t N>
struct is_array<std::array<T, N>> : std::true_type {
    using value_type = T;
    static constexpr std::size_t size = N;
};

template <typename T>
struct is_string_map : std::false_type {};
template <typename V, typename C, typename A>
struct is_string_map<std::map<std::string, V, C, A>> : std::true_type {
    using mapped_type = V;
};
template <typename V, typename H, typename E, typename A>
struct is_string_map<std::unordered_map<std::string, V, H, E, A>> : std::true_type {
    using mapped_type = V;
};

template <typename T>
auto decodeAdl(const Node &node, T &value, int) -> decltype(yaml_decode(node, value), bool()) {
    return static_cast<bool>(yaml_decode(node, value));
}

template <typename T>
bool decodeAdl(const Node &, T &, long) {
    static_assert(always_false<T>::value, "unsupported YAML decode type; provide yaml_decode(const rm::yaml::Node &, T &)");
    return false;
}

template <typename T>
auto encodeAdl(Node &node, const T &value, int) -> decltype(yaml_encode(node, value), bool()) {
    return static_cast<bool>(yaml_encode(node, value));
}

template <typename T>
bool encodeAdl(Node &, const T &, long) {
    static_assert(always_false<T>::value, "unsupported YAML encode type; provide yaml_encode(rm::yaml::Node &, const T &)");
    return false;
}

} // namespace detail

template <typename T>
bool Node::read(T &value) const {
    using U = std::remove_cv_t<std::remove_reference_t<T>>;
    if constexpr (std::is_same_v<U, std::string>) {
        if (!isScalar())
            return false;
        value.assign(scalar());
        return true;
    } else if constexpr (std::is_same_v<U, bool>) {
        return readBool(value);
    } else if constexpr (std::is_enum_v<U>) {
        using E = std::underlying_type_t<U>;
        E raw{};
        if (!read(raw))
            return false;
        value = static_cast<U>(raw);
        return true;
    } else if constexpr (std::is_integral_v<U> && std::is_signed_v<U>) {
        int64_t raw{};
        if (!readSigned(raw) || raw < static_cast<int64_t>(std::numeric_limits<U>::min()) || raw > static_cast<int64_t>(std::numeric_limits<U>::max()))
            return false;
        value = static_cast<U>(raw);
        return true;
    } else if constexpr (std::is_integral_v<U> && std::is_unsigned_v<U>) {
        uint64_t raw{};
        if (!readUnsigned(raw) || raw > static_cast<uint64_t>(std::numeric_limits<U>::max()))
            return false;
        value = static_cast<U>(raw);
        return true;
    } else if constexpr (std::is_floating_point_v<U>) {
        double raw{};
        if (!readFloating(raw) || raw < -static_cast<double>(std::numeric_limits<U>::max()) || raw > static_cast<double>(std::numeric_limits<U>::max()))
            return false;
        value = static_cast<U>(raw);
        return true;
    } else if constexpr (detail::is_optional<U>::value) {
        if (isNull()) {
            value.reset();
            return true;
        }
        typename detail::is_optional<U>::value_type item{};
        if (!read(item))
            return false;
        value = std::move(item);
        return true;
    } else if constexpr (detail::is_vector<U>::value) {
        if (!isSequence())
            return false;
        U result;
        result.reserve(size());
        for (std::size_t i = 0; i < size(); ++i) {
            typename detail::is_vector<U>::value_type item{};
            if (!(*this)[i].read(item))
                return false;
            result.push_back(std::move(item));
        }
        value = std::move(result);
        return true;
    } else if constexpr (detail::is_array<U>::value) {
        if (!isSequence() || size() != detail::is_array<U>::size)
            return false;
        U result{};
        for (std::size_t i = 0; i < result.size(); ++i)
            if (!(*this)[i].read(result[i]))
                return false;
        value = std::move(result);
        return true;
    } else if constexpr (detail::is_string_map<U>::value) {
        if (!isMap())
            return false;
        U result;
        for (const auto name : keys()) {
            typename detail::is_string_map<U>::mapped_type item{};
            if (!(*this)[name].read(item))
                return false;
            result.emplace(std::string(name), std::move(item));
        }
        value = std::move(result);
        return true;
    } else {
        return detail::decodeAdl(*this, value, 0);
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
    return read(value) ? value : std::move(fallback);
}

template <typename T>
bool Node::write(const T &value) {
    using U = std::remove_cv_t<std::remove_reference_t<T>>;
    if (!valid()) {
        return false;
    } else if constexpr (std::is_same_v<U, std::string> || std::is_same_v<U, std::string_view> || std::is_convertible_v<const T &, std::string_view>) {
        setScalar(std::string_view(value));
        return true;
    } else if constexpr (std::is_same_v<U, bool>) {
        return writeBool(value);
    } else if constexpr (std::is_enum_v<U>) {
        return write(static_cast<std::underlying_type_t<U>>(value));
    } else if constexpr (std::is_integral_v<U> && std::is_signed_v<U>) {
        return writeSigned(static_cast<int64_t>(value));
    } else if constexpr (std::is_integral_v<U> && std::is_unsigned_v<U>) {
        return writeUnsigned(static_cast<uint64_t>(value));
    } else if constexpr (std::is_floating_point_v<U>) {
        return writeFloating(static_cast<double>(value));
    } else if constexpr (detail::is_optional<U>::value) {
        if (!value) {
            setNull();
            return true;
        }
        return write(*value);
    } else if constexpr (detail::is_vector<U>::value || detail::is_array<U>::value) {
        makeSequence();
        for (const auto &item : value)
            if (!push(item))
                return false;
        return true;
    } else if constexpr (detail::is_string_map<U>::value) {
        makeMap();
        for (const auto &[name, item] : value)
            if (!set(name, item))
                return false;
        return true;
    } else {
        return detail::encodeAdl(*this, value, 0);
    }
}

template <typename T>
bool Node::set(std::string_view name, const T &value) {
    auto child = ensure(name);
    return child.write(value);
}

template <typename T>
bool Node::push(const T &value) {
    auto child = append();
    return child.write(value);
}

//! @endcond

//! @} yaml

} // namespace rm::yaml

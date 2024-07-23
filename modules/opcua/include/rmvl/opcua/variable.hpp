/**
 * @file variable.hpp
 * @author zhaoxi (535394140@qq.com)
 * @brief 变量（类型）
 * @version 2.2
 * @date 2024-03-29
 *
 * @copyright Copyright 2024 (c), zhaoxi
 *
 */

#pragma once

#include <any>
#include <memory>
#include <utility>
#include <vector>

#include "rmvl/core/util.hpp"

#include "utilities.hpp"

namespace rm
{

//! @addtogroup opcua
//! @{

enum AccessLevel : uint8_t
{
    VARIABLE_READ = 1U,  //!< 读权限
    VARIABLE_WRITE = 2U, //!< 写权限
};

//! OPC UA 变量类型
class VariableType final
{
public:
    //! 命名空间索引，默认为 `1`
    uint16_t ns{1U};

    /**
     * @brief 浏览名称 BrowseName
     * @brief
     * - 属于非服务器层面的 ID 号，可用于完成路径搜索
     * @brief
     * - 同一个命名空间 `ns` 下该名称不能重复
     */
    std::string browse_name{};

    /**
     * @brief 展示名称 DisplayName
     * @brief
     * - 在服务器上对外展示的名字 - `en-US`
     * @brief
     * - 同一个命名空间 `ns` 下该名称可以相同
     */
    std::string display_name{};
    //! 变量类型的描述 - `zh-CN`
    std::string description{};

private:
    //! 默认数据
    std::any _value;
    //! 数据类型
    DataType _data_type{};
    //! 数据大小
    UA_UInt32 _size{};

public:
    /**
     * @brief 字面量字符串构造，设置默认值
     *
     * @param[in] str 字面量字符串
     */
    template <size_t N>
    VariableType(const char (&str)[N]) : _value(str), _data_type(DataType(typeid(const char *))), _size(1) {}

    /**
     * @brief 单值构造，设置默认值
     *
     * @tparam Tp 变量的存储数据类型，必须是基础类型或者 `const char *` 表示的字符串类型
     * @param[in] val 标量、数量值
     */
    template <typename Tp, typename Enable = std::enable_if_t<std::is_fundamental_v<Tp> || std::is_same_v<Tp, const char *>>>
    VariableType(Tp &&val) : _value(val), _data_type(DataType(typeid(Tp))), _size(1) {}

    /**
     * @brief 列表构造，设置默认值
     *
     * @tparam Tp 变量的存储数据类型，必须是基础类型
     * @param[in] arr 列表、数组
     */
    template <typename Tp, typename Enable = std::enable_if_t<std::is_fundamental_v<Tp> && !std::is_same_v<bool, Tp>>>
    VariableType(const std::vector<Tp> &arr) : _value(arr), _data_type(DataType(typeid(Tp))), _size(arr.size()) {}

    /**
     * @brief 将变量类型节点转化为指定类型的数据
     *
     * @tparam Tp 变量类型的数据类型
     * @param[in] val 变量类型节点
     * @return Tp 该数据类型的数据
     */
    template <typename Tp>
    static inline Tp cast(const rm::VariableType &val) { return std::any_cast<Tp>(val.data()); }

    //! 获取默认数据
    inline const auto &data() const { return _value; }

    //! 获取数据类型
    inline DataType getDataType() const { return _data_type; }

    //! 获取大小 @note 未初始化则返回 `0`
    inline UA_UInt32 size() const { return _size; }
};

//! OPC UA 变量
class Variable final
{
public:
    //! 命名空间索引，默认为 `1`
    uint16_t ns{1U};

    /**
     * @brief 浏览名称 BrowseName
     * @brief
     * - 属于非服务器层面的 ID 号，可用于完成路径搜索
     * @brief
     * - 同一个命名空间 `ns` 下该名称不能重复
     */
    std::string browse_name{};

    /**
     * @brief 展示名称 DisplayName
     * @brief
     * - 在服务器上对外展示的名字 - `en-US`
     * @brief
     * - 同一个命名空间 `ns` 下该名称可以相同
     */
    std::string display_name{};
    //! 变量的描述
    std::string description{};
    //! 访问性
    uint8_t access_level{};

private:
    /**
     * @brief 对应的用 `rm::VariableType` 表示的变量类型
     * @brief
     * - 默认情况下为 `nullptr`，添加至 `rm::Server` 时表示采用 `BaseDataVariableType` 作为其变量类型
     * @brief
     * - 作为变量类型节点、变量节点之间链接的依据
     */
    VariableType *_type{nullptr};
    //! 数据
    std::any _value;
    //! 数据类型
    DataType _data_type{};
    //! 数据大小
    UA_UInt32 _size{};

public:
    Variable() = default;

    /**
     * @brief 字面量字符串构造，设置默认值
     *
     * @param[in] str 字面量字符串
     */
    template <unsigned int N>
    Variable(const char (&str)[N]) : access_level(3U), _value(str), _data_type(DataType(typeid(const char *))), _size(1) {}

    /**
     * @brief 单值构造
     *
     * @tparam Tp 变量的存储数据类型，必须是基础类型、`const char *` 或 `const char (&)[N]` 表示的字符串类型
     * @param[in] val 标量、数量值
     */
    template <typename Tp, typename Enable = std::enable_if_t<std::is_fundamental_v<Tp> || std::is_same_v<Tp, const char *>>>
    Variable(const Tp &val) : access_level(3U), _value(val), _data_type(DataType(typeid(Tp))), _size(1) {}

    /**
     * @brief 列表构造
     *
     * @tparam Tp 变量的存储数据类型，必须是非 `bool` 的基础类型
     * @param[in] arr 列表、数组
     */
    template <typename Tp, typename Enable = std::enable_if_t<std::is_fundamental_v<Tp> && !std::is_same_v<bool, Tp>>>
    Variable(const std::vector<Tp> &arr) : access_level(3U), _value(arr), _data_type(DataType(typeid(Tp))), _size(arr.size()) {}

    /**
     * @brief 从变量类型构造新的变量节点
     *
     * @param[in] vtype 既存的待作为变量节点类型信息的使用 `rm::VariableType` 表示的变量类型
     */
    explicit Variable(VariableType &vtype) : access_level(3U), _type(&vtype), _value(vtype.data()), _data_type(vtype.getDataType()), _size(vtype.size()) {}

    /**
     * @brief 比较两个变量是否相等，当且仅当两个变量的数据类型、维数、数据值均相等时返回
     *        `true`，而不考虑变量的名称、描述等信息
     *
     * @param[in] val 另一个变量
     * @return 是否相等
     */
    bool operator==(const Variable &val) const;

    /**
     * @brief 比较两个变量是否不等
     * @see `rm::Variable::operator==`
     *
     * @param[in] val 另一个变量
     * @return 是否不等
     */
    inline bool operator!=(const Variable &val) const { return !(*this == val); }

    //! 判断变量节点是否为空
    constexpr bool empty() const { return _size == 0; }

    /**
     * @brief 将变量节点转化为指定类型的数据
     *
     * @tparam Tp 变量的数据类型
     * @param[in] val 变量节点
     * @return Tp 该数据类型的数据
     */
    template <typename Tp>
    static inline Tp cast(const rm::Variable &val) { return std::any_cast<Tp>(val.data()); }

    /**
     * @brief 将变量节点转化为指定类型的数据
     *
     * @tparam Tp 变量的数据类型
     * @return 该数据类型的数据
     */
    template <typename Tp>
    inline Tp cast() { return std::any_cast<Tp>(this->data()); }

    /**
     * @brief 获取用 `rm::VariableType` 表示的变量类型
     *
     * @see _type
     * @return 变量类型
     */
    inline const VariableType *type() const { return _type; }

    //! 获取数据
    inline const auto &data() const { return _value; }

    //! 获取形如 `UA_TYPES_<xxx>` 的数据类型
    inline DataType getDataType() const { return _data_type; }

    //! 获取大小 @note 未初始化则返回 `0`
    inline UA_UInt32 size() const { return _size; }
};

/**
 * @brief 创建变量类型，BrowseName、DisplayName、Description 均为变量类型的名称
 *
 * @param[in] val 变量类型的名称
 * @param[in] ... 构造列表
 */
#define uaCreateVariableType(val, ...) \
    rm::VariableType val{__VA_ARGS__}; \
    val.browse_name = val.display_name = val.description = #val

/**
 * @brief 创建变量，BrowseName、DisplayName、Description 均为变量类型的名称
 *
 * @param[in] val 变量的名称
 * @param[in] ... 构造列表
 */
#define uaCreateVariable(val, ...) \
    rm::Variable val{__VA_ARGS__}; \
    val.browse_name = val.display_name = val.description = #val

//! @} opcua

} // namespace rm

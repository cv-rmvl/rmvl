/**
 * @file variable.hpp
 * @author zhaoxi (535394140@qq.com)
 * @brief 变量（类型）
 * @version 2.1
 * @date 2024-03-07
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

//! OPC UA 变量类型
class VariableType final
{
public:
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
    UA_TypeFlag _data_type{};
    //! 数据大小
    UA_UInt32 _size{};

public:
    /**
     * @brief 字面量字符串构造，设置默认值
     *
     * @param[in] str 字面量字符串
     */
    template <size_t N>
    VariableType(const char (&str)[N]) : _value(str), _data_type(typeflag.at(typeid(const char *))), _size(1) {}

    /**
     * @brief 单值构造，设置默认值
     *
     * @tparam Tp 变量的存储数据类型，必须是基础类型或者 `const char *` 表示的字符串类型
     * @param[in] val 标量、数量值
     */
    template <typename Tp, typename Enable = std::enable_if_t<std::is_fundamental_v<Tp> || std::is_same_v<Tp, const char *>>>
    VariableType(Tp &&val) : _value(val), _data_type(typeflag.at(typeid(Tp))), _size(1) {}

    /**
     * @brief 列表构造，设置默认值
     *
     * @tparam Tp 变量的存储数据类型，必须是基础类型
     * @param[in] arr 列表、数组
     */
    template <typename Tp, typename Enable = std::enable_if_t<std::is_fundamental_v<Tp> && !std::is_same_v<bool, Tp>>>
    VariableType(const std::vector<Tp> &arr) : _value(arr), _data_type(typeflag.at(typeid(Tp))), _size(arr.size()) {}

    VariableType(const VariableType &val) : browse_name(val.browse_name), display_name(val.display_name), description(val.description),
                                            _value(val._value), _data_type(val._data_type), _size(val._size) {}

    VariableType(VariableType &&val) : browse_name(std::move(val.browse_name)), display_name(std::move(val.display_name)),
                                       description(std::move(val.description)), _value(std::move(val._value)),
                                       _data_type(std::exchange(val._data_type, 0)), _size(std::exchange(val._size, 0)) {}

    VariableType &operator=(const VariableType &val);

    VariableType &operator=(VariableType &&val);

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
    inline UA_TypeFlag getDataType() const { return _data_type; }

    //! 获取大小 @note 未初始化则返回 `0`
    inline UA_UInt32 size() const { return _size; }
};

//! OPC UA 变量
class Variable final
{
public:
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
    UA_TypeFlag _data_type{};
    //! 数据大小
    UA_UInt32 _size{};
    //! 访问性
    uint8_t _access_level{};

public:
    Variable() = default;

    /**
     * @brief 字面量字符串构造，设置默认值
     *
     * @param[in] str 字面量字符串
     */
    template <unsigned int N>
    Variable(const char (&str)[N]) : _value(str), _data_type(typeflag.at(typeid(const char *))), _size(1), _access_level(3U) {}

    /**
     * @brief 单值构造
     *
     * @tparam Tp 变量的存储数据类型，必须是基础类型、`const char *` 或 `const char (&)[N]` 表示的字符串类型
     * @param[in] val 标量、数量值
     */
    template <typename Tp, typename Enable = std::enable_if_t<std::is_fundamental_v<Tp> || std::is_same_v<Tp, const char *>>>
    Variable(const Tp &val) : _value(val), _data_type(typeflag.at(typeid(Tp))), _size(1), _access_level(3U) {}

    /**
     * @brief 列表构造
     *
     * @tparam Tp 变量的存储数据类型，必须是非 `bool` 的基础类型
     * @param[in] arr 列表、数组
     */
    template <typename Tp, typename Enable = std::enable_if_t<std::is_fundamental_v<Tp> && !std::is_same_v<bool, Tp>>>
    Variable(const std::vector<Tp> &arr) : _value(arr), _data_type(typeflag.at(typeid(Tp))), _size(arr.size()), _access_level(3U) {}

    /**
     * @brief 从变量类型构造新的变量节点
     *
     * @param[in] vtype 既存的待作为变量节点类型信息的使用 `rm::VariableType` 表示的变量类型
     */
    explicit Variable(VariableType &vtype) : _type(&vtype), _value(vtype.data()), _data_type(vtype.getDataType()),
                                             _size(vtype.size()), _access_level(3U) {}

    Variable(const Variable &val) : browse_name(val.browse_name), display_name(val.display_name), description(val.description), _type(val._type),
                                    _value(val._value), _data_type(val._data_type), _size(val._size), _access_level(val._access_level) {}

    Variable(Variable &&val) : browse_name(std::move(val.browse_name)), display_name(std::move(val.display_name)), description(std::move(val.description)),
                               _type(std::exchange(val._type, nullptr)), _value(std::move(val._value)), _data_type(std::exchange(val._data_type, 0)),
                               _size(std::exchange(val._size, 0)), _access_level(std::exchange(val._access_level, 0)) {}

    Variable &operator=(const Variable &val);

    Variable &operator=(Variable &&val);

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
    inline UA_TypeFlag getDataType() const { return _data_type; }

    //! 获取大小 @note 未初始化则返回 `0`
    inline UA_UInt32 size() const { return _size; }

    /**
     * @brief 设置访问性
     *
     * @param[in] access_level 访问性
     */
    inline void setAccessLevel(uint8_t access_level) { _access_level = access_level; }

    //! 获取访问性
    inline uint8_t getAccessLevel() const { return _access_level; }
};

/**
 * @brief 创建变量类型，BrowseName、DisplayName、Description 均为变量类型的名称
 *
 * @param[in] val 变量类型的名称
 * @param[in] ... 构造列表
 */
#define uaCreateVariableType(val, ...) \
    rm::VariableType val(__VA_ARGS__); \
    val.browse_name = val.display_name = val.description = #val

/**
 * @brief 创建变量，BrowseName、DisplayName、Description 均为变量类型的名称
 *
 * @param[in] val 变量的名称
 * @param[in] ... 构造列表
 */
#define uaCreateVariable(val, ...) \
    rm::Variable val(__VA_ARGS__); \
    val.browse_name = val.display_name = val.description = #val

//! @} opcua

} // namespace rm

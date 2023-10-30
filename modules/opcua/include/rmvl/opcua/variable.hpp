/**
 * @file variable.hpp
 * @author zhaoxi (535394140@qq.com)
 * @brief 变量（类型）
 * @version 1.0
 * @date 2023-10-20
 *
 * @copyright Copyright 2023 (c), zhaoxi
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
    //! 维数
    UA_UInt32 _dims{};

public:
    /**
     * @brief 字面量字符串构造，设置默认值
     *
     * @param[in] str 字面量字符串
     */
    template <size_t N>
    VariableType(const char str[N]) : _value(str), _data_type(typeflag[typeid(const char *)]), _dims(1) {}

    /**
     * @brief 单值构造，设置默认值
     *
     * @tparam Tp 变量的存储数据类型，必须是基础类型或者 `const char *` 表示的字符串类型
     * @param[in] val 标量、数量值
     */
    template <typename Tp, typename Enable = std::enable_if_t<std::is_fundamental_v<Tp> || std::is_same_v<Tp, const char *>>>
    VariableType(Tp &&val) : _value(val), _data_type(typeflag[typeid(Tp)]), _dims(1) {}

    /**
     * @brief 列表构造，设置默认值
     *
     * @tparam Tp 变量的存储数据类型，必须是基础类型
     * @param[in] arr 列表、数组
     */
    template <typename Tp, typename Enable = std::enable_if_t<std::is_fundamental_v<Tp> && !std::is_same_v<bool, Tp>>>
    VariableType(const std::vector<Tp> &arr) : _value(arr), _data_type(typeflag[typeid(Tp)]), _dims(arr.size()) {}

    VariableType(const VariableType &val) : browse_name(val.browse_name), display_name(val.display_name), description(val.description),
                                            _value(val._value), _data_type(val._data_type), _dims(val._dims) {}

    VariableType(VariableType &&val) : browse_name(std::move(val.browse_name)), display_name(std::move(val.display_name)), description(std::move(val.description)),
                                       _value(std::move(val._value)), _data_type(std::exchange(val._data_type, 0)), _dims(std::exchange(val._dims, 0)) {}

    VariableType &operator=(const VariableType &val);

    VariableType &operator=(VariableType &&val);

    //! 获取默认数据
    inline const auto &getValue() const { return _value; }

    //! 获取数据类型
    inline UA_TypeFlag getDataType() const { return _data_type; }

    //! 获取数组维度 @note 单独的数则返回 `1`，未初始化则返回 `0`
    inline const UA_UInt32 &getArrayDimensions() const { return _dims; }

    /**
     * @brief 获取默认数据的阶数、秩
     *
     * @return 阶数
     * @retval `UA_VALUERANK_SCALAR` 或 `1`
     */
    inline int getValueRank() const { return _dims == 1 ? UA_VALUERANK_SCALAR : 1; }
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
    //! 维数
    UA_UInt32 _dims{};
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
    Variable(const char (&str)[N]) : _value(str), _data_type(typeflag[typeid(const char *)]), _dims(1) {}

    /**
     * @brief 单值构造
     *
     * @tparam Tp 变量的存储数据类型，必须是基础类型或者 `const char *` 表示的字符串类型
     * @param[in] val 标量、数量值
     */
    template <typename Tp, typename Enable = std::enable_if_t<std::is_fundamental_v<Tp> || std::is_same_v<Tp, const char *>>>
    Variable(Tp &&val) : _value(val), _data_type(typeflag[typeid(Tp)]), _dims(1), _access_level(3U) {}

    /**
     * @brief 列表构造
     *
     * @tparam Tp 变量的存储数据类型，必须是非 `bool` 的基础类型
     * @param[in] arr 列表、数组
     */
    template <typename Tp, typename Enable = std::enable_if_t<std::is_fundamental_v<Tp> && !std::is_same_v<bool, Tp>>>
    Variable(const std::vector<Tp> &arr) : _value(arr), _data_type(typeflag[typeid(Tp)]), _dims(arr.size()), _access_level(3U) {}

    /**
     * @brief 从变量类型构造新的变量节点
     *
     * @param[in] vtype 既存的待作为变量节点类型信息的使用 `rm::VariableType` 表示的变量类型
     */
    explicit Variable(VariableType &vtype) : _type(&vtype), _value(vtype.getValue()), _data_type(vtype.getDataType()),
                                             _dims(vtype.getArrayDimensions()), _access_level(3U) {}

    Variable(const Variable &val) : browse_name(val.browse_name), _type(val._type), display_name(val.display_name), description(val.description),
                                    _value(val._value), _data_type(val._data_type), _dims(val._dims), _access_level(val._access_level) {}

    Variable(Variable &&val) : browse_name(std::move(val.browse_name)), _type(std::exchange(val._type, nullptr)), display_name(std::move(val.display_name)), description(std::move(val.description)),
                               _value(std::move(val._value)), _data_type(std::exchange(val._data_type, 0)), _dims(std::exchange(val._dims, 0)), _access_level(std::exchange(val._access_level, 0)) {}

    Variable &operator=(const Variable &val);

    Variable &operator=(Variable &&val);

    /**
     * @brief 获取用 `rm::VariableType` 表示的变量类型
     *
     * @see _type
     * @return 变量类型
     */
    inline const VariableType *getType() const { return _type; }

    //! 获取数据
    inline const auto &getValue() const { return _value; }

    //! 获取数据类型
    inline UA_TypeFlag getDataType() const { return _data_type; }

    //! 获取数组维度指针 @note 单独的数则返回 `1`，未初始化则返回 `0`
    inline const UA_UInt32 &getArrayDimensions() const { return _dims; }

    /**
     * @brief 设置访问性
     *
     * @param[in] access_level 访问性
     */
    inline void setAccessLevel(uint8_t access_level) { _access_level = access_level; }

    //! 获取访问性
    inline uint8_t getAccessLevel() const { return _access_level; }

    /**
     * @brief 获取数据阶数、秩
     *
     * @return 数据阶数
     * @retval `UA_VALUERANK_SCALAR` 或 `1`
     */
    inline int getValueRank() const { return _dims == 1 ? UA_VALUERANK_SCALAR : 1; }
};

//! @} opcua

namespace helper
{

//! @addtogroup opcua
//! @{

/**
 * @brief `rm::Variable` 转化为无类型的 `UA_Variant`
 *
 * @warning 此方法一般不直接使用
 * @param[in] val `rm::Variable` 表示的变量
 * @return 用 `UA_Variant` 表示变量节点内置数据的堆空间指针
 */
UA_Variant *cvtVariable(const Variable &val);

/**
 * @brief `rm::VariableType` 转化为无类型的 `UA_Variant`
 *
 * @warning 此方法一般不直接使用
 * @param[in] val `rm::VariableType` 表示的变量类型
 * @return 用 `UA_Variant` 表示变量类型节点内置数据的堆空间指针
 */
UA_Variant *cvtVariable(const VariableType &vtype);

//! @} opcua

} // namespace helper

//! @addtogroup opcua
//! @{

//! @} opcua

} // namespace rm

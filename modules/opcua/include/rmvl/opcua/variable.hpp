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
#include <functional>
#include <memory>
#include <utility>

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
class RMVL_EXPORTS_W VariableType final
{
public:
    RMVL_W VariableType() = default;

    /**
     * @brief 单值构造，设置默认值
     *
     * @tparam Tp 变量的存储数据类型，必须是可包含 `cv` 限定符的基础类型及其引用类型
     * @param[in] val 标量、数量值
     */
    template <typename Tp, typename DecayT = typename std::decay_t<Tp>, typename = std::enable_if_t<std::is_fundamental_v<DecayT>>>
    RMVL_W_SUBST("VT")
    VariableType(Tp val) : _value(val), _data_type(DataType(typeid(DecayT))), _size(1) {}

    /**
     * @brief 字符串构造，设置默认值
     *
     * @param[in] str 字符串
     */
    RMVL_W VariableType(const std::string &str) : _value(str), _data_type(DataType(typeid(std::string))), _size(1) {}

    /**
     * @brief 字符串字面量构造
     *
     * @tparam N 字符串长度
     * @param[in] str 字符串
     */
    template <std::size_t N>
    VariableType(const char (&str)[N]) : VariableType(std::string(str, N)) {}

    /**
     * @brief 列表构造，设置默认值
     *
     * @tparam Tp 变量的存储数据类型，必须是基础类型
     * @param[in] arr 列表、数组
     */
    template <typename Tp, typename Enable = std::enable_if_t<std::is_fundamental_v<Tp> && !std::is_same_v<bool, Tp>>>
    RMVL_W_SUBST("VT_List")
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

    /**
     * @brief 将变量节点转化为指定类型的数据
     *
     * @tparam Tp 变量的数据类型
     * @return 该数据类型的数据
     */
    template <typename Tp>
    RMVL_W_SUBST("VT_cast")
    inline Tp cast() const { return std::any_cast<Tp>(this->data()); }

    //! 获取默认数据
    inline const auto &data() const { return _value; }

    //! 获取数据类型
    RMVL_W inline DataType getDataType() const { return _data_type; }

    //! 判断变量类型节点是否为空
    RMVL_W constexpr bool empty() const { return _size == 0; }

    //! 获取大小 @note 未初始化则返回 `0`
    RMVL_W inline uint32_t size() const { return _size; }

    //! 命名空间索引，默认为 `1`
    RMVL_W_RW uint16_t ns{1U};

    /**
     * @brief 浏览名称 BrowseName
     * @brief
     * - 属于非服务器层面的 ID 号，可用于完成路径搜索
     * @brief
     * - 同一个命名空间 `ns` 下该名称不能重复
     */
    RMVL_W_RW std::string browse_name{};

    /**
     * @brief 展示名称 DisplayName
     * @brief
     * - 在服务器上对外展示的名字 - `en-US`
     * @brief
     * - 同一个命名空间 `ns` 下该名称可以相同
     */
    RMVL_W_RW std::string display_name{};
    //! 变量类型的描述 - `zh-CN`
    RMVL_W_RW std::string description{};

private:
    //! 默认数据
    std::any _value;
    //! 数据类型
    DataType _data_type{};
    //! 数据大小
    uint32_t _size{};
};

//! OPC UA 变量
class RMVL_EXPORTS_W Variable final
{
public:
    RMVL_W Variable() = default;

    /**
     * @brief 单值构造
     *
     * @tparam Tp 变量的存储数据类型，必须是可包含 `cv` 限定符的基础类型及其引用类型
     * @param[in] val 标量、数量值
     */
    template <typename Tp, typename DecayT = typename std::decay_t<Tp>, typename = std::enable_if_t<std::is_fundamental_v<DecayT>>>
    RMVL_W_SUBST("V")
    Variable(Tp val) : _value(val), _data_type(DataType(typeid(DecayT))), _size(1) {}

    /**
     * @brief 字符串构造
     *
     * @param[in] str 字符串
     */
    RMVL_W Variable(const std::string &str) : _value(str), _data_type(DataType(typeid(std::string))), _size(1) {}

    /**
     * @brief 字符串字面量构造
     *
     * @tparam N 字符串长度
     * @param[in] str 字符串
     */
    template <std::size_t N>
    Variable(const char (&str)[N]) : Variable(std::string(str, N)) {}

    /**
     * @brief 列表构造
     *
     * @tparam Tp 变量的存储数据类型，必须是非 `bool` 的基础类型
     * @param[in] arr 列表、数组
     */
    template <typename Tp, typename Enable = std::enable_if_t<std::is_fundamental_v<Tp> && !std::is_same_v<bool, Tp>>>
    RMVL_W_SUBST("V_List")
    Variable(const std::vector<Tp> &arr) : _value(arr), _data_type(DataType(typeid(Tp))), _size(static_cast<UA_UInt32>(arr.size())) {}

    /**
     * @brief 从变量类型创建新的变量节点
     *
     * @param[in] vtype 既存的待作为变量节点类型信息的使用 `rm::VariableType` 表示的变量类型
     * @return 新的变量节点
     */
    RMVL_W static inline Variable makeFrom(const VariableType &vtype) { return Variable(vtype); }

    /**
     * @brief 比较两个变量是否相等，当且仅当两个变量的数据类型、维数、数据值均相等时返回
     *        `true`，而不考虑变量的名称、描述等信息
     *
     * @param[in] val 另一个变量
     * @return 是否相等
     */
    RMVL_W bool operator==(const Variable &val) const;

    /**
     * @brief 比较两个变量是否不等
     * @see `rm::Variable::operator==`
     *
     * @param[in] val 另一个变量
     * @return 是否不等
     */
    RMVL_W bool operator!=(const Variable &val) const { return !(*this == val); }

    //! 判断变量节点是否为空
    RMVL_W bool empty() const { return _size == 0; }

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
    RMVL_W_SUBST("V_cast")
    inline Tp cast() const { return std::any_cast<Tp>(this->data()); }

    //! 基本数据类型转换函数
    template <typename Tp, typename DecayT = typename std::decay_t<Tp>, typename = std::enable_if_t<std::is_fundamental_v<DecayT>>>
    operator Tp() const { return std::any_cast<Tp>(_value); }

    //! 字符串数据类型转换函数
    operator std::string() const { return std::any_cast<std::string>(_value); }

    //! 列表数据类型转换函数
    template <typename Tp, typename Enable = std::enable_if_t<std::is_fundamental_v<Tp> && !std::is_same_v<bool, Tp>>>
    operator std::vector<Tp>() const { return std::any_cast<std::vector<Tp>>(_value); }

    /**
     * @brief 获取用 `rm::VariableType` 表示的变量类型
     * - 添加至 `rm::Server` 时表示采用 `BaseDataVariableType` 作为其变量类型
     * - 作为变量类型节点、变量节点之间链接的依据
     *
     * @return 变量类型
     */
    RMVL_W inline const VariableType type() const { return _type; }

    //! 获取数据
    inline const auto &data() const { return _value; }

    //! 获取形如 `UA_TYPES_<xxx>` 的数据类型
    RMVL_W inline DataType getDataType() const { return _data_type; }

    //! 获取大小 @note 未初始化则返回 `0`
    RMVL_W inline uint32_t size() const { return _size; }

private:
    explicit Variable(const VariableType &vtype) : _type(vtype), _value(vtype.data()), _data_type(vtype.getDataType()), _size(vtype.size()) {}

public:
    //! 命名空间索引，默认为 `1`
    RMVL_W_RW uint16_t ns{1U};

    /**
     * @brief 浏览名称 BrowseName
     * @brief
     * - 属于非服务器层面的 ID 号，可用于完成路径搜索
     * @brief
     * - 同一个命名空间 `ns` 下该名称不能重复
     */
    RMVL_W_RW std::string browse_name{};

    /**
     * @brief 展示名称 DisplayName
     * @brief
     * - 在服务器上对外展示的名字 - `en-US`
     * @brief
     * - 同一个命名空间 `ns` 下该名称可以相同
     */
    RMVL_W_RW std::string display_name{};
    //! 变量的描述
    RMVL_W_RW std::string description{};
    //! 访问性
    RMVL_W_RW uint8_t access_level{3U};

private:
    //! 变量类型
    VariableType _type{};
    //! 数据
    std::any _value;
    //! 数据类型
    DataType _data_type{};
    //! 数据大小
    uint32_t _size{};
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

//! 变量列表别名
using Variables = std::vector<Variable>;

/**
 * @brief 数据源回调函数，Read 函数指针定义
 *
 * @param[in] nd 待读取的变量节点的 `NodeId`
 * @return 向服务器提供的待读取的变量
 */
using DataSourceRead = std::function<Variable(const NodeId &)>;

/**
 * @brief 数据源回调函数，Write 函数指针定义
 *
 * @param[in] nd 待写入的变量节点的 `NodeId`
 * @param[in] value 从服务器接收到的变量，一般用于写入外部数据
 */
using DataSourceWrite = std::function<void(const NodeId &, const Variable &)>;

/**
 * @brief OPC UA 数据源变量
 * @note 数据源变量节点不同于变量节点的值回调
 * @note
 * - 值回调是在现有变量节点之上添加读取 **前** 和写入 **后** 的回调函数，本质上仍然是从服务器中获取数据
 * @note
 * - 数据源变量节点会把每次 IO 都绑定到各自的回调函数中，即可以重定向到一个实际的物理过程中，从而跟服务器本身的数据读写脱离关系
 */
struct RMVL_EXPORTS_W_AG DataSourceVariable
{
    //! 命名空间索引，默认为 `1`
    RMVL_W_RW uint16_t ns{1U};

    /**
     * @brief 浏览名称 BrowseName
     * @brief
     * - 属于非服务器层面的 ID 号，可用于完成路径搜索
     * @brief
     * - 同一个命名空间 `ns` 下该名称不能重复
     */
    RMVL_W_RW std::string browse_name{};

    /**
     * @brief 展示名称 DisplayName
     * @brief
     * - 在服务器上对外展示的名字 - `en-US`
     * @brief
     * - 同一个命名空间 `ns` 下该名称可以相同
     */
    RMVL_W_RW std::string display_name{};
    //! 变量的描述
    RMVL_W_RW std::string description{};
    //! 访问性
    RMVL_W_RW uint8_t access_level{};

    /**
     * @brief 数据源 Read 回调函数
     *
     * @param[in] nd `const rm::NodeId &` 类型节点 ID
     * @return `rm::Variable`
     */
    RMVL_W_RW DataSourceRead on_read{};

    /**
     * @brief 数据源 Write 回调函数
     *
     * @param[in] nd `const rm::NodeId &` 类型节点 ID
     * @param[in] value `const rm::Variable &` 类型变量
     */
    RMVL_W_RW DataSourceWrite on_write{};
};

//! @} opcua

} // namespace rm

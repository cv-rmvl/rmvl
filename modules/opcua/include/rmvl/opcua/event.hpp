/**
 * @file event.hpp
 * @author zhaoxi (535394140@qq.com)
 * @brief OPC UA 事件
 * @version 2.2
 * @date 2023-11-13
 *
 * @copyright Copyright 2023 (c), zhaoxi
 *
 */

#pragma once

#include "variable.hpp"

namespace rm
{

//! @addtogroup opcua
//! @{

//! OPC UA 事件类型
class RMVL_EXPORTS_W EventType
{
public:
    //! 构造 `rm::EventType` 对象类型
    RMVL_W EventType() = default;

    /**
     * @brief 添加非默认属性至事件类型中
     *
     * @param[in] browse_name 非默认属性的浏览名 BrowseName
     * @param[in] prop `int` 整型属性值
     */
    RMVL_W inline void add(const std::string &browse_name, int prop) { _properties.insert({browse_name, prop}); }

    /**
     * @brief 访问指定的非默认属性
     *
     * @param[in] browse_name 非默认属性的浏览名 BrowseName
     * @return `int` 整型非默认属性的左值引用
     */
    RMVL_W_SUBST("ET_Idx") int &operator[](const std::string &browse_name) { return _properties[browse_name]; }

    /**
     * @brief 获取 `int` 整型的非默认属性列表
     * @see Event::data()
     *
     * @return 属性列表
     */
    RMVL_W inline const std::unordered_map<std::string, int> &data() const { return _properties; }

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
    //! 对象类型的描述 - `zh-CN`
    RMVL_W_RW std::string description{};

private:
    std::unordered_map<std::string, int> _properties; //!< 非默认属性列表（仅支持 `int` 整型）
};

//! OPC UA 事件
class RMVL_EXPORTS_W Event
{
public:
    //! 构造 `rm::Event` 对象类型
    RMVL_W Event() = default;

    /**
     * @brief 从事件类型创建新的事件
     *
     * @param[in] etype 既存的待作为事件类型信息的使用 `rm::EventType` 表示的变量类型
     * @return 新的事件
     */
    RMVL_W static inline Event makeFrom(const EventType &etype) { return Event(etype); }

    /**
     * @brief 添加非默认属性至事件类型中
     *
     * @param[in] browse_name 非默认属性的浏览名 BrowseName
     * @param[in] prop `int` 整型属性值
     */
    RMVL_W inline void add(const std::string &browse_name, int prop) { _properties.insert({browse_name, prop}); }

    /**
     * @brief 访问指定的非默认属性
     *
     * @param[in] browse_name 非默认属性的浏览名 BrowseName
     * @return `int` 整型非默认属性的左值引用
     */
    RMVL_W_SUBST("E_Idx") inline int &operator[](const std::string &browse_name) { return _properties[browse_name]; }

    /**
     * @brief 获取 `int` 整型的非默认属性列表
     * @brief rmvl/opcua 模块支持修改的 **默认属性** 包括：
     * |  BrowseName  |    类型    |             含义             |        参考        |
     * | :----------: | :--------: | :--------------------------: | :----------------: |
     * | `SourceName` |  `String`  |          事件源名称          | Event::source_name |
     * |  `Message`   |  `String`  | 事件消息，包含关于事件的描述 |   Event::message   |
     * |  `Severity`  |  `UInt16`  |         事件严重程度         |  Event::severity   |
     *
     * @return 非默认属性列表
     */
    RMVL_W inline const std::unordered_map<std::string, int> &data() const { return _properties; }

    //! 获取事件类型
    RMVL_W inline EventType type() const { return _type; }

    RMVL_W_RW uint16_t ns{1U};         //!< 命名空间索引，默认为 `1`
    RMVL_W_RW std::string source_name; //!< 默认属性：事件源名称
    RMVL_W_RW std::string message;     //!< 默认属性：事件消息，包含关于事件的描述
    RMVL_W_RW uint16_t severity{};     //!< 默认属性：事件严重程度

private:
    Event(const EventType &etype) : _type(etype), _properties(etype.data()) {}

    EventType _type{};                                //!< 事件类型
    std::unordered_map<std::string, int> _properties; //!< 非默认属性列表（仅支持 `int` 整型）
};

//! @} opcua

} // namespace rm

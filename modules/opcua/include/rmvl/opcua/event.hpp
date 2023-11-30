/**
 * @file event.hpp
 * @author zhaoxi (535394140@qq.com)
 * @brief OPC UA 事件
 * @version 1.0
 * @date 2023-11-13
 *
 * @copyright Copyright 2023 (c), zhaoxi
 *
 */

#pragma once

#include <string>
#include <unordered_map>

// #include <open62541/types.h>

#include "variable.hpp"

namespace rm
{

//! @addtogroup opcua
//! @{

//! OPC UA 事件类型
class EventType
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
    //! 对象类型的描述 - `zh-CN`
    std::string description{};

private:
    std::unordered_map<std::string, int> _properties; //!< 非默认属性列表（仅支持 `int` 整型）

public:
    //! 构造 `rm::EventType` 对象类型
    EventType() = default;

    EventType(const EventType &val) : browse_name(val.browse_name), display_name(val.display_name),
                                      description(val.description), _properties(val._properties) {}

    EventType(EventType &&val) : browse_name(std::move(val.browse_name)), display_name(std::move(val.display_name)),
                                 description(std::move(val.description)), _properties(std::move(val._properties)) {}

    EventType &operator=(const EventType &val);
    EventType &operator=(EventType &&val);

    /**
     * @brief 添加非默认属性至事件类型中
     *
     * @param[in] browse_name 非默认属性的浏览名 BrowseName
     * @param[in] prop `int` 整型属性值
     */
    inline void add(const std::string &browse_name, int prop) { _properties[browse_name] = prop; }

    /**
     * @brief 访问指定的非默认属性
     *
     * @param[in] browse_name 非默认属性的浏览名 BrowseName
     * @return `int` 整型非默认属性的左值引用
     */
    inline int &operator[](const std::string &browse_name) { return _properties[browse_name]; }

    /**
     * @brief 获取 `int` 整型的非默认属性列表
     * @see Event::data()
     *
     * @return 属性列表
     */
    inline const auto &data() const { return _properties; }
};

//! OPC UA 事件
class Event
{
public:
    std::string source_name; //!< 默认属性：事件源名称
    std::string message;     //!< 默认属性：事件消息，包含关于事件的描述
    uint16_t severity{};     //!< 默认属性：事件严重程度

private:
    EventType *_type{nullptr};                        //!< 事件类型的指针
    std::unordered_map<std::string, int> _properties; //!< 非默认属性列表（仅支持 `int` 整型）

public:
    //! 构造 `rm::Event` 对象类型
    Event() = default;

    /**
     * @brief 从事件类型构造新的事件
     *
     * @param[in] etype 既存的待作为事件类型信息的使用 `rm::EventType` 表示的变量类型
     */
    Event(EventType &etype) : _type(&etype), _properties(etype.data()) {}

    Event(const Event &val) : _type(val._type), _properties(val._properties) {}
    Event(Event &&val) : _type(std::exchange(val._type, nullptr)), _properties(std::move(val._properties)) {}

    Event &operator=(const Event &val);
    Event &operator=(Event &&val);

    /**
     * @brief 添加非默认属性至事件类型中
     *
     * @param[in] browse_name 非默认属性的浏览名 BrowseName
     * @param[in] prop `int` 整型属性值
     */
    inline void add(const std::string &browse_name, int prop) { _properties.insert({browse_name, prop}); }

    /**
     * @brief 访问指定的非默认属性
     *
     * @param[in] browse_name 非默认属性的浏览名 BrowseName
     * @return `int` 整型非默认属性的左值引用
     */
    inline int &operator[](const std::string &browse_name) { return _properties[browse_name]; }

    /**
     * @brief 获取 `int` 整型的非默认属性列表
     * @brief rmvl/opcua 模块支持修改的 **默认属性** 包括：
     * |  BrowseName  |    类型    |             含义             |
     * | :----------: | :--------: | :--------------------------: |
     * | `SourceName` |  `String`  |          事件源名称          |
     * |  `Message`   |  `String`  | 事件消息，包含关于事件的描述 |
     * |  `Severity`  |  `UInt16`  |         事件严重程度         |
     *
     * @return 非默认属性列表
     */
    inline const auto &data() const { return _properties; }

    /**
     * @brief 设置事件类型
     *
     * @param[in] type 既存的待作为事件类型信息的使用 `rm::EventType` 表示的变量类型
     */
    inline void setType(EventType &type) { _type = &type; }

    //! 获取事件类型
    inline const EventType *type() const { return _type; }
};

//! @} opcua

} // namespace rm

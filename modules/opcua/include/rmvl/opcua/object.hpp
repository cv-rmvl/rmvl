/**
 * @file object.hpp
 * @author zhaoxi (535394140@qq.com)
 * @brief 对象（类型）
 * @version 2.2
 * @date 2024-03-29
 *
 * @copyright Copyright 2023 (c), zhaoxi
 *
 */

#pragma once

#include "method.hpp"
#include "variable.hpp"

namespace rm
{

//! @addtogroup opcua
//! @{

/**
 * @brief OPC UA 对象类型
 * @brief
 * - 所有属性均为 `Mandatory`
 * @brief
 * - 数据包含变量节点列表、方法节点列表
 */
class ObjectType final
{
public:
    //! 命名空间索引，默认为 `1`
    uint16_t ns{1U};

    /**
     * @brief 浏览名称 BrowseName
     * @brief
     * - 属于非服务器层面的 ID 号，可用于完成路径搜索
     * @brief
     * - 作为对象类型节点、对象节点之间链接的依据
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
    //! 继承的 `rm::ObjectType` 对象类型
    ObjectType *_base{nullptr};

    /**
     * @brief 变量节点
     * @brief
     * - `Key`: 浏览名 BrowseName
     * @brief
     * - `Value`: 用 `rm::Variable` 表示的变量
     */
    std::unordered_map<std::string, Variable> _variables;

    /**
     * @brief 方法节点
     * @brief
     * - `Key`: 浏览名 BrowseName
     * @brief
     * - `Value`: 用 `rm::Method` 表示的方法
     */
    std::unordered_map<std::string, Method> _methods;

public:
    //! 构造 `rm::ObjectType` 对象类型
    ObjectType() = default;

    /**
     * @brief 添加变量节点至 `rm::ObjectType` 对象类型中
     *
     * @param[in] variable 变量节点
     */
    inline void add(const Variable &variable) { _variables[variable.browse_name] = variable; }

    /**
     * @brief 访问指定的变量节点
     *
     * @param[in] browse_name 变量节点的浏览名 BrowseName
     * @return 用 `rm::Variable` 表示的变量的左值引用
     */
    inline Variable &operator[](const std::string &browse_name) { return _variables[browse_name]; }

    /**
     * @brief 添加方法节点至 `rm::ObjectType` 对象类型中
     *
     * @param[in] method 方法节点
     */
    inline void add(const Method &method) { _methods[method.browse_name] = method; }

    /**
     * @brief 设置基类 `rm::ObjectType` 对象类型
     *
     * @param[in] otype 既存的待作为基类的 `rm::ObjectType` 对象类型
     */
    inline void setBase(ObjectType &otype) { _base = &otype; }

    /**
     * @brief 获取基类 `rm::ObjectType` 对象类型
     *
     * @return 基类 `rm::ObjectType`
     */
    inline const ObjectType *getBase() const { return _base; }

    
    inline const auto &getVariables() const { return _variables; }

    /**
     * @brief 获取 `rm::Method` 表示的方法节点的列表
     *
     * @return 方法节点列表
     */
    inline const auto &getMethods() const { return _methods; }
};

//! OPC UA 对象
class Object final
{
    //! 对应的用 `rm::ObjectType` 表示的对象类型
    ObjectType *_type;

    /**
     * @brief 变量节点
     * @brief
     * - `Key`: 浏览名 BrowseName
     * @brief
     * - `Value`: 用 `rm::Variable` 表示的变量
     */
    std::unordered_map<std::string, Variable> _variables;

    /**
     * @brief 方法节点
     * @brief
     * - `Key`: 浏览名 BrowseName
     * @brief
     * - `Value`: 用 `rm::Method` 表示的方法
     */
    std::unordered_map<std::string, Method> _methods;

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
    //! 对象的描述 - `zh-CN`
    std::string description{};

    Object() : _type(nullptr) {}

    /**
     * @brief 从对象类型构造新的对象节点
     *
     * @param[in] otype 既存的待作为对象节点类型信息的使用 `rm::ObjectType` 表示的变量类型
     */
    explicit Object(ObjectType &otype) : _type(&otype), _variables(otype.getVariables()), _methods(otype.getMethods()) {}

    //! 获取对象类型 `rm::ObjectType`
    inline const ObjectType *type() const { return _type; }

    /**
     * @brief 添加（额外的）变量节点至 `rm::Object` 对象中
     *
     * @param[in] variable 变量节点
     */
    inline void add(const Variable &variable) { _variables[variable.browse_name] = variable; }

    /**
     * @brief 添加（额外的）方法节点至 `rm::Object` 对象中
     *
     * @param[in] method 方法节点
     */
    inline void add(const Method &method) { _methods[method.browse_name] = method; }

    /**
     * @brief 访问指定的变量节点
     *
     * @param[in] browse_name 变量节点的浏览名 BrowseName
     * @return 用 `rm::Variable` 表示的变量的左值引用
     */
    inline Variable &operator[](const std::string &browse_name) { return _variables[browse_name]; }

    /**
     * @brief 获取 `rm::Variable` 表示的变量节点的列表
     *
     * @return 变量节点列表
     */
    inline const auto &getVariables() const { return _variables; }

    /**
     * @brief 获取 `rm::Method` 表示的方法节点的列表
     *
     * @return 方法节点列表
     */
    inline const auto &getMethods() const { return _methods; }
};

//! @} opcua

} // namespace rm

/**
 * @file method.hpp
 * @author zhaoxi (535394140@qq.com)
 * @brief 方法节点
 * @version 2.2
 * @date 2023-10-23
 *
 * @copyright Copyright 2023 (c), zhaoxi
 *
 */

#pragma once

#include <functional>
#include <string>
#include <vector>

#include "variable.hpp"

namespace rm
{

class ServerView;

//! @addtogroup opcua
//! @{

/**
 * @brief OPC UA 方法参数信息
 * @note 不储备任何调用时数据，仅用于描述方法参数
 */
struct Argument final
{
    std::string name;          //!< 参数名称
    DataType data_type{};      //!< 参数数据类型 @note 形如 `UA_TYPES_<xxx>` 的类型标志位
    uint32_t dims{1U};         //!< 参数维数，单数据则是 `1`，数组则是数组长度 @warning 不能为 `0`
    std::string description{}; //!< 参数描述
};

/**
 * @brief OPC UA 方法回调函数
 *
 * @param[in] server_view 服务器视图，指代当前服务器
 * @param[in] obj_id 方法节点所在对象的 `NodeId`
 * @param[in] args 输入参数列表
 * @return 输出参数列表
 */
using MethodCallback = std::function<OutputVariables(ServerView, const NodeId &, InputVariables)>;

//! OPC UA 方法
class Method final
{
public:
    Method() = default;

    template <typename Callable, typename = std::enable_if_t<std::is_convertible_v<Callable, MethodCallback>>>
    Method(Callable cb) : func(cb) {}

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

    //! 方法的描述
    std::string description{};

    //! 传入参数列表
    std::vector<Argument> iargs{};

    //! 传出参数列表
    std::vector<Argument> oargs{};

    //! 方法回调函数
    MethodCallback func{};
};

//! @} opcua

} // namespace rm

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

#include <string>
#include <utility>
#include <vector>

#include "utilities.hpp"

using UA_MethodCallback = UA_StatusCode (*)(
    UA_Server *, const UA_NodeId *, void *, const UA_NodeId *, void *,
    const UA_NodeId *, void *, size_t, const UA_Variant *, size_t, UA_Variant *);

namespace rm
{

//! @addtogroup opcua
//! @{

//! OPC UA 方法参数
struct Argument final
{
    std::string name;        //!< 方法名
    UA_TypeFlag data_type{}; //!< 参数数据类型 @note 形如 `UA_TYPES_<xxx>` 的类型标志位
    uint32_t dims{1U};       //!< 参数维数 @warning 不能为 `0`

    Argument() = default;

    /**
     * @brief 构造 `rm::Argument` 方法参数
     *
     * @param[in] n 方法名
     * @param[in] dt 参数数据类型
     * @param[in] dm 参数维数（默认为 `1`）
     */
    Argument(std::string_view n, UA_TypeFlag dt, uint32_t dm = 1) : name(n), data_type(dt), dims(dm) {}
};

//! OPC UA 方法
struct Method final
{
    //! 命名空间索引，默认为 `1`
    uint16_t ns{1U};
    
    /**
     * @brief 浏览名称 BrowseName
     * @brief
     * - 属于非服务器层面的 ID 号，可用于完成路径搜索
     * @brief
     * - 同一个命名空间 `ns` 下该名称不能重复
     */
    std::string browse_name;

    /**
     * @brief 展示名称 DisplayName
     * @brief
     * - 在服务器上对外展示的名字 - `en-US`
     * @brief
     * - 同一个命名空间 `ns` 下该名称可以相同
     */
    std::string display_name;

    //! 方法的描述
    std::string description;

    //! 传入参数列表
    std::vector<Argument> iargs;

    //! 传出参数列表
    std::vector<Argument> oargs;

    /**
     * @brief 方法回调函数
     * @brief 函数原型为
     * @code{.cpp}
     * UA_StatusCode foo(
     *     UA_Server *server, const UA_NodeId *sessionId, void *sessionContext, const UA_NodeId *methodId,
     *     void *methodContext, const UA_NodeId *objectId, void *objectContext, size_t inputSize, const UA_Variant *input,
     *     size_t outputSize, UA_Variant *output);
     * @endcode
     *
     */
    UA_MethodCallback func{nullptr};

    Method() = default;

    /**
     * @brief 使用方法回调函数构造 Method
     *
     * @note 由于可发生隐式转换，因此可传入函数、函数指针以及无捕获列表的 `lambda` 表达式
     * @param[in] f 可隐式转换为 `UA_MethodCallback` 函数指针类型的可调用对象
     */
    Method(UA_MethodCallback f) : func(f) {}
};

//! @} opcua

namespace helper
{

/**
 * @brief `rm::Argument` 转化为 `UA_Argument`
 *
 * @warning 此方法一般不直接使用
 * @param[in] arg `rm::Argument` 表示的方法
 * @return `UA_Argument` 表示的方法
 */
UA_Argument cvtArgument(const Argument &arg);

} // namespace helper

} // namespace rm

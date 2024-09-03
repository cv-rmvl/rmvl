/**
 * @file cvt.hpp
 * @author zhaoxi (535394140@qq.com)
 * @brief 变量、变量类型与 UA_Variant 之间的转换
 * @version 1.0
 * @date 2024-03-07
 *
 * @copyright Copyright 2024 (c), zhaoxi
 *
 */

#pragma once

#include "rmvl/opcua/method.hpp"
#include "rmvl/opcua/variable.hpp"

namespace rm::helper
{

/**
 * @brief `rm::Variable` 转化为 `UA_Variant`
 *
 * @warning 此方法一般不直接使用
 * @param[in] val `rm::Variable` 表示的变量
 * @return `UA_Variant` 表示变量节点的内置数据
 */
UA_Variant cvtVariable(const Variable &val) noexcept;

/**
 * @brief `UA_Variant` 转化为 `rm::Variable`
 *
 * @warning 此方法一般不直接使用
 * @param[in] p_val `UA_Variant` 表示的变量
 * @return 用 `rm::Variable` 表示的变量节点
 */
Variable cvtVariable(const UA_Variant &p_val) noexcept;

/**
 * @brief `rm::VariableType` 转化为 `UA_Variant`
 *
 * @warning 此方法一般不直接使用
 * @param[in] vtype `rm::VariableType` 表示的变量类型
 * @return 用 `UA_Variant` 表示的变量类型节点的内置数据
 */
UA_Variant cvtVariable(const VariableType &vtype) noexcept;

/**
 * @brief `rm::Argument` 转化为 `UA_Argument`
 *
 * @warning 此方法一般不直接使用
 * @param[in] arg `rm::Argument` 表示的方法
 * @return `UA_Argument` 表示的方法
 */
UA_Argument cvtArgument(const Argument &arg) noexcept;

} // namespace rm::helper

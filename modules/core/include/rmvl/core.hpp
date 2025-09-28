/**
 * @file core.hpp
 * @author zhaoxi (535394140@qq.com)
 * @brief RMVL 核心模块汇总头文件
 * @version 2.0
 * @date 2023-04-18
 *
 * @copyright Copyright 2023 (c), zhaoxi
 *
 */

#pragma once

/**
 * @defgroup core RMVL 核心模块
 * @{
 * @brief 涵盖了有关异常处理、定时器、编程工具等相关内容
 * @details
 * - 异常处理可参考 rm::Exception 类，提供了异常处理的相关函数
 * - 定时器可参考 rm::Timer 类
 * - 编程工具包括了 @ref core_reflect 、 @ref core_meta 、@ref core_str 和 @ref io 等
 * @defgroup core_reflect 聚合体编译期反射
 * @defgroup core_meta 元编程
 * @defgroup core_str Python 风格的字符串处理
 * @} core
 */

/**
 * @defgroup para 参数模块
 * @{
 * @brief RMVL 参数模块
 * @details
 * - 该模块包含了 RMVL 中所有的参数类，用于存储各个模块的参数
 * - 每个子参数模块均包含对应的类，以及对应的全局参数变量
 * - 每个参数模块均提供了一个运行时参数加载的函数，用于从 `YAML` 文件中加载参数
 *
 * @see 在引言中提及了有关 @ref intro_parameters_manager 的内容可供参考
 * @}
 */

#include <rmvl/rmvl_modules.hpp>

// 通用
#include "core/str.hpp"
#include "core/timer.hpp"
#include "core/util.hpp"
#include "core/version.hpp"

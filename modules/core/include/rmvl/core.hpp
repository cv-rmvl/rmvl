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
 *   @brief 涵盖了有关异常处理、定时器、编程工具等相关内容
 *   @details
 *   - 异常处理可参考 rm::Exception 类，提供了异常处理的相关函数
 *   - 定时器可参考 rm::Timer 类
 *   - 编程工具包括了 @ref core_reflect 、 @ref core_meta 、@ref core_str 等
 *   @defgroup core_reflect 聚合体编译期反射
 *   @{
 *     @brief 提供了 C++17 和 C++20 两种实现的聚合体编译期反射功能
 *     @details
 *     - 该模块提供了对聚合体的编译期反射功能，使得用户可以在编译期获取聚合体的成员信息
 *     - 该模块的内容定义在 rm::reflect 命名空间中，提供了 rm::reflect::size 和 rm::reflect::for_each 两个主要的函数模板
 *   @}
 *   @defgroup core_meta 元编程
 *   @{
 *     @brief 目前提供了 hash 生成相关的 Type Traits
 *   @}
 *   @defgroup core_str Python 风格的字符串处理
 *   @{
 *     @brief 提供了 Python 风格的字符串处理函数，例如 split、strip 等
 *   @}
 * @}
 */

/**
 * @defgroup rmvlpara 参数模块
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

/**
 * @defgroup rmvlmsg 消息模块
 * @{
 * @brief RMVL 消息模块
 * @details
 * 该模块包含了 RMVL 中提供的所有消息类，包含 `std`、`geometry`、`sensor`、`motion` 和 `viz` 共 5 个消息分组，提供了常用类型的序列化与反序列化功能
 * @}
 */

#include <rmvl/rmvl_modules.hpp>

// 通用
#include "core/str.hpp"
#include "core/timer.hpp"
#include "core/util.hpp"
#include "core/version.hpp"

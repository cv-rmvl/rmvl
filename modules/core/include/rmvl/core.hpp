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
 * @brief 提供异常处理、定时器、编程工具和 YAML 数据读写等基础功能
 * @see core_reflect、core_meta、core_str、core_timer、core_yaml
 */

/**
 * @defgroup rmvlpara 参数模块
 * @brief 提供各模块的参数类、全局参数对象以及 YAML 运行时加载功能
 * @see 在引言中提及了有关 @ref intro_parameters_manager 的内容可供参考
 */

/**
 * @defgroup rmvlmsg 消息模块
 * @brief 提供 `std`、`geometry`、`sensor`、`motion` 和 `viz` 消息及其序列化功能
 */

/**
 * @defgroup rmvlsrv 服务模块
 * @brief 提供 `std` 和 `sensor` 服务的请求、响应及其序列化功能
 */

#include <rmvl/rmvl_modules.hpp>

// 通用
#include "core/str.hpp"
#include "core/timer.hpp"
#include "core/util.hpp"
#include "core/version.hpp"
#include "core/yaml.hpp"

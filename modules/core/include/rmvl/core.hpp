/**
 * @file core.hpp
 * @author RoboMaster Vision Community
 * @brief
 * @version 1.0
 * @date 2023-04-18
 *
 * @copyright Copyright 2023 (c), RoboMaster Vision Community
 *
 */

#pragma once

//! @defgroup core RMVL 核心模块

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

#include "core/util.hpp"
#include "core/version.hpp"

#include "core/dataio.hpp"
#include "core/kalman.hpp"
#include "core/numcal.hpp"
#include "core/serial.hpp"
#include "core/timer.hpp"

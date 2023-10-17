/**
 * @file decider.hpp
 * @author RoboMaster Vision Community
 * @brief 
 * @version 1.0
 * @date 2022-08-09
 * 
 * @copyright Copyright 2023 (c), RoboMaster Vision Community
 * 
 */

#pragma once

/**
 * @defgroup decider 决策模块（功能模块）
 * @{
 *     @defgroup gyro_decider 整车状态决策模块
 *     @defgroup translation_decider 平移目标决策模块
 *     @defgroup rune_decider 激活、未激活神符决策模块
 * @}
 */

#include <rmvl/rmvl_modules.hpp>

#include "decider/decider.h"

#ifdef HAVE_RMVL_GYRO_DECIDER
#include "decider/gyro_decider.h"
#endif // HAVE_RMVL_GYRO_DECIDER

#ifdef HAVE_RMVL_TRANSLATION_DECIDER
#include "decider/translation_decider.h"
#endif // HAVE_RMVL_TRANSLATION_DECIDER

#ifdef HAVE_RMVL_RUNE_DECIDER
#include "decider/rune_decider.h"
#endif // HAVE_RMVL_RUNE_DECIDER

/**
 * @file combo.hpp
 * @author RoboMaster Vision Community
 * @brief
 * @version 1.0
 * @date 2022-08-08
 *
 * @copyright Copyright 2023 (c), RoboMaster Vision Community
 *
 */

#pragma once

/**
 * @defgroup combo 特征组合体（数据组件）
 * @{
 *     @defgroup combo_armor 机器人装甲板
 *     @defgroup combo_rune 能量机关神符
 * @}
 */

#include <rmvl/rmvl_modules.hpp>

#include "combo/combo.h"

#ifdef HAVE_RMVL_ARMOR
#include "combo/armor.h"
#endif // HAVE_RMVL_ARMOR

#ifdef HAVE_RMVL_RUNE
#include "combo/rune.h"
#endif // HAVE_RMVL_RUNE

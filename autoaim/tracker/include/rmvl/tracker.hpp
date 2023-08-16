/**
 * @file tracker.hpp
 * @author RoboMaster Vision Community
 * @brief 
 * @version 1.0
 * @date 2022-08-09
 * 
 * @copyright Copyright 2023 (c), RoboMaster Vision Community
 * 
 */

/**
 * @defgroup tracker 追踪器/特征组合时间序列（数据组件）
 * @{
 *     @defgroup armor_tracker 装甲板时间序列
 *     @defgroup gyro_tracker 旋转装甲时间序列
 *     @defgroup rune_tracker 神符时间序列
 *     @defgroup top_armor_tracker 顶部装甲时间序列
 * @}
 */

#pragma once

#include <rmvl/rmvl_modules.hpp>

#include "tracker/tracker.h"

#ifdef HAVE_RMVL_ARMOR_TRACKER
#include "tracker/armor_tracker.h"
#endif //! HAVE_RMVL_ARMOR_TRACKER

#ifdef HAVE_RMVL_GYRO_TRACKER
#include "tracker/gyro_tracker.h"
#endif //! HAVE_RMVL_GYRO_TRACKER

#ifdef HAVE_RMVL_TOP_ARMOR_TRACKER
#include "tracker/top_armor_tracker.h"
#endif //! HAVE_RMVL_TOP_ARMOR_TRACKER

#ifdef HAVE_RMVL_RUNE_TRACKER
#include "tracker/rune_tracker.h"
#endif //! HAVE_RMVL_RUNE_TRACKER

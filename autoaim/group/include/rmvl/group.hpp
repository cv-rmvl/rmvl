/**
 * @file group.hpp
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
 * @defgroup group 相关的追踪器集合（数据组件）
 * @{
 *     @defgroup gyro_group 整车状态序列组
 *     @defgroup rune_group 神符序列组
 * @}
 */

#include <rmvl/rmvl_modules.hpp>

#include "group/group.h"

#ifdef HAVE_RMVL_GYRO_GROUP
#include "group/gyro_group.h"
#endif //! HAVE_RMVL_GYRO_GROUP

#ifdef HAVE_RMVL_RUNE_GROUP
#include "group/rune_group.h"
#endif //! HAVE_RMVL_RUNE_GROUP

/**
 * @file compensator.hpp
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
 * @defgroup compensator 补偿模块（功能模块）
 * @{
 *     @defgroup gravity_compensator 重力模型补偿
 *     @defgroup gyro_compensator 整车状态估计补偿模块
 * @}
 */

#include <rmvl/rmvl_modules.hpp>

#include "compensator/compensator.h"

#ifdef HAVE_RMVL_GRAVITY_COMPENSATOR
#include "compensator/gravity_compensator.h"
#endif // HAVE_RMVL_GRAVITY_COMPENSATOR

#ifdef HAVE_RMVL_GYRO_COMPENSATOR
#include "compensator/gyro_compensator.h"
#endif // HAVE_RMVL_GYRO_COMPENSATOR

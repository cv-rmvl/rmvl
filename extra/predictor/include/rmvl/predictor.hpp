/**
 * @file predictor.hpp
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
 * @defgroup predictor 目标预测模块（功能模块）
 * @{
 *     @defgroup armor_predictor 装甲预测模块
 *     @defgroup gyro_predictor 整车状态预测模块
 *     @defgroup rune_predictor 神符预测模块
 *     @defgroup spi_rune_predictor 系统参数辨识神符预测模块
 * @}
 */

#include <rmvl/rmvl_modules.hpp>

#include "predictor/predictor.h"

#ifdef HAVE_RMVL_ARMOR_PREDICTOR
#include "predictor/armor_predictor.h"
#endif //! HAVE_RMVL_ARMOR_PREDICTOR

#ifdef HAVE_RMVL_GYRO_PREDICTOR
#include "predictor/gyro_predictor.h"
#endif //! HAVE_RMVL_GYRO_PREDICTOR

#ifdef HAVE_RMVL_RUNE_PREDICTOR
#include "predictor/rune_predictor.h"
#endif //! HAVE_RMVL_RUNE_PREDICTOR

#ifdef HAVE_RMVL_SPI_RUNE_PREDICTOR
#include "predictor/spi_rune_predictor.h"
#endif //! HAVE_RMVL_SPI_RUNE_PREDICTOR

/**
 * @file detector.hpp
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
 * @defgroup detector 检测与识别模块（功能模块）
 * @{
 * @defgroup armor_detector 装甲板识别模块
 * @defgroup gyro_detector 整车状态识别模块
 * @defgroup rune_detector 激活、未激活神符识别模块
 * @defgroup tag_detector AprilTag(Tag25h9) 识别模块
 * @}
 */

#include <rmvl/rmvl_modules.hpp>

#include "detector/detector.h"

#ifdef HAVE_RMVL_ARMOR_DETECTOR
#include "detector/armor_detector.h"
#endif // HAVE_RMVL_ARMOR_DETECTOR

#ifdef HAVE_RMVL_GYRO_DETECTOR
#include "detector/gyro_detector.h"
#endif // HAVE_RMVL_GYRO_DETECTOR

#ifdef HAVE_RMVL_RUNE_DETECTOR
#include "detector/rune_detector.h"
#endif // HAVE_RMVL_RUNE_DETECTOR

#ifdef HAVE_RMVL_TAG_DETECTOR
#include "detector/tag_detector.h"
#endif // HAVE_RMVL_TAG_DETECTOR

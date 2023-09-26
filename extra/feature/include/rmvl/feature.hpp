/**
 * @file feature.hpp
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
@defgroup feature 图像轮廓特征（数据组件）
@{
    @defgroup light_blob 装甲模块灯条
    @defgroup pilot 前哨、基地的引导灯
    @defgroup rune_center 神符旋转中心特征
    @defgroup rune_target 神符靶心特征
    @defgroup tag AprilTag(Tag25h9) 视觉标签
@}
 */

#include <rmvl/rmvl_modules.hpp>

#include "feature/feature.h"

#ifdef HAVE_RMVL_LIGHT_BLOB
#include "feature/light_blob.h"
#endif //! HAVE_RMVL_LIGHT_BLOB

#ifdef HAVE_RMVL_PILOT
#include "feature/pilot.h"
#endif //! HAVE_RMVL_PILOT

#ifdef HAVE_RMVL_RUNE_TARGET
#include "feature/rune_target.h"
#endif //! HAVE_RMVL_RUNE_TARGET

#ifdef HAVE_RMVL_RUNE_CENTER
#include "feature/rune_center.h"
#endif //! HAVE_RMVL_RUNE_CENTER

#ifdef HAVE_RMVL_TAG
#include "feature/tag.h"
#endif // HAVE_RMVL_TAG

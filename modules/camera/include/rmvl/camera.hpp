/**
 * @file camera.hpp
 * @author RoboMaster Vision Community
 * @brief 相机调用
 * @version 1.0
 * @date 2021-10-01
 *
 * @copyright Copyright 2021 (c), RoboMaster Vision Community
 *
 */

#pragma once

//! @defgroup camera 相机模块

#include <rmvl/rmvl_modules.hpp>

#ifdef HAVE_RMVL_CAMERA
#include "camera/camutils.hpp"

#ifdef HAVE_RMVL_MV_CAMERA
#include "camera/mv_camera.h"
#endif // HAVE_RMVL_MV_CAMERA

#ifdef HAVE_RMVL_HIK_CAMERA
#include "camera/hik_camera.h"
#endif // HAVE_RMVL_HIK_CAMERA

#ifdef HAVE_RMVL_OPT_CAMERA
#include "camera/opt_camera.h"
#endif // HAVE_RMVL_OPT_CAMERA

#ifdef HAVE_RMVL_GALAXY_CAMERA
#include "camera/galaxy_camera.h"
#endif // HAVE_RMVL_GALAXY_CAMERA

#endif // HAVE_RMVL_CAMERA

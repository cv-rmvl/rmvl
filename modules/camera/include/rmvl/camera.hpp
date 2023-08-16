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

/**
 * @defgroup camera 相机模块
 * @{
 *     @defgroup mv_camera 迈德威视（MindVision）相机库
 *     @defgroup hik_camera 海康机器人（HikRobot）工业相机库
 * @}
 */

//! @addtogroup camera
//! @{

#include <rmvl/rmvl_modules.hpp>

#ifdef HAVE_RMVL_CAMERA
#include "camera/camera_define.h"

#ifdef HAVE_RMVL_MV_VIDEO_CAPTURE
#include "camera/mv_video_capture.h"
#endif //! HAVE_RMVL_MV_VIDEO_CAPTURE

#ifdef HAVE_RMVL_HIK_VIDEO_CAPTURE
#include "camera/hik_video_capture.h"
#endif //! HAVE_RMVL_HIK_VIDEO_CAPTURE

#endif //! HAVE_RMVL_CAMERA

//! @} camera

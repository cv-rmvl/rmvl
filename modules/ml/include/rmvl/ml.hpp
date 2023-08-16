/**
 * @file ml.hpp
 * @author RoboMaster Vision Community
 * @brief 
 * @version 1.0
 * @date 2023-05-20
 * 
 * @copyright Copyright 2023 (c), RoboMaster Vision Community
 * 
 */

#pragma once

/**
 * @defgroup ml 机器学习与深度学习支持库
 * @{
 *     @defgroup ml_ort ONNX-Runtime 分类网络部署库
 * @}
 */

#include <rmvl/rmvl_modules.hpp>

#ifdef HAVE_RMVL_ORT
#include "ml/ort.h"
#endif //! HAVE_RMVL_ORT

/**
 * @file ml.hpp
 * @author RMVL Community
 * @brief 
 * @version 2.0
 * @date 2024-06-02
 * 
 * @copyright Copyright 2024 (c), RMVL Community
 * 
 */

#pragma once

/**
 * @defgroup ml 机器学习与深度学习支持库
 * @{
 *     @defgroup ml_ort onnxruntime 多态部署库
 * @}
 */

#include <rmvl/rmvl_modules.hpp>

#ifdef HAVE_RMVL_ORT
#include "ml/ort.h"
#endif // HAVE_RMVL_ORT

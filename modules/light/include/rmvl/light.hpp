/**
 * @file light.hpp
 * @author zhaoxi (535394140@qq.com)
 * @brief 
 * @version 1.0
 * @date 2023-10-04
 * 
 * @copyright Copyright 2023 (c), zhaoxi
 * 
 */

/**
 * @defgroup light 光源控制器
 * @{
 * @defgroup opt_light_control OPT 奥普特 GigE 光源控制库
 * @defgroup hik_light_control 海康机器人 RS-232 光源控制库
 * @}
 */

#include <rmvl/rmvl_modules.hpp>

#ifdef HAVE_RMVL_OPT_LIGHT_CONTROL
#include "light/opt_light_control.h"
#endif // HAVE_RMVL_OPT_LIGHT_CONTROL

#ifdef HAVE_RMVL_HIK_LIGHT_CONTROL
#include "light/hik_light_control.h"
#endif // HAVE_RMVL_HIK_LIGHT_CONTROL

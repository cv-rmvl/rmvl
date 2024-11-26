/**
 * @file lightutils.hpp
 * @author zhaoxi (535394140@qq.com)
 * @brief 光源控制基本配置信息
 * @version 1.0
 * @date 2024-11-25
 *
 * @copyright Copyright 2024 (c), zhaoxi
 *
 */

#pragma once

#include "rmvl/core/util.hpp"

namespace rm
{

//! 光源控制器通信模式
enum class LightHandleMode
{
    IP,     //!< IP 地址
    Key,    //!< 设备序列号
    Serial, //!< 串口通信
};

//! 光源控制器初始化配置模式
struct RMVL_EXPORTS_W_AG LightConfig
{
    RMVL_W_RW LightHandleMode handle_mode{}; //!< 通信模式
};

} // namespace rm

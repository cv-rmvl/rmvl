/**
 * @file timer.hpp
 * @author zhaoxi (535394140@qq.com)
 * @brief 定时、计时模块
 * @version 1.0
 * @date 2023-10-16
 *
 * @copyright Copyright 2023 (c), zhaoxi
 *
 */

#pragma once

#include "rmvldef.hpp"

#include <chrono>

//! @addtogroup core
//! @{
//! @defgroup core_timer 定时、计时模块
//! @}

namespace rm {

//! @addtogroup core_timer
//! @{

//! 计时器类
class RMVL_EXPORTS_W Time {
public:
    //! 返回 Epoch 时间（单位：秒）
    RMVL_W static inline int64_t now_s() {
        return std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
    }

    //! 返回 Epoch 时间（单位：毫秒）
    RMVL_W static inline int64_t now() {
        return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
    }

    //! 返回 Epoch 时间（单位：微秒）
    RMVL_W static inline int64_t now_us() {
        return std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
    }
};

//! @} core_timer

} // namespace rm

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

#include <chrono>

#include "rmvldef.hpp"

//! @addtogroup core
//! @{
//! @defgroup core_timer 定时、计时模块
//! @}

namespace rm
{

//! @addtogroup core_timer
//! @{

/**
 * @brief 定时器
 * @brief
 * - 在程序开始时调用 `reset()` 函数，之后调用 `now()` 函数即可返回从构造初期到现在经过的时间
 */
class RMVL_EXPORTS_W Timer
{
    using steady_clock = std::chrono::steady_clock;

    static steady_clock::time_point _tick; //!< 起始时间

public:
    RMVL_W Timer() { reset(); }
    Timer(const Timer &) = delete;
    void operator=(const Timer &) = delete;

    //! 重置定时器
    RMVL_W static void reset() { _tick = steady_clock::now(); }
    //! 返回从构造初期到现在经过的时间（单位：s）
    RMVL_W static double now() { return std::chrono::duration_cast<std::chrono::duration<double>>(steady_clock::now() - _tick).count(); }
};

inline std::chrono::steady_clock::time_point Timer::_tick = {};

//! 全局定时器对象
inline Timer timer;

//! @} core_timer

} // namespace rm

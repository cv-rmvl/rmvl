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

//! @addtogroup core
//! @{
//! @defgroup core_timer 定时、计时模块
//! @}

namespace rm
{

//! @addtogroup core_timer
//! @{

//! 定时器
class Timer
{
    using steady_clock = std::chrono::steady_clock;

    static steady_clock::time_point _tick; //!< 起始时间

public:
    Timer() { reset(); }
    Timer(const Timer &) = delete;
    void operator=(const Timer &) = delete;

    //! 重置定时器
    static inline void reset() { _tick = steady_clock::now(); }
    //! 返回从构造初期到现在经过的时间（单位：s）
    static inline double now() { return std::chrono::duration_cast<std::chrono::duration<double>>(steady_clock::now() - _tick).count(); }
};

inline std::chrono::steady_clock::time_point Timer::_tick = {};

//! 全局定时器对象
inline Timer timer;

//! @} core_timer

}; // namespace rm

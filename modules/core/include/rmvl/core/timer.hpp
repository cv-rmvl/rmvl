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
public:
    RMVL_W Timer() { reset(); }

    //! 重置定时器
    RMVL_W static void reset();
    //! 返回从构造初期到现在经过的时间（单位：ms）
    RMVL_W static double now();

    /**
     * @brief 当前线程休眠指定时间
     * 
     * @param[in] t 时间间隔（单位：ms）
     */
    RMVL_W static void sleep_for(double t);

    /**
     * @brief 当前线程休眠至指定时间
     * 
     * @param[in] t 时间点（单位：ms）
     */
    RMVL_W static void sleep_until(double t);
};

//! @} core_timer

} // namespace rm

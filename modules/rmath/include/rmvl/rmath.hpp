/**
 * @file rmath.hpp
 * @author RoboMaster Vision Community
 * @brief 额外数据函数库
 * @version 1.0
 * @date 2021-06-14
 *
 * @copyright Copyright 2021 (c), RoboMaster Vision Community
 *
 */

#pragma once

/**
 * @defgroup rmath 数据结构与算法及实用数学库
 * @{
 *     @defgroup union_find 并查集
 *     @defgroup ra_heap 支持随机访问的堆结构
 * @}
 */

#include <rmvl/rmvl_modules.hpp>

#ifdef HAVE_RMVL_RMATH
#include "rmath/uty_math.hpp"

#include "rmath/result_pnp.hpp"
#include "rmath/transform.h"
#include "rmath/ew_topsis.hpp"

#include "rmath/union_find.hpp"
#include "rmath/ra_heap.hpp"
#endif //! HAVE_RMVL_RMATH

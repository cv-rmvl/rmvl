/**
 * @file rmvldef.hpp
 * @author RoboMaster Vision Community
 * @brief
 * @version 1.0
 * @date 2023-08-01
 *
 * @copyright Copyright 2023 (c), RoboMaster Vision Community
 *
 */

#pragma once

//! @addtogroup core
//! @{

#ifndef __RMVL_CAT
#define __RMVL_CAT__(x, y) x##y
#define __RMVL_CAT_(x, y) __RMVL_CAT__(x, y)
#define __RMVL_CAT(x, y) __RMVL_CAT_(x, y)
#endif

#ifndef __RMVL_EXPAND
#define __RMVL_EXPAND(x) x
#endif

#ifdef RMVL_Func
// keep current value (through RMVL port file)
#elif defined __GNUC__ || (defined(__cpluscplus) && (__cpluscplus >= 201103))
#define RMVL_Func __func__
#elif defined __clang__ && (__clang_minor__ * 100 + __clang_major__ >= 305)
#define RMVL_Func __func__
#elif defined(__STDC_VERSION__) && (__STDC_VERSION >= 199901)
#define RMVL_Func __func__
#elif defined _MSC_VER
#define RMVL_Func __FUNCTION__
#elif defined(__INTEL_COMPILER) && (_INTEL_COMPILER >= 600)
#define RMVL_Func __FUNCTION__
#elif defined __IBMCPP__ && __IBMCPP__ >= 500
#define RMVL_Func __FUNCTION__
#elif defined __BORLAND__ && (__BORLANDC__ >= 0x550)
#define RMVL_Func __FUNC__
#else
#define RMVL_Func "<unknown>"
#endif

/****************************** static analysys ******************************/

// In practice, some macro are not processed correctly (noreturn is not detected).
// We need to use simplified definition for them.
#ifndef RMVL_STATIC_ANALYSIS
#if defined(__KLOCWORK__) || defined(__clang_analyzer__) || defined(__COVERITY__)
#define RMVL_STATIC_ANALYSIS 1
#endif
#else
#if defined(RMVL_STATIC_ANALYSIS) && !(__RMVL_CAT(1, RMVL_STATIC_ANALYSIS) == 1) // defined and not empty
#if RMVL_STATIC_ANALYSIS == 0
#undef RMVL_STATIC_ANALYSIS
#endif
#endif
#endif

//! @} core

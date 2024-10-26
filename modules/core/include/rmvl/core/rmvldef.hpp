#pragma once

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

#ifndef RMVL_EXPORTS
#if (defined _WIN32 || defined WINCE || defined __CYGWIN__) && defined(RMVLAPI_EXPORTS)
#define RMVL_EXPORTS __declspec(dllexport)
#elif defined __GNUC__ && __GNUC__ >= 4 && (defined(RMVLAPI_EXPORTS) || defined(__APPLE__))
#define RMVL_EXPORTS __attribute__((visibility("default")))
#endif
#endif

#ifndef RMVL_EXPORTS
#define RMVL_EXPORTS
#endif

/************************* 为生成包装器生成特殊信息宏 *************************/

#define RMVL_EXPORTS_W RMVL_EXPORTS    //!< 导出符号并生成包装器代码
#define RMVL_EXPORTS_W_AG RMVL_EXPORTS //!< 导出符号，指定为聚合类，并生成包装器代码
#define RMVL_W                         //!< 为方法生成包装器代码
#define RMVL_W_SUBST(str)              //!< 为方法生成包装器代码，并指定从 `misc` 中替换
#define RMVL_W_RW                      //!< 为读写属性生成包装器代码

/******************************** 静态检查分析 ********************************/

// 实际上，某些宏未正确处理（未检测到 noreturn），我们需要为它们使用简化的定义。
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

/*********************************** 代码块 ***********************************/

// 在类中定义 PIMPL 模式的相关代码
#define RMVL_IMPL                               \
    class Impl;                                 \
    struct ImplDeleter                          \
    {                                           \
        void operator()(Impl *) const noexcept; \
    };                                          \
    std::unique_ptr<Impl, ImplDeleter> _impl

//! PIMPL 模式的删除器的完整定义
#define RMVL_IMPL_DEF(class_name) \
    void class_name::ImplDeleter::operator()(class_name::Impl *p) const noexcept { delete p; }

/**
 * @file util.hpp
 * @author RoboMaster Vision Community
 * @brief
 * @version 1.0
 * @date 2022-11-03
 *
 * @copyright Copyright 2023 (c), RoboMaster Vision Community
 *
 */

#pragma once

#ifndef __cplusplus
#error core.hpp header must be compiled as C++
#endif

#include <rmvl/rmvl_modules.hpp>

#include <cstdio>
#include <string>

#include "rmvldef.hpp"

//! @addtogroup core
//! @{

#define HIGHLIGHT_(msg...)                    \
    do                                        \
    {                                         \
        printf("\033[35minfo - \033[0m" msg); \
        printf("\n");                         \
    } while (false)

#define WARNING_(msg...)                      \
    do                                        \
    {                                         \
        printf("\033[33mwarn - \033[0m" msg); \
        printf("\n");                         \
    } while (false)

#define PASS_(msg...)                         \
    do                                        \
    {                                         \
        printf("\033[32minfo - \033[0m" msg); \
        printf("\n");                         \
    } while (false)

#define ERROR_(msg...)                       \
    do                                       \
    {                                        \
        printf("\033[31m err - \033[0m" msg); \
        printf("\n");                        \
    } while (false)

#define INFO_(msg...)          \
    do                         \
    {                          \
        printf("info - " msg); \
        printf("\n");          \
    } while (false)

#ifdef NDEBUG
#define DEBUG_WARNING_(msg...) ((void)0)
#define DEBUG_ERROR_(msg...) ((void)0)
#define DEBUG_HIGHLIGHT_(msg...) ((void)0)
#define DEBUG_INFO_(msg...) ((void)0)
#define DEBUG_PASS_(msg...) ((void)0)
#else
#define DEBUG_WARNING_(msg...) WARNING_(msg)
#define DEBUG_ERROR_(msg...) ERROR_(msg)
#define DEBUG_HIGHLIGHT_(msg...) HIGHLIGHT_(msg)
#define DEBUG_INFO_(msg...) INFO_(msg)
#define DEBUG_PASS_(msg...) PASS_(msg)
#endif

typedef int RMVLErrorCode; //!< 重定义 `int` 为 RMVLErrorCode

//! @brief RMVL 错误码
enum : RMVLErrorCode
{
    RMVL_StsOk = 0,           //!< 没有错误 No Error
    RMVL_StsBackTrace = -1,   //!< 回溯 Backtrace
    RMVL_StsError = -2,       //!< 未指定（未知）错误 Unspecified (Unknown) error
    RMVL_StsNoMem = -3,       //!< 内存不足 Insufficient memory
    RMVL_StsBadArg = -4,      //!< 参数异常 Bad argument
    RMVL_StsBadSize = -5,     //!< 数组大小不正确 Incorrect size of the array
    RMVL_StsNullPtr = -6,     //!< 空指针 Null pointer
    RMVL_StsNotaNumber = -7,  //!< 非数 Not a Number (nan)
    RMVL_StsDivByZero = -8,   //!< 发生了除以 `0` 的情况 Division by zero occurred
    RMVL_StsOutOfRange = -9,  //!< 其中一个参数的值超出了范围 One of the arguments' values is out of range
    RMVL_StsAssert = -10,     //!< 断言失败 Assertion failed
    RMVL_StsInvFmt = -11,     //!< 无效格式 Invalid format
    RMVL_BadDynamicType = -12 //!< 动态类型转换错误 Bad dynamic_cast type
};

const char *rmvlErrorStr(RMVLErrorCode status);

namespace rm
{

/**
 * @brief 返回使用类 `printf` 表达式格式化的文本字符串。
 * @note 该函数的作用类似于 `sprintf`，但形成并返回一个 STL 字符串。它可用于在 Exception 构造函数中形成错误消息。
 *
 * @param[in] fmt 与 `printf` 兼容的格式化说明符。
 * @details
 * | 类型 | 限定符 |
 * | ---- | ------ |
 * |`const char*`|`%s`|
 * |`char`|`%c`|
 * |`float` or `double`|`%f`,`%g`|
 * |`int`, `long`, `long long`|`%d`, `%ld`, ``%lld`|
 * |`unsigned`, `unsigned long`, `unsigned long long`|`%u`, `%lu`, `%llu`|
 * |`uint64` \f$\to\f$ `uintmax_t`, `int64` \f$\to\f$ `intmax_t`|`%ju`, `%jd`|
 * |`size_t`|`%zu`|
 */
std::string format(const char *fmt, ...);

/**
 * @brief 触发非法内存操作
 * @note 当调用该函数时，默认错误处理程序会发出一个硬件异常，这可以使调试更加方便
 */
inline void breakOnError()
{
    static volatile int *p = nullptr;
    *p = 0;
}

/**
 * @brief 该类封装了有关程序中发生的错误的所有或几乎所有必要信息。异常通常是通过 RMVL_Error
 *        和 RMVL_Error_ 宏隐式构造和抛出的 @see error
 */
class Exception final : public std::exception
{
public:
    //! @brief 默认构造
    Exception() : code(RMVL_StsOk), line(0) {}

    /**
     * @brief 完整的构造函数。通常不显式调用构造函数。而是使用宏 RMVL_Error()、RMVL_Error_()
     *        和 RMVL_Assert()
     */
    Exception(int _code, const std::string &_err, const std::string &_func, const std::string &_file, int _line);

    virtual ~Exception() noexcept = default;

    //! @return 错误描述和上下文作为文本字符串
    inline const char *what() const noexcept override { return msg.c_str(); }

    std::string msg;  //!< 格式化的错误信息
    int code;         //!< 错误码 @see RMVLErrorCode
    std::string err;  //!< 错误描述
    std::string func; //!< 函数名，仅在编译器支持获取时可用

    std::string file; //!< 发生错误的源文件名
    int line;         //!< 源文件中发生错误的行号
};

/**
 * @brief
 *
 * @param exc
 */
inline void throwError(const Exception &exc) { throw exc; }

#ifdef NDEBUG
#define RMVL_ERRHANDLE(exc) throwError(exc)
#else
#define RMVL_ERRHANDLE(...) breakOnError()
#endif

/**
 * @brief 发出错误信号并引发异常
 * @note 该函数将错误信息打印到stderr
 *
 * @param[in] _code 错误码
 * @param[in] _err 错误描述
 * @param[in] _func 函数名，仅在编译器支持获取时可用
 * @param[in] _file 发生错误的源文件名
 * @param[in] _line 源文件中发生错误的行号
 * @see RMVL_Error, RMVL_Error_, RMVL_Assert
 */
void error(int _code, const std::string &_err, const char *_func, const char *_file, int _line);

/**
 * @brief 返回完整的配置输出，返回值是原始的 CMake 输出，包括版本控制系统修订，编译器版本，编译器标志，启用的模块和第三方库等。
 *
 * @return const&
 */
const std::string &getBuildInformation();

} // namespace rm

/**
 * @brief 调用错误处理程序
 * @note 目前，错误处理程序将错误代码和错误消息打印到标准错误流 `stderr`。在 Debug
 *       配置中，它会引发内存访问冲突，以便调试器可以分析执行堆栈和所有参数。在
 *       Release 配置中，抛出异常。
 *
 * @param[in] code 一种 RMVLErrorCode 错误码
 * @param[in] msg 错误信息
 */
#define RMVL_Error(code, msg) rm::error(code, msg, RMVL_Func, __FILE__, __LINE__)

/**
 * @brief 调用错误处理程序
 * @note 该宏可用于动态构造错误消息，以包含一些动态信息，例如
 * @code
 *  // 请注意格式化文本消息周围的额外括号
 *  RMVL_Error_(StsOutOfRange, "the value at (%d, %d) = %g is out of range", badPt.x, badPt.y, badValue);
 * @endcode
 * @param[in] code 一种 RMVLErrorCode 错误码
 * @param[in] fmt 格式化字符串
 * @param[in] args 括号中带有类似 printf 格式的错误信息
 */
#define RMVL_Error_(code, fmt, args...) rm::error(code, rm::format(fmt, args), RMVL_Func, __FILE__, __LINE__)

/**
 * @brief 在运行时检查条件，如果失败则抛出异常
 * @note 宏 RMVL_Assert (以及 RMVL_DbgAssert) 对指定的表达式求值。如果它是0，宏抛出一个错误。宏
 *       RMVL_Assert 将会在 Debug 和 Release 的配置下检查条件，而 RMVL_DbgAssert 只会在 Debug
 *       的配置下生效。
 * @see RMVLErrorCode
 */
#define RMVL_Assert(expr) (!!(expr)) ? (void(0)) : rm::error(RMVL_StsAssert, #expr, RMVL_Func, __FILE__, __LINE__)

//! 在 Debug 条件下或启用静态分析工具的情况下，在运行时检查条件，如果失败则抛出异常
#if defined NDEBUG || defined RMVL_STATIC_ANALYSIS
/** replaced with RMVL_Assert(expr) in Debug configuration */
#define RMVL_DbgAssert(expr)
#else
#define RMVL_DbgAssert(expr) RMVL_Assert(expr)
#endif

namespace reflect
{

//! Constructor helper
struct init
{
    template <typename Tp>
    operator Tp(); // No need to define
};

template <std::size_t N>
struct size_tag : size_tag<N - 1>
{
};
template <>
struct size_tag<0>
{
};

#if __cplusplus < 202002L

template <typename Tp>
constexpr auto size(size_tag<10>) -> decltype(Tp{init{}, init{}, init{}, init{}, init{}, init{}, init{}, init{}, init{}, init{}}, 0u) { return 10u; }
template <typename Tp>
constexpr auto size(size_tag<9>) -> decltype(Tp{init{}, init{}, init{}, init{}, init{}, init{}, init{}, init{}, init{}}, 0u) { return 9u; }
template <typename Tp>
constexpr auto size(size_tag<8>) -> decltype(Tp{init{}, init{}, init{}, init{}, init{}, init{}, init{}, init{}}, 0u) { return 9u; }
template <typename Tp>
constexpr auto size(size_tag<7>) -> decltype(Tp{init{}, init{}, init{}, init{}, init{}, init{}, init{}}, 0u) { return 7u; }
template <typename Tp>
constexpr auto size(size_tag<6>) -> decltype(Tp{init{}, init{}, init{}, init{}, init{}, init{}}, 0u) { return 6u; }
template <typename Tp>
constexpr auto size(size_tag<5>) -> decltype(Tp{init{}, init{}, init{}, init{}, init{}}, 0u) { return 5u; }
template <typename Tp>
constexpr auto size(size_tag<4>) -> decltype(Tp{init{}, init{}, init{}, init{}}, 0u) { return 4u; }
template <typename Tp>
constexpr auto size(size_tag<3>) -> decltype(Tp{init{}, init{}, init{}}, 0u) { return 3u; }
template <typename Tp>
constexpr auto size(size_tag<2>) -> decltype(Tp{init{}, init{}}, 0u) { return 2; }
template <typename Tp>
constexpr auto size(size_tag<1>) -> decltype(Tp{init{}}, 0u) { return 1u; }
template <typename Tp>
constexpr auto size(size_tag<0>) -> decltype(Tp{}, 0u) { return 0u; }

#endif

} // namespace reflect

/**
 * @brief 获取指定类型的成员个数
 *
 * @tparam Tp 聚合类类型
 * @return 成员个数
 */
template <typename Tp>
#if __cplusplus < 202002L
constexpr std::size_t size()
{
    static_assert(std::is_aggregate_v<Tp>);
    return reflect::size<Tp>(reflect::size_tag<10>{});
}
#else
consteval std::size_t size(auto &&...args)
{
    static_assert(std::is_aggregate_v<Tp>);
    if constexpr (!requires { Tp{args...}; })
        return sizeof...(args) - 1;
    else
        return size<Tp>(args..., reflect::init{});
}
#endif

/**
 * @brief 遍历聚合类的每一个数据成员
 *
 * @tparam Tp 聚合类类型
 * @tparam Callable 可调用对象类型
 * @param[in] val 聚合类对象
 * @param[in] f 可调用对象
 */
template <typename Tp, typename Callable>
void for_each(const Tp &val, Callable &&f)
{
    static_assert(std::is_aggregate_v<Tp>);
    if constexpr (size<Tp>() == 10u)
    {
        const auto &[m0, m1, m2, m3, m4, m5, m6, m7, m8, m9] = val;
        f(m0), f(m1), f(m2), f(m3), f(m4), f(m5), f(m6), f(m7), f(m8), f(m9);
    }
    else if constexpr (size<Tp>() == 9u)
    {
        const auto &[m0, m1, m2, m3, m4, m5, m6, m7, m8] = val;
        f(m0), f(m1), f(m2), f(m3), f(m4), f(m5), f(m6), f(m7), f(m8);
    }
    else if constexpr (size<Tp>() == 8u)
    {
        const auto &[m0, m1, m2, m3, m4, m5, m6, m7] = val;
        f(m0), f(m1), f(m2), f(m3), f(m4), f(m5), f(m6), f(m7);
    }
    else if constexpr (size<Tp>() == 7u)
    {
        const auto &[m0, m1, m2, m3, m4, m5, m6] = val;
        f(m0), f(m1), f(m2), f(m3), f(m4), f(m5), f(m6);
    }
    else if constexpr (size<Tp>() == 6u)
    {
        const auto &[m0, m1, m2, m3, m4, m5] = val;
        f(m0), f(m1), f(m2), f(m3), f(m4), f(m5);
    }
    else if constexpr (size<Tp>() == 5u)
    {
        const auto &[m0, m1, m2, m3, m4] = val;
        f(m0), f(m1), f(m2), f(m3), f(m4);
    }
    else if constexpr (size<Tp>() == 4u)
    {
        const auto &[m0, m1, m2, m3] = val;
        f(m0), f(m1), f(m2), f(m3);
    }
    else if constexpr (size<Tp>() == 3u)
    {
        const auto &[m0, m1, m2] = val;
        f(m0), f(m1), f(m2);
    }
    else if constexpr (size<Tp>() == 2u)
    {
        const auto &[m0, m1] = val;
        f(m0), f(m1);
    }
    else if constexpr (size<Tp>() == 1u)
    {
        const auto &[m0] = val;
        f(m0);
    }
}

//! @} core

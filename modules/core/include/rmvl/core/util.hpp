/**
 * @file util.hpp
 * @author zhaoxi (535394140@qq.com)
 * @brief
 * @version 1.0
 * @date 2022-11-03
 *
 * @copyright Copyright 2023 (c), zhaoxi
 *
 */

#pragma once

#ifndef __cplusplus
#error core.hpp header must be compiled as C++
#endif

#include <rmvl/rmvl_modules.hpp>

#include <cstdint>
#include <cstdio>
#include <string>

#include "rmvldef.hpp"

//! @addtogroup core
//! @{

#define HIGHLIGHT_(...)                               \
    do                                                \
    {                                                 \
        printf("\033[35minfo - \033[0m" __VA_ARGS__); \
        printf("\n");                                 \
    } while (false)

#define WARNING_(...)                                 \
    do                                                \
    {                                                 \
        printf("\033[33mwarn - \033[0m" __VA_ARGS__); \
        printf("\n");                                 \
    } while (false)

#define PASS_(...)                                    \
    do                                                \
    {                                                 \
        printf("\033[32minfo - \033[0m" __VA_ARGS__); \
        printf("\n");                                 \
    } while (false)

#define ERROR_(...)                                   \
    do                                                \
    {                                                 \
        printf("\033[31m err - \033[0m" __VA_ARGS__); \
        printf("\n");                                 \
    } while (false)

#define INFO_(...)                     \
    do                                 \
    {                                  \
        printf("info - " __VA_ARGS__); \
        printf("\n");                  \
    } while (false)

#ifdef NDEBUG
#define DEBUG_WARNING_(...) ((void)0)
#define DEBUG_ERROR_(...) ((void)0)
#define DEBUG_HIGHLIGHT_(...) ((void)0)
#define DEBUG_INFO_(...) ((void)0)
#define DEBUG_PASS_(...) ((void)0)
#else
#define DEBUG_WARNING_(...) WARNING_(__VA_ARGS__)
#define DEBUG_ERROR_(...) ERROR_(__VA_ARGS__)
#define DEBUG_HIGHLIGHT_(...) HIGHLIGHT_(__VA_ARGS__)
#define DEBUG_INFO_(...) INFO_(__VA_ARGS__)
#define DEBUG_PASS_(...) PASS_(__VA_ARGS__)
#endif

//! @brief RMVL 错误码
enum RMVLErrorCode : int
{
    RMVL_StsOk = 0,           //!< 没有错误 No Error
    RMVL_StsBackTrace = -1,   //!< 回溯 Backtrace
    RMVL_StsError = -2,       //!< 未指定（未知）错误 Unspecified (Unknown) error
    RMVL_StsNoMem = -3,       //!< 内存不足 Insufficient memory
    RMVL_StsBadArg = -4,      //!< 参数异常 Bad argument
    RMVL_StsBadSize = -5,     //!< 数组大小不正确 Incorrect size of the array
    RMVL_StsBadFunc = -6,     //!< 功能不支持 Unsupported function
    RMVL_StsNullPtr = -7,     //!< 空指针 Null pointer
    RMVL_StsNotaNumber = -8,  //!< 非数 Not a Number (nan)
    RMVL_StsDivByZero = -9,   //!< 发生了除以 `0` 的情况 Division by zero occurred
    RMVL_StsOutOfRange = -10, //!< 其中一个参数的值超出了范围 One of the arguments' values is out of range
    RMVL_StsAssert = -11,     //!< 断言失败 Assertion failed
    RMVL_StsInvFmt = -12,     //!< 无效格式 Invalid format
    RMVL_BadDynamicType = -13 //!< 动态类型转换错误 Bad dynamic_cast type,
};

//! @} core

namespace rm
{

//! @addtogroup core
//! @{

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
 * |`int`, `long`, `long long`|`%d`, `%ld`, `%lld`|
 * |`unsigned`, `unsigned long`, `unsigned long long`|`%u`, `%lu`, `%llu`|
 * |`uint64_t` \f$\to\f$ `uintmax_t`, `int64_t` \f$\to\f$ `intmax_t`|`%ju`, `%jd`|
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
     * @brief 完整的构造函数。通常不显式调用构造函数。而是使用宏 RMVL_Error() 、 RMVL_Error_()
     *        和 RMVL_Assert()
     */
    Exception(int _code, std::string_view _err, std::string_view _func, std::string_view _file, int _line);

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

//! 抛出异常
inline void throwError(const Exception &exc) { throw exc; }

#ifdef NDEBUG
#define RMVL_ERRHANDLE(exc) throwError(exc)
#else
#define RMVL_ERRHANDLE(...) breakOnError()
#endif

/**
 * @brief 发出错误信号并引发异常
 * @note 该函数将错误信息打印到 stderr
 *
 * @param[in] _code 错误码
 * @param[in] _err 错误描述
 * @param[in] _func 函数名，仅在编译器支持获取时可用
 * @param[in] _file 发生错误的源文件名
 * @param[in] _line 源文件中发生错误的行号
 * @see RMVL_Error, RMVL_Error_, RMVL_Assert
 */
void error(int _code, std::string_view _err, const char *_func, const char *_file, int _line);

/**
 * @brief 返回完整的配置输出
 *
 * @return 原始的 CMake 输出，包括版本控制系统修订，编译器版本，编译器标志，启用的模块和第三方库等。
 * @retval 配置、构建信息字符串
 */
const char *getBuildInformation();

//! @} core

} // namespace rm

//! @addtogroup core
//! @{

/**
 * @brief 调用错误处理程序
 * @note 目前，错误处理程序将错误代码和错误消息打印到标准错误流 `stderr`。在 Debug
 *       配置中，它会引发内存访问冲突，以便调试器可以分析执行堆栈和所有参数。在
 *       Release 配置中，抛出异常。
 *
 * @param[in] code 一种 RMVLErrorCode 错误码
 * @param[in] msg 错误信息
 */
#define RMVL_Error(code, msg) ::rm::error(code, msg, RMVL_Func, __FILE__, __LINE__)

/**
 * @brief 调用错误处理程序
 * @note 该宏可用于动态构造错误消息，以包含一些动态信息，例如
 * @code
 * // 请注意格式化文本消息周围的额外括号
 * RMVL_Error_(RMVL_StsBadArg, "Bad channel of the input argument: \"input_image\", chn = %d", C);
 * @endcode
 * @param[in] code 一种 RMVLErrorCode 错误码
 * @param[in] fmt 格式化字符串
 * @param[in] ... 括号中带有类似 printf 格式的错误信息
 */
#define RMVL_Error_(code, fmt, ...) ::rm::error(code, ::rm::format(fmt, __VA_ARGS__), RMVL_Func, __FILE__, __LINE__)

/**
 * @brief 在运行时检查条件，如果失败则抛出异常
 * @note 宏 RMVL_Assert (以及 RMVL_DbgAssert) 对指定的表达式求值。如果它是0，宏抛出一个错误。宏
 *       RMVL_Assert 将会在 Debug 和 Release 的配置下检查条件，而 RMVL_DbgAssert 只会在 Debug
 *       的配置下生效。
 * @see RMVLErrorCode
 */
#define RMVL_Assert(expr) (!!(expr)) ? (void(0)) : ::rm::error(RMVL_StsAssert, #expr, RMVL_Func, __FILE__, __LINE__)

//! 在 Debug 条件下或启用静态分析工具的情况下，在运行时检查条件，如果失败则抛出异常
#if defined NDEBUG || defined RMVL_STATIC_ANALYSIS
/** replaced with RMVL_Assert(expr) in Debug configuration */
#define RMVL_DbgAssert(expr)
#else
#define RMVL_DbgAssert(expr) RMVL_Assert(expr)
#endif

//! @} core

namespace rm
{

namespace reflect
{

namespace helper
{

//! @cond

//! Constructor helper
struct init
{
    template <typename Tp>
    operator Tp(); // No need to define
};

template <std::size_t N>
struct size_tag : size_tag<N - 1>
{};
template <>
struct size_tag<0>
{};

#if __cplusplus < 202002L
template <typename Tp>
constexpr auto size(size_tag<12>) -> decltype(Tp{init{}, init{}, init{}, init{}, init{}, init{}, init{}, init{}, init{}, init{}, init{}, init{}}, 0u) { return 12u; }
template <typename Tp>
constexpr auto size(size_tag<11>) -> decltype(Tp{init{}, init{}, init{}, init{}, init{}, init{}, init{}, init{}, init{}, init{}, init{}}, 0u) { return 11u; }
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

//! @endcond

} // namespace helper

//! @addtogroup core_reflect
//! @{

/**
 * @brief 获取指定类型的成员个数
 * @note 成员个数不要超过 `12`
 *
 * @tparam Tp 聚合类类型
 * @return 成员个数
 */
template <typename Tp>
#if __cplusplus < 202002L
constexpr std::size_t size()
{
    static_assert(std::is_aggregate_v<std::remove_reference_t<Tp>>);
    return helper::size<std::remove_reference_t<Tp>>(helper::size_tag<12>{});
}
#else
consteval std::size_t size(auto &&...args)
{
    static_assert(std::is_aggregate_v<std::remove_reference_t<Tp>>);
    if constexpr (!requires { std::remove_reference_t<Tp>{args...}; })
        return sizeof...(args) - 1;
    else
        return size<std::remove_reference_t<Tp>>(args..., helper::init{});
}
#endif

/**
 * @brief 遍历聚合类的每一个数据成员
 * @note 成员个数不要超过 `12`
 *
 * @tparam Tp 聚合类类型
 * @tparam Callable 可调用对象类型
 * @param[in] val 聚合类对象
 * @param[in] f 可调用对象
 */
template <typename Tp, typename Callable>
inline void for_each(Tp &&val, Callable &&f)
{
    static_assert(std::is_aggregate_v<std::remove_reference_t<Tp>>);
    if constexpr (size<std::remove_reference_t<Tp>>() == 12u)
    {
        auto &&[m0, m1, m2, m3, m4, m5, m6, m7, m8, m9, m10, m11] = val;
        f(m0), f(m1), f(m2), f(m3), f(m4), f(m5), f(m6), f(m7), f(m8), f(m9), f(m10), f(m11);
    }
    else if constexpr (size<std::remove_reference_t<Tp>>() == 11u)
    {
        auto &&[m0, m1, m2, m3, m4, m5, m6, m7, m8, m9, m10] = val;
        f(m0), f(m1), f(m2), f(m3), f(m4), f(m5), f(m6), f(m7), f(m8), f(m9), f(m10);
    }
    else if constexpr (size<std::remove_reference_t<Tp>>() == 10u)
    {
        auto &&[m0, m1, m2, m3, m4, m5, m6, m7, m8, m9] = val;
        f(m0), f(m1), f(m2), f(m3), f(m4), f(m5), f(m6), f(m7), f(m8), f(m9);
    }
    else if constexpr (size<std::remove_reference_t<Tp>>() == 9u)
    {
        auto &&[m0, m1, m2, m3, m4, m5, m6, m7, m8] = val;
        f(m0), f(m1), f(m2), f(m3), f(m4), f(m5), f(m6), f(m7), f(m8);
    }
    else if constexpr (size<std::remove_reference_t<Tp>>() == 8u)
    {
        auto &&[m0, m1, m2, m3, m4, m5, m6, m7] = val;
        f(m0), f(m1), f(m2), f(m3), f(m4), f(m5), f(m6), f(m7);
    }
    else if constexpr (size<std::remove_reference_t<Tp>>() == 7u)
    {
        auto &&[m0, m1, m2, m3, m4, m5, m6] = val;
        f(m0), f(m1), f(m2), f(m3), f(m4), f(m5), f(m6);
    }
    else if constexpr (size<std::remove_reference_t<Tp>>() == 6u)
    {
        auto &&[m0, m1, m2, m3, m4, m5] = val;
        f(m0), f(m1), f(m2), f(m3), f(m4), f(m5);
    }
    else if constexpr (size<std::remove_reference_t<Tp>>() == 5u)
    {
        auto &&[m0, m1, m2, m3, m4] = val;
        f(m0), f(m1), f(m2), f(m3), f(m4);
    }
    else if constexpr (size<std::remove_reference_t<Tp>>() == 4u)
    {
        auto &&[m0, m1, m2, m3] = val;
        f(m0), f(m1), f(m2), f(m3);
    }
    else if constexpr (size<std::remove_reference_t<Tp>>() == 3u)
    {
        auto &&[m0, m1, m2] = val;
        f(m0), f(m1), f(m2);
    }
    else if constexpr (size<std::remove_reference_t<Tp>>() == 2u)
    {
        auto &&[m0, m1] = val;
        f(m0), f(m1);
    }
    else if constexpr (size<std::remove_reference_t<Tp>>() == 1u)
    {
        auto &&[m0] = val;
        f(m0);
    }
}

/**
 * @brief 判断两个聚合类数据是否相同
 * @note 成员个数不要超过 `12`
 *
 * @tparam Tp 聚合类类型
 * @param[in] lhs 左操作数
 * @param[in] rhs 右操作数
 */
template <typename Tp>
inline bool equal(const Tp &lhs, const Tp &rhs)
{
    static_assert(std::is_aggregate_v<std::remove_reference_t<Tp>>);
    if constexpr (size<std::remove_reference_t<Tp>>() == 12u)
    {
        const auto &[l0, l1, l2, l3, l4, l5, l6, l7, l8, l9, l10, l11] = lhs;
        const auto &[r0, r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, r11] = rhs;
        return l0 == r0 && l1 == r1 && l2 == r2 && l3 == r3 && l4 == r4 && l5 == r5 && l6 == r6 && l7 == r7 && l8 == r8 && l9 == r9 && l10 == r10 && l11 == r11;
    }
    else if constexpr (size<std::remove_reference_t<Tp>>() == 11u)
    {
        const auto &[l0, l1, l2, l3, l4, l5, l6, l7, l8, l9, l10] = lhs;
        const auto &[r0, r1, r2, r3, r4, r5, r6, r7, r8, r9, r10] = rhs;
        return l0 == r0 && l1 == r1 && l2 == r2 && l3 == r3 && l4 == r4 && l5 == r5 && l6 == r6 && l7 == r7 && l8 == r8 && l9 == r9 && l10 == r10;
    }
    else if constexpr (size<std::remove_reference_t<Tp>>() == 10u)
    {
        const auto &[l0, l1, l2, l3, l4, l5, l6, l7, l8, l9] = lhs;
        const auto &[r0, r1, r2, r3, r4, r5, r6, r7, r8, r9] = rhs;
        return l0 == r0 && l1 == r1 && l2 == r2 && l3 == r3 && l4 == r4 && l5 == r5 && l6 == r6 && l7 == r7 && l8 == r8 && l9 == r9;
    }
    else if constexpr (size<std::remove_reference_t<Tp>>() == 9u)
    {
        const auto &[l0, l1, l2, l3, l4, l5, l6, l7, l8] = lhs;
        const auto &[r0, r1, r2, r3, r4, r5, r6, r7, r8] = rhs;
        return l0 == r0 && l1 == r1 && l2 == r2 && l3 == r3 && l4 == r4 && l5 == r5 && l6 == r6 && l7 == r7 && l8 == r8;
    }
    else if constexpr (size<std::remove_reference_t<Tp>>() == 8u)
    {
        const auto &[l0, l1, l2, l3, l4, l5, l6, l7] = lhs;
        const auto &[r0, r1, r2, r3, r4, r5, r6, r7] = rhs;
        return l0 == r0 && l1 == r1 && l2 == r2 && l3 == r3 && l4 == r4 && l5 == r5 && l6 == r6 && l7 == r7;
    }
    else if constexpr (size<std::remove_reference_t<Tp>>() == 7u)
    {
        const auto &[l0, l1, l2, l3, l4, l5, l6] = lhs;
        const auto &[r0, r1, r2, r3, r4, r5, r6] = rhs;
        return l0 == r0 && l1 == r1 && l2 == r2 && l3 == r3 && l4 == r4 && l5 == r5 && l6 == r6;
    }
    else if constexpr (size<std::remove_reference_t<Tp>>() == 6u)
    {
        const auto &[l0, l1, l2, l3, l4, l5] = lhs;
        const auto &[r0, r1, r2, r3, r4, r5] = rhs;
        return l0 == r0 && l1 == r1 && l2 == r2 && l3 == r3 && l4 == r4 && l5 == r5;
    }
    else if constexpr (size<std::remove_reference_t<Tp>>() == 5u)
    {
        const auto &[l0, l1, l2, l3, l4] = lhs;
        const auto &[r0, r1, r2, r3, r4] = rhs;
        return l0 == r0 && l1 == r1 && l2 == r2 && l3 == r3 && l4 == r4;
    }
    else if constexpr (size<std::remove_reference_t<Tp>>() == 4u)
    {
        const auto &[l0, l1, l2, l3] = lhs;
        const auto &[r0, r1, r2, r3] = rhs;
        return l0 == r0 && l1 == r1 && l2 == r2 && l3 == r3;
    }
    else if constexpr (size<std::remove_reference_t<Tp>>() == 3u)
    {
        const auto &[l0, l1, l2] = lhs;
        const auto &[r0, r1, r2] = rhs;
        return l0 == r0 && l1 == r1 && l2 == r2;
    }
    else if constexpr (size<std::remove_reference_t<Tp>>() == 2u)
    {
        const auto &[l0, l1] = lhs;
        const auto &[r0, r1] = rhs;
        return l0 == r0 && l1 == r1;
    }
    else if constexpr (size<std::remove_reference_t<Tp>>() == 1u)
    {
        const auto &[l0] = lhs;
        const auto &[r0] = rhs;
        return l0 == r0;
    }
}

//! @} core_reflect

} // namespace reflect

//! @addtogroup core_meta
//! @{

/**
 * @brief 专为聚合类添加的 hash 生成可调用对象
 *
 * @tparam Tp 聚合体类型
 */
template <typename Tp, typename Enable = std::enable_if_t<std::is_aggregate_v<Tp>>>
struct hash_aggregate
{
    std::size_t operator()(const Tp &r) const
    {
        std::size_t retval{};
        reflect::for_each(r, [&retval](const auto &val) {
            // boost::hash_combine
            retval = retval ^ (std::hash<std::remove_cv_t<std::remove_reference_t<decltype(val)>>>{}(val) << 1);
        });
        return retval;
    }
};

/**
 * @brief 哈希生成函数类型 traits
 * @note 类型别名 `hash_func`
 * @tparam Tp 数据类型
 */
template <typename Tp, typename Enable = void>
struct hash_traits;

/**
 * @brief 非聚合类哈希生成函数类型 traits
 *
 * @tparam Tp 数据类型
 */
template <typename Tp>
struct hash_traits<Tp, std::enable_if_t<!std::is_aggregate_v<Tp>>>
{
    using hash_func = std::hash<Tp>;
};

/**
 * @brief 聚合类哈希生成函数类型 traits
 *
 * @tparam Tp 数据类型
 */
template <typename Tp>
struct hash_traits<Tp, std::enable_if_t<std::is_aggregate_v<Tp>>>
{
    using hash_func = hash_aggregate<Tp>;
};

//! @} core_meta

} // namespace rm

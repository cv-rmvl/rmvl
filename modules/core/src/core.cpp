/**
 * @file core.cpp
 * @author RoboMaster Vision Community
 * @brief
 * @version 1.0
 * @date 2023-04-21
 *
 * @copyright Copyright 2023 (c), RoboMaster Vision Community
 *
 */

#include <chrono>
#include <cstdarg>
#include <thread>

#include "rmvl/core/str.hpp"
#include "rmvl/core/timer.hpp"
#include "rmvl/core/util.hpp"

namespace rm
{

static constexpr const char *rmvlErrorStr(int status)
{
    switch (status)
    {
    case RMVL_StsOk:
        return "No Error";
    case RMVL_StsBackTrace:
        return "Backtrace";
    case RMVL_StsError:
        return "Unspecified error";
    case RMVL_StsNoMem:
        return "Insufficient memory";
    case RMVL_StsBadArg:
        return "Bad argument";
    case RMVL_StsBadSize:
        return "Incorrect size of the array";
    case RMVL_StsBadFunc:
        return "Incorrect function";
    case RMVL_StsNullPtr:
        return "Null pointer";
    case RMVL_StsNotaNumber:
        return "Not a number (nan)";
    case RMVL_StsDivByZero:
        return "Division by zero occurred";
    case RMVL_StsOutOfRange:
        return "One of the arguments\' values is out of range";
    case RMVL_StsAssert:
        return "Assertion failed";
    case RMVL_StsInvFmt:
        return "Invalid format";
    case RMVL_BadDynamicType:
        return "Bad dynamic_cast type";
    default:
        return "Unknown error/status code";
    }
}

std::string format(const char *fmt, ...)
{
    char buf[1024]{};
    va_list args;
    va_start(args, fmt);
#ifdef _WIN32
    vsprintf_s(buf, fmt, args);
#else
    vsprintf(buf, fmt, args);
#endif
    va_end(args);
    std::string str(buf);
    return str;
}

Exception::Exception(int _code, std::string_view _err, std::string_view _func, std::string_view _file, int _line)
    : code(_code), err(_err), func(_func), file(_file), line(_line)
{
    if (!func.empty())
        msg = format("RMVL %s: %d: \033[31;1merror\033[0m: (%d:%s) in function \"%s\"\n\033[34m"
                     ">>>>>>>> message >>>>>>>>\033[0m\n%s\n\033[34m<<<<<<<< message <<<<<<<<\033[0m\n",
                     file.c_str(), line, code, rmvlErrorStr(code), func.c_str(), err.c_str());
    else
        msg = format("RMVL %s: %d: \033[31;1merror\033[0m: (%d:%s)\n\033[34m"
                     ">>>>>>>> message >>>>>>>>\033[0m\n%s\n\033[34m<<<<<<<< message <<<<<<<<\033[0m\n",
                     file.c_str(), line, code, rmvlErrorStr(code), err.c_str());
}

void error(int _code, std::string_view _err, const char *_func, const char *_file, int _line)
{
    Exception exc(_code, _err, _func, _file, _line);

    RMVL_ERRHANDLE(exc);

#ifdef _GNUC_
#if !defined _clang_ && !defined _APPLE_
    // this suppresses this warning: "noreturn" function does return [enabled by default]
    _builtin_trap();
    // or use infinite loop: for (;;) {}
#endif
#endif // _GNUC_
}

const char *getBuildInformation()
{
    static const char *build_info =
#include "version_string.inc"
        ;
    return build_info;
}

namespace str
{

std::vector<std::string> split(std::string_view str, std::string_view delim)
{
    std::vector<std::string> res;
    if (str.empty())
        return res;
    std::string::size_type start = str.find_first_not_of(delim);
    std::string::size_type index = str.find(delim, start);
    while (index != std::string::npos)
    {
        res.emplace_back(str.substr(start, index - start));
        start = str.find_first_not_of(delim, index);
        index = str.find(delim, start);
    }
    res.emplace_back(str.substr(start));
    return res;
}

std::string join(const std::vector<std::string> &strs, std::string_view delim)
{
    std::string res;
    if (strs.empty())
        return res;
    for (const auto &str : strs)
        res += str + delim.data();
    res.pop_back();
    return res;
}

std::string_view strip(std::string_view str)
{
    auto str_begin = str.find_first_not_of(" \t\n\r");
    auto str_end = str.find_last_not_of(" \t\n\r") + 1;
    return str.substr(str_begin, str_end - str_begin);
}

std::string lower(std::string_view str)
{
    std::string res(str);
    for (auto &c : res)
        c = std::tolower(c);
    return res;
}

std::string upper(std::string_view str)
{
    std::string res(str);
    for (auto &c : res)
        c = std::toupper(c);
    return res;
}

} // namespace str

//////////////////////////////////////////// Timer ////////////////////////////////////////////

static std::chrono::steady_clock::time_point g_init_tick = {};

void Timer::reset() { g_init_tick = std::chrono::steady_clock::now(); }

double Timer::now()
{
    return std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(std::chrono::steady_clock::now() - g_init_tick).count();
}

void Timer::sleep_for(double t)
{
    std::this_thread::sleep_for(std::chrono::duration<double, std::milli>(t));
}

void Timer::sleep_until(double t)
{
    auto tick = g_init_tick + std::chrono::duration<double, std::milli>(t);
    std::this_thread::sleep_until(tick);
}

} // namespace rm

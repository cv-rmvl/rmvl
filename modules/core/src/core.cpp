/**
 * @file core.cpp
 * @author zhaoxi (535394140@qq.com)
 * @brief
 * @version 1.0
 * @date 2023-04-21
 *
 * @copyright Copyright 2023 (c), zhaoxi
 *
 */

#include <cstdarg>
#include <cstring>

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
    vsprintf(buf, fmt, args);
    va_end(args);
    std::string str(buf);
    return str;
}

Exception::Exception(int _code, std::string_view _err, std::string_view _func, std::string_view _file, int _line)
    : code(_code), err(_err), func(_func), file(_file), line(_line)
{
    if (!func.empty())
        msg = format("RMVL %s:%d: \033[31;1merror\033[0m: (%d:%s) in function \"%s\"\n\033[34m"
                         ">>>>>>>> message >>>>>>>>\033[0m\n%s\n\033[34m<<<<<<<< message <<<<<<<<\033[0m\n",
                         file.c_str(), line, code, rmvlErrorStr(code), func.c_str(), err.c_str());
    else
        msg = format("RMVL %s:%d: \033[31;1merror\033[0m: (%d:%s)\n\033[34m"
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
    constexpr const char *build_info =
#include "version_string.inc"
        ;
    return build_info;
}

} // namespace rm

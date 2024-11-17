/**
 * @file type_to_string.cpp
 * @author RoboMaster Vision Community
 * @brief 类型 / 字符转换
 * @version 1.0
 * @date 2023-09-16
 *
 * @copyright Copyright 2023 (c), RoboMaster Vision Community
 *
 */

#include "rmvl/types.hpp"

namespace rm
{

std::string RMStatus::to_string(RobotType type)
{
    constexpr const char *type_map[] = {"UNKNOWN", "HERO", "ENGINEER", "INFANTRY_3", "INFANTRY_4", "INFANTRY_5", "OUTPOST", "BASE", "SENTRY"};
    return type_map[static_cast<int>(type)];
}

std::string RMStatus::to_string(TagType type)
{
    constexpr const char *type_map[] = {
        "UNKNOWN",
        "0", "1", "2", "3", "4", "5", "6", "7", "8", "9",
        "A", "B", "C", "D", "E", "F", "G", "H", "I", "J",
        "K", "L", "M", "N", "O", "P", "Q", "R", "S", "T",
        "U", "V", "W", "X", "Y", "Z"};
    return type_map[static_cast<int>(type)];
}

} // namespace rm

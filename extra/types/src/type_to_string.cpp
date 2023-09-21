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

std::string rm::RMStatus::to_string(rm::RobotType type)
{
    switch (type)
    {
    case rm::RobotType::HERO:
        return "HERO";
    case rm::RobotType::ENGINEER:
        return "ENGINEER";
    case rm::RobotType::INFANTRY_3:
        return "INFANTRY_3";
    case rm::RobotType::INFANTRY_4:
        return "INFANTRY_4";
    case rm::RobotType::INFANTRY_5:
        return "INFANTRY_5";
    case rm::RobotType::OUTPOST:
        return "OUTPOST";
    case rm::RobotType::BASE:
        return "BASE";
    case rm::RobotType::SENTRY:
        return "SENTRY";
    default:
        return "UNKNOWN";
    }
}

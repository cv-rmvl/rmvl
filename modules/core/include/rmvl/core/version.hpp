/**
 * @file version.hpp
 * @author RoboMaster Vision Community
 * @brief RMVL 版本控制
 * @version 1.0
 *
 * @copyright Copyright 2023 (c), RoboMaster Vision Community
 *
 */

#pragma once

#define RMVL_VERSION_MAJOR 1
#define RMVL_VERSION_MINOR 1
#define RMVL_VERSION_PATCH 0
#define RMVL_VERSION_STATUS ""

#define RMVLAUX_STR_EXP(__A) #__A
#define RMVLAUX_STR(__A) RMVLAUX_STR_EXP(__A)

#define RMVL_VERSION RMVLAUX_STR(RMVL_VERSION_MAJOR) "." RMVLAUX_STR(RMVL_VERSION_MINOR) "." RMVLAUX_STR(RMVL_VERSION_PATCH) RMVL_VERSION_STATUS

namespace rm
{

/**
 * @brief 返回库版本字符串
 * @note "For example: 3.6.0-dev"
 * @see getVersionMajor, getVersionMinor, getVersionPatch
 */
inline const char *getVersionString() { return RMVL_VERSION; }

//! 返回主要库版本
inline int getVersionMajor() { return RMVL_VERSION_MAJOR; }

//! 返回次要库版本
inline int getVersionMinor() { return RMVL_VERSION_MINOR; }

//! 返回库版本的修订字段
inline int getVersionPatch() { return RMVL_VERSION_PATCH; }

} // namespace rm

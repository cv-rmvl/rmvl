/**
 * @file version.hpp
 * @author zhaoxi (535394140@qq.com)
 * @brief RMVL 版本控制
 * @version 1.0
 *
 * @copyright Copyright 2023 (c), zhaoxi
 *
 */

#pragma once

#include "rmvldef.hpp"

#define RMVL_VERSION_MAJOR 2
#define RMVL_VERSION_MINOR 4
#define RMVL_VERSION_PATCH 0
#define RMVL_VERSION_STATUS "-dev"

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
constexpr const char *getVersionString() { return RMVL_VERSION; }

RMVL_EXPORTS_W constexpr const char *version() { return getVersionString(); }

//! 返回主要库版本
constexpr int getVersionMajor() { return RMVL_VERSION_MAJOR; }

//! 返回次要库版本
constexpr int getVersionMinor() { return RMVL_VERSION_MINOR; }

//! 返回库版本的修订字段
constexpr int getVersionPatch() { return RMVL_VERSION_PATCH; }

} // namespace rm

/**
 * @file str.hpp
 * @author zhaoxi (535394140@qq.com)
 * @brief Python 风格的字符串处理
 * @version 1.0
 * @date 2024-12-24
 *
 * @copyright Copyright 2024 (c), zhaoxi
 *
 */

#pragma once

#include <string>
#include <string_view>
#include <vector>

namespace rm::str
{

//! @addtogroup core_str
//! @{

/**
 * @brief 字符串分割
 * 
 * @param[in] str 待分割的字符串
 * @param[in] delim 分割符
 * @return 分割后的字符串数组
 */
std::vector<std::string> split(std::string_view str, std::string_view delim);

/**
 * @brief 字符串连接
 * 
 * @param[in] strs 待连接的字符串数组
 * @param[in] delim 连接符
 * @return 连接后的字符串
 */
std::string join(const std::vector<std::string> &strs, std::string_view delim);

//! @} core_str

} // namespace rm::str
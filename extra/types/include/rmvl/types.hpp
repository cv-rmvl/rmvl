/**
 * @file types.hpp
 * @author zhaoxi (535394140@qq.com)
 * @brief 状态类型系统
 * @version 3.0
 * @date 2025-01-03
 *
 * @copyright Copyright 2025 (c), zhaoxi
 *
 */

#pragma once

#include <string>
#include <string_view>
#include <unordered_map>

#include "rmvl/core/rmvldef.hpp"

//! @defgroup types 状态类型系统

namespace rm
{

//! @addtogroup types
//! @{

//! 状态类型系统
class RMVL_EXPORTS_W StateInfo
{
public:
    RMVL_W StateInfo() = default;

    /**
     * @brief 添加状态类型，示例代码如下：
     * @code {.cpp}
     * StateInfo state;
     * state.add("tag1: 0");          // 添加 tag1 状态类型，值为 '0'
     * state.add("tag2: a, tag3: +"); // 添加 tag2 状态类型，值为 'a'，tag3 状态类型，值为 '+'
     * state.add("tag4");             // 添加 tag4 状态类型
     * state.add("tag5: ABC, xx1");   // 添加 tag5 状态类型，值为 'ABC'，xx1 状态类型，值为空
     * @endcode
     *
     * @param[in] type 状态类型
     * @return 是否添加成功
     */
    RMVL_W void add(std::string_view type);

    /**
     * @brief 移除状态类型
     *
     * @param[in] key 状态类型
     * @return 是否移除成功
     */
    RMVL_W bool remove(std::string_view key);

    /**
     * @brief 是否包含状态类型
     *
     * @param[in] key 状态类型
     * @return 是否包含
     */
    RMVL_W bool contains(std::string_view key) const;

    //! 清空状态类型
    RMVL_W void clear();

    //! 状态类型是否为空
    RMVL_W bool empty() const;

    /**
     * @brief 获取状态类型
     *
     * @param[in] key 状态类型
     * @return 状态
     */
    std::string_view at(std::string_view key) const;

    /**
     * @brief 设置状态类型
     *
     * @param[in] key 状态类型
     * @return 状态
     */
    std::string &at(std::string_view key);

    /**
     * @brief 访问状态类型
     *
     * @param[in] key 状态类型
     * @return 状态
     */
    std::string &operator[](std::string_view key);

    RMVL_W_SUBST("At")

private:
    std::unordered_map<std::string, std::string> _states; //!< 状态类型
};

//! @} types

} // namespace rm

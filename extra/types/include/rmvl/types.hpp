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
#include <variant>

#include "rmvl/core/rmvldef.hpp"

//! @defgroup types 状态类型系统

namespace rm
{

//! @addtogroup types
//! @{

using StateType = std::variant<double, std::string>; //!< 状态类型

//! 状态类型系统
class RMVL_EXPORTS_W StateInfo
{
public:
    RMVL_W StateInfo() = default;

    /**
     * @brief 添加数值状态
     * @code {.cpp}
     * StateInfo state;
     * state.add("tag1", 1.2); // 添加 tag1 状态类型，值为 1.2
     * @endcode
     *
     * @param[in] key 状态类型名称
     * @param[in] val 数值状态值
     */
    RMVL_W void add(std::string_view key, double val);

    /**
     * @brief 添加字符串状态
     * @code {.cpp}
     * StateInfo state;
     * state.add("tag1", "hello"); // 添加 tag1 状态类型，值为 "hello"
     * @endcode
     *
     * @param[in] key 状态类型名称
     * @param[in] str 字符串状态值
     */
    RMVL_W void add(std::string_view key, std::string_view str);

    /**
     * @brief 移除状态类型
     *
     * @param[in] key 状态类型名称
     * @return 是否移除成功
     */
    RMVL_W bool remove(std::string_view key);

    /**
     * @brief 是否包含状态类型
     *
     * @param[in] key 状态类型名称
     * @return 是否包含
     */
    RMVL_W bool contains(std::string_view key) const noexcept;

    //! 清空状态类型
    RMVL_W void clear() noexcept;

    //! 状态类型是否为空
    RMVL_W bool empty() const noexcept;

    /**
     * @brief 获取状态
     *
     * @param[in] key 状态类型名称
     * @return 状态
     */
    const StateType &at(std::string_view key) const;

    /**
     * @brief 获取数值状态，若状态类型不是数值类型，则抛出 `std::bad_variant_access` 异常
     *
     * @param[in] key 状态类型名称
     * @return 状态
     */
    RMVL_W double at_numeric(std::string_view key) const;

    /**
     * @brief 获取字符串状态，若状态类型不是字符串类型，则抛出 `std::bad_variant_access` 异常
     *
     * @param[in] key 状态类型名称
     * @return 状态
     */
    RMVL_W const std::string &at_string(std::string_view key) const;

    /**
     * @brief 设置状态
     *
     * @param[in] key 状态类型名称
     * @return 状态
     */
    StateType &at(std::string_view key);

    /**
     * @brief 访问状态
     *
     * @param[in] key 状态类型名称
     * @return 状态
     */
    StateType &operator[](std::string_view key) noexcept;

    RMVL_W_SUBST("At")

private:
    std::unordered_map<std::string, StateType> _states; //!< 状态散列表
};

//! @} types

} // namespace rm

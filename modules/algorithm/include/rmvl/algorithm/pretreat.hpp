/**
 * @file pretreat.hpp
 * @author zhaoxi (535394140@qq.com)
 * @brief 图像预处理模块
 * @version 1.0
 * @date 2024-06-05
 *
 * @copyright Copyright 2024 (c), zhaoxi
 *
 */

#pragma once

#include <opencv2/core/mat.hpp>

namespace rm
{

//! @addtogroup algorithm
//! @{
//! @defgroup algorithm_pretreat 图像预处理模块
//! @{
//! @brief 提供了二值化等图像预处理功能
//! @} algorithm_pretreat
//! @} algorithm

//! @addtogroup algorithm_pretreat
//! @{

//! 像素通道枚举
enum PixChannel : uint8_t
{
    BLUE,  //!< 蓝色通道
    GREEN, //!< 绿色通道
    RED,   //!< 红色通道
    AUTO   //!< 自动处理（未定义通道）
};

/**
 * @brief 通道相减二值化
 *
 * @code{.cpp}
 * cv::Mat bin = src[ch1] - src[ch2];
 * @endcode
 *
 * @param[in] src 通道类型为 BGR 的原图像
 * @param[in] ch1 通道1
 * @param[in] ch2 通道2
 * @param[in] threshold 相减阈值，像素通道相减的值若小于该阈值则置 `0`，大于则置 `255`
 * @return 二值图像
 */
cv::Mat binary(cv::Mat src, PixChannel ch1, PixChannel ch2, uint8_t threshold);

/**
 * @brief 亮度阈值二值化
 *
 * @param[in] src 通道类型为 BGR 或 Mono8 的原图像
 * @param[in] threshold 亮度阈值，像素亮度小于该阈值则置 `0`，大于则置 `255`
 * @return 二值图像
 */
cv::Mat binary(cv::Mat src, uint8_t threshold);

//! @} algorithm_pretreat

} // namespace rm

/**
 * @file pretreat.h
 * @author RoboMaster Vision Community
 * @brief Header of the image pretreating module
 * @version 1.0
 * @date 2022-11-23
 *
 * @copyright Copyright 2023 (c), RoboMaster Vision Community
 *
 */

#pragma once

#include <opencv2/core/mat.hpp>

namespace rm
{

//! @addtogroup imgproc
//! @{

//! 像素通道枚举
enum PixChannel : uint8_t
{
    BLUE = 0U,  //!< 蓝色通道
    GREEN = 1U, //!< 绿色通道
    RED = 2U    //!< 红色通道
};

/**
 * @brief 通道相减二值化
 *
 * @code {.cpp}
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

//! @} imgproc

} // namespace rm

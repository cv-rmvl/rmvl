/**
 * @file cv.hpp
 * @author zhaoxi (535394140@qq.com)
 * @brief OpenCV 消息转换定义
 * @version 1.0
 * @date 2026-01-28
 *
 * @copyright Copyright 2026 (c), zhaoxi
 *
 */

#pragma once

#include "rmvl/rmvl_modules.hpp"

#ifdef HAVE_OPENCV

#include <opencv2/core/mat.hpp>

#include "rmvlmsg/sensor/image.hpp"

namespace rm {

//! @addtogroup lpss
//! @{

/**
 * @brief 从 Image 消息转换为 cv::Mat
 * 
 * @param[in] img_msg Image 图像消息
 * @return OpenCV 图像矩阵
 */
cv::Mat from_msg(const msg::Image &img_msg);

/**
 * @brief 从 cv::Mat 转换为 Image 消息
 * 
 * @param[in] img OpenCV 图像矩阵
 * @param[in] encoding 图像编码格式，包括 "rgb8", "bgr8", "mono8", "mono16", "rgba8",
 * "bgra8", "bayer_rggb8" 和 "bayer_bggr8"
 * @return Image 图像消息
 */
msg::Image to_msg(cv::Mat img, std::string_view encoding);

//! @} lpss

} // namespace rm

#endif
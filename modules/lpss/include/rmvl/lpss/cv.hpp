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

#include <opencv2/core/affine.hpp>

#define LPSS_CV_VERSION CV_VERSION_MAJOR * 10000 + CV_VERSION_MINOR * 100 + CV_VERSION_REVISION
#if LPSS_CV_VERSION >= 40501
#include <opencv2/core/quaternion.hpp>
#endif

#include "rmvlmsg/geometry/point.hpp"
#include "rmvlmsg/geometry/point32.hpp"
#include "rmvlmsg/geometry/transform.hpp"
#include "rmvlmsg/sensor/image.hpp"

namespace rm {

//! @addtogroup lpss
//! @{

//! OpenCV 与 LPSS 消息转换命名空间
namespace cvmsg {

/**
 * @brief 从 Point 消息转换为 cv::Point3d
 *
 * @param[in] pt_msg Point 消息对象
 * @return cv::Point3d 表示的三维点对象
 */
cv::Point3d from_msg(const msg::Point &pt_msg);

/**
 * @brief 从 cv::Point3d 转换为 Point 消息
 *
 * @param[in] pt OpenCV 三维点对象
 * @return Point 消息对象
 */
msg::Point to_msg(const cv::Point3d &pt);

/**
 * @brief 从 Point32 消息转换为 cv::Point3f
 *
 * @param[in] pt_msg Point32 消息对象
 * @return cv::Point3f 表示的三维点对象
 */
cv::Point3f from_msg(const msg::Point32 &pt_msg);

/**
 * @brief 从 cv::Point3f 转换为 Point32 消息
 *
 * @param[in] pt OpenCV 三维点对象
 * @return Point32 消息对象
 */
msg::Point32 to_msg(const cv::Point3f &pt);

/**
 * @brief 从 Vector3 消息转换为 cv::Vec3d
 *
 * @param[in] vec_msg Vector3 消息对象
 * @return cv::Vec3d 表示的三维向量对象
 */
cv::Vec3d from_msg(const msg::Vector3 &vec_msg);

/**
 * @brief 从 cv::Vec3d 转换为 Vector3 消息
 *
 * @param[in] vec OpenCV 三维向量对象
 * @return Vector3 消息对象
 */
msg::Vector3 to_msg(const cv::Vec3d &vec);

#if LPSS_CV_VERSION >= 40501

/**
 * @brief 从 Quaternion 消息转换为 cv::Quatd
 *
 * @param[in] quat_msg Quaternion 消息对象
 * @return cv::Quatd 表示的四元数对象
 */
cv::Quatd from_msg(const msg::Quaternion &quat_msg);

/**
 * @brief 从 cv::Quatd 转换为 Quaternion 消息
 *
 * @param[in] quat OpenCV 四元数对象
 * @return Quaternion 消息对象
 */
msg::Quaternion to_msg(const cv::Quatd &quat);

/**
 * @brief 从 Transform 消息转换为 cv::Affine3d
 *
 * @param[in] tf_msg Transform 消息对象
 * @return cv::Affine3d 表示的仿射变换对象
 */
cv::Affine3d from_msg(const msg::Transform &tf_msg);

/**
 * @brief 从 cv::Affine3d 转换为 Transform 消息
 *
 * @param[in] tf OpenCV 仿射变换对象
 * @return Transform 消息对象
 */
msg::Transform to_msg(const cv::Affine3d &tf);

#endif

/**
 * @brief 从 Image 消息转换为 cv::Mat
 *
 * @param[in] img_msg Image 图像消息
 * @return cv::Mat 表示的图像矩阵
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

} // namespace cvmsg

//! @} lpss

} // namespace rm

#endif
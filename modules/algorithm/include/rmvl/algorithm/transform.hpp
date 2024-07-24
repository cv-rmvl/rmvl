/**
 * @file transform.hpp
 * @author RoboMaster Vision Community
 * @brief 坐标变换
 * @version 1.0
 * @date 2023-01-12
 *
 * @copyright Copyright 2023 (c), RoboMaster Vision Community
 *
 */

#pragma once

#ifdef HAVE_OPENCV

#include <opencv2/core/types.hpp>

#include "math.hpp"

namespace rm
{

//! @addtogroup algorithm
//! @{

//! 欧拉角转轴枚举
enum EulerAxis : int
{
    X = 0, //!< X 轴
    Y = 1, //!< Y 轴
    Z = 2  //!< Z 轴
};

/**
 * @brief 计算相机中心与目标中心之间的相对角度
 * @note 公式推导参考函数 @ref calculateRelativeCenter() ，现直接给出结果 \f[\begin{bmatrix}
 *       \tan{yaw}\\\tan{pitch}\\1\end{bmatrix}=\begin{bmatrix}f_x&0&u_0\\0&f_y&v_0\\0&0&1
 *       \end{bmatrix}^{-1}\begin{bmatrix}u\\v\\1\end{bmatrix}\f]
 *
 * @param[in] cameraMatrix 相机内参矩阵
 * @param[in] center 像素坐标系下的目标中心
 * @return 相对角度，目标在图像右方，point.x 为正，目标在图像下方，point.y 为正
 */
cv::Point2f calculateRelativeAngle(const cv::Matx33f &cameraMatrix, cv::Point2f center);

/**
 * @brief 计算目标中心在像素坐标系下的坐标
 * @note 由针孔相机模型中的相似三角形关系推出下列公式，其中 \f$(u, v)\f$ 为像素坐标系下的坐标，
 *       \f$(X, Y, Z)\f$ 为相机坐标系下的坐标\f[u=f_x\frac XZ+u_0=f_x\tan{yaw}+u_0\f] \f[v=f_y
 *       \frac YZ+v_0=f_y\tan{pitch}+v_0\f] 写成矩阵相乘的方式：\f[\begin{bmatrix}u\\v\\1
 *       \end{bmatrix}=\frac1Z\begin{bmatrix}f_x&0&u_0\\0&f_y&v_0\\0&0&1\end{bmatrix}
 *       \begin{bmatrix}X\\Y\\Z\end{bmatrix}=\begin{bmatrix}f_x&0&u_0\\0&f_y&v_0\\0&0&1
 *       \end{bmatrix}\begin{bmatrix}\tan{yaw}\\\tan{pitch}\\1\end{bmatrix}\f]
 *
 * @param[in] cameraMatrix 相机内参矩阵
 * @param[in] angle 目标中心与相机中心的相对角度
 * @return 目标中心在像素坐标系下的坐标
 */
cv::Point2f calculateRelativeCenter(const cv::Matx33f &cameraMatrix, cv::Point2f angle);

/**
 * @brief 计算 3D 目标点在像素坐标系下的坐标
 *
 * @param[in] cameraMatrix 相机内参矩阵
 * @param[in] distCoeffs 相机畸变参数
 * @param[in] center3d 目标点在相机坐标系下的坐标
 * @return 目标点在像素坐标系下的坐标
 */
cv::Vec2f cameraConvertToPixel(const cv::Matx33f &cameraMatrix, const cv::Matx51f &distCoeffs, const cv::Vec3f &center3d);

/**
 * @brief 计算 3D 目标点在像素坐标系下的坐标
 *
 * @param[in] cameraMatrix 相机内参矩阵
 * @param[in] distCoeffs 相机畸变参数
 * @param[in] center3d 目标点在相机坐标系下的坐标
 * @return 目标点在像素坐标系下的坐标
 */
inline cv::Point2f cameraConvertToPixel(const cv::Matx33f &cameraMatrix, const cv::Matx51f &distCoeffs, const cv::Point3f &center3d)
{
    return cameraConvertToPixel(cameraMatrix, distCoeffs, cv::Vec3f(center3d));
}

/**
 * @brief 欧拉角转换为旋转矩阵
 *
 * @tparam Tp 数据类型
 * @param[in] val 角度数值（弧度制）
 * @param[in] axis 转轴
 * @return Matx 格式的旋转矩阵
 */
template <typename Tp>
inline cv::Matx<Tp, 3, 3> euler2Mat(Tp val, EulerAxis axis)
{
    Tp s = std::sin(val), c = std::cos(val);
    switch (axis)
    {
    case X:
        return {1, 0, 0, 0, c, -s, 0, s, c};
    case Y:
        return {c, 0, s, 0, 1, 0, -s, 0, c};
    case Z:
        return {c, -s, 0, s, c, 0, 0, 0, 1};
    default:
        RMVL_Error_(RMVL_StsBadArg, "Bad argument of the \"axis\": %d", static_cast<int>(axis));
        return cv::Matx<Tp, 3, 3>::eye();
    }
}

//! @} algorithm

} // namespace rm

#endif // HAVE_OPENCV
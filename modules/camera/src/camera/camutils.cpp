/**
 * @file camutils.cpp
 * @author zhaoxi (535394140@qq.com)
 * @brief
 * @version 1.0
 * @date 2023-12-13
 *
 * @copyright Copyright 2023 (c), zhaoxi
 *
 */

#include <Eigen/Dense>

#include <opencv2/calib3d.hpp>
#include <opencv2/core/eigen.hpp>

#include "rmvl/camera/camutils.hpp"
#include "rmvl/algorithm/math.hpp"
#include "rmvlpara/camera/camera.h"

void rm::CameraExtrinsics::tvec(const cv::Vec3f &tvec)
{
    _tvec = tvec;
    // 同步更新T
    for (int i = 0; i < 3; ++i)
        _t(i, 3) = tvec(i);
    // 同步更新距离
    _distance = sqrt((tvec.t() * tvec)(0));
}

void rm::CameraExtrinsics::rvec(const cv::Vec3f &rvec)
{
    _rvec = rvec;
    // 同步更新R
    cv::Rodrigues(_rvec, _r);
    // 同步更新T
    for (int i = 0; i < 3; ++i)
        for (int j = 0; j < 3; ++j)
            _t(i, j) = _r(i, j);
    // 同步更新欧拉角
    Eigen::Matrix3f rotated_matrix; // 旋转矩阵
    // 类型转换
    cv::cv2eigen(_r, rotated_matrix);
    // 获取欧拉角
    auto euler_angles = rotated_matrix.eulerAngles(para::camera_param.EULER_0, para::camera_param.EULER_1, para::camera_param.EULER_2);
    _roll = rad2deg(euler_angles[2]);
    _pitch = rad2deg(euler_angles[1]);
    _yaw = rad2deg(euler_angles[0]);
}

void rm::CameraExtrinsics::R(const cv::Matx33f &R)
{
    _r = R;
    // 同步更新旋转向量
    cv::Rodrigues(_r, _rvec);
    for (int i = 0; i < 3; ++i)
        for (int j = 0; j < 3; ++j)
            _t(i, j) = _r(i, j);
    Eigen::Matrix3f rotated_matrix;
    // 类型转换
    cv::cv2eigen(_r, rotated_matrix);
    auto euler_angles = rotated_matrix.eulerAngles(para::camera_param.EULER_0, para::camera_param.EULER_1, para::camera_param.EULER_2);
    _roll = rad2deg(euler_angles[2]);
    _pitch = rad2deg(euler_angles[1]);
    _yaw = rad2deg(euler_angles[0]);
}

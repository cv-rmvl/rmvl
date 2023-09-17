/**
 * @file parameters.hpp
 * @author RoboMaster Vision Community
 * @brief 相机内外参数定义
 * @version 0.1
 * @date 2023-01-12
 *
 * @copyright Copyright 2023 (c), RoboMaster Vision Community
 *
 */

#pragma once

#include <Eigen/Dense>

#include <opencv2/calib3d.hpp>
#include <opencv2/core/eigen.hpp>

#include "rmvl/rmath/uty_math.hpp"

#include "rmvlpara/camera.hpp"

namespace rm
{

//! @addtogroup rmath
//! @{

//! 相机外参
template <typename Tp = float>
class CameraExtrinsics
{
    Tp _yaw = 0;
    Tp _pitch = 0;
    Tp _roll = 0;
    Tp _distance = 0;
    cv::Vec<Tp, 3> _tvec;
    cv::Vec<Tp, 3> _rvec;
    cv::Matx<Tp, 3, 3> _r = cv::Matx<Tp, 3, 3>::eye();
    cv::Matx<Tp, 4, 4> _t = cv::Matx<Tp, 4, 4>::eye();

public:
    //! 获取平移向量
    inline cv::Vec<Tp, 3> tvec() const { return _tvec; }
    //! 获取旋转向量
    inline cv::Vec<Tp, 3> rvec() const { return _rvec; }
    //! 获取旋转矩阵
    inline cv::Matx<Tp, 3, 3> R() const { return _r; }
    //! 获取外参矩阵
    inline cv::Matx<Tp, 4, 4> T() const { return _t; }
    //! 获取yaw
    inline Tp yaw() const { return _yaw; }
    //! 获取pitch
    inline Tp pitch() const { return _pitch; }
    //! 获取roll
    inline Tp roll() const { return _roll; }
    //! 获取距离
    inline Tp distance() const { return _distance; }

    //! 设置平移向量
    inline void tvec(const cv::Vec<Tp, 3> &tvec)
    {
        _tvec = tvec;
        // 同步更新T
        for (int i = 0; i < 3; ++i)
            _t(i, 3) = tvec(i);
        // 同步更新距离
        _distance = sqrt((tvec.t() * tvec)(0));
    }
    //! 设置旋转向量
    inline void rvec(const cv::Vec<Tp, 3> &rvec)
    {
        _rvec = rvec;
        // 同步更新R
        Rodrigues(_rvec, _r);
        updateEulerT();
    }
    //! 设置旋转矩阵
    inline void R(const cv::Matx<Tp, 3, 3> &R)
    {
        _r = R;
        // 同步更新旋转向量
        Rodrigues(_r, _rvec);
        updateEulerT();
    }

    /**
     * @brief 设置距离
     *
     * @param distance 距离
     */
    inline void distance(const Tp &distance) { _distance = distance; }

private:
    inline void updateEulerT()
    {
        // 同步更新T
        for (int i = 0; i < 3; ++i)
            for (int j = 0; j < 3; ++j)
                _t(i, j) = _r(i, j);
        // 同步更新欧拉角
        Eigen::Matrix<Tp, 3, 3> rotated_matrix; // 旋转矩阵
        // 类型转换
        cv2eigen(_r, rotated_matrix);
        // 获取欧拉角
        Eigen::Matrix<Tp, 3, 1> euler_angles = rotated_matrix.eulerAngles(para::camera_param.EULER_0,
                                                                          para::camera_param.EULER_1,
                                                                          para::camera_param.EULER_2);
        _roll = rad2deg(euler_angles[2]);
        _pitch = rad2deg(euler_angles[1]);
        _yaw = rad2deg(euler_angles[0]);
    }
};

//! @}

} // namespace rm

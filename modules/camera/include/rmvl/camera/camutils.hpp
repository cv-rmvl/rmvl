/**
 * @file camutils.hpp
 * @author RoboMaster Vision Community
 * @brief
 * @version 1.0
 * @date 2022-09-30
 *
 * @copyright Copyright 2023 (c), RoboMaster Vision Community
 *
 */

#pragma once

#include <Eigen/Dense>

#include <opencv2/calib3d.hpp>
#include <opencv2/core/eigen.hpp>

#include "rmvl/rmath/uty_math.hpp"
#include "rmvlpara/camera/camera.h"

namespace rm
{

//! @addtogroup camera
//! @{

//! 采集模式 Camera grab mode
enum GrabMode : int
{
    GRAB_CONTINUOUS = 0, //!< 连续采样 Continuous grabbing
    GRAB_SOFTWARE = 1,   //!< 软触发 Grab by software trigger
    GRAB_HARDWARE = 2    //!< 硬触发 Grab by hardware trigger
};

//! 处理模式 Camera retrieve mode
using RetrieveMode = int;
enum _RetrieveMode : RetrieveMode
{
    RETRIEVE_SDK = 0x01, //!< 使用官方 SDK 进行处理 Retrieve using official SDK
    RETRIEVE_CV = 0x02,  //!< 使用 OpenCV 的 'cvtColor' 进行处理 Retrieve using cvtColor function in OpenCV
    RETRIEVE_LUT = 0x10  //!< 使用 LUT 进行处理，需配合 RETRIEVE_CV 进行使用 Retrieve using cvtColor function with Look - Up Table
};

//! 相机事件 Camera Activities
enum CameraActivities : uint16_t
{
    CAMERA_ONCE_WB = 0x0101,     //!< 执行一次白平衡 Preform once white balance
    CAMERA_SOFT_TRIGGER = 0x0102 //!< 执行软触发 Preform a software trigger
};

//! 相机属性 Camera Properties
enum CameraProperties : uint16_t
{
    // ------------- 设备属性 Devise Properties -------------
    CAMERA_AUTO_EXPOSURE = 0x001,   //!< 自动曝光 Automatic exposure
    CAMERA_AUTO_WB = 0x002,         //!< 自动白平衡 Automatic white balance
    CAMERA_MANUAL_EXPOSURE = 0x003, //!< 手动曝光 Manual exposure
    CAMERA_MANUAL_WB = 0x004,       //!< 手动白平衡 Manual white balance
    CAMERA_EXPOSURE = 0x005,        //!< 曝光值 Expusure
    CAMERA_GAIN = 0x006,            //!< 模拟增益 Analog gain
    CAMERA_GAMMA = 0x007,           //!< Gamma 值 Gamma
    CAMERA_WB_RGAIN = 0x008,        //!< 白平衡红色分量 Red channel gain of white balance
    CAMERA_WB_GGAIN = 0x009,        //!< 白平衡绿色分量 Green channel gain of white balance
    CAMERA_WB_BGAIN = 0x00a,        //!< 白平衡蓝色分量 Blue channel gain of white balance
    CAMERA_CONTRAST = 0x00c,        //!< 对比度 Contrast ratio
    CAMERA_SATURATION = 0x00d,      //!< 饱和度 Saturation ratio
    CAMERA_SHARPNESS = 0x00e,       //!< 锐度 Sharpness ratio
    CAMERA_FRAME_HEIGHT = 0x00f,    //!< 图像帧高度 Frame height
    CAMERA_FRAME_WIDTH = 0x010,     //!< 图像帧宽度 Frame width

    // ------------ 处理属性 Process Properties -------------
    CAMERA_TRIGGER_DELAY = 0x041,  //!< 硬触发采集延迟（微秒）Hard trigger collection delay (μs)
    CAMERA_TRIGGER_COUNT = 0x042,  //!< 单次触发时的触发帧数 The number of frames in a single trigger
    CAMERA_TRIGGER_PERIOD = 0x043, //!< 单次触发时多次采集的周期（微秒）The period of multiple collection for a single trigger (μs)
};

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
    inline const cv::Vec<Tp, 3> &tvec() const { return _tvec; }
    //! 获取旋转向量
    inline const cv::Vec<Tp, 3> &rvec() const { return _rvec; }
    //! 获取旋转矩阵
    inline const cv::Matx<Tp, 3, 3> &R() const { return _r; }
    //! 获取外参矩阵
    inline const cv::Matx<Tp, 4, 4> &T() const { return _t; }
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

//! @} camera

} // namespace rm

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

//! @warning 此处若仅包含 <opencv2/core/mat.hpp> 会链接错误，原因暂时不清楚
#include <opencv2/core.hpp>

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
class CameraExtrinsics
{
    float _yaw{};
    float _pitch{};
    float _roll{};
    float _distance{};
    cv::Vec3f _tvec;
    cv::Vec3f _rvec;
    cv::Matx33f _r = cv::Matx33f::eye();
    cv::Matx44f _t = cv::Matx44f::eye();

public:
    //! 获取平移向量
    inline const cv::Vec3f &tvec() const { return _tvec; }
    //! 获取旋转向量
    inline const cv::Vec3f &rvec() const { return _rvec; }
    //! 获取旋转矩阵
    inline const cv::Matx33f &R() const { return _r; }
    //! 获取外参矩阵
    inline const cv::Matx44f &T() const { return _t; }
    //! 获取yaw
    inline float yaw() const { return _yaw; }
    //! 获取pitch
    inline float pitch() const { return _pitch; }
    //! 获取roll
    inline float roll() const { return _roll; }
    //! 获取距离
    inline float distance() const { return _distance; }

    /**
     * @brief 设置平移向量
     * 
     * @param[in] tvec 平移向量
     */
    void tvec(const cv::Vec3f &tvec);

    /**
     * @brief 设置旋转向量
     * 
     * @param[in] rvec 旋转向量
     */
    void rvec(const cv::Vec3f &rvec);

    /**
     * @brief 设置旋转矩阵
     * 
     * @param[in] R 旋转矩阵
     */
    void R(const cv::Matx33f &R);

    /**
     * @brief 设置距离
     *
     * @param[in] distance 距离
     */
    inline void distance(float distance) { _distance = distance; }
};

//! @} camera

} // namespace rm

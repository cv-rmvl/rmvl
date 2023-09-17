/**
 * @file definitions.h
 * @author RoboMaster Vision Community
 * @brief
 * @version 1.0
 * @date 2022-09-30
 *
 * @copyright Copyright 2023 (c), RoboMaster Vision Community
 *
 */

#pragma once

#include <memory>

#include <opencv2/videoio.hpp>

namespace rm
{

//! @addtogroup camera
//! @{

using capture_ptr = std::unique_ptr<cv::VideoCapture>;

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

//! RoboMaster Video Capture Activities
enum RMVideoActivities : uint16_t
{
    CAP_ACT_RM_ONCE_WB = 0x0101,     //!< 执行一次白平衡 Preform once white balance
    CAP_ACT_RM_SOFT_TRIGGER = 0x0102 //!< 执行软触发 Preform a software trigger
};

//! RoboMaster Video Capture Properties
enum RMVideoProperties : uint16_t
{
    // ------------- Devise Properties -------------
    CAP_PROP_RM_AUTO_EXPOSURE = 0x001,   //!< 自动曝光 Automatic exposure
    CAP_PROP_RM_AUTO_WB = 0x002,         //!< 自动白平衡 Automatic white balance
    CAP_PROP_RM_MANUAL_EXPOSURE = 0x003, //!< 手动曝光 Manual exposure
    CAP_PROP_RM_MANUAL_WB = 0x004,       //!< 手动白平衡 Manual white balance
    CAP_PROP_RM_EXPOSURE = 0x005,        //!< 曝光值 Expusure
    CAP_PROP_RM_GAIN = 0x006,            //!< 模拟增益 Analog gain
    CAP_PROP_RM_GAMMA = 0x007,           //!< Gamma 值 Gamma
    CAP_PROP_RM_WB_RGAIN = 0x008,        //!< 白平衡红色分量 Red channel gain of white balance
    CAP_PROP_RM_WB_GGAIN = 0x009,        //!< 白平衡绿色分量 Green channel gain of white balance
    CAP_PROP_RM_WB_BGAIN = 0x00a,        //!< 白平衡蓝色分量 Blue channel gain of white balance
    CAP_PROP_RM_CONTRAST = 0x00c,        //!< 对比度 Contrast ratio
    CAP_PROP_RM_SATURATION = 0x00d,      //!< 饱和度 Saturation ratio
    CAP_PROP_RM_SHARPNESS = 0x00e,       //!< 锐度 Sharpness ratio
    CAP_PROP_RM_FRAME_HEIGHT = 0x00f,    //!< 图像帧高度 Frame height
    CAP_PROP_RM_FRAME_WIDTH = 0x010,     //!< 图像帧宽度 Frame width

    // ------------ Process Properties -------------
    CAP_PROP_RM_TRIGGER_DELAY = 0x041,  //!< 硬触发采集延迟（微秒）Hard trigger collection delay (μs)
    CAP_PROP_RM_TRIGGER_COUNT = 0x042,  //!< 单次触发时的触发帧数 The number of frames in a single trigger
    CAP_PROP_RM_TRIGGER_PERIOD = 0x043, //!< 单次触发时多次采集的周期（微秒）The period of multiple collection for a single trigger (μs)
};

//! @} camera

} // namespace rm

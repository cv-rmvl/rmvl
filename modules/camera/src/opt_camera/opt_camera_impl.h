/**
 * @file opt_camera_impl.h
 * @author zhaoxi (535394140@qq.com)
 * @brief 奥普特机器视觉相机库实现
 * @version 1.0
 * @date 2023-12-15
 *
 * @copyright Copyright 2023 (c), zhaoxi
 *
 */

#pragma once

#include <OPTApi.h>
#include <OPTDefines.h>

#include "rmvl/camera/opt_camera.h"

//! Redefines the mode of creating the camera handle
using CameraMode = OPT_ECreateHandleMode;

#define HANDLE_INDEX modeByIndex     //!< Index (0, 1, 2 ...)
#define HANDLE_KEY modeByCameraKey   //!< Manufacture: S/N
#define HANDLE_ID modeByDeviceUserID //!< Manual ID
#define HANDLE_IP modeByIPAddress    //!< IP Address

namespace rm
{

class OptCamera::Impl
{
    // 设备信息
    CameraMode _camera_mode;     //!< 相机句柄创建方式
    std::string _camera_info;    //!< 相机句柄的字符串信息
    OPT_HANDLE _hCamera;         //!< 相机设备句柄
    OPT_DeviceList _device_list; //!< 设备列表
    uint _buffer_size{10};       //!< 相机帧缓存数量
    bool _is_opened{};           //!< 相机是否打开

    // 图像数据
    OPT_Frame _src_frame; //!< SDK 直接得到的 Frame 类型指针

    // 图像信息
    double _retrieve_mode{RETRIEVE_SDK}; //!< 检索模式
    double _exposure_time{-1};           //!< 曝光时间 ms
    double _gain{-1};                    //!< 全通道增益
    double _r_gain{-1};                  //!< 红色增益
    double _g_gain{-1};                  //!< 绿色增益
    double _b_gain{-1};                  //!< 蓝色增益
    double _gamma{-1};                   //!< Gamma
    double _saturation{-1};              //!< 饱和度
};

} // namespace rm

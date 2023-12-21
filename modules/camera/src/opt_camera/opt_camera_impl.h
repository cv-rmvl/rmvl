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

namespace rm
{

class OptCamera::Impl
{
    // 设备信息
    CameraConfig _init_mode;       //!< 相机初始化配置模式
    std::string _camera_info;      //!< 相机句柄的字符串信息
    OPT_HANDLE _handle{};          //!< 相机设备句柄
    OPT_DeviceList _device_list{}; //!< 设备列表
    uint _buffer_size{10};         //!< 相机帧缓存数量
    bool _is_opened{};             //!< 相机是否打开

    // 图像数据
    OPT_Frame _src_frame; //!< SDK 直接得到的 Frame 类型指针

public:
    Impl(CameraConfig init_mode, std::string_view handle_info) noexcept;
    ~Impl() noexcept;

    //! 打开相机
    bool open() noexcept;
    //! 相机重连
    bool reconnect() noexcept;
    //! 设置相机参数
    bool set(int propId, double value) noexcept;
    //! 获取相机参数
    double get(int propId) const noexcept;
    //! 相机处理
    bool retrieve(cv::OutputArray image) noexcept;
    //! 读取图片
    bool read(cv::OutputArray image) noexcept;
    //! 相机设备是否打开
    inline bool isOpened() const noexcept { return _is_opened; }
    //! 释放资源
    void release() noexcept;
};

} // namespace rm

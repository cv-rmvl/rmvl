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

#include "rmvl/camera/camutils.hpp"
#include "rmvl/camera/opt_camera.h"

namespace rm {

class OptCamera::Impl {
public:
    Impl(CameraConfig init_mode, std::string_view handle_info) noexcept;
    ~Impl() noexcept;

    //! 加载相机参数
    void load(const para::OptCameraParam &param);
    //! 打开相机
    bool open() noexcept;
    //! 相机重连
    bool reconnect() noexcept;
    //! 设置相机参数
    template <typename Tp, typename Enable = std::enable_if_t<std::is_same_v<Tp, bool> || std::is_same_v<Tp, int64_t> || std::is_same_v<Tp, double>>>
    bool set(CameraProperties prop_id, Tp value) noexcept;
    //! 获取相机参数
    double get(CameraProperties prop_id) const noexcept;
    //! 触发相机事件
    bool trigger(CameraEvents event_id) const noexcept;
    //! 相机处理
    bool retrieve(cv::OutputArray image) noexcept;
    //! 读取图片
    bool read(cv::OutputArray image) noexcept;
    //! 相机设备是否打开
    inline bool isOpened() const noexcept { return _is_opened; }
    //! 释放资源
    void release() noexcept;

private:
    // 设备信息
    CameraConfig _init_mode;       //!< 相机初始化配置模式
    std::string _camera_info;      //!< 相机句柄的字符串信息
    OPT_HANDLE _handle{};          //!< 相机设备句柄
    OPT_DeviceList _device_list{}; //!< 设备列表
    uint32_t _buffer_size{10};     //!< 相机帧缓存数量
    bool _is_opened{};             //!< 相机是否打开

    // 图像数据
    OPT_Frame _src_frame; //!< SDK 直接得到的 Frame 类型指针
};

} // namespace rm

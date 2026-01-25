/**
 * @file mv_camera_impl.h
 * @author zhaoxi (535394140@qq.com)
 * @brief 工业相机实现
 * @version 1.0
 * @date 2023-12-13
 *
 * @copyright Copyright 2023 (c), zhaoxi
 *
 */

#pragma once

#ifdef _WIN32
#include <Windows.h>
#endif // _WIN32
#include <CameraApi.h>

#include "rmvl/camera/mv_camera.h"

namespace rm {

class MvCamera::Impl {
    // ------------------------- 相机信息 -------------------------
    bool _is_opened = false;         //!< 相机是否打开
    std::string _camera_id;          //!< 指定相机的串口号
    CameraHandle _handle;            //!< 相机设备句柄
    INT _camera_counts = 8;          //!< 相机设备数量
    tSdkCameraDevInfo *_camera_list; //!< 相机设备信息列表
    CameraSdkStatus _status;         //!< 相机状态

    // ------------------------- 处理信息 -------------------------
    BYTE *_pbyBuffer = nullptr;  //!< 缓冲区指针
    BYTE *_pbyOut = nullptr;     //!< 处理后的缓冲区指针
    GrabMode _grab_mode;         //!< 图像采集模式
    RetrieveMode _retrieve_mode; //!< 图像处理模式

    // ------------------------- 图像信息 -------------------------
    tSdkFrameHead _frame_info; //!< 图像帧头信息
    int _channel = 3;          //!< 通道数

public:
    Impl(CameraConfig init_mode, std::string_view serial) noexcept;

    ~Impl() noexcept;

    //! 加载相机参数
    void load(const para::MvCameraParam &param);

    //! 设置相机参数
    template <typename Tp, typename Enable = std::enable_if_t<std::is_same_v<Tp, bool> || std::is_same_v<Tp, int> || std::is_same_v<Tp, double>>>
    bool set(CameraProperties propId, Tp value) noexcept;

    //! 获取相机参数
    double get(CameraProperties propId) const noexcept;

    //! 触发相机事件
    bool trigger(CameraEvents eventId) const noexcept;

    //! 相机是否打开
    inline bool isOpened() const noexcept { return _is_opened; }

    //! 相机采集
    inline bool grab() noexcept { return CameraGetImageBuffer(_handle, &_frame_info, &_pbyBuffer, 1000) == CAMERA_STATUS_SUCCESS; }

    //! 相机处理
    bool retrieve(cv::OutputArray image) noexcept;

    //! 从相机设备中读取图像
    inline bool read(cv::OutputArray image) noexcept {
        if (grab())
            retrieve(image);
        else
            reconnect();
        return !image.empty();
    }

    //! 打开相机
    bool open() noexcept;

    //! 相机重连
    bool reconnect() noexcept;

private:
    //! 释放相机资源
    inline void release() noexcept { CameraUnInit(_handle); }
};

} // namespace rm

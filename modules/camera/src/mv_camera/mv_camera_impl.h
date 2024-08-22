/**
 * @file mv_camera_MvCameraImpl.h
 * @author zhaoxi (535394140@qq.com)
 * @brief 工业相机实现
 * @version 1.0
 * @date 2023-12-13
 *
 * @copyright Copyright 2023 (c), zhaoxi
 *
 */

#pragma once

#include <CameraApi.h>

#include "rmvl/camera/mv_camera.h"

namespace rm
{

class MvCamera::Impl
{
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
    tSdkFrameHead _frame_info;   //!< 图像帧头信息
    int _channel = 3;            //!< 通道数
    bool _auto_exposure = false; //!< 相机自动曝光
    double _exposure = 1200;     //!< 相机设备曝光时间
    int _gain = 1;               //!< 全通道增益
    int _r_gain = 100;           //!< 图像红色通道增益
    int _g_gain = 100;           //!< 图像绿色通道增益
    int _b_gain = 100;           //!< 图像蓝色通道增益
    int _gamma = 100;            //!< 图像 Gamma 值
    int _contrast = 100;         //!< 图像对比度
    int _saturation = 100;       //!< 图像饱和度
    int _sharpness = 0;          //!< 图像锐度

public:
    Impl(CameraConfig init_mode, std::string_view serial) noexcept;

    ~Impl() noexcept;

    //! 设置相机参数
    bool set(int propId, double value) noexcept;

    //! 获取相机参数
    double get(int propId) const noexcept;

    //! 相机是否打开
    inline bool isOpened() const noexcept { return _is_opened; }

    //! 相机采集
    inline bool grab() noexcept { return CameraGetImageBuffer(_handle, &_frame_info, &_pbyBuffer, 1000) == CAMERA_STATUS_SUCCESS; }

    //! 相机处理
    bool retrieve(cv::OutputArray image) noexcept;

    //! 从相机设备中读取图像
    inline bool read(cv::Mat image) noexcept
    {
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

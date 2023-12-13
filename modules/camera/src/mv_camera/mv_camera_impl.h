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

#include "rmvl/camera/mv_camera.h"

namespace rm
{

class MvCamera::Impl
{
    // ------------------------- 相机信息 -------------------------
    bool _is_opened = false;         //!< 相机是否打开
    std::string _camera_id;          //!< 指定相机的串口号
    CameraHandle _hCamera;           //!< 相机设备句柄
    INT _camera_counts = 8;          //!< 相机设备数量
    tSdkCameraDevInfo *_camera_list; //!< 相机设备信息列表
    CameraSdkStatus _status;         //!< 相机状态

    // ------------------------- 处理信息 -------------------------
    BYTE *_pbyBuffer = nullptr;  //!< 缓冲区指针
    BYTE *_pbyOut = nullptr;     //!< 处理后的缓冲区指针
    GrabMode _grab_mode;         //!< 图像采集模式
    RetrieveMode _retrieve_mode; //!< 图像处理模式
    cv::Mat _look_up_table;      //!< 单通道 LUT 表

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
    Impl(GrabMode grab_mode, RetrieveMode retrieve_mode, std::string_view serial,
                 const std::vector<int> &decode_param) noexcept;

    ~Impl() noexcept;

    //! 设置相机参数
    bool set(int propId, double value) noexcept;

    //! 获取相机参数
    double get(int propId) const noexcept;

    //! 相机是否打开 Camera is turned on?
    inline bool isOpened() const noexcept { return _is_opened; }

    /**
     * @brief 相机采集 Camera grabbing
     *
     * @return 是否成功采集 Success to grab?
     */
    inline bool grab() noexcept
    {
        _status = CameraGetImageBuffer(_hCamera, &_frame_info, &_pbyBuffer, 1000);
        return _status == CAMERA_STATUS_SUCCESS;
    }

    /**
     * @brief 相机处理
     *
     * @param image 输出图像
     * @param flag 相机处理模式
     * @return 是否成功处理
     */
    bool retrieve(cv::OutputArray image, RetrieveMode flag) noexcept;

    /**
     * @brief 从相机设备中读取图像
     *
     * @param image 待读入的图像
     * @return 是否读取成功
     */
    inline bool read(cv::OutputArray image) noexcept
    {
        if (grab())
            retrieve(image, _retrieve_mode);
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
    inline void release() noexcept { CameraUnInit(_hCamera); }

    /**
     * @brief LUT 表映射
     *
     * @param lut std::vector 类型的 LUT 映射表
     * @return 是否转换成功
     */
    bool initLUT(const std::vector<int> &lut) noexcept;
};

} // namespace rm

/**
 * @file mv_camera.h
 * @author RoboMaster Vision Community
 * @brief MindVision camera driver header file
 * @version 1.0
 * @date 2018-12-08
 *
 * @copyright Copyright 2023 (c), RoboMaster Vision Community
 *
 */

#pragma once

#include <opencv2/videoio.hpp>

#include <CameraApi.h>

#include "camutils.hpp"

namespace rm
{

//! @addtogroup mv_camera
//! @{

//! @example samples/camera/mv/sample_mv_auto_calib.cpp 迈德威视相机自动标定例程
//! @example samples/camera/mv/sample_mv_manual_calib.cpp 迈德威视相机手动标定例程
//! @example samples/camera/mv/sample_mv_mono.cpp 迈德威视单相机例程
//! @example samples/camera/mv/sample_mv_multi.cpp 迈德威视多相机例程
//! @example samples/camera/mv/sample_mv_writer.cpp 迈德威视相机录屏例程

//! 迈德威视相机库 MindVision camera library
class MvCamera final : public cv::VideoCapture
{
    // -------------------------- 相机信息 Device information -------------------------
    bool _is_opened = false;         //!< 相机是否打开 Whether the camera is opened
    std::string _camera_id;          //!< 指定相机的串口号 Serial number of the specific camera
    CameraHandle _hCamera;           //!< 相机设备句柄 Handle of the camera device
    INT _camera_counts = 8;          //!< 相机设备数量 The number of the camera devices
    tSdkCameraDevInfo *_camera_list; //!< 相机设备信息列表 List of the device information of the camera
    CameraSdkStatus _status;         //!< 相机状态 Camera status

    // ------------------------- 处理信息 Process information -------------------------
    BYTE *_pbyBuffer = nullptr;  //!< 缓冲区指针 Buffer pointer
    BYTE *_pbyOut = nullptr;     //!< 处理后的缓冲区指针 Buffer pointer after process
    GrabMode _grab_mode;         //!< 图像采集模式 Image grab mode
    RetrieveMode _retrieve_mode; //!< 图像处理模式 Image retrieval method
    cv::Mat _look_up_table;      //!< 单通道 LUT 表 Single-channel LUT mapping table

    // -------------------------- 图像信息 Image information --------------------------
    tSdkFrameHead _frame_info;   //!< 图像帧头信息 Image frame header information
    int _channel = 3;            //!< 通道数 The number of channels
    bool _auto_exposure = false; //!< 相机自动曝光 Automatic camera exposure
    double _exposure = 1200;     //!< 相机设备曝光时间 Camera device exposure time
    int _gain = 1;               //!< 全通道增益 Gain of all channels
    int _r_gain = 100;           //!< 图像红色通道增益 Red channel gain of the image
    int _g_gain = 100;           //!< 图像绿色通道增益 Green channel gain of the image
    int _b_gain = 100;           //!< 图像蓝色通道增益 Blue channel gain of the image
    double _gamma = 100;         //!< 图像 Gamma 值 Image Gamma
    double _contrast = 100;      //!< 图像对比度 Image Contrast
    double _saturation = 100;    //!< 图像饱和度 Image Saturation
    double _sharpness = 0;       //!< 图像锐度 Image sharpness

    // 通过使用私有化成员的方式来禁用基类方法
    // Use the privatization statement so that the base class method is not used
    using VideoCapture::open;

public:
    using ptr = std::unique_ptr<MvCamera>;
    using const_ptr = std::unique_ptr<const MvCamera>;

    /**
     * @brief 构造函数 Constructor
     *
     * @param[in] grab_mode 相机采集模式 Camera grab mode
     * @param[in] retrieve_mode 相机处理模式 Camera retrieve mode
     * @param[in] serial 相机唯一序列号 Camera unique serial number
     * @param[in] decode_param 解码/转码参数 Decoding parameters
     */
    MvCamera(GrabMode grab_mode, RetrieveMode retrieve_mode, std::string_view serial = "", const std::vector<int> &decode_param = std::vector<int>());

    MvCamera(const MvCamera &) = delete;
    MvCamera(MvCamera &&) = delete;

    /**
     * @brief 构建 MvCamera 对象 Construct MvCamera object
     *
     * @param[in] grab_mode 相机采集模式 Camera grab mode
     * @param[in] retrieve_mode 相机处理模式 Camera retrieve mode
     * @param[in] serial 相机唯一序列号 Camera unique serial number
     * @param[in] decode_param 解码/转码参数 Decoding parameters
     */
    static inline std::unique_ptr<MvCamera> make_capture(GrabMode grab_mode, RetrieveMode retrieve_mode, std::string_view serial = "", const std::vector<int> &decode_param = std::vector<int>())
    {
        return std::make_unique<MvCamera>(grab_mode, retrieve_mode, serial, decode_param);
    }

    ~MvCamera() override;

    /**
     * @brief 设置相机参数/事件 Set the camera parameter or activity
     *
     * @param propId 参数/事件编号 The ID of the parameter or activity
     * @param value 参数/事件值 The value of the parameter or activity
     * @return 是否设置成功 Set successfully?
     */
    bool set(int propId, double value = 0.0) override;

    /**
     * @brief 获取相机参数 Get the camera parameter
     *
     * @param propId 参数编号 The ID of the parameter
     * @return 参数值 The value of the parameter
     */
    double get(int propId) const override;

    /**
     * @brief 相机是否打开 Camera is turned on?
     */
    inline bool isOpened() const override { return _is_opened; }

    /**
     * @brief 释放相机资源 Releasing camera resources
     */
    inline void release() override { CameraUnInit(_hCamera); }

    /**
     * @brief 相机采集 Camera grabbing
     *
     * @return 是否成功采集 Success to grab?
     */
    inline bool grab() override
    {
        _status = CameraGetImageBuffer(_hCamera, &_frame_info, &_pbyBuffer, 1000);
        return _status == CAMERA_STATUS_SUCCESS;
    }

    /**
     * @brief 相机处理 Camera retrieve
     *
     * @param image 输出图像 Output image
     * @param flag 相机处理模式 Camera retrieve mode
     * @return 是否成功处理 Retrieve successfully?
     */
    bool retrieve(cv::OutputArray image, RetrieveMode flag) override;

    /**
     * @brief 从相机设备中读取图像 Read image from the camera device
     *
     * @param image 待读入的图像 The image to read in
     * @return 是否读取成功 Read successfully?
     */
    inline bool read(cv::OutputArray image) override
    {
        if (grab())
            retrieve(image, _retrieve_mode);
        else
            reconnect();
        return !image.empty();
    }

    /**
     * @brief 从相机设备中读取图像 Read image from the camera device
     *
     * @param image 待读入的图像 The image to read in
     */
    virtual MvCamera &operator>>(cv::Mat &image) override
    {
        read(image);
        return *this;
    }

    /**
     * @brief 打开相机 Open the camera device
     *
     * @return 是否成功打开 Open Successfully?
     */
    bool open();

    /**
     * @brief 相机重连 Camera reconnecting
     *
     * @return 是否成功重连 Reconnect successfully?
     */
    bool reconnect();

private:
    /**
     * @brief LUT 表映射 Look-Up Table mapping
     *
     * @param lut std::vector 类型的 LUT 映射表 std::vector type's LUT mapping table
     * @return 是否转换成功 Convert successfully?
     */
    bool initLUT(const std::vector<int> &lut);
};

//! @} mv_camera

} // namespace rm

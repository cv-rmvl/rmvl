/**
 * @file hik_video_capture.h
 * @author RoboMaster Vision Community
 * @brief Hik Robot 工业相机库
 * @version 1.0
 * @date 2023-06-13
 *
 * @copyright Copyright 2023 (c), RoboMaster Vision Community
 *
 */

#pragma once

#include <opencv2/imgproc.hpp>

#include <MvCameraControl.h>

#include "definitions.h"
#include "logging.h"

namespace rm
{

//! @addtogroup hik_camera
//! @{

//! @example samples/camera/hik/sample_hik_manual_calib.cpp 海康机器人工业相机手动标定例程
//! @example samples/camera/hik/sample_hik_mono.cpp 海康机器人工业相机——单相机例程
//! @example samples/camera/hik/sample_hik_multi.cpp 海康机器人工业相机——多相机例程
//! @example samples/camera/hik/sample_hik_writer.cpp 海康机器人工业相机录屏例程

//! 海康机器人相机库 HikRobot camera library
class HikVideoCapture final : public cv::VideoCapture
{
    // -------------------------- 相机信息 Device information -------------------------
    void *_handle;                   //!< 相机设备句柄 Handle of the camera device
    MV_CC_DEVICE_INFO_LIST _devices; //!< 设备信息列表 Device information list
    GrabMode _grab_mode;             //!< 采集模式 Grab mode
    RetrieveMode _retrieve_mode;     //!< 处理模式 Retrieve mode
    std::string _serial;             //!< 相机序列号 Camera Serial Number (S/N)
    bool _opened = false;            //!< 相机是否打开 Is the camera opened

    // -------------------------- 图像信息 Image information --------------------------
    MV_FRAME_OUT _p_out;          //!< 输出图像的数据及信息 Output image data and information
    std::vector<uchar> _p_dstbuf; //!< 输出数据缓存 Output data buffer

    using VideoCapture::grab;
    using VideoCapture::open;

public:
    /**
     * @brief 构造函数 Constructor
     *
     * @param[in] grab_mode 相机采集模式 Camera grab mode
     * @param[in] retrieve_mode 相机处理模式 Camera retrieve mode
     * @param[in] serial 相机唯一序列号 Camera unique serial number
     */
    HikVideoCapture(GrabMode grab_mode, RetrieveMode retrieve_mode, std::string_view serial = "");

    HikVideoCapture(const HikVideoCapture &) = delete;
    HikVideoCapture(HikVideoCapture &&) = delete;

    //! 析构函数 Destructor
    ~HikVideoCapture() override;

    /**
     * @brief 构建 HikVideoCapture 对象 Construct HikVideoCapture object
     * @note 此相机库仅支持 USB 相机设备，暂时对 GigE 网口相机不兼容
     * @note This camera library only supports USB camera devices and is temporarily not
     *       compatible with GigE netport cameras
     *
     * @param[in] grab_mode 相机采集模式 Camera grab mode
     * @param[in] retrieve_mode 相机处理模式 Camera retrieve mode
     * @param[in] serial 相机唯一序列号 Camera unique serial number
     */
    static inline std::unique_ptr<HikVideoCapture> make_capture(GrabMode grab_mode, RetrieveMode retrieve_mode,
                                                                std::string_view serial = "")
    {
        return std::make_unique<HikVideoCapture>(grab_mode, retrieve_mode, serial);
    }

    /**
     * @brief 设置相机参数/事件 Set the camera parameter or activity
     *
     * @param[in] propId 参数/事件编号 The ID of the parameter or activity
     * @param[in] value 参数/事件值 The value of the parameter or activity
     * @return 是否设置成功 Set successfully?
     */
    bool set(int propId, double value = 0.0) override;

    /**
     * @brief 获取相机参数 Get the camera parameter
     *
     * @param[in] propId 参数编号 The ID of the parameter
     * @return 参数值 The value of the parameter
     */
    double get(int propId) const override;

    /**
     * @brief 相机是否打开 Is the camera turned on?
     */
    inline bool isOpened() const override { return _opened; }

    /**
     * @brief 释放相机资源 Releasing camera resources
     */
    void release() override;

    /**
     * @brief 相机处理 Camera retrieve
     *
     * @param[out] image 输出图像 Output image
     * @param[in] flag 相机处理模式 Camera retrieve mode
     * @return 是否成功处理 Retrieve successfully?
     */
    bool retrieve(cv::OutputArray image, RetrieveMode flag) override;

    /**
     * @brief 从相机设备中读取图像 Read image from the camera device
     *
     * @param[out] image 待读入的图像 The image to read in
     * @return 是否读取成功 Read successfully?
     */
    bool read(cv::OutputArray image) override;

    /**
     * @brief 从相机设备中读取图像 Read image from the camera device
     *
     * @param image 待读入的图像 The image to read in
     */
    virtual HikVideoCapture &operator>>(cv::Mat &image) override
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
     * @brief 错误码转字符串
     *
     * @param[in] code 错误码
     * @return 字符串
     */
    const char *errorCode2Str(unsigned int code);
};

//! @} hik_camera

} // namespace rm

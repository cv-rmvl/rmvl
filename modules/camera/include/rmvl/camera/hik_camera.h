/**
 * @file hik_camera.h
 * @author RoboMaster Vision Community
 * @brief Hik Robot 工业相机库
 * @version 2.0
 * @date 2023-12-14
 *
 * @copyright Copyright 2023 (c), RoboMaster Vision Community
 *
 */

#pragma once

#include <MvCameraControl.h>

#include "camutils.hpp"
#include "rmvl/core/util.hpp"

namespace rm
{

//! @addtogroup camera
//! @{
//! @defgroup hik_camera 海康机器人（HikRobot）工业相机库
//! @}

//! @addtogroup hik_camera
//! @{

//! @example samples/camera/hik/sample_hik_manual_calib.cpp 海康机器人工业相机手动标定例程
//! @example samples/camera/hik/sample_hik_mono.cpp 海康机器人工业相机——单相机例程
//! @example samples/camera/hik/sample_hik_multi.cpp 海康机器人工业相机——多相机例程
//! @example samples/camera/hik/sample_hik_writer.cpp 海康机器人工业相机录屏例程

//! 海康机器人相机库 HikRobot camera library
class HikCamera final
{
public:
    using ptr = std::unique_ptr<HikCamera>;
    using const_ptr = std::unique_ptr<const HikCamera>;

    //! Pointer to the implementation class
    class Impl;

    /**
     * @brief 构造函数 Constructor
     *
     * @param[in] grab_mode 相机采集模式 Camera grab mode
     * @param[in] retrieve_mode 相机处理模式 Camera retrieve mode
     * @param[in] serial 相机唯一序列号 Camera unique serial number
     */
    HikCamera(GrabMode grab_mode, RetrieveMode retrieve_mode, std::string_view serial = "");

    HikCamera(const HikCamera &) = delete;
    HikCamera(HikCamera &&val) : _impl(std::exchange(val._impl, nullptr)) {}

    //! 析构函数 Destructor
    ~HikCamera();

    /**
     * @brief 构建 HikCamera 对象 Construct HikCamera object
     * @note 此相机库仅支持 USB 相机设备，暂时对 GigE 网口相机不兼容
     * @note This camera library only supports USB camera devices and is temporarily not
     *       compatible with GigE netport cameras
     *
     * @param[in] grab_mode 相机采集模式 Camera grab mode
     * @param[in] retrieve_mode 相机处理模式 Camera retrieve mode
     * @param[in] serial 相机唯一序列号 Camera unique serial number
     */
    static std::unique_ptr<HikCamera> make_capture(GrabMode grab_mode, RetrieveMode retrieve_mode, std::string_view serial = "")
    {
        return std::unique_ptr<HikCamera>(new HikCamera(grab_mode, retrieve_mode, serial));
    }

    /**
     * @brief 设置相机参数/事件 Set the camera parameter or activity
     *
     * @param[in] propId 参数/事件编号 The ID of the parameter or activity
     * @param[in] value 参数/事件值 The value of the parameter or activity
     * @return 是否设置成功 Set successfully?
     */
    bool set(int propId, double value = 0.0);

    /**
     * @brief 获取相机参数 Get the camera parameter
     *
     * @param[in] propId 参数编号 The ID of the parameter
     * @return 参数值 The value of the parameter
     */
    double get(int propId) const;

    //! 相机是否打开 Is the camera turned on?
    bool isOpened() const;

    /**
     * @brief 从相机设备中读取图像 Read image from the camera device
     *
     * @param[out] image 待读入的图像 The image to read in
     * @return 是否读取成功 Read successfully?
     */
    bool read(cv::OutputArray image);

    /**
     * @brief 从相机设备中读取图像 Read image from the camera device
     *
     * @param image 待读入的图像 The image to read in
     */
    HikCamera &operator>>(cv::Mat &image)
    {
        read(image);
        return *this;
    }

    /**
     * @brief 相机重连 Camera reconnecting
     *
     * @return 是否成功重连 Reconnect successfully?
     */
    bool reconnect();

private:
    Impl *_impl;
};

//! @} hik_camera

} // namespace rm

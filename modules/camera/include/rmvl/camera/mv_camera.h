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

#include <CameraApi.h>

#include "camutils.hpp"
#include "rmvl/core/util.hpp"

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
class MvCamera final
{
public:
    using ptr = std::unique_ptr<MvCamera>;
    using const_ptr = std::unique_ptr<const MvCamera>;

    //! Pointer to the implementation class
    class Impl;

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
    MvCamera(MvCamera &&val) : _impl(std::exchange(val._impl, nullptr)) {}

    ~MvCamera();

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

    //! 相机是否打开 Camera is turned on?
    bool isOpened() const;

    /**
     * @brief 从相机设备中读取图像 Read image from the camera device
     *
     * @param[in] image 待读入的图像 The image to read in
     * @return 是否读取成功 Read successfully?
     */
    bool read(cv::OutputArray image);

    /**
     * @brief 从相机设备中读取图像 Read image from the camera device
     *
     * @param[out] image 待读入的图像 The image to read in
     */
    inline MvCamera &operator>>(cv::Mat &image)
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

//! @} mv_camera

} // namespace rm

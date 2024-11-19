/**
 * @file mv_camera.h
 * @author zhaoxi (535394140@qq.com)
 * @brief 迈德威视相机库
 * @version 3.0
 * @date 2023-12-14
 *
 * @copyright Copyright 2023 (c), zhaoxi
 *
 */

#pragma once

#include "camutils.hpp"
#include "rmvl/core/util.hpp"

namespace rm
{

//! @addtogroup camera
//! @{
//! @defgroup mv_camera 迈德威视 USB3.0 相机库
//! @}

//! @addtogroup mv_camera
//! @{

//! @example samples/camera/mv/sample_mv_auto_calib.cpp 迈德威视相机自动标定例程
//! @example samples/camera/mv/sample_mv_manual_calib.cpp 迈德威视相机手动标定例程
//! @example samples/camera/mv/sample_mv_mono.cpp 迈德威视单相机例程
//! @example samples/camera/mv/sample_mv_multi.cpp 迈德威视多相机例程
//! @example samples/camera/mv/sample_mv_writer.cpp 迈德威视相机录屏例程

//! 迈德威视相机库
class RMVL_EXPORTS_W MvCamera final
{
    RMVL_IMPL;

public:
    using ptr = std::unique_ptr<MvCamera>;
    using const_ptr = std::unique_ptr<const MvCamera>;

    /**
     * @brief 构造 MvCamera 对象
     *
     * @param[in] init_mode 相机初始化配置模式，需要配置 rm::GrabMode 和 rm::RetrieveMode
     * @param[in] serial 相机唯一序列号
     */
    RMVL_W MvCamera(CameraConfig init_mode, std::string_view serial = "");

    //! @cond
    MvCamera(const MvCamera &) = delete;
    MvCamera(MvCamera &&val) : _impl(std::exchange(val._impl, nullptr)) {}
    //! @endcond

    /**
     * @brief 构建 MvCamera 对象
     *
     * @param[in] init_mode 相机初始化配置模式，需要配置 rm::GrabMode 和 rm::RetrieveMode
     * @param[in] serial 相机唯一序列号
     */
    static inline std::unique_ptr<MvCamera> make_capture(CameraConfig init_mode, std::string_view serial = "")
    {
        return std::make_unique<MvCamera>(init_mode, serial);
    }

    //! 获取相机库版本
    RMVL_W static std::string version();

    /**
     * @brief 设置相机参数/事件
     *
     * @param[in] propId 参数/事件编号
     * @param[in] value 参数/事件值
     * @return 是否设置成功
     */
    RMVL_W bool set(int propId, double value = 0.0);

    /**
     * @brief 获取相机参数
     *
     * @param[in] propId 参数编号
     * @return 参数值
     */
    RMVL_W double get(int propId) const;

    //! 相机是否打开
    RMVL_W bool isOpened() const;

    /**
     * @brief 从相机设备中读取图像
     *
     * @param[out] image 待读入的图像
     * @return 是否读取成功
     */
    bool read(cv::OutputArray image);

    //! @cond

    /**
     * @brief 从相机设备中读取图像
     *
     * @return 是否读取成功和读取到的图像
     */
    RMVL_W inline std::pair<bool, cv::Mat> read()
    {
        cv::Mat img;
        bool res = read(img);
        return {res, img};
    }

    //! @endcond

    /**
     * @brief 从相机设备中读取图像
     *
     * @param[out] image 待读入的图像
     */
    inline MvCamera &operator>>(cv::Mat &image)
    {
        read(image);
        return *this;
    }

    /**
     * @brief 相机重连
     *
     * @return 是否成功重连
     */
    RMVL_W bool reconnect();
};

//! @} mv_camera

} // namespace rm

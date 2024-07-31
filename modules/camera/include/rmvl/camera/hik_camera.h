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

//! 海康机器人相机库
class HikCamera final
{
    class Impl;

public:
    using ptr = std::unique_ptr<HikCamera>;
    using const_ptr = std::unique_ptr<const HikCamera>;

    /**
     * @brief 创建 HikCamera 对象
     *
     * @param[in] init_mode 相机初始化配置模式，需要配置 GrabMode 和 RetrieveMode
     * @param[in] serial 相机唯一序列号
     */
    HikCamera(CameraConfig init_mode, std::string_view serial = "");

    HikCamera(const HikCamera &) = delete;
    HikCamera(HikCamera &&val) : _impl(std::exchange(val._impl, nullptr)) {}
    ~HikCamera();

    /**
     * @brief 构建 HikCamera 对象
     * @note 此相机库仅支持 USB 相机设备，暂时对 GigE 网口相机不兼容
     *
     * @param[in] init_mode 相机初始化配置模式，需要配置 GrabMode 和 RetrieveMode
     * @param[in] serial 相机唯一序列号
     * @return HikCamera 对象独享指针
     */
    static inline std::unique_ptr<HikCamera> make_capture(CameraConfig init_mode, std::string_view serial = "") { return std::make_unique<HikCamera>(init_mode, serial); }

    /**
     * @brief 设置相机参数/事件
     *
     * @param[in] propId 参数/事件编号
     * @param[in] value 参数/事件值
     * @return 是否设置成功
     */
    bool set(int propId, double value = 0.0);

    /**
     * @brief 获取相机参数
     *
     * @param[in] propId 参数编号
     * @return 参数值
     */
    double get(int propId) const;

    //! 相机是否打开
    bool isOpened() const;

    /**
     * @brief 从相机设备中读取图像
     *
     * @param[out] image 待读入的图像
     * @return 是否读取成功
     */
    bool read(cv::OutputArray image);

    /**
     * @brief 从相机设备中读取图像
     *
     * @param[out] image 待读入的图像
     */
    HikCamera &operator>>(cv::Mat &image)
    {
        read(image);
        return *this;
    }

    /**
     * @brief 相机重连
     *
     * @return 是否成功重连
     */
    bool reconnect();

private:
    Impl *_impl;
};

//! @} hik_camera

} // namespace rm

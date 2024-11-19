/**
 * @file opt_camera.h
 * @author zhaoxi (535394140@qq.com)
 * @brief 奥普特机器视觉相机库
 * @version 1.0
 * @date 2023-12-15
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
//! @defgroup opt_camera 奥普特机器视觉 USB3.0/GigE 系列工业相机库
//! @}

//! @addtogroup opt_camera
//! @{

//! 奥普特机器视觉相机库
class RMVL_EXPORTS_W OptCamera final
{
    RMVL_IMPL;

public:
    using ptr = std::unique_ptr<OptCamera>;
    using const_ptr = std::unique_ptr<const OptCamera>;

    /**
     * @brief 创建 OptCamera 对象
     *
     * @param[in] init_mode 相机初始化配置模式，需要配置 rm::HandleMode、 rm::GrabMode 和 rm::RetrieveMode，如果 rm::GrabMode
     *                      配置为 `rm::GrabMode::Hardware`，则需要添加触发通道的配置项，例如 `rm::TriggerChannel::Chn1`
     * @param[in] handle_info 句柄信息，例如序列号、IP、用户标识
     */
    RMVL_W OptCamera(CameraConfig init_mode, std::string_view handle_info = "");

    //! @cond
    OptCamera(const OptCamera &) = delete;
    OptCamera(OptCamera &&val) : _impl(std::exchange(val._impl, nullptr)) {}
    //! @endcond

    /**
     * @brief 构建 OptCamera 对象
     *
     * @param[in] init_mode 相机初始化配置模式，需要配置 HandleMode、GrabMode 和 RetrieveMode，如果 GrabMode
     *                      配置为 `GrabMode::Hardware`，则需要添加触发通道的配置项，例如 `TriggerChannel::Chn1`
     * @param[in] handle_info 句柄信息，例如序列号、IP、用户标识
     */
    static std::unique_ptr<OptCamera> make_capture(CameraConfig init_mode, std::string_view handle_info = "")
    {
        return std::unique_ptr<OptCamera>(new OptCamera(init_mode, handle_info));
    }

    /**
     * @brief 设置相机参数、触发相机事件
     *
     * @param[in] prop_id 参数/事件编号
     * @param[in] value 参数/事件值
     * @return 是否设置成功
     */
    RMVL_W bool set(int prop_id, double value = 0.0);

    /**
     * @brief 获取相机参数
     *
     * @param[in] prop_id 参数编号
     * @return 参数值
     */
    RMVL_W double get(int prop_id) const;

    //! 相机是否已经打开
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
     * @brief 相机重连
     *
     * @return 是否重连成功
     */
    RMVL_W bool reconnect();
};

//! @} opt_camera

} // namespace rm

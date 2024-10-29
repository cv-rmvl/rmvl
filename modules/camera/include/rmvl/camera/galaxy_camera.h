/**
 * @file galaxy_camera.h
 * @author zhaoxi (535394140@qq.com)
 * @brief 大恒图像 Galaxy USB3.0/GigE 系列工业相机库
 * @version 1.0
 * @date 2024-10-27
 * 
 * @copyright Copyright 2024 (c), zhaoxi
 * 
 */

#pragma once

#include "camutils.hpp"
#include "rmvl/core/util.hpp"

namespace rm
{

//! @addtogroup camera
//! @{
//! @defgroup galaxy_camera 大恒图像 Galaxy USB3.0/GigE 系列工业相机库
//! @}

//! @addtogroup galaxy_camera
//! @{

//! 大恒图像 Galaxy 系列工业相机库
class RMVL_EXPORTS_W GalaxyCamera final
{
    RMVL_IMPL;

public:
    using ptr = std::unique_ptr<GalaxyCamera>;
    using const_ptr = std::unique_ptr<const GalaxyCamera>;

    /**
     * @brief 创建 GalaxyCamera 对象
     *
     * @param[in] init_mode 相机初始化配置模式，需要配置 rm::GrabMode 、 rm::RetrieveMode 和 rm::HandleMode
     * @param[in] id 相机标识，可以是序列号、IP 地址等
     */
    RMVL_W GalaxyCamera(CameraConfig init_mode, std::string_view id = "");

    //! @cond
    GalaxyCamera(const GalaxyCamera &) = delete;
    GalaxyCamera(GalaxyCamera &&) = default;
    //! @endcond

    //! 获取相机库版本
    RMVL_W static std::string version();

    /**
     * @brief 构建 GalaxyCamera 对象
     *
     * @param[in] init_mode 相机初始化配置模式，需要配置 rm::GrabMode 、 rm::RetrieveMode 和 rm::HandleMode
     * @param[in] id 相机标识，可以是序列号、IP 地址等
     * @return GalaxyCamera 对象独享指针
     */
    static inline std::unique_ptr<GalaxyCamera> make_capture(CameraConfig init_mode, std::string_view id = "") { return std::make_unique<GalaxyCamera>(init_mode, id); }

    /**
     * @brief 设置相机参数/事件
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
    GalaxyCamera &operator>>(cv::Mat &image)
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

//! @} galaxy_camera

} // namespace rm

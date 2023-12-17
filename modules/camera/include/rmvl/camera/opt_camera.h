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

namespace rm
{

//! @addtogroup camera
//! @{
//! @defgroup opt_camera 奥普特机器视觉相机库
//! @}

//! @addtogroup opt_camera
//! @{

//! 奥普特机器视觉相机库
class OptCamera final
{
public:
    using ptr = std::unique_ptr<MvCamera>;
    using const_ptr = std::unique_ptr<const MvCamera>;

    //! Pointer to the implementation class
    class Impl;

    /**
     * @brief 设置相机参数、触发相机事件
     *
     * @param[in] prop_id 参数/事件编号
     * @param[in] value 参数/事件值
     * @return 是否设置成功
     */
    bool set(int prop_id, double value = 0.0);

    /**
     * @brief 获取相机参数
     *
     * @param[in] prop_id 参数编号
     * @return 参数值
     */
    double get(int prop_id) const;

    //! 相机是否已经打开
    bool isOpened() const;

    /**
     * @brief 从相机设备中读取图像
     *
     * @param[out] image 待读入的图像
     * @return 是否读取成功
     */
    bool read(cv::OutputArray image);

    /**
     * @brief 相机重连
     *
     * @return 是否重连成功
     */
    bool reconnect();

private:
    Impl *_impl;
};

//! @} opt_camera

} // namespace rm

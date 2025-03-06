/**
 * @file hik_light_control.h
 * @author zhaoxi (535394140@qq.com)
 * @brief 海康机器人 RS-232 光源控制库
 * @version 1.0
 * @date 2024-11-25
 *
 * @copyright Copyright 2024 (c), zhaoxi
 *
 */

#pragma once

#include <memory>
#include <vector>

#include "lightutils.hpp"

namespace rm
{

//! @addtogroup hik_light_control
//! @{

//! 海康机器人光源控制器
class RMVL_EXPORTS_W HikLightController
{
    RMVL_IMPL;

public:
    /**
     * @brief 构造海康机器人光源控制器对象，并使用建立连接
     *
     * @param[in] cfg 光源控制器配置
     * @param[in] id 光源控制器唯一标识，目前只支持串口设备名称
     */
    RMVL_W HikLightController(const LightConfig &cfg, std::string_view id);

    //! @cond
    HikLightController(const HikLightController &) = delete;
    HikLightController(HikLightController &&) = default;
    HikLightController &operator=(const HikLightController &) = delete;
    HikLightController &operator=(HikLightController &&) = default;
    ~HikLightController();
    //! @endcond

    //! 光源控制器是否打开
    RMVL_W bool isOpened() const;

    /**
     * @brief 设置为常亮模式，即打开全部通道
     *
     * @return 指定通道是否打开成功？
     */
    RMVL_W bool open();

    /**
     * @brief 设置为常灭模式，即关闭全部通道
     *
     * @return 指定通道是否关闭成功？
     */
    RMVL_W bool close();

    /**
     * @brief 获取指定通道的光源亮度
     *
     * @param[in] chn 指定通道，范围: `1 ~ 8`
     * @return 若读取成功，返回 \f$[0, 255]\f$ 的值，否则返回 \f$-1\f$
     */
    RMVL_W int get(int chn) const;

    /**
     * @brief 设置指定通道的光源亮度
     *
     * @param[in] chn 指定通道，范围: `1 ~ 8`
     * @param[in] val 指定通道的光源亮度
     * @return 是否设置成功？
     */
    RMVL_W bool set(int chn, int val);
};

//! @} hik_light_control

} // namespace rm
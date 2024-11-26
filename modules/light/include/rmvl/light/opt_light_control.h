/**
 * @file opt_light_control.h
 * @author zhaoxi (535394140@qq.com)
 * @brief OPT 奥普特 GigE 光源控制库
 * @version 1.0
 * @date 2023-10-04
 *
 * @copyright Copyright 2023 (c), zhaoxi
 *
 */

#pragma once

#include <memory>
#include <vector>

#include "lightutils.hpp"

namespace rm
{

//! @addtogroup opt_light_control
//! @{

//! OPT 奥普特光源控制器
class RMVL_EXPORTS_W OPTLightController
{
    RMVL_IMPL;

public:
    /**
     * @brief 构造奥普特光源控制器对象，并使用建立连接
     *
     * @param[in] cfg 光源控制器配置
     * @param[in] id 光源控制器唯一标识，可以是 IP 地址或者唯一序列号
     */
    RMVL_W OPTLightController(const LightConfig &cfg, std::string_view id);

    //! @cond
    OPTLightController(const OPTLightController &) = delete;
    OPTLightController(OPTLightController &&) = default;
    OPTLightController &operator=(const OPTLightController &) = delete;
    OPTLightController &operator=(OPTLightController &&) = default;
    //! @endcond

    //! 光源控制器是否打开
    RMVL_W bool isOpened() const noexcept;

    /**
     * @brief 打开指定通道
     *
     * @param[in] channels 要打开的通道的索引组成的 `std::vector` ，范围: [1 ~ 32]（十进制格式）
     * @return 指定通道是否打开成功？
     */
    RMVL_W bool open(const std::vector<int> &channels) noexcept;

    /**
     * @brief 打开全部通道
     *
     * @return 指定通道是否打开成功？
     */
    RMVL_W bool open() noexcept;

    /**
     * @brief 关闭指定通道
     *
     * @param[in] channels 要关闭的通道的索引组成的 `std::vector` ，范围: [1 ~ 32]（十进制格式）
     * @return 指定通道是否关闭成功？
     */
    RMVL_W bool close(const std::vector<int> &channels) noexcept;

    /**
     * @brief 关闭全部通道
     *
     * @return 指定通道是否关闭成功？
     */
    RMVL_W bool close() noexcept;

    /**
     * @brief 获取指定通道的光源亮度
     *
     * @param[in] channel 指定通道
     * @return 若读取成功，返回 \f$[0, 255]\f$ 的值，否则返回 \f$-1\f$
     */
    RMVL_W int getIntensity(int channel) const noexcept;

    /**
     * @brief 设置指定通道的光源亮度
     *
     * @param[in] channel 指定通道
     * @param[in] intensity 指定通道的光源亮度
     * @return 是否设置成功？
     */
    RMVL_W bool setIntensity(int channel, int intensity) noexcept;

    /**
     * @brief 光源控制器软触发指定通道
     *
     * @param[in] channel 指定通道，范围: [1 ~ 8]（十进制格式）
     * @param[in] time 触发时间，单位: 10ms
     * @return 是否成功触发？
     */
    RMVL_W bool trigger(int channel, int time) const noexcept;
};

//! @} opt_light_control

} // namespace rm

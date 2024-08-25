/**
 * @file opt_light_control.h
 * @author zhaoxi (535394140@qq.com)
 * @brief
 * @version 1.0
 * @date 2023-10-04
 *
 * @copyright Copyright 2023 (c), zhaoxi
 *
 */

#pragma once

#include <string>
#include <vector>

#include "rmvl/core/rmvldef.hpp"

namespace rm
{

//! @addtogroup opt_light_control
//! @{

//! OPT 光源控制器 IP 配置信息
struct RMVL_EXPORTS_W_AG LightIpConfig
{
    RMVL_W_RW std::string ip;              //!< IP 地址
    RMVL_W_RW std::string subnet_mask;     //!< 子网掩码
    RMVL_W_RW std::string default_gateway; //!< 默认网关
};

//! OPT 奥普特光源控制器
class RMVL_EXPORTS_W OPTLightController
{
    using OPTController_StatusCode = long;

public:
    //! 构造新 OPTLightController 对象
    RMVL_W OPTLightController() = default;
    //! 禁止从 OPTLightController 对象复制构造
    OPTLightController(const OPTLightController &) = delete;
    //! 移动 OPTLightController 对象
    OPTLightController(OPTLightController &&obj) = default;

    ~OPTLightController() { disconnect(); }

    /**
     * @brief 使用 IP 地址创建 EtherNet 以太网连接
     *
     * @param[in] ip_config IP 配置信息
     * @return 连接是否成功建立？
     */
    RMVL_W bool connect(const LightIpConfig &ip_config);

    /**
     * @brief 使用设备序列号创建 EtherNet 以太网连接
     *
     * @param[in] SN 设备序列号
     * @return 连接是否成功建立？
     */
    RMVL_W bool connect(std::string_view SN);

    /**
     * @brief 断开已存在网口的连接
     */
    RMVL_W bool disconnect();

    /**
     * @brief 打开指定通道
     *
     * @param[in] channels 要打开的通道的索引组成的 `std::vector` ，范围: [1 ~ 32]（十进制格式）
     * @return 指定通道是否打开成功？
     */
    RMVL_W bool openChannels(const std::vector<int> &channels);

    /**
     * @brief 打开全部通道
     *
     * @return 指定通道是否打开成功？
     */
    RMVL_W bool openAllChannels();

    /**
     * @brief 关闭指定通道
     *
     * @param[in] channels 要关闭的通道的索引组成的 `std::vector` ，范围: [1 ~ 32]（十进制格式）
     * @return 指定通道是否关闭成功？
     */
    RMVL_W bool closeChannels(const std::vector<int> &channels);

    /**
     * @brief 关闭全部通道
     *
     * @return 指定通道是否关闭成功？
     */
    RMVL_W bool closeAllChannels();

    /**
     * @brief 获取指定通道的光源亮度
     *
     * @param[in] channel 指定通道
     * @return 若读取成功，返回 \f$[0, 255]\f$ 的值，否则返回 \f$-1\f$
     */
    RMVL_W int getIntensity(int channel) const;

    /**
     * @brief 设置指定通道的光源亮度
     *
     * @param[in] channel 指定通道
     * @param[in] intensity 指定通道的光源亮度
     * @return 是否设置成功？
     */
    RMVL_W bool setIntensity(int channel, int intensity);

    /**
     * @brief 光源控制器软触发指定通道
     *
     * @param[in] channel 指定通道
     * @param[in] time 触发时间，单位: 10ms
     * @return 是否成功触发？
     */
    RMVL_W bool trigger(int channel, int time) const;

private:
    bool _init{};        //!< 初始化标志位
    long long _handle{}; //!< 光源控制器句柄
};

//! @} opt_light_control

} // namespace rm

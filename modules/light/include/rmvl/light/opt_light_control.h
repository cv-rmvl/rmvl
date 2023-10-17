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

namespace rm
{

//! @addtogroup opt_light_control
//! @{

//! IP 配置聚合类
struct IPConfig
{
    std::string ip;              //!< IP 地址
    std::string subnet_mask;     //!< 子网掩码
    std::string default_gateway; //!< 默认网管
};

//! OPT 奥普特光源控制器
class OPTLightController
{
    using OPTController_StatusCode = long;

    bool _init = false;  //!< 初始化标志位
    long long _handle{}; //!< 光源控制器句柄

public:
    //! 构造新 OPTLightController 对象
    OPTLightController() = default;
    //! 禁止从 OPTLightController 对象复制构造
    OPTLightController(const OPTLightController &) = delete;
    //! 移动 OPTLightController 对象
    OPTLightController(OPTLightController &&obj);

    //! 析构 OPTLightController 对象
    ~OPTLightController() { disconnect(); }

    /**
     * @brief 使用 IP 地址创建 EtherNet 以太网连接
     *
     * @param ip_config IP 配置信息
     * @return 连接是否成功建立？
     */
    bool connect(const IPConfig &ip_config);

    /**
     * @brief 使用设备序列号创建 EtherNet 以太网连接
     *
     * @return 连接是否成功建立？
     */
    bool connect(const char *SN);

    /**
     * @brief 断开已存在网口的连接
     */
    bool disconnect();

    /**
     * @brief 打开指定通道
     *
     * @param[in] channels 要打开的通道的索引组成的 `std::vector` ，范围:[1 ~ 32]（十进制格式）
     *
     * @return 指定通道是否打开成功？
     */
    bool openChannels(const std::vector<int> &channels);

    /**
     * @brief 打开全部通道
     *
     * @return 指定通道是否打开成功？
     */
    bool openAllChannels();

    /**
     * @brief 关闭指定通道
     *
     * @param[in] channels 要关闭的通道的索引组成的 `std::vector` ，范围:[1 ~ 32]（十进制格式）
     * @return 指定通道是否关闭成功？
     */
    bool closeChannels(const std::vector<int> &channels);

    /**
     * @brief 关闭全部通道
     *
     * @return 指定通道是否关闭成功？
     */
    bool closeAllChannels();

    /**
     * @brief 获取指定通道的光源亮度
     *
     * @param[in] channel 指定通道
     * @return 若读取成功，返回 \f$[0, 255]\f$ 的值，否则返回 \f$-1\f$
     */
    int getIntensity(int channel);

    /**
     * @brief 设置指定通道的光源亮度
     *
     * @param[in] channel 指定通道
     * @param[in] intensity 指定通道的光源亮度
     * @return 是否设置成功？
     */
    bool setIntensity(int channel, int intensity);

    /**
     * @brief 光源控制器软触发指定通道
     *
     * @param[in] channel 指定通道
     * @param[in] time 触发时间，单位: 10ms
     * @return 是否成功触发？
     */
    bool trigger(int channel, int time);
};

//! @} opt_light_control

} // namespace rm

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

#include <memory>
#include <unordered_map>
#include <vector>

#include <OPTController.h>
#include <OPTErrorCode.h>

namespace rm
{

//! @addtogroup opt_light_control
//! @{

//! IP 配置聚合类
struct IPConfig
{
    const char *ip;              //!< IP 地址
    const char *subnet_mask;     //!< 子网掩码
    const char *default_gateway; //!< 默认网管
};

using OPTController_StatusCode = long;

//! OPT 奥普特光源控制器
class LightController
{
    bool _init = false;                                      //!< 初始化标志位
    OPTController_Handle _handle;                            //!< 光源控制器句柄
    OPTController_StatusCode _status_code;                   //!< 运行时状态码
    static std::unordered_map<int, std::string> _error_code; //!< 错误码哈希表

public:
    //! 构造新 LightController 对象
    LightController() { initErrorCode(); }

    //! 析构 LightController 对象
    ~LightController() { disconnect(); }

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
    inline bool trigger(int channel, int time)
    {
        _status_code = OPTController_SoftwareTrigger(_handle, channel, time);
        return _status_code == OPT_SUCCEED;
    }

private:
    //! 初始化 'error code' 散列表
    void initErrorCode();

    /**
     * @brief 获取错误信息
     *
     * @param[in] flag 状态码
     * @return 错误信息
     */
    inline std::string getErrorString(int flag) { return _error_code[flag]; }
};

//! @} opt_light_control

} // namespace rm

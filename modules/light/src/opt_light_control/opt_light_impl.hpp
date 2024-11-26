/**
 * @file opt_impl.hpp
 * @author zhaoxi (535394140@qq.com)
 * @brief OPT 奥普特光源控制库实现
 * @version 1.0
 * @date 2024-11-25
 *
 * @copyright Copyright 2024 (c), zhaoxi
 *
 */

#pragma once

#include "rmvl/light/opt_light_control.h"

namespace rm
{

//! OPT 奥普特光源控制器
class OPTLightController::Impl
{
public:
    Impl(const LightConfig &cfg, std::string_view id);

    Impl(const Impl &) = delete;
    Impl(Impl &&) = default;

    ~Impl();

    //! 光源控制器是否打开
    inline bool isOpened() const noexcept { return _init; }

    //! 打开指定通道
    bool open(const std::vector<int> &channels) noexcept;

    //! 打开全部通道
    bool open() noexcept;

    //! 关闭指定通道
    bool close(const std::vector<int> &channels) noexcept;

    //! 关闭全部通道
    bool close() noexcept;

    //! 获取指定通道的光源亮度
    int getIntensity(int channel) const noexcept;

    //! 设置指定通道的光源亮度
    bool setIntensity(int channel, int intensity) noexcept;

    //! 光源控制器软触发指定通道
    bool trigger(int channel, int time) const noexcept;

private:
    bool disconnect() noexcept;

    bool _init{};        //!< 初始化标志位
    long long _handle{}; //!< 光源控制器句柄
};

} // namespace rm

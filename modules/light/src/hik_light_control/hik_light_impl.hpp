/**
 * @file hik_light_impl.hpp
 * @author zhaoxi (535394140@qq.com)
 * @brief 海康机器人 RS-232 光源控制库实现
 * @version 1.0
 * @date 2024-11-25
 *
 * @copyright Copyright 2024 (c), zhaoxi
 *
 */

#pragma once

#include "rmvl/light/hik_light_control.h"

#include "rmvl/core/io.hpp"

namespace rm
{

//! 海康机器人光源控制器
class HikLightController::Impl
{
public:
    Impl(const LightConfig &cfg, std::string_view id);

    Impl(const Impl &) = delete;
    Impl(Impl &&) = default;

    //! 光源控制器是否打开
    inline bool isOpened() const { return _sp != nullptr && _sp->isOpened(); }

    //! 设置为常亮模式，即打开全部通道
    bool open();

    //! 设置为常灭模式，即关闭全部通道
    bool close();

    //! 获取指定通道的光源亮度
    int get(int chn) const;

    //! 设置指定通道的光源亮度
    bool set(int chn, int val);

private:
    std::unique_ptr<SerialPort> _sp{}; //!< 串口对象
};

} // namespace rm

/**
 * @file gpio.hpp
 * @author zhaoxi (535394140@qq.com)
 * @brief Linux GPIO 字符设备 v2 接口
 * @version 1.0
 * @date 2026-10-13
 *
 * @copyright Copyright 2026 (c), zhaoxi
 *
 */

#pragma once

#ifdef __linux__

#include <cstdint>
#include <optional>
#include <vector>

#include "async.hpp"

namespace rm {

//! @addtogroup io_peripheral
//! @{

//! GPIO 边沿事件的同步读取模式，电平读写不受此选项影响
enum class GpioReadMode : uint8_t {
    BLOCK,   //!< 等待直到有边沿事件
    NONBLOCK //!< 无边沿事件时立即返回 std::nullopt
};

//! GPIO 方向
enum class GpioDirection {
    Input,
    Output
};

//! 逻辑边沿，Active Low 时上升沿表示物理下降沿
enum class GpioEdge {
    None,
    Rising,
    Falling,
    Both
};

//! GPIO 上下拉，Default 保留驱动配置
enum class GpioBias {
    Default,
    Disabled,
    PullUp,
    PullDown
};

//! GPIO 请求配置，所有线路使用相同的配置
struct GpioOptions {
    GpioDirection direction{GpioDirection::Input}; //!< 输入或输出
    GpioEdge edge{GpioEdge::None};                 //!< 输入边沿检测
    GpioBias bias{GpioBias::Default};              //!< 上下拉配置
    bool active_low{};                             //!< 低电平有效
    uint64_t initial_values{};                     //!< 输出初始逻辑电平，bit i 对应 offsets[i]
    uint32_t debounce_us{};                        //!< 输入消抖时间，单位为微秒
};

//! GPIO 边沿事件
struct GpioEvent {
    GpioEdge edge{GpioEdge::None}; //!< 逻辑边沿
    uint32_t offset{};             //!< 控制器内线路偏移
    uint64_t timestamp_ns{};       //!< CLOCK_MONOTONIC 时间戳，单位为纳秒
    uint32_t sequence{};           //!< 请求内事件序号，可用于检测事件丢失
};

/**
 * @brief 一组 GPIO 线路的独占请求，要求 Linux GPIO v2 ABI（内核 5.10 起）
 * @note
 * - 模式只影响 wait()，read()/write() 可能阻塞；
 * - 同一请求不能并发调用 wait()。
 * @throws std::system_error IO 操作失败
 */
class Gpio {
public:
    /**
     * @brief 打开控制器并申请 1 至 64 条线路
     * @param[in] device 控制器路径，例如 /dev/gpiochip0
     * @param[in] offsets 互不重复的控制器内线路偏移，不是排针编号
     * @param[in] options 线路配置
     * @param[in] mode 边沿等待模式
     */
    Gpio(std::string_view device, std::vector<uint32_t> offsets, GpioOptions options = {}, GpioReadMode mode = GpioReadMode::BLOCK);

    //! @cond
    ~Gpio();
    Gpio(Gpio &&) noexcept;
    Gpio &operator=(Gpio &&) noexcept;
    Gpio(const Gpio &) = delete;
    Gpio &operator=(const Gpio &) = delete;
    //! @endcond

    //! @return 逻辑电平位图，bit i 对应构造时的 offsets[i]
    uint64_t read() const;

    /**
     * @brief 设置所有申请线路的逻辑电平
     * @param[in] values 电平位图，仅能对输出请求调用
     */
    void write(uint64_t values) const;

    //! @return 边沿事件，NONBLOCK 且无事件时返回 std::nullopt
    std::optional<GpioEvent> wait() const;

    //! @return 是否持有有效线路请求，移动后的对象返回 false
    bool isOpened() const noexcept;

    //! @return 借用的线路请求 fd，不可关闭或改变其非阻塞状态
    int handle() const noexcept;

private:
    struct Impl;
    std::shared_ptr<Impl> _impl;
};

//! @}

#if __cplusplus >= 202002L

namespace async {

//! @addtogroup io_peripheral
//! @{

/**
 * @brief GPIO 原生边沿事件协程接口，通过 epoll 等待
 * @note 对象与 IOContext 须存活到任务结束，同一请求不能并发 wait()。
 */
class Gpio {
public:
    /**
     * @brief 创建异步 GPIO 请求
     * @param[in] context 异步执行上下文
     * @param[in] device 控制器路径
     * @param[in] offsets 线路偏移
     * @param[in] options 线路配置
     */
    Gpio(IOContext &context, std::string_view device, std::vector<uint32_t> offsets, GpioOptions options = {});

    //! @return 等待到的边沿事件
    Task<GpioEvent> wait();

    //! @return 是否持有有效请求
    bool isOpened() const noexcept { return _gpio->isOpened(); }

private:
    IOContext &_ctx;
    std::shared_ptr<::rm::Gpio> _gpio;
};

//! @}

} // namespace async

#endif

} // namespace rm

#endif

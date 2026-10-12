/**
 * @file can.hpp
 * @author zhaoxi (535394140@qq.com)
 * @brief Linux SocketCAN 帧收发
 * @version 1.0
 * @date 2026-10-13
 *
 * @copyright Copyright 2026 (c), zhaoxi
 *
 */

#pragma once

#ifdef __linux__

#include <array>
#include <cstdint>
#include <optional>
#include <vector>

#include "async.hpp"

namespace rm {
//! @addtogroup io_peripheral
//! @{
//! CAN 同步 IO 模式，同时控制帧读取和发送
enum class CanReadMode : uint8_t {
    BLOCK,   //!< 等待直到可读或可写
    NONBLOCK //!< 无帧可读时返回 std::nullopt，暂不可写时返回 false
};

//! 经典 CAN 或 CAN FD 帧，id 不包含 Linux 的标志位
struct CanFrame {
    uint32_t id{};                  //!< 标准帧 11 位，扩展帧 29 位，错误帧为错误位图
    std::array<uint8_t, 64> data{}; //!< 有效数据位于 [0, size)
    uint8_t size{};                 //!< 字节数，经典 CAN 为 0 至 8，FD 另支持 12/16/20/24/32/48/64
    bool extended{};                //!< 扩展帧
    bool remote{};                  //!< 远程请求帧，只用于经典 CAN
    bool error{};                   //!< 接收到的错误帧，禁止通过 write() 发送
    bool fd{};                      //!< CAN FD 帧
    bool bitrate_switch{};          //!< CAN FD 数据阶段切换速率
    bool error_state_indicator{};   //!< CAN FD 错误状态指示
};
//! 接收过滤器，id 和 mask 使用 SocketCAN 原始格式，可包含 EFF/RTR 标志位
struct CanFilter {
    uint32_t id{};   //!< 过滤 ID
    uint32_t mask{}; //!< ID 掩码
};
/**
 * @brief SocketCAN 原始套接字
 * @note
 * - CAN 接口、波特率及 CAN FD 需提前配置；
 * - 同一 socket 只允许一个读等待和一个写等待。
 * @throws std::system_error IO 操作失败
 */
class Can {
public:
    /**
     * @brief 绑定 CAN 网络接口
     * @param[in] interface 接口名，例如 can0 或 vcan0
     * @param[in] mode 收发模式
     * @param[in] enable_fd 是否允许 CAN FD 帧
     */
    explicit Can(std::string_view interface, CanReadMode mode = CanReadMode::BLOCK, bool enable_fd = false);
    //! @cond
    ~Can();
    Can(Can &&) noexcept;
    Can &operator=(Can &&) noexcept;
    Can(const Can &) = delete;
    Can &operator=(const Can &) = delete;
    //! @endcond
    //! @return 接收到的帧，NONBLOCK 且无数据时返回 nullopt
    std::optional<CanFrame> read() const;
    /**
     * @brief 发送一帧
     * @param[in] frame 待发送帧
     * @return 是否已提交到内核，NONBLOCK 且暂不可写时返回 false
     * @note 发送成功不代表对端已收到，不自动重发。
     */
    bool write(const CanFrame &frame) const;
    /** @brief 设置接收过滤器
     * @param[in] filters 过滤器列表，空列表关闭普通数据帧接收
     */
    void setFilters(const std::vector<CanFilter> &filters) const;
    /** @brief 设置错误帧接收掩码，默认不接收错误帧
     * @param[in] mask SocketCAN 错误位图
     */
    void setErrorMask(uint32_t mask) const;
    //! @return 借用的 socket fd，不可关闭或修改非阻塞状态
    int handle() const noexcept;
    //! @return 是否持有 socket，移动后的对象返回 false
    bool isOpened() const noexcept;

private:
    struct Impl;
    std::shared_ptr<Impl> _impl;
};
//! @}

#if __cplusplus >= 202002L
namespace async {
//! @addtogroup io_peripheral
//! @{
//! SocketCAN 协程接口，对象和 IOContext 必须存活到任务结束
class Can {
public:
    /** @brief 创建异步 CAN socket
     * @param[in] context 异步执行上下文
     * @param[in] interface CAN 接口名
     * @param[in] enable_fd 是否启用 CAN FD
     */
    Can(IOContext &context, std::string_view interface, bool enable_fd = false);
    //! @return 接收到的帧
    Task<CanFrame> read();
    /** @brief 异步发送一帧
     * @param[in] frame 待发送帧，复制到任务中
     * @return 发送完成任务，失败时抛出异常
     */
    Task<> write(CanFrame frame);
    /** @brief 设置接收过滤器
     * @param[in] filters 过滤器列表
     */
    void setFilters(const std::vector<CanFilter> &filters) const { _can->setFilters(filters); }
    /** @brief 设置错误帧接收掩码
     * @param[in] mask 错误位图
     */
    void setErrorMask(uint32_t mask) const { _can->setErrorMask(mask); }
    //! @return 是否持有 socket
    bool isOpened() const noexcept { return _can->isOpened(); }

private:
    IOContext &_ctx;
    std::shared_ptr<::rm::Can> _can;
};
//! @}
} // namespace async
#endif
} // namespace rm
#endif

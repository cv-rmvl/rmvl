/**
 * @file i2c.hpp
 * @author zhaoxi (535394140@qq.com)
 * @brief Linux i2c-dev 组合事务接口
 * @version 1.0
 * @date 2026-10-13
 *
 * @copyright Copyright 2026 (c), zhaoxi
 *
 */

#pragma once

#ifdef __linux__

#include <cstdint>
#include <memory>
#include <string_view>
#include <vector>

namespace rm {

//! @addtogroup io_peripheral
//! @{

//! I2C 事务中的一段消息，各段之间使用 repeated START
struct I2cMessage {
    uint16_t address{};        //!< 不含读写位的从机地址
    bool reading{};            //!< true 表示读消息
    bool ten_bit{};            //!< 使用十位地址
    std::vector<uint8_t> data; //!< 写数据或预先确定长度的读缓冲区
};

/**
 * @brief Linux I2C_RDWR 事务接口，不实现外设寄存器协议
 * @note
 * - 仅支持阻塞事务，不自动重试。
 * - 同一对象的事务串行执行，不保证并发调用顺序。
 * @throws std::system_error IO 操作失败
 */
class I2c {
public:
    /**
     * @brief 打开 I2C 适配器，要求支持 I2C_FUNC_I2C
     * @param[in] device 总线路径，例如 /dev/i2c-1
     */
    explicit I2c(std::string_view device);

    //! @cond
    ~I2c();
    I2c(I2c &&) noexcept;
    I2c &operator=(I2c &&) noexcept;
    I2c(const I2c &) = delete;
    I2c &operator=(const I2c &) = delete;
    //! @endcond

    /**
     * @brief 提交一笔组合事务，读写缓冲区归事务所有
     * @param[in] messages 1 至 42 段消息，每段最多 8192 字节
     * @return 完成的消息，读段的数据已填充
     */
    std::vector<I2cMessage> transfer(std::vector<I2cMessage> messages);

    //! @return 是否持有适配器，移动后的对象返回 false
    bool isOpened() const noexcept;

private:
    struct Impl;
    std::unique_ptr<Impl> _impl;
};

//! @}

} // namespace rm

#endif

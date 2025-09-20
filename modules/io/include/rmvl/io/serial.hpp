/**
 * @file serial.hpp
 * @author zhaoxi (535394140@qq.com)
 * @brief 同步/异步的数据链路层串口通信框架
 * @details 提供对串口设备的同步和异步读写操作，支持多种波特率和读取模式
 * - 同步模式下提供阻塞和非阻塞读取
 * - 异步模式在阻塞状态下实现协程调度，从而实现非阻塞读取的功能
 * @version 1.0
 * @date 2025-07-31
 *
 * @copyright Copyright 2025 (c), zhaoxi
 *
 */

#pragma once

#include <cstdint>
#include <cstdio>
#include <cstring>
#include <string>

#include "rmvl/core/rmvldef.hpp"

#include "async.hpp"

namespace rm {

//! @addtogroup io_serial
//! @{

//! 波特率
enum class BaudRate : uint8_t {
    BR_1200,   //!< 波特率 1200
    BR_2400,   //!< 波特率 2400
    BR_4800,   //!< 波特率 4800
    BR_9600,   //!< 波特率 9600
    BR_19200,  //!< 波特率 19200
    BR_38400,  //!< 波特率 38400
    BR_57600,  //!< 波特率 57600
    BR_115200, //!< 波特率 115200
};

//! 同步模式串口数据读取模式
enum class SerialReadMode : uint8_t {
    BLOCK,   //!< 同步阻塞模式，即读取数据时会一直等待直到有数据到来
    NONBLOCK //!< 同步非阻塞模式，即读取数据时不会等待，如果没有数据到来则立即返回 `-1`
};

//! 同步模式串行接口通信库
class RMVL_EXPORTS_W SerialPort {
public:
    /**
     * @brief 构造新串口对象
     *
     * @param[in] device 设备名
     * @param[in] baud_rate 波特率
     * @param[in] read_mode 读取模式
     */
    RMVL_W SerialPort(std::string_view device, BaudRate baud_rate, SerialReadMode read_mode = {});

    //! @cond
    SerialPort(const SerialPort &) = delete;
    SerialPort(SerialPort &&) = default;
    SerialPort &operator=(const SerialPort &) = delete;
    SerialPort &operator=(SerialPort &&) = default;
    ~SerialPort();
    //! @endcond

    /**
     * @brief 从串口读取数据到聚合体中
     * @note 每次读取后会清空缓冲区
     *
     * @tparam Tp 读取到聚合体的类型
     * @param[in] head_flag 头帧
     * @param[in] tail_flag 尾帧
     * @param[out] data 读取的聚合体数据
     * @return 是否读取成功
     */
    template <typename Tp, typename Enable = std::enable_if_t<std::is_aggregate_v<Tp>>>
    bool read(uint8_t head_flag, uint8_t tail_flag, Tp &data) {
        bool retval{};
        constexpr int LENGTH = 512, SIZE = sizeof(Tp);
        uint8_t buffer[LENGTH]{};
        auto len_result = fdread(buffer, LENGTH);
        for (long int i = 0; (i + SIZE + 1) < len_result; i++)
            if (buffer[i] == head_flag && buffer[i + SIZE + 1] == tail_flag) {
                auto p = std::memcpy(&data, &buffer[i + 1], SIZE);
                if (p == &data)
                    retval = true;
            }
        return retval;
    }

    /**
     * @brief 不带头尾标志的数据读取，从串口读取数据到聚合体中
     *
     * @tparam Tp 读取到聚合体的类型
     * @param[out] data 读取的聚合体数据
     * @return 是否读取成功
     */
    template <typename Tp, typename Enable = std::enable_if_t<std::is_aggregate_v<Tp>>>
    bool read(Tp &data) {
        bool retval{};
        constexpr int MAX_LENGTH = 512, MAX_READ_DST = sizeof(Tp);
        char buffer[MAX_LENGTH]{};
        auto len_result = fdread(buffer, MAX_LENGTH);
        if (len_result > 0 && len_result <= MAX_READ_DST) {
            auto p = std::memcpy(&data, buffer, len_result);
            if (p == &data)
                retval = true;
        }
        return retval;
    }

    /**
     * @brief 不带头尾标志的数据读取，从串口读取字符串
     *
     * @param[out] data 读取的字符串
     * @return 是否读取成功
     */
    bool read(std::string &data);

    template <typename Tp, typename Enable = std::enable_if_t<std::is_aggregate_v<Tp> || std::is_same_v<Tp, std::string>>>
    SerialPort &operator>>(Tp &data) { return (this->read(data), *this); }

    /**
     * @brief 数据写入串口
     * @note 每次写入前会清空缓冲区
     *
     * @tparam Tp 写入聚合体的类型
     * @param[in] data 要写入的聚合体
     * @return 是否写入成功
     */
    template <typename Tp, typename Enable = std::enable_if_t<std::is_aggregate_v<Tp>>>
    bool write(const Tp &data) { return (sizeof(data) == fdwrite(&data, sizeof(data))); }

    /**
     * @brief 写入字符串到串口
     *
     * @param[in] data 待写入的字符串
     * @return 是否写入成功
     */
    bool write(std::string_view data) { return fdwrite(data.data(), data.length()) > 0; }

    template <typename Tp, typename Enable = std::enable_if_t<std::is_aggregate_v<Tp> || std::is_same_v<Tp, std::string_view>>>
    SerialPort &operator<<(const Tp &data) { return (this->write(data), *this); }

    //! 串口是否打开
    inline bool isOpened() const { return _is_open; }

    RMVL_W_SUBST("Serial")

protected:
    //! 写入数据（基于文件描述符）
    long int fdwrite(const void *data, size_t len);

    //! 读取数据（基于文件描述符）
    long int fdread(void *data, size_t len);

    FileDescriptor _fd{INVALID_FD}; //!< 文件描述符（文件句柄）

    bool _is_open{};           //!< 串口打开标志位
    std::string _device;       //!< 设备名
    BaudRate _baud_rate;       //!< 波特率
    SerialReadMode _read_mode; //!< 同步读取模式

private:
    void open();  //! 打开串口
    void close(); //! 关闭串口
};

//! @} io_serial

#if __cplusplus >= 202002L

namespace async {

//! @addtogroup io_serial
//! @{

//! 异步串行接口通信库，仅支持读写字符串
class SerialPort : public ::rm::SerialPort {
public:
    /**
     * @brief 构造新串口对象
     *
     * @param[in] io_context 异步 I/O 执行上下文
     * @param[in] device 设备名
     * @param[in] baud_rate 波特率
     */
    SerialPort(IOContext &io_context, std::string_view device, BaudRate baud_rate);

    //! @cond
    SerialPort(const SerialPort &) = delete;
    SerialPort(SerialPort &&) = default;
    SerialPort &operator=(const SerialPort &) = delete;
    SerialPort &operator=(SerialPort &&) = default;
    ~SerialPort() = default;
    //! @endcond

    //! 串口读等待器
    class SerialReadAwaiter : public AsyncReadAwaiter {
    public:
        /**
         * @brief 创建串口读等待器
         *
         * @param[in] ctx 异步 I/O 执行上下文
         * @param[in] fd 需要监听的文件描述符（文件句柄）
         */
        SerialReadAwaiter(IOContext &ctx, FileDescriptor fd) : AsyncReadAwaiter(ctx, fd) {}

        //! @cond
        std::string await_resume() noexcept;
        //! @endcond
    };

    /**
     * @brief 异步读取串口数据
     *
     * @code {.cpp}
     * // 使用示例
     * std::string str = co_await serial.read();
     * @endcode
     * @return 读取到的字符串数据
     */
    SerialReadAwaiter read() { return {_ctx, _fd}; }

    template <typename Tp>
    SerialPort &operator>>(Tp &) = delete;

    //! 串口写等待器
    class SerialWriteAwaiter : public AsyncWriteAwaiter {
    public:
        /**
         * @brief 创建串口写等待器
         *
         * @param[in] ctx 异步 I/O 执行上下文
         * @param[in] fd 需要监听的文件描述符（文件句柄）
         * @param[in] data 待写入的数据
         */
        SerialWriteAwaiter(IOContext &ctx, FileDescriptor fd, std::string_view data) : AsyncWriteAwaiter(ctx, fd, data) {}

        //! @cond
        bool await_resume() noexcept;
        //! @endcond
    };

    /**
     * @brief 异步写入串口数据
     *
     * @param[in] data 待写入的数据
     * @return 是否写入成功
     * @code {.cpp}
     * // 使用示例
     * bool success = co_await serial.write("12345");
     * @endcode
     */
    SerialWriteAwaiter write(std::string_view data) { return {_ctx, _fd, data}; }

    template <typename Tp>
    SerialPort &operator<<(const Tp &) = delete;

private:
    IOContextRef _ctx; //!< 异步 I/O 执行上下文
};

//! @} io_serial

}; // namespace async

#endif // __cplusplus >= 202002L

} // namespace rm

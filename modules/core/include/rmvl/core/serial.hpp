/**
 * @file serial.hpp
 * @author RoboMaster Vision Community
 * @brief Unix 串口类
 * @version 2.0
 * @date 2018-12-08
 *
 * @copyright Copyright 2018 (c), RoboMaster Vision Community
 *
 */

#pragma once

#include <string>

#ifdef HAVE_TERMIOS_H
#include <termios.h>
#endif // HAVE_TERMIOS_H

namespace rm
{

//! @addtogroup core
//! @{
//! @defgroup serial 串口通信模块
//! @{
//! @brief 基于 GNU C `<termios.h>` 的串行接口通信
//! @} serial
//! @} core

#if defined(HAVE_DIRENT_H) && defined(HAVE_FCNTL_H) && \
    defined(HAVE_UNISTD_H) && defined(HAVE_TERMIOS_H)

//! @addtogroup serial
//! @{

//! 串行接口通信库
class SerialPort
{
    int _fd{};           //!< 文件描述符
    termios _option;     //!< 终端控制
    bool _is_open{};     //!< 串口打开标志位
    std::string _device; //!< 设备名
    int _baud_rate;      //!< 波特率

public:
    /**
     * @brief 构造新 SerialPort 对象，并自动打开
     *
     * @param[in] device 设备名，默认为空
     * @param[in] baud_rate 波特率，默认为 `B115200`
     */
    explicit SerialPort(const std::string &device = {}, int baud_rate = B115200)
        : _device(device), _baud_rate(baud_rate) { open(); }

    //! 析构串口对象
    ~SerialPort() { close(); }

    //! 打开指定串口，或自动搜索所有可用的设备，并尝试打开第一个
    void open();

    //! 关闭串口
    void close();

    /**
     * @brief 从串口读取数据到结构体
     * @note 每次读取后会清空缓冲区
     *
     * @tparam Tp 读取到结构体的类型
     * @param[in] head_flag 头帧
     * @param[in] tail_flag 尾帧
     * @param[out] data 读取的结构体数据
     * @return 是否读取成功
     */
    template <typename Tp>
    inline bool read(unsigned char head_flag, unsigned char tail_flag, Tp &data)
    {
        bool retval{false};
        constexpr int LENGTH = 512;
        constexpr int SIZE = sizeof(Tp);
        unsigned char buffer[LENGTH] = {0};
        ssize_t len_result = fdread(buffer, LENGTH);
        for (ssize_t i = 0; (i + SIZE + 1) < len_result; i++)
            if (buffer[i] == head_flag && buffer[i + SIZE + 1] == tail_flag)
            {
                auto p = memcpy(&data, &buffer[i + 1], SIZE);
                if (p == &data)
                    retval = true;
            }
        return retval;
    }

    /**
     * @brief 结构体数据写入串口
     * @note 每次写入前会清空缓冲区
     *
     * @tparam Tp 写入结构体的类型
     * @param[in] data 要写入的结构体
     * @return 是否写入成功
     */
    template <typename Tp>
    inline bool write(const Tp &data)
    {
        ssize_t len_result = fdwrite(&data, sizeof(data));
        return (sizeof(data) == len_result);
    }

    //! 串口是否打开
    inline bool isOpened() const { return _is_open; };

private:
    /**
     * @brief 写入数据（基于文件描述符）
     *
     * @param[in] data 数据头指针（首地址）
     * @param[in] len 待写入字长
     * @return 是否完整写入
     */
    ssize_t fdwrite(void *data, size_t len);

    /**
     * @brief 读取数据（基于文件描述符）
     *
     * @param[in] data 数据头指针（首地址）
     * @param[in] len 待读入字长
     * @return 读取的数据长度
     */
    ssize_t fdread(void *data, size_t len);
};

//! @} serial

#endif

} // namespace rm

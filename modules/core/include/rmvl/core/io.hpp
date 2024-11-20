/**
 * @file io.hpp
 * @author zhaoxi (535394140@qq.com)
 * @brief 数据 IO 与通信模块
 * @version 2.0
 * @date 2024-10-03
 *
 * @copyright Copyright 2024 (c), zhaoxi
 *
 */

#pragma once

#include <array>
#include <iosfwd>
#include <memory>
#include <string>
#include <vector>

#include "rmvldef.hpp"

namespace rm
{

//! @addtogroup core
//! @{
//! @defgroup core_io 数据 IO 与通信
//! @}

//! @addtogroup core_io
//! @{

////////////////////////////////// 结构化数据读写 //////////////////////////////////

//! 移动位置信息
struct RMVL_EXPORTS_W_AG Translation
{
    RMVL_W_RW float x = 0.f;  //!< x 方向位置、距离（向右运动为正）\f$p_x\f$
    RMVL_W_RW float y = 0.f;  //!< y 方向位置、距离（向下运动为正）\f$p_y\f$
    RMVL_W_RW float z = 0.f;  //!< z 方向位置、距离（向前运动为正）\f$p_z\f$
    RMVL_W_RW float vx = 0.f; //!< x 方向速度（向右运动为正）\f$v_x\f$
    RMVL_W_RW float vy = 0.f; //!< y 方向速度（向下运动为正）\f$v_y\f$
    RMVL_W_RW float vz = 0.f; //!< z 方向速度（向前运动为正）\f$v_z\f$
};

//! 转动姿态信息
struct RMVL_EXPORTS_W_AG Rotation
{
    RMVL_W_RW float yaw = 0.f;         //!< 偏转角（向右运动为正）
    RMVL_W_RW float pitch = 0.f;       //!< 俯仰角（向下运动为正）
    RMVL_W_RW float roll = 0.f;        //!< 滚转角（顺时针运动为正）
    RMVL_W_RW float yaw_speed = 0.f;   //!< 偏转角速度（向右运动为正）
    RMVL_W_RW float pitch_speed = 0.f; //!< 俯仰角速度（向下运动为正）
    RMVL_W_RW float roll_speed = 0.f;  //!< 滚转角速度（顺时针运动为正）
};

//! IMU 数据
struct RMVL_EXPORTS_W_AG ImuData
{
    RMVL_W_RW Translation translation; //!< 移动位置数据
    RMVL_W_RW Rotation rotation;       //!< 转动姿态数据

    /**
     * @brief 导出 IMU 数据，可以是输出到控制台，也可以是输出到文件
     * @brief
     * - 以 `imu_data` 的整体写入到输出流对象的末尾
     *
     * @param[in] out 输出流对象
     * @param[in] data 待写入的 IMU 数据
     */
    static void write(std::ostream &out, const ImuData &data) noexcept;

    /**
     * @brief 导入 IMU 数据，可以是从控制台读取，也可以是从文件读取
     * @brief
     * - 读取的文件形如以下内容
     * @code{.txt}
     * 1.9, 2.11, 3.12, 4.13, 5.14, 6.15, 7.16, 8.17, 9.18, 10.19, 11.20, 12.21,
     * 13.22, 14.23, 15.24, 16.25, 17.26, 18.27, 19.28, 20.29, 21.30, 22.31, 23.32, 24.33,
     * @endcode
     * @brief
     * - 例如，第一次调用 `read` 方法时，IMU 平移的数据为 `(1.9, 2.11, 3.12, 4.13, 5.14, 6.15)`
     *   旋转的数据为 `(7.16, 8.17, 9.18, 10.19, 11.20, 12.21)`
     *
     * @param[in] in 输入流对象
     * @param[out] data 读取出的 IMU 数据
     */
    static void read(std::istream &in, ImuData &data) noexcept;

    /**
     * @brief 导出所有 IMU 数据到文件
     *
     * @param[in] output_file 输出文件路径
     * @param[in] datas 待写入的所有 IMU 数据
     */
    RMVL_W static void write(std::string_view output_file, const std::vector<ImuData> &datas) noexcept;

    /**
     * @brief 从文件导入所有 IMU 数据
     *
     * @param[in] input_file 输入文件路径
     * @return 读取出的所有 IMU 数据
     */
    RMVL_W static std::vector<ImuData> read(std::string_view input_file) noexcept;
};

/// @example samples/tutorial_code/io/sample_read_corners.cpp 角点数据读取例程
/// @example samples/tutorial_code/io/sample_write_corners.cpp 角点数据写入例程

/**
 * @brief 导出角点数据
 * @brief
 * - 以 `corners` 的整体写入到输出流对象的末尾
 * @brief
 * - 文件内容可参见 @ref readCorners
 *
 * @param[in] out 输出流对象
 * @param[in] corners 待写入的角点数据
 */
void writeCorners(std::ostream &out, const std::vector<std::vector<std::array<float, 2>>> &corners);

/**
 * @brief 导入角点数据
 * @brief
 * - 读取的文件形如以下内容
 * @code{.txt}
 * 1.9, 2.11,
 * 3.12, 4.13, 5.14, 6.15,
 * ---
 * 7.16, 8.17,
 * ---
 * @endcode
 * @brief
 * - 第一次调用 `readCorners` 时读取到的内容为 `{(1.9, 2.11)} 和 {(3.12, 4.13), (5.14, 6.15)}`
 *
 * @param[in] in 输入流对象
 * @param[out] corners 读取出的角点数据
 */
void readCorners(std::istream &in, std::vector<std::vector<std::array<float, 2>>> &corners);

///////////////////////////////////// 串口通信 /////////////////////////////////////

enum class BaudRate
{
    BR_57600,  //!< 波特率 57600
    BR_115200, //!< 波特率 115200
    BR_230400, //!< 波特率 230400
};

//! 串行接口通信库
class RMVL_EXPORTS SerialPort
{
public:
    /**
     * @brief 构造新 SerialPort 对象
     *
     * @param[in] device 设备名
     * @param[in] baud_rate 波特率，一般为 `BaudRate::BR_115200`
     */
    SerialPort(std::string_view device, BaudRate baud_rate);

    SerialPort(const SerialPort &) = delete;
    SerialPort(SerialPort &&) = default;
    SerialPort &operator=(const SerialPort &) = delete;
    SerialPort &operator=(SerialPort &&) = default;

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
        constexpr int LENGTH = 512, SIZE = sizeof(Tp);
        unsigned char buffer[LENGTH] = {0};
        auto len_result = fdread(buffer, LENGTH);
        for (long int i = 0; (i + SIZE + 1) < len_result; i++)
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
    inline bool write(const Tp &data) { return (sizeof(data) == fdwrite(&data, sizeof(data))); }

    //! 串口是否打开
    bool isOpened() const;

private:
    //! 写入数据（基于文件描述符）
    long int fdwrite(const void *data, size_t len);

    //! 读取数据（基于文件描述符）
    long int fdread(void *data, size_t len);

    RMVL_IMPL;
};

//////////////////////////////////// 进程间通信 ////////////////////////////////////

//! 命名管道服务端
class RMVL_EXPORTS_W PipeServer
{
    RMVL_IMPL;

public:
    PipeServer(const PipeServer &) = delete;
    PipeServer(PipeServer &&) = default;
    PipeServer &operator=(const PipeServer &) = delete;
    PipeServer &operator=(PipeServer &&) = default;

    /**
     * @brief 在文件系统中创建新的命名管道并打开，销毁时自动移除该管道
     * @note
     * - Windows 命名管道在构造时会等待客户端连接并阻塞，除非已有客户端连接
     *
     * @param[in] name 命名管道名称，Windows 下的命名管道名称为 `\\.\pipe\` +`name`, Linux
     *                 下的命名管道名称为 `/tmp/` + `name`，长度不超过 256 个字符
     */
    RMVL_W PipeServer(std::string_view name);

    /**
     * @brief 从管道读取数据
     *
     * @param[out] data 读取的数据
     * @return 是否读取成功
     */
    bool read(std::string &data);

    //! @cond
    RMVL_W inline std::tuple<bool, std::string> read()
    {
        std::string data;
        return {read(data), data};
    }
    //! @endcond

    inline bool operator>>(std::string &data) { return read(data); }

    /**
     * @brief 向管道写入数据
     *
     * @param[in] data 待写入的数据
     * @return 是否写入成功
     */
    RMVL_W bool write(std::string_view data);

    inline bool operator<<(std::string_view data) { return write(data); }
};

//! 命名管道客户端
class RMVL_EXPORTS_W PipeClient
{
    RMVL_IMPL;

public:
    PipeClient(const PipeClient &) = delete;
    PipeClient(PipeClient &&) = default;
    PipeClient &operator=(const PipeClient &) = delete;
    PipeClient &operator=(PipeClient &&) = default;

    /**
     * @brief 打开存在的命名管道
     *
     * @param[in] name 命名管道名称
     */
    RMVL_W PipeClient(std::string_view name);

    /**
     * @brief 从管道读取数据
     *
     * @param[out] data 读取的数据
     * @return 是否读取成功
     */
    bool read(std::string &data);

    //! @cond
    RMVL_W inline std::tuple<bool, std::string> read()
    {
        std::string data;
        return {read(data), data};
    }
    //! @endcond

    inline bool operator>>(std::string &data) { return read(data); }

    /**
     * @brief 向管道写入数据
     *
     * @param[in] data 待写入的数据
     * @return 是否写入成功
     */
    RMVL_W bool write(std::string_view data);

    inline bool operator<<(std::string_view data) { return write(data); }
};

//! @} core_io

} // namespace rm

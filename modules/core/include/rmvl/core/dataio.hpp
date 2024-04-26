/**
 * @file dataio.hpp
 * @author RoboMaster Vision Community
 * @brief 陀螺仪数据
 * @version 1.0
 * @date 2023-01-12
 *
 * @copyright Copyright 2023 (c), RoboMaster Vision Community
 *
 */

#pragma once

#include <array>
#include <iosfwd>
#include <vector>

namespace rm
{

//! @addtogroup core
//! @{
//!     @defgroup core_dataio 数据读写（I/O）
//! @}

//! @addtogroup core_dataio
//! @{

//! 陀螺仪数据
struct GyroData
{
    //! 移动姿态信息
    struct Translation
    {
        float x = 0.f;  //!< x 方向位置、距离（向右运动为正）\f$p_x\f$
        float y = 0.f;  //!< y 方向位置、距离（向下运动为正）\f$p_y\f$
        float z = 0.f;  //!< z 方向位置、距离（向前运动为正）\f$p_z\f$
        float vx = 0.f; //!< x 方向速度（向右运动为正）\f$v_x\f$
        float vy = 0.f; //!< y 方向速度（向下运动为正）\f$v_y\f$
        float vz = 0.f; //!< z 方向速度（向前运动为正）\f$v_z\f$
    } translation;

    //! 转动姿态信息
    struct Rotation
    {
        float yaw = 0.f;         //!< 偏转角（向右运动为正）
        float pitch = 0.f;       //!< 俯仰角（向下运动为正）
        float roll = 0.f;        //!< 滚转角（顺时针运动为正）
        float yaw_speed = 0.f;   //!< 偏转角速度（向右运动为正）
        float pitch_speed = 0.f; //!< 俯仰角速度（向下运动为正）
        float roll_speed = 0.f;  //!< 滚转角速度（顺时针运动为正）
    } rotation;

    /**
     * @brief 导出陀螺仪数据，可以是输出到控制台，也可以是输出到文件
     * @brief
     * - 以 `gyro_data` 的整体写入到输出流对象的末尾
     *
     * @param[in] out 输出流对象
     * @param[in] data 待写入的陀螺仪数据
     */
    static void write(std::ostream &out, const GyroData &data) noexcept;

    /**
     * @brief 导入陀螺仪数据，可以是从控制台读取，也可以是从文件读取
     * @brief
     * - 读取的文件形如以下内容
     * @code{.txt}
     * 1.9, 2.11, 3.12, 4.13, 5.14, 6.15, 7.16, 8.17, 9.18, 10.19, 11.20, 12.21,
     * 13.22, 14.23, 15.24, 16.25, 17.26, 18.27, 19.28, 20.29, 21.30, 22.31, 23.32, 24.33,
     * @endcode
     * @brief
     * - 例如，第一次调用 `read` 方法时，陀螺仪平移的数据为 `(1.9, 2.11, 3.12, 4.13, 5.14, 6.15)`
     *   旋转的数据为 `(7.16, 8.17, 9.18, 10.19, 11.20, 12.21)`
     *
     * @param[in] in 输入流对象
     * @param[out] data 读取出的陀螺仪数据
     */
    static void read(std::istream &in, GyroData &data) noexcept;
};

/// @example samples/tutorial_code/dataio/sample_read_corners.cpp 角点数据读取例程
/// @example samples/tutorial_code/dataio/sample_write_corners.cpp 角点数据写入例程

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

//! @} core_dataio

} // namespace rm

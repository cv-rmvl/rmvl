/**
 * @file util.hpp
 * @author zhaoxi (535394140@qq.com)
 * @brief 数据 IO 工具库
 * @version 1.0
 * @date 2025-08-15
 *
 * @copyright Copyright 2025 (c), zhaoxi
 *
 */

#pragma once

#include <array>
#include <iosfwd>
#include <string_view>
#include <vector>

#include "rmvl/core/rmvldef.hpp"

namespace rm {

//! @addtogroup io
//! @{

////////////////////////////////// 结构化数据读写 //////////////////////////////////

//! 移动位置信息
struct RMVL_EXPORTS_W_AG Translation {
    RMVL_W_RW float x = 0.f;  //!< x 方向位置、距离（向右运动为正）\f$p_x\f$
    RMVL_W_RW float y = 0.f;  //!< y 方向位置、距离（向下运动为正）\f$p_y\f$
    RMVL_W_RW float z = 0.f;  //!< z 方向位置、距离（向前运动为正）\f$p_z\f$
    RMVL_W_RW float vx = 0.f; //!< x 方向速度（向右运动为正）\f$v_x\f$
    RMVL_W_RW float vy = 0.f; //!< y 方向速度（向下运动为正）\f$v_y\f$
    RMVL_W_RW float vz = 0.f; //!< z 方向速度（向前运动为正）\f$v_z\f$
};

//! 转动姿态信息
struct RMVL_EXPORTS_W_AG Rotation {
    RMVL_W_RW float yaw = 0.f;         //!< 偏转角（向右运动为正）
    RMVL_W_RW float pitch = 0.f;       //!< 俯仰角（向下运动为正）
    RMVL_W_RW float roll = 0.f;        //!< 滚转角（顺时针运动为正）
    RMVL_W_RW float yaw_speed = 0.f;   //!< 偏转角速度（向右运动为正）
    RMVL_W_RW float pitch_speed = 0.f; //!< 俯仰角速度（向下运动为正）
    RMVL_W_RW float roll_speed = 0.f;  //!< 滚转角速度（顺时针运动为正）
};

//! IMU 数据
struct RMVL_EXPORTS_W_AG ImuData {
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

/**
 * @brief 重载 IMU 数据的输入输出流操作符
 *
 * @code {.cpp}
 * // 使用示例
 * ImuData data1{}, data2{};
 * std::cout << data1 << data2 << std::endl;
 * @endcode
 *
 * @param[in] os 输出流对象
 * @param[in] data 待写入的 IMU 数据
 * @return 输出流对象
 */
inline std::ostream &operator<<(std::ostream &os, const ImuData &data) { return ImuData::write(os, data), os; }

/**
 * @brief 重载 IMU 数据的输入输出流操作符
 *
 * @param[in] is 输入流对象
 * @param[out] data 读取出的 IMU 数据
 * @return 输入流对象
 */
inline std::istream &operator>>(std::istream &is, ImuData &data) { return ImuData::read(is, data), is; }

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

//! @} io

} // namespace rm

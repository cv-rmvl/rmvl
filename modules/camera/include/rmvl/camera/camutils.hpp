/**
 * @file camutils.hpp
 * @author zhaoxi (535394140@qq.com)
 * @brief 相机基本配置信息
 * @version 1.0
 * @date 2022-09-30
 *
 * @copyright Copyright 2023 (c), zhaoxi
 *
 */

#pragma once

#include <opencv2/core.hpp>

#include "rmvl/core/rmvldef.hpp"

namespace rm {

//! @addtogroup camera
//! @{

//! 相机外部触发通道
enum class TriggerChannel : uint8_t {
    Chn0, //!< 通道 0
    Chn1, //!< 通道 1
    Chn2, //!< 通道 2
    Chn3  //!< 通道 3
};

//! 相机采集模式
enum class GrabMode : uint8_t {
    Continuous, //!< 连续采样
    Software,   //!< 软触发
    Hardware,   //!< 硬触发
    RotaryEnc   //!< 旋转编码器触发
};

//! 相机句柄创建方式
enum class HandleMode : uint8_t {
    Index, //!< 相机的索引号 `(0, 1, 2 ...)`
    Key,   //!< 制造商：序列号 S/N
    MAC,   //!< 相机的 MAC 地址
    IP     //!< IP 地址
};

//! 相机数据处理模式
enum class RetrieveMode : uint8_t {
    OpenCV, //!< 使用 OpenCV 的 'cvtColor' 进行处理
    SDK,    //!< 使用官方 SDK 进行处理
};

//! 相机初始化配置模式
struct RMVL_EXPORTS_W_AG CameraConfig {
    RMVL_W_RW TriggerChannel trigger_channel{TriggerChannel::Chn1}; //!< 触发通道
    RMVL_W_RW GrabMode grab_mode{GrabMode::Continuous};             //!< 采集模式
    RMVL_W_RW HandleMode handle_mode{HandleMode::Key};              //!< 句柄创建方式
    RMVL_W_RW RetrieveMode retrieve_mode{RetrieveMode::OpenCV};     //!< 数据处理模式

    /**
     * @brief 创建相机初始化配置模式
     *
     * @param[in] modes 配置模式参数包
     */
    template <typename... Args>
    static inline CameraConfig create(Args &&...modes) {
        CameraConfig config;
        config.set(std::forward<Args>(modes)...);
        return config;
    }

    /**
     * @brief 设置配置模式
     *
     * @param[in] modes 配置模式参数包
     */
    template <typename... Args>
    inline void set(Args... modes) { (..., conf(modes)); }

private:
    inline void conf(TriggerChannel chn) { trigger_channel = chn; }
    inline void conf(GrabMode mode) { grab_mode = mode; }
    inline void conf(HandleMode mode) { handle_mode = mode; }
    inline void conf(RetrieveMode mode) { retrieve_mode = mode; }
};

//! 相机运行时属性
enum class CameraProperties : uint8_t {
    // ---------------- 设备属性 ----------------
    auto_exposure,  //!< 自动曝光
    auto_wb,        //!< 自动白平衡
    exposure,       //!< 曝光值
    gain,           //!< 模拟增益
    gamma,          //!< Gamma 值
    wb_rgain,       //!< 白平衡红色分量
    wb_ggain,       //!< 白平衡绿色分量
    wb_bgain,       //!< 白平衡蓝色分量
    contrast,       //!< 对比度
    saturation,     //!< 饱和度
    sharpness,      //!< 锐度
    frame_height,   //!< 图像帧高度
    frame_width,    //!< 图像帧宽度
    trigger_delay,  //!< 硬触发采集延迟（微秒\f$μs\f$）
    trigger_count,  //!< 单次触发时的触发帧数
    trigger_period, //!< 单次触发时多次采集的周期（微秒\f$μs\f$）
};

//! 相机运行时事件
enum CameraEvents : uint8_t {
    once_exposure, //!< 单次曝光
    once_wb,       //!< 执行单次白平衡
    software,      //!< 执行软触发
};

//! 相机外参
class RMVL_EXPORTS_W CameraExtrinsics {
public:
    //! 获取平移向量
    RMVL_W inline const cv::Vec3f &tvec() const { return _tvec; }
    //! 获取旋转向量
    RMVL_W inline const cv::Vec3f &rvec() const { return _rvec; }
    //! 获取旋转矩阵
    RMVL_W inline const cv::Matx33f &R() const { return _r; }
    //! 获取外参矩阵
    RMVL_W inline const cv::Matx44f &T() const { return _t; }
    //! 获取yaw
    RMVL_W inline float yaw() const { return _yaw; }
    //! 获取pitch
    RMVL_W inline float pitch() const { return _pitch; }
    //! 获取roll
    RMVL_W inline float roll() const { return _roll; }
    //! 获取距离
    RMVL_W inline float distance() const { return _distance; }

    /**
     * @brief 设置平移向量
     *
     * @param[in] tvec 平移向量
     */
    RMVL_W void tvec(const cv::Vec3f &tvec);

    /**
     * @brief 设置旋转向量
     *
     * @param[in] rvec 旋转向量
     */
    RMVL_W void rvec(const cv::Vec3f &rvec);

    /**
     * @brief 设置旋转矩阵
     *
     * @param[in] R 旋转矩阵
     */
    RMVL_W void R(const cv::Matx33f &R);

    /**
     * @brief 设置距离
     *
     * @param[in] distance 距离
     */
    RMVL_W inline void distance(float distance) { _distance = distance; }

private:
    float _yaw{};                        //!< 外参 Yaw 角
    float _pitch{};                      //!< 外参 Pitch 角
    float _roll{};                       //!< 外参 Roll 角
    float _distance{};                   //!< 物距
    cv::Vec3f _tvec;                     //!< 平移向量
    cv::Vec3f _rvec;                     //!< 旋转向量
    cv::Matx33f _r = cv::Matx33f::eye(); //!< 旋转矩阵
    cv::Matx44f _t = cv::Matx44f::eye(); //!< 外参矩阵
};

//! @} camera

} // namespace rm

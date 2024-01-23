/**
 * @file camutils.hpp
 * @author RoboMaster Vision Community
 * @brief
 * @version 1.0
 * @date 2022-09-30
 *
 * @copyright Copyright 2023 (c), RoboMaster Vision Community
 *
 */

#pragma once

#include <opencv2/core.hpp>

namespace rm
{

//! @addtogroup camera
//! @{

//! 相机外部触发通道
enum class TriggerChannel : uint8_t
{
    Chn0, //!< 通道 0
    Chn1, //!< 通道 1
    Chn2, //!< 通道 2
    Chn3  //!< 通道 3
};

//! 相机采集模式
enum class GrabMode : uint8_t
{
    Continuous, //!< 连续采样
    Software,   //!< 软触发
    Hardware,   //!< 硬触发
    RotaryEnc   //!< 旋转编码器触发
};

//! 相机句柄创建方式
enum class HandleMode : uint8_t
{
    Index, //!< 相机的索引号 `(0, 1, 2 ...)`
    Key,   //!< 制造商：序列号 S/N
    ID,    //!< 手动设置的相机 ID
    IP     //!< IP 地址
};

//! 相机数据处理模式
enum class RetrieveMode : uint8_t
{
    OpenCV, //!< 使用 OpenCV 的 'cvtColor' 进行处理
    SDK,    //!< 使用官方 SDK 进行处理
};

//! 相机初始化配置模式
struct CameraConfig
{
    TriggerChannel trigger_channel : 4 {TriggerChannel::Chn1}; //!< 触发通道
    GrabMode grab_mode : 4 {GrabMode::Continuous};             //!< 采集模式
    HandleMode handle_mode : 4 {HandleMode::Key};              //!< 句柄创建方式
    RetrieveMode retrieve_mode : 4 {RetrieveMode::OpenCV};     //!< 数据处理模式

    /**
     * @brief 设置触发通道
     *
     * @param[in] chn 触发通道
     */
    inline CameraConfig &set(TriggerChannel chn)
    {
        trigger_channel = chn;
        return *this;
    }

    /**
     * @brief 设置采集模式
     *
     * @param[in] mode 采集模式
     */
    inline CameraConfig &set(GrabMode mode)
    {
        grab_mode = mode;
        return *this;
    }

    /**
     * @brief 设置句柄创建方式
     *
     * @param[in] mode 句柄创建方式
     */
    inline CameraConfig &set(HandleMode mode)
    {
        handle_mode = mode;
        return *this;
    }

    /**
     * @brief 设置数据处理模式
     *
     * @param[in] mode 数据处理模式
     */
    inline CameraConfig &set(RetrieveMode mode)
    {
        retrieve_mode = mode;
        return *this;
    }
};

//! 相机运行时属性
enum CameraProperties : uint16_t
{
    // ---------------- 设备属性 ----------------
    CAMERA_AUTO_EXPOSURE = 0x1001,   //!< 自动曝光
    CAMERA_AUTO_WB = 0x1002,         //!< 自动白平衡
    CAMERA_MANUAL_EXPOSURE = 0x1003, //!< 手动曝光
    CAMERA_MANUAL_WB = 0x1004,       //!< 手动白平衡
    CAMERA_EXPOSURE = 0x1005,        //!< 曝光值
    CAMERA_GAIN = 0x1006,            //!< 模拟增益
    CAMERA_GAMMA = 0x1007,           //!< Gamma 值
    CAMERA_WB_RGAIN = 0x1008,        //!< 白平衡红色分量
    CAMERA_WB_GGAIN = 0x1009,        //!< 白平衡绿色分量
    CAMERA_WB_BGAIN = 0x100a,        //!< 白平衡蓝色分量
    CAMERA_CONTRAST = 0x100c,        //!< 对比度
    CAMERA_SATURATION = 0x100d,      //!< 饱和度
    CAMERA_SHARPNESS = 0x100e,       //!< 锐度
    CAMERA_FRAME_HEIGHT = 0x100f,    //!< 图像帧高度
    CAMERA_FRAME_WIDTH = 0x1010,     //!< 图像帧宽度

    // ---------------- 处理属性 ----------------
    CAMERA_TRIGGER_DELAY = 0x1101,  //!< 硬触发采集延迟（微秒\f$μs\f$）
    CAMERA_TRIGGER_COUNT = 0x1102,  //!< 单次触发时的触发帧数
    CAMERA_TRIGGER_PERIOD = 0x1103, //!< 单次触发时多次采集的周期（微秒\f$μs\f$）
};

//! 相机运行时事件
enum CameraEvents : uint16_t
{
    CAMERA_ONCE_WB = 0x2001,     //!< 执行一次白平衡
    CAMERA_SOFT_TRIGGER = 0x2002 //!< 执行软触发
};

//! 相机外参
class CameraExtrinsics
{
    float _yaw{};
    float _pitch{};
    float _roll{};
    float _distance{};
    cv::Vec3f _tvec;
    cv::Vec3f _rvec;
    cv::Matx33f _r = cv::Matx33f::eye();
    cv::Matx44f _t = cv::Matx44f::eye();

public:
    //! 获取平移向量
    inline const cv::Vec3f &tvec() const { return _tvec; }
    //! 获取旋转向量
    inline const cv::Vec3f &rvec() const { return _rvec; }
    //! 获取旋转矩阵
    inline const cv::Matx33f &R() const { return _r; }
    //! 获取外参矩阵
    inline const cv::Matx44f &T() const { return _t; }
    //! 获取yaw
    inline float yaw() const { return _yaw; }
    //! 获取pitch
    inline float pitch() const { return _pitch; }
    //! 获取roll
    inline float roll() const { return _roll; }
    //! 获取距离
    inline float distance() const { return _distance; }

    /**
     * @brief 设置平移向量
     *
     * @param[in] tvec 平移向量
     */
    void tvec(const cv::Vec3f &tvec);

    /**
     * @brief 设置旋转向量
     *
     * @param[in] rvec 旋转向量
     */
    void rvec(const cv::Vec3f &rvec);

    /**
     * @brief 设置旋转矩阵
     *
     * @param[in] R 旋转矩阵
     */
    void R(const cv::Matx33f &R);

    /**
     * @brief 设置距离
     *
     * @param[in] distance 距离
     */
    inline void distance(float distance) { _distance = distance; }
};

//! @} camera

} // namespace rm

/**
 * @file hik_camera_impl.h
 * @author zhaoxi (535394140@qq.com)
 * @brief 海康相机实现
 * @version 1.0
 * @date 2023-12-13
 *
 * @copyright Copyright 2023 (c), zhaoxi
 *
 */

#pragma once

#include <MvCameraControl.h>

#include "rmvl/camera/hik_camera.h"

namespace rm
{

class HikCamera::Impl
{
    // -------------------------- 相机信息 --------------------------
    void *_handle;                   //!< 相机设备句柄
    MV_CC_DEVICE_INFO_LIST _devices; //!< 设备信息列表
    CameraConfig _init_mode;         //!< 初始化配置模式
    std::string _serial;             //!< 相机序列号 S/N
    bool _opened{};                  //!< 相机是否打开

    // -------------------------- 图像信息 --------------------------
    MV_FRAME_OUT _p_out;          //!< 输出图像的数据及信息
    std::vector<uchar> _p_dstbuf; //!< 输出数据缓存

public:
    /**
     * @brief 构造函数
     *
     * @param init_mode 相机初始化配置模式
     * @param serial 相机唯一序列号
     */
    Impl(CameraConfig init_mode, std::string_view serial) noexcept;

    //! 析构函数
    ~Impl() noexcept;

    /**
     * @brief 设置相机参数/事件
     *
     * @param propId 参数/事件编号
     * @param value 参数/事件值
     * @return 是否设置成功
     */
    bool set(int propId, double value) noexcept;

    /**
     * @brief 获取相机参数
     *
     * @param propId 参数编号
     * @return 参数值
     */
    double get(int propId) const noexcept;

    //! 相机是否打开
    inline bool isOpened() const noexcept { return _opened; }

    //! 释放相机资源
    void release() noexcept;

    /**
     * @brief 相机处理
     *
     * @param image 输出图像
     * @param flag 相机处理模式
     * @return 是否成功处理
     */
    bool retrieve(cv::OutputArray image, RetrieveMode flag) noexcept;

    /**
     * @brief 从相机设备中读取图像
     *
     * @param image 待读入的图像
     * @return 是否读取成功
     */
    bool read(cv::OutputArray image) noexcept;

    //! 打开相机
    bool open() noexcept;

    //! 相机重连
    bool reconnect() noexcept;

private:
    //! 错误码转字符串
    const char *errorCode2Str(unsigned int code) noexcept;
};

} // namespace rm

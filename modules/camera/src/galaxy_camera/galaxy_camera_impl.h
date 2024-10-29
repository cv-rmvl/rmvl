/**
 * @file galaxy_camera_impl.h
 * @author zhaoxi (535394140@qq.com)
 * @brief
 * @version 1.0
 * @date 2024-10-27
 *
 * @copyright Copyright 2024 (c), zhaoxi
 *
 */

#pragma once

#include "rmvl/camera/galaxy_camera.h"

namespace rm
{

class GalaxyCamera::Impl
{
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

    //! 打开相机
    bool open() noexcept;

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

    //! 相机重连
    bool reconnect() noexcept;

private:
    CameraConfig _config{};  //!< 相机配置
    std::string _id{};       //!< 相机唯一标识
    GX_DEV_HANDLE _handle{}; //!< 相机句柄
    bool _opened{};          //!< 相机是否打开

#ifdef __linux__
    PGX_FRAME_BUFFER _buffer{}; //!< 帧缓存
#else
    GX_FRAME_DATA _data{}; //!< 帧数据
    int64_t _payload{};    //!< 负载大小
#endif // __linux__
};

//! 获取 GX_STATUS 错误信息
const char *getGXError() noexcept;

} // namespace rm

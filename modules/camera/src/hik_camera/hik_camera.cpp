/**
 * @file hik_camera.cpp
 * @author RoboMaster Vision Community
 * @brief HikRobot 工业相机库
 * @version 1.0
 * @date 2023-06-13
 *
 * @copyright Copyright 2023 (c), RoboMaster Vision Community
 *
 */

#include <unistd.h>
#include <unordered_set>

#include <opencv2/imgproc.hpp>

#include "rmvl/camera/hik_camera.h"

namespace rm
{

//! MV_CC_PIXEL_CONVERT_PARAM 的初始化变量
static MV_CC_PIXEL_CONVERT_PARAM MV_CC_PIXEL_CONVERT_PARAM_Init =
    {.nWidth = 0,
     .nHeight = 0,
     .enSrcPixelType = PixelType_Gvsp_Undefined,
     .pSrcData = nullptr,
     .nSrcDataLen = 0,
     .enDstPixelType = PixelType_Gvsp_Undefined,
     .pDstBuffer = nullptr,
     .nDstLen = 0,
     .nDstBufferSize = 0,
     .nRes = {0}};

HikCamera::HikCamera(GrabMode grab_mode, RetrieveMode retrieve_mode, std::string_view serial)
    : _grab_mode(grab_mode), _retrieve_mode(retrieve_mode), _serial(serial)
{
    _opened = open();
}

HikCamera::~HikCamera()
{
    release();
}

void HikCamera::release()
{
    // 停止取流
    auto ret = MV_CC_StopGrabbing(_handle);
    if (ret != MV_OK)
    {
        ERROR_("failed to stop grabbing (error: \"%s\")", errorCode2Str(ret));
        return;
    }
    // 关闭设备
    ret = MV_CC_CloseDevice(_handle);
    if (ret != MV_OK)
    {
        ERROR_("failed to close device (error: \"%s\")", errorCode2Str(ret));
        return;
    }
    // 销毁句柄
    ret = MV_CC_DestroyHandle(_handle);
    if (ret != MV_OK)
    {
        ERROR_("failed to destroy handle (error: \"%s\")", errorCode2Str(ret));
        return;
    }
}

bool HikCamera::open()
{
    // ----------------------- 设备枚举 -----------------------
    int ret = MV_OK;
    ret = MV_CC_EnumDevices(MV_USB_DEVICE, &_devices);
    if (ret == MV_OK)
        PASS_("camera enum status: %s", errorCode2Str(ret));
    else
        INFO_("camera enum status: %s", errorCode2Str(ret));
    unsigned int nums = _devices.nDeviceNum;
    INFO_("camera quantity: %u", nums);
    // 无设备连接
    if (nums == 0)
    {
        ERROR_("could not find the camera devise.");
        return false;
    }
    // ------------------ 选择设备并创建句柄 ------------------
    std::unordered_map<std::string, MV_CC_DEVICE_INFO *> sn_device;
    // 设备列表指针数组
    auto device_info_arr = _devices.pDeviceInfo;
    // 哈希匹配
    for (decltype(nums) i = 0; i < nums; ++i)
    {
        if (device_info_arr[i] == nullptr)
            break;
        std::string sn = reinterpret_cast<char *>(device_info_arr[i]->SpecialInfo.stUsb3VInfo.chSerialNumber);
        sn_device[sn] = device_info_arr[i];
    }
    // 查找指定设备，否则选取第一个设备
    if (sn_device.find(_serial) != sn_device.end())
        ret = MV_CC_CreateHandle(&_handle, sn_device[_serial]);
    else
        ret = MV_CC_CreateHandle(&_handle, device_info_arr[0]);
    if (ret != MV_OK)
    {
        ERROR_("failed to create handle (error: \"%s\")", errorCode2Str(ret));
        return false;
    }
    // ----------------------- 打开相机 -----------------------
    ret = MV_CC_OpenDevice(_handle);
    if (ret != MV_OK)
    {
        ERROR_("failed to open device (error: \"%s\")", errorCode2Str(ret));
        return false;
    }
    // --------------------- 设置工作模式 ---------------------
    if (_grab_mode == GRAB_CONTINUOUS)
        ret = MV_CC_SetEnumValue(_handle, "TriggerMode", 0); // 连续采样
    else
        ret = MV_CC_SetEnumValue(_handle, "TriggerMode", 1); // 触发模式
    if (ret != MV_OK)
    {
        ERROR_("failed to set trigger mode (error: \"%s\")", errorCode2Str(ret));
        return false;
    }
    // 开始取流
    ret = MV_CC_StartGrabbing(_handle);
    if (ret != MV_OK)
    {
        ERROR_("failed to start grabbing (error: \"%s\")", errorCode2Str(ret));
        return false;
    }
    return true;
}

/**
 * @brief MvGvspPixelType 转换为 cv::ColorConversionCodes
 *
 * @param[in] pixel_type
 * @return cv::ColorConversionCodes
 */
inline cv::ColorConversionCodes pixelType2CVType(MvGvspPixelType pixel_type)
{
    static std::unordered_map<MvGvspPixelType, cv::ColorConversionCodes> convertion =
        {{PixelType_Gvsp_BayerGR8, cv::COLOR_BayerGR2RGB},
         {PixelType_Gvsp_BayerRG8, cv::COLOR_BayerRG2RGB},
         {PixelType_Gvsp_BayerGB8, cv::COLOR_BayerGB2RGB},
         {PixelType_Gvsp_BayerBG8, cv::COLOR_BayerBG2RGB}};
    return convertion[pixel_type];
}

bool HikCamera::retrieve(cv::OutputArray image, RetrieveMode flag)
{
    // --------------------- 前置信息准备 ---------------------
    auto &frame_info = _p_out.stFrameInfo;
    // 当前格式
    auto pixel_type = frame_info.enPixelType;
    // 单通道标志位集合
    std::unordered_set<MvGvspPixelType> mono_set =
        {PixelType_Gvsp_Mono1p, PixelType_Gvsp_Mono2p, PixelType_Gvsp_Mono4p,
         PixelType_Gvsp_Mono8, PixelType_Gvsp_Mono8_Signed,
         PixelType_Gvsp_Mono10, PixelType_Gvsp_Mono10_Packed,
         PixelType_Gvsp_Mono12, PixelType_Gvsp_Mono12_Packed,
         PixelType_Gvsp_Mono14, PixelType_Gvsp_Mono16};
    if (pixel_type == PixelType_Gvsp_Undefined)
    {
        ERROR_("undefined pixel type");
        image.assign(cv::Mat());
        return false;
    }
    // 通道数
    uint channel = mono_set.find(pixel_type) == mono_set.end() ? 3 : 1;
    auto frame_size = frame_info.nWidth * frame_info.nHeight * channel;
    if (_p_dstbuf.size() != frame_size)
        _p_dstbuf.resize(frame_size);
    // ---------------------- 解码、转码 ----------------------
    auto ret = MV_OK;
    // MV_CC_ConvertPixelType
    if (flag == RETRIEVE_SDK)
    {
        MV_CC_PIXEL_CONVERT_PARAM cvt_param = MV_CC_PIXEL_CONVERT_PARAM_Init;
        cvt_param.nWidth = frame_info.nWidth;              // 图像宽
        cvt_param.nHeight = frame_info.nHeight;            // 图像高
        cvt_param.pSrcData = _p_out.pBufAddr;              // 输入数据缓存
        cvt_param.nSrcDataLen = frame_info.nFrameLen;      // 输入数据大小
        cvt_param.enSrcPixelType = frame_info.enPixelType; // 源像素格式
        cvt_param.enDstPixelType = channel == 1            // 目标像素格式
                                       ? PixelType_Gvsp_Mono8
                                       : PixelType_Gvsp_BGR8_Packed;
        cvt_param.pDstBuffer = _p_dstbuf.data();     // 输出数据缓存
        cvt_param.nDstBufferSize = _p_dstbuf.size(); // 提供的输出缓冲区大小
        ret = MV_CC_ConvertPixelType(_handle, &cvt_param);
        if (ret == MV_OK)
        {
            image.assign(cv::Mat(cv::Size(cvt_param.nWidth, cvt_param.nHeight),
                                 channel == 1 ? CV_8UC1 : CV_8UC3, _p_dstbuf.data()));
            return true;
        }
        ERROR_("failed to convert the pixel type (error: \"%s\")", errorCode2Str(ret));
    }
    // cv::cvtColor
    else if (flag == RETRIEVE_CV)
    {
        cv::Mat src_image(cv::Size(frame_info.nWidth, frame_info.nHeight), CV_8U, _p_out.pBufAddr);
        if (channel == 1)
            image.assign(src_image);
        else
        {
            cv::Mat dst_image;
            cv::cvtColor(src_image, dst_image, pixelType2CVType(pixel_type));
            image.assign(dst_image);
        }
        return true;
    }
    // 无效参数
    else
        ERROR_("failed to retrieve, invalid retrieve mode: %d.", _retrieve_mode);
    // 处理失败默认操作
    image.assign(cv::Mat());
    return false;
}

bool HikCamera::read(cv::OutputArray image)
{
    // 获取图像地址
    auto ret = MV_CC_GetImageBuffer(_handle, &_p_out, 1000);
    if (ret == MV_OK)
        retrieve(image, _retrieve_mode);
    else
    {
        WARNING_("No data in getting image buffer");
        reconnect();
    }
    // 释放图像缓存
    if (_p_out.pBufAddr != nullptr)
    {
        ret = MV_CC_FreeImageBuffer(_handle, &_p_out);
        if (ret != MV_OK)
        {
            ERROR_("failed to free image buffer (error: \"%s\")", errorCode2Str(ret));
            return false;
        }
    }
    return !image.empty();
}

bool HikCamera::reconnect()
{
    INFO_("camera device reconnect");
    release();
    sleep(1);
    open();
    return true;
}

} // namespace rm

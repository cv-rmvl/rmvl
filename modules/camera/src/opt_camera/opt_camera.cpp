/**
 * @file opt_camera.cpp
 * @author zhaoxi (535394140@qq.com)
 * @brief 奥普特机器视觉相机库
 * @version 1.0
 * @date 2023-12-15
 *
 * @copyright Copyright 2023 (c), zhaoxi
 *
 */

#include <opencv2/imgproc.hpp>
#include <thread>

#include "opt_camera_impl.h"

namespace rm
{

RMVL_IMPL_DEF(OptCamera)

OptCamera::OptCamera(CameraConfig init_mode, std::string_view handle_info) : _impl(new OptCamera::Impl(init_mode, handle_info)) {}
OptCamera::~OptCamera() { delete _impl; }
bool OptCamera::set(int propId, double value) { return _impl->set(propId, value); }
double OptCamera::get(int propId) const { return _impl->get(propId); }
bool OptCamera::isOpened() const { return _impl->isOpened(); }
bool OptCamera::read(cv::OutputArray image) { return _impl->read(image); }
bool OptCamera::reconnect() { return _impl->reconnect(); }

/**
 * @brief 将 OPT 的像素类型转换为 OpenCV 的图像类型
 *
 * @param pixel_type
 * @return cv::ColorConversionCodes
 */
static inline cv::ColorConversionCodes optPixelType2CVType(OPT_EPixelType pixel_type)
{
    static std::unordered_map<OPT_EPixelType, cv::ColorConversionCodes> convertion =
        {{gvspPixelBayGR8, cv::COLOR_BayerGR2RGB},
         {gvspPixelBayRG8, cv::COLOR_BayerRG2RGB},
         {gvspPixelBayGB8, cv::COLOR_BayerGB2RGB},
         {gvspPixelBayBG8, cv::COLOR_BayerBG2RGB}};
    return convertion[pixel_type];
}

/**
 * @brief 将 OPT 的错误码转换为字符串
 *
 * @param status
 * @return constexpr const char*
 */
static constexpr const char *optGetErrorString(int status)
{
    switch (status)
    {
    case OPT_ERROR:
        return "Generic error";
    case OPT_INVALID_HANDLE:
        return "Error or invalid handle";
    case OPT_INVALID_PARAM:
        return "Incorrect parameter";
    case OPT_INVALID_FRAME_HANDLE:
        return "Error or invalid frame handle";
    case OPT_INVALID_FRAME:
        return "Invalid frame";
    case OPT_INVALID_RESOURCE:
        return "Camera / Event / Stream and so on resource invalid";
    case OPT_INVALID_IP:
        return "Device's and PC's subnet is mismatch IP";
    case OPT_NO_MEMORY:
        return "Malloc memery failed";
    case OPT_INSUFFICIENT_MEMORY:
        return "Insufficient memory";
    case OPT_ERROR_PROPERTY_TYPE:
        return "Property type error";
    case OPT_INVALID_ACCESS:
        return "Property not accessible, or not be read / written, or read / written failed";
    case OPT_INVALID_RANGE:
        return "The property's value is out of range, or is not integer multiple of the step";
    case OPT_NOT_SUPPORT:
        return "Device not supported function";
    default:
        return "Successed, no error";
    }
}

OptCamera::Impl::Impl(CameraConfig init_mode, std::string_view handle_info) noexcept
    : _init_mode(init_mode), _camera_info(handle_info) { open(); }

OptCamera::Impl::~Impl() noexcept
{
    if (_handle != nullptr)
        release();
}

bool OptCamera::Impl::open() noexcept
{
    // 提取初始化模式
    auto handle_mode = _init_mode.handle_mode;
    auto grab_mode = _init_mode.grab_mode;
    auto trigger_chn = _init_mode.trigger_channel;
    // 第一次置零相机状态用于判断是否检测到相机
    int status = OPT_EnumDevices(&_device_list, interfaceTypeAll);
    int camera_counts = _device_list.nDevNum;
    INFO_("opt - Camera enum status: %s", optGetErrorString(status));
    INFO_("opt - Camera counts: %d", camera_counts);
    // 没有连接设备
    if (camera_counts == 0)
    {
        ERROR_("opt - Could not find the camera devise.");
        return false;
    }
    // 创建设备句柄
    static constexpr OPT_ECreateHandleMode handle_map[4] = {modeByIndex, modeByCameraKey, modeByDeviceUserID, modeByIPAddress};
    status = OPT_CreateHandle(&_handle, handle_map[static_cast<size_t>(handle_mode)], reinterpret_cast<void *>(const_cast<char *>(_camera_info.c_str())));
    if (status != OPT_OK)
    {
        ERROR_("opt - Failed to create camera handle! %s", optGetErrorString(status));
        return false;
    }
    // 打开相机
    status = OPT_Open(_handle);
    if (status != OPT_OK)
    {
        ERROR_("opt - Failed to open camera! %s", optGetErrorString(status));
        return false;
    }
    // 设置采集方式
    status = OPT_SetEnumFeatureSymbol(_handle, "AcquisitionMode", grab_mode == GrabMode::Continuous ? "Continuous" : "SingleFrame");
    if (status != OPT_OK)
    {
        ERROR_("opt - Failed to set acquisition mode: %s", optGetErrorString(status));
        return false;
    }
    // 设置触发模式，连续采集不采用触发模式
    OPT_SetEnumFeatureSymbol(_handle, "TriggerMode", grab_mode == GrabMode::Continuous ? "Off" : "On");
    if (status != OPT_OK)
    {
        ERROR_("opt - Failed to set trigger mode: %s", optGetErrorString(status));
        return false;
    }
    // 为单帧采集（触发模式下）设置触发源
    if (grab_mode != GrabMode::Continuous)
    {
        // 为硬触发设置触发通道
        if (grab_mode == GrabMode::Hardware)
        {
            if (trigger_chn == TriggerChannel::Chn1)
                status = OPT_SetEnumFeatureSymbol(_handle, "TriggerSource", "Line1");
            else if (trigger_chn == TriggerChannel::Chn2)
                status = OPT_SetEnumFeatureSymbol(_handle, "TriggerSource", "Line2");
        }
        else
            status = OPT_SetEnumFeatureSymbol(_handle, "TriggerSource", "Software");
    }
    if (status != OPT_OK)
    {
        ERROR_("opt - Failed to set trigger source: %s", optGetErrorString(status));
        return false;
    }
    return true;
}

bool OptCamera::Impl::read(cv::OutputArray image) noexcept
{
    if (!OPT_IsGrabbing(_handle))
    {
        auto status = OPT_StartGrabbing(_handle);
        if (status != OPT_OK)
        {
            ERROR_("opt - Failed to start grabbing: %s", optGetErrorString(status));
            return false;
        }
    }
    return retrieve(image);
}

bool OptCamera::Impl::retrieve(cv::OutputArray image) noexcept
{
    // 获取图像
    int status = OPT_GetFrame(_handle, &_src_frame, 1000);
    if (status != OPT_OK)
    {
        ERROR_("opt - Failed to get frame: %s", optGetErrorString(status));
        image.assign(cv::Mat());
        return false;
    }
    // 图像格式
    OPT_FrameInfo &frame_info = _src_frame.frameInfo;
    // 像素格式
    OPT_EPixelType &pixel_format = _src_frame.frameInfo.pixelFormat;
    if (pixel_format != gvspPixelMono8 && pixel_format != gvspPixelBayRG8)
    {
        ERROR_("opt - Unknown pixel format, only support Mono8 and BayRG8.");
        image.assign(cv::Mat());
        return false;
    }
    // SDK 参数处理
    RetrieveMode flag = _init_mode.retrieve_mode;
    if (flag == RetrieveMode::SDK)
    {
        uchar *bgr_data = nullptr;
        if (pixel_format == gvspPixelBayRG8) // BGR
        {
            uint dstBufSize = 3 * frame_info.width * frame_info.height;
            bgr_data = new uchar[dstBufSize];
            OPT_PixelConvertParam convert_param;
            convert_param.nWidth = frame_info.width;
            convert_param.nHeight = frame_info.height;
            convert_param.ePixelFormat = pixel_format;
            convert_param.nPaddingX = frame_info.paddingX;
            convert_param.nPaddingY = frame_info.paddingY;
            convert_param.eBayerDemosaic = demosaicNearestNeighbor;
            convert_param.eDstPixelFormat = gvspPixelBGR8;
            convert_param.pDstBuf = bgr_data;
            convert_param.nDstBufSize = dstBufSize;
            OPT_PixelConvert(_handle, &convert_param);
        }
        else
            bgr_data = _src_frame.pData;
        if (bgr_data)
        {
            image.assign(cv::Mat(cv::Size(frame_info.width, frame_info.height),
                                 pixel_format == gvspPixelMono8 ? CV_8UC1 : CV_8UC3, bgr_data));
            OPT_ReleaseFrame(_handle, &_src_frame);
            delete[] bgr_data;
            return true;
        }
    }
    // cvtColor
    else if (flag == RetrieveMode::OpenCV)
    {
        cv::Mat bayer_img(cv::Size(frame_info.width, frame_info.height), CV_8U, _src_frame.pData);
        if (pixel_format == gvspPixelMono8)
            image.assign(bayer_img);
        else // gvspPixelBayRG8
        {
            cv::Mat bgr_img;
            cvtColor(bayer_img, bgr_img, optPixelType2CVType(pixel_format));
            image.assign(bgr_img);
        }
        OPT_ReleaseFrame(_handle, &_src_frame);
        return true;
    };
    // retrieve failed
    ERROR_("opt - Failed to retrieve, unsupported mode: %d.", static_cast<int>(flag));
    image.assign(cv::Mat());
    return false;
}

void OptCamera::Impl::release() noexcept
{
    if (OPT_IsOpen(_handle))
    {
        if (OPT_IsGrabbing(_handle))
        {
            OPT_StopGrabbing(_handle);
            OPT_ClearFrameBuffer(_handle);
            OPT_ReleaseFrame(_handle, &_src_frame);
        }
        OPT_Close(_handle);
        OPT_DestroyHandle(_handle);
    }
}

bool OptCamera::Impl::reconnect() noexcept
{
    using namespace std::chrono_literals;
    WARNING_("opt - Reconnecting...");
    release();
    std::this_thread::sleep_for(200ms);
    return open();
}

} // namespace rm

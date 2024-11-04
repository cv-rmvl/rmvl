/**
 * @file galaxy_camera.cpp
 * @author zhaoxi (535394140@qq.com)
 * @brief
 * @version 1.0
 * @date 2024-10-27
 *
 * @copyright Copyright 2024 (c), zhaoxi
 *
 */

#include <thread>
#include <unordered_set>

#include <DxImageProc.h>
#include <GxIAPI.h>

#include <opencv2/imgproc.hpp>

#include "galaxy_camera_impl.h"

#include "rmvlpara/camera/galaxy_camera.h"

namespace rm
{

RMVL_IMPL_DEF(GalaxyCamera)

GalaxyCamera::GalaxyCamera(CameraConfig config, std::string_view id) : _impl(new GalaxyCamera::Impl(config, id)) {}
bool GalaxyCamera::set(int prop_id, double value) { return _impl->set(prop_id, value); }
double GalaxyCamera::get(int prop_id) const { return _impl->get(prop_id); }
bool GalaxyCamera::isOpened() const { return _impl->isOpened(); }
bool GalaxyCamera::read(cv::OutputArray image) { return _impl->read(image); }
bool GalaxyCamera::reconnect() { return _impl->reconnect(); }

std::string GalaxyCamera::version() { return GXGetLibVersion(); }

const char *getGXError() noexcept
{
    size_t size = 256;
    static char errinfo[256];
    GXGetLastError(nullptr, errinfo, &size);
    return errinfo;
}

GalaxyCamera::Impl::Impl(CameraConfig config, std::string_view id) noexcept : _config(config), _id(id)
{
    _opened = open();
}

bool GalaxyCamera::Impl::open() noexcept
{
    auto status = GXInitLib();
    getGXError();
    if (status != GX_STATUS_SUCCESS)
    {
        ERROR_("(galaxy) Failed to initialize the library (error: \"%s\")", getGXError());
        return false;
    }
    // ----------------------- 设备枚举 -----------------------
    uint32_t nums{};
    status = GXUpdateDeviceList(&nums, 1000);
    if (status != GX_STATUS_SUCCESS)
    {
        ERROR_("(galaxy) Failed to update device list (error: \"%s\")", getGXError());
        return false;
    }
    if (nums == 0)
    {
        ERROR_("(galaxy) Could not find the camera device.");
        return false;
    }
    // ------------------ 选择设备并创建句柄 ------------------
    std::unordered_set<std::string> id_info_set{};
    std::vector<GX_DEVICE_BASE_INFO> info(nums);
    size_t base_info_size = nums * sizeof(GX_DEVICE_BASE_INFO);
    status = GXGetAllDeviceBaseInfo(info.data(), &base_info_size);
    if (status == GX_STATUS_SUCCESS)
        PASS_("(galaxy) Get device info success.");
    else
    {
        ERROR_("(galaxy) Failed to get device info (error: \"%s\")", getGXError());
        return false;
    }
    GX_OPEN_PARAM open_param{};
    open_param.accessMode = GX_ACCESS_EXCLUSIVE;
    if (_config.handle_mode == HandleMode::Key)
    {
        for (uint32_t i = 0; i < nums; ++i)
            id_info_set.insert(info[i].szSN);
        open_param.openMode = GX_OPEN_SN;
    }
    else if (_config.handle_mode == HandleMode::IP || _config.handle_mode == HandleMode::MAC)
    {
        for (uint32_t i = 0; i < nums; ++i)
        {
            GX_DEVICE_IP_INFO ipinfo;
            if (info[i].deviceClass != GX_DEVICE_CLASS_GEV)
                continue;
            status = GXGetDeviceIPInfo(i + 1, &ipinfo);
            if (status != GX_STATUS_SUCCESS)
            {
                ERROR_("(galaxy) Failed to get device ip info (error: \"%s\")", getGXError());
                return false;
            }
            if (_config.handle_mode == HandleMode::IP)
                id_info_set.insert(ipinfo.szIP);
            else
                id_info_set.insert(ipinfo.szMAC);
        }
        open_param.openMode = _config.handle_mode == HandleMode::IP ? GX_OPEN_IP : GX_OPEN_MAC;
    }
    else if (_config.handle_mode == HandleMode::Index)
    {
        for (uint32_t i = 0; i < nums; ++i)
            id_info_set.insert(std::to_string(i + 1));
        open_param.openMode = GX_OPEN_INDEX;
    }
    else
    {
        ERROR_("(galaxy) Invalid handle mode, please use \"Key\", \"IP\" or \"Index\" mode.");
        return false;
    }
    if (id_info_set.empty())
    {
        ERROR_("(galaxy) Could not find the camera device that fit the conditions.");
        return false;
    }
    INFO_("(galaxy) Quantity of cameras that fit the conditions: %zu", id_info_set.size());

    // ----------------------- 打开相机 -----------------------
    if (_id.empty())
        status = GXOpenDeviceByIndex(1, &_handle);
    else
    {
        if (id_info_set.find(_id) == id_info_set.end())
        {
            ERROR_("(galaxy) Could not find the specific camera device.");
            return false;
        }
        open_param.pszContent = const_cast<char *>(_id.c_str());
        status = GXOpenDevice(&open_param, &_handle);
    }
    if (status != GX_STATUS_SUCCESS)
    {
        ERROR_("(galaxy) Failed to open device (error: \"%s\")", getGXError());
        return false;
    }
    // --------------------- 设置工作模式 ---------------------
    auto grab_mode = _config.grab_mode;
    if (grab_mode == GrabMode::Continuous)
    {
        GXSetEnum(_handle, GX_ENUM_TRIGGER_MODE, GX_TRIGGER_MODE_OFF);
        GXSetEnum(_handle, GX_ENUM_ACQUISITION_MODE, GX_ACQ_MODE_CONTINUOUS);
    }
    else
    {
        GXSetEnum(_handle, GX_ENUM_TRIGGER_MODE, GX_TRIGGER_MODE_ON);
        GXSetEnum(_handle, GX_ENUM_ACQUISITION_MODE, GX_ACQ_MODE_SINGLE_FRAME);
        GXSetEnum(_handle, GX_ENUM_TRIGGER_ACTIVATION, GX_TRIGGER_ACTIVATION_RISINGEDGE);
        return false;
    }
#ifdef _WIN32
    // 针对 Windows 平台的网口相机，设置流通道包长值
    if (_config.handle_mode == HandleMode::IP || _config.handle_mode == HandleMode::MAC)
    {
        uint32_t packet_size{};
        GXGetOptimalPacketSize(_handle, &packet_size);
        status = GXSetInt(_handle, GX_INT_GEV_PACKETSIZE, packet_size);
    }
#endif // _WIN32

    // 设置曝光模式
    GXSetEnum(_handle, GX_ENUM_EXPOSURE_MODE, GX_EXPOSURE_MODE_TIMED);
    GXSetEnum(_handle, GX_ENUM_EXPOSURE_TIME_MODE, GX_EXPOSURE_TIME_MODE_STANDARD);

    // ---------------------- 设置触发源 ----------------------
    if (grab_mode == GrabMode::Software)
        GXSetEnum(_handle, GX_ENUM_TRIGGER_SOURCE, GX_TRIGGER_SOURCE_SOFTWARE);
    else if (grab_mode == GrabMode::Hardware)
    {
        switch (_config.trigger_channel)
        {
        case TriggerChannel::Chn0:
            GXSetEnum(_handle, GX_ENUM_TRIGGER_SOURCE, GX_TRIGGER_SOURCE_LINE0);
            break;
        case TriggerChannel::Chn1:
            GXSetEnum(_handle, GX_ENUM_TRIGGER_SOURCE, GX_TRIGGER_SOURCE_LINE1);
            break;
        case TriggerChannel::Chn2:
            GXSetEnum(_handle, GX_ENUM_TRIGGER_SOURCE, GX_TRIGGER_SOURCE_LINE2);
            break;
        case TriggerChannel::Chn3:
            GXSetEnum(_handle, GX_ENUM_TRIGGER_SOURCE, GX_TRIGGER_SOURCE_LINE3);
            break;
        default:
            break;
        }
    }
    // ----------------------- 开始取流 -----------------------
#ifdef __linux__
    status = GXStreamOn(_handle);
#else
    status = GXGetInt(_handle, GX_INT_PAYLOAD_SIZE, &_payload);
    if (status != GX_STATUS_SUCCESS || _payload <= 0)
    {
        ERROR_("(galaxy) Failed to get payload size (error: \"%s\")", getGXError());
        return false;
    }
    _data.pImgBuf = malloc(_payload);
    status = GXSendCommand(_handle, GX_COMMAND_ACQUISITION_START);
#endif
    if (status != GX_STATUS_SUCCESS)
    {
        ERROR_("(galaxy) Failed to start stream (error: \"%s\")", getGXError());
        return false;
    }
    _opened = true;
    return true;
}

GalaxyCamera::Impl::~Impl() noexcept { release(); }

bool GalaxyCamera::Impl::read(cv::OutputArray image) noexcept
{
    // ----------------------- 获取图像 -----------------------
#ifdef __linux__
    auto status = GXDQBuf(_handle, &_buffer, 1000);
    if (status != GX_STATUS_SUCCESS || _buffer->nStatus != GX_FRAME_STATUS_SUCCESS)
    {
        WARNING_("(galaxy) Failed to dequeue buffer (error: \"%s\")", getGXError());
        reconnect();
        return false;
    }
#else
    auto status = GXGetImage(_handle, &_data, 1000);
    if (status != GX_STATUS_SUCCESS || _data.nStatus != GX_FRAME_STATUS_SUCCESS)
    {
        WARNING_("(galaxy) Failed to dequeue buffer (error: \"%s\")", getGXError());
        reconnect();
        return false;
    }
#endif // __linux__
    // ----------------------- 处理图像 -----------------------
    auto flag = retrieve(image, _config.retrieve_mode);
    // ----------------------- 释放图像 -----------------------
#ifdef __linux__
    GXQBuf(_handle, _buffer);
#endif // __linux__
    return flag;
}

bool GalaxyCamera::Impl::retrieve(cv::OutputArray image, RetrieveMode flag) noexcept
{
    // --------------------- 前置信息准备 ---------------------
#ifdef __linux__
    int32_t pixel_format = _buffer->nPixelFormat;
    uint32_t width = _buffer->nWidth, height = _buffer->nHeight;
    void *buffer = _buffer->pImgBuf;
#else
    int32_t pixel_format = _data.nPixelFormat;
    uint32_t width = _data.nWidth, height = _data.nHeight;
    void *buffer = _data.pImgBuf;
#endif // __linux__
    // ---------------------- 解码、转码 ----------------------
    if (pixel_format == GX_PIXEL_FORMAT_MONO8)
        image.assign(cv::Mat(height, width, CV_8UC1, buffer));
    else if (pixel_format == GX_PIXEL_FORMAT_BAYER_GR8 || pixel_format == GX_PIXEL_FORMAT_BAYER_RG8 ||
             pixel_format == GX_PIXEL_FORMAT_BAYER_GB8 || pixel_format == GX_PIXEL_FORMAT_BAYER_BG8)
    {
        if (flag == RetrieveMode::SDK)
        {
            cv::Mat retval(height, width, CV_8UC3);
            const static std::unordered_map<int32_t, DX_PIXEL_COLOR_FILTER> bayer_map{
                {GX_PIXEL_FORMAT_BAYER_GR8, BAYERGR},
                {GX_PIXEL_FORMAT_BAYER_RG8, BAYERRG},
                {GX_PIXEL_FORMAT_BAYER_GB8, BAYERGB},
                {GX_PIXEL_FORMAT_BAYER_BG8, BAYERBG},
            };
            auto dx_status = DxRaw8toRGB24Ex(buffer, retval.data, width, height, RAW2RGB_NEIGHBOUR,
                                             bayer_map.at(pixel_format), false, DX_ORDER_BGR);
            if (dx_status != DX_OK)
            {
                ERROR_("(galaxy) Failed to convert bayer to rgb (error: \"%d\")", dx_status);
                return false;
            }
            image.assign(retval);
        }
        else if (flag == RetrieveMode::OpenCV)
        {
            cv::Mat src_img(height, width, CV_8U, buffer);
            cv::Mat dst_img;
            const static std::unordered_map<int32_t, int> bayer_map{
                {GX_PIXEL_FORMAT_BAYER_GR8, cv::COLOR_BayerGR2BGR},
                {GX_PIXEL_FORMAT_BAYER_RG8, cv::COLOR_BayerRG2BGR},
                {GX_PIXEL_FORMAT_BAYER_GB8, cv::COLOR_BayerGB2BGR},
                {GX_PIXEL_FORMAT_BAYER_BG8, cv::COLOR_BayerBG2BGR},
            };
            cv::cvtColor(src_img, dst_img, bayer_map.at(pixel_format));
            image.assign(dst_img);
        }
        else
        {
            ERROR_("(galaxy) Failed to retrieve, invalid retrieve mode, please use \"SDK\" mode.");
            return false;
        }
    }
    else
    {
        ERROR_("(galaxy) Invalid pixel format, please use \"Mono8\" or \"BayerXX8\" format.");
        return false;
    }
    return true;
}

bool GalaxyCamera::Impl::reconnect() noexcept
{
    using namespace std::chrono_literals;
    INFO_("(hik) Camera device reconnect");
    release();
    std::this_thread::sleep_for(100ms);
    return open();
}

void GalaxyCamera::Impl::release() noexcept
{
    // ------------------ 停止取流、释放资源 ------------------
#ifdef __linux__
    auto status = GXStreamOff(_handle);
#else
    auto status = GXSendCommand(_handle, GX_COMMAND_ACQUISITION_STOP);
#endif // __linux__
    if (status != GX_STATUS_SUCCESS)
        ERROR_("(galaxy) Failed to stop stream (error: \"%s\")", getGXError());
#ifndef __linux__
    free(_data.pImgBuf);
#endif
    // ----------------------- 关闭相机 -----------------------
    status = GXCloseDevice(_handle);
    if (status != GX_STATUS_SUCCESS)
        ERROR_("(galaxy) Failed to close device (error: \"%s\")", getGXError());
    GXCloseLib();
}

} // namespace rm

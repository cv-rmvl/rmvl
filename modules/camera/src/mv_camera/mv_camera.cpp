/**
 * @file mv_camera.cpp
 * @author RoboMaster Vision Community
 * @brief 工业相机驱动
 * @version 1.0
 * @date 2018-12-08
 *
 * @copyright Copyright 2021 (c), RoboMaster Vision Community
 *
 */

#include <thread>

#include <opencv2/imgproc.hpp>

#include "mv_camera_impl.h"

namespace rm
{

MvCamera::MvCamera(CameraConfig init_mode, std::string_view serial) : _impl(new MvCamera::Impl(init_mode, serial)) {}
MvCamera::~MvCamera() { delete _impl; }
bool MvCamera::set(int propId, double value) { return _impl->set(propId, value); }
double MvCamera::get(int propId) const { return _impl->get(propId); }
bool MvCamera::isOpened() const { return _impl->isOpened(); }
bool MvCamera::read(cv::Mat image) { return _impl->read(image); }
bool MvCamera::reconnect() { return _impl->reconnect(); }

MvCamera::Impl::Impl(CameraConfig init_mode, std::string_view serial) noexcept
    : _camera_id(serial), _camera_list(new tSdkCameraDevInfo[_camera_counts]),
      _grab_mode(init_mode.grab_mode), _retrieve_mode(init_mode.retrieve_mode) { open(); }

MvCamera::Impl::~Impl() noexcept
{
    delete[] _camera_list;
    _camera_list = nullptr;
    delete[] _pbyOut;
    _pbyOut = nullptr;
    release();
}

bool MvCamera::Impl::open() noexcept
{
    CameraSdkInit(1);
    // 枚举设备，并建立设备列表
    _status = CameraEnumerateDevice(_camera_list, &_camera_counts);
    if (_status == CAMERA_STATUS_SUCCESS)
        PASS_("mv - camera enum status: %s", CameraGetErrorString(_status));
    else
        INFO_("mv - camera enum status: %s", CameraGetErrorString(_status));
    INFO_("mv - camera quantity: %d", _camera_counts);
    // 无设备连接
    if (_camera_counts == 0)
    {
        ERROR_("mv - could not find the camera devise.");
        return false;
    }
    // Key: 序列号, Val: 相机设备信息哈希表
    std::unordered_map<std::string, tSdkCameraDevInfo *> id_info;
    for (INT enum_idx = 0; enum_idx < _camera_counts; ++enum_idx)
        id_info[_camera_list[enum_idx].acSn] = &_camera_list[enum_idx];
    // 若无序列号，初始化第一个相机
    if (_camera_id.empty())
        _status = CameraInit(_camera_list, -1, -1, &_handle);
    // 根据序列号初始化相机
    else if (id_info.find(_camera_id) != id_info.end())
        _status = CameraInit(id_info[_camera_id], -1, -1, &_handle);
    else
    {
        ERROR_("mv - could not find the camera according to the specific S/N");
        return false;
    }
    // 初始化失败
    if (_status == CAMERA_STATUS_SUCCESS)
        PASS_("mv - camera initial status: %s", CameraGetErrorString(_status));
    else
    {
        ERROR_("mv - failed to initial the camera device: %s", CameraGetErrorString(_status));
        return false;
    }

    // 设置采集模式
    CameraSetTriggerMode(_handle, static_cast<int>(_grab_mode));
    // 整合的设备描述信息
    tSdkCameraCapbility capability;
    CameraGetCapability(_handle, &capability);
    // 让SDK进入工作模式
    CameraPlay(_handle);
    if (capability.sIspCapacity.bMonoSensor)
    {
        _channel = 1;
        CameraSetIspOutFormat(_handle, CAMERA_MEDIA_TYPE_MONO8);
    }
    else
    {
        _channel = 3;
        CameraSetIspOutFormat(_handle, CAMERA_MEDIA_TYPE_BGR8);
    }

    if (_pbyOut == nullptr)
        _pbyOut = new BYTE[capability.sResolutionRange.iHeightMax * capability.sResolutionRange.iWidthMax * _channel];

    _is_opened = true;
    return _is_opened;
}

/**
 * @brief Media type 转换为 ColorConversionCodes
 *
 * @param[in] media_type
 * @return ColorConversionCodes
 */
static inline cv::ColorConversionCodes mediaType2CVType(UINT media_type)
{
    static std::unordered_map<UINT, cv::ColorConversionCodes> convertion =
        {{CAMERA_MEDIA_TYPE_BAYGR8, cv::COLOR_BayerGR2RGB},
         {CAMERA_MEDIA_TYPE_BAYRG8, cv::COLOR_BayerRG2RGB},
         {CAMERA_MEDIA_TYPE_BAYGB8, cv::COLOR_BayerGB2RGB},
         {CAMERA_MEDIA_TYPE_BAYBG8, cv::COLOR_BayerBG2RGB}};
    return convertion[media_type];
}

bool MvCamera::Impl::retrieve(cv::OutputArray image) noexcept
{
    if (_channel != 1 && _channel != 3)
    {
        ERROR_("mv - camera image _channel: %d.", _channel);
        image.assign(cv::Mat());
        CameraReleaseImageBuffer(_handle, _pbyBuffer);
        return false;
    }
    // SDK
    if (_retrieve_mode == RetrieveMode::SDK)
    {
        CameraImageProcess(_handle, _pbyBuffer, _pbyOut, &_frame_info);
        bool retflag = false;
        if (_pbyOut != nullptr)
        {
            image.assign(
                cv::Mat(cv::Size(_frame_info.iWidth, _frame_info.iHeight), _frame_info.uiMediaType == CAMERA_MEDIA_TYPE_MONO8 ? CV_8UC1 : CV_8UC3, _pbyOut));
            CameraReleaseImageBuffer(_handle, _pbyBuffer);
            retflag = true;
        }
        else
        {
            image.assign(cv::Mat());
            ERROR_("mv - failed to retrieve, retrieve mode: %d.", static_cast<int>(_retrieve_mode));
            retflag = false;
        }
        return retflag;
    }
    // CV
    else if (_retrieve_mode == RetrieveMode::OpenCV)
    {
        cv::Mat bayerImg(cv::Size(_frame_info.iWidth, _frame_info.iHeight), CV_8U, _pbyBuffer);
        if (_channel == 1)
        {
            cv::flip(bayerImg, bayerImg, 0);
            image.assign(bayerImg);
        }
        else if (_channel == 3)
        {
            cv::Mat bgrImg;
            // cvtColor has a good effect processed by multy-thread，which is
            // suitable for the device with weak single core performance
            cvtColor(bayerImg, bgrImg, mediaType2CVType(_frame_info.uiMediaType));
            image.assign(bgrImg);
        }
        CameraReleaseImageBuffer(_handle, _pbyBuffer);
        return true;
    }

    ERROR_("mv - failed to retrieve, retrieve mode: %d.", static_cast<int>(_retrieve_mode));
    image.assign(cv::Mat());
    return false;
}

bool MvCamera::Impl::reconnect() noexcept
{
    using namespace std::chrono_literals;    
    INFO_("mv - camera device reconnect");
    release();
    std::this_thread::sleep_for(100ms);

    // 重置相机数量
    _camera_counts = 8;
    open();
    // 还原参数设置
    _auto_exposure ? set(CAMERA_AUTO_EXPOSURE, 0) : set(CAMERA_MANUAL_EXPOSURE, 0);
    set(CAMERA_EXPOSURE, _exposure);
    set(CAMERA_GAIN, _gain);
    set(CAMERA_WB_RGAIN, _r_gain);
    set(CAMERA_WB_GGAIN, _g_gain);
    set(CAMERA_WB_BGAIN, _b_gain);
    return true;
}

} // namespace rm

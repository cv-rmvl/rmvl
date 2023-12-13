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

#include <unistd.h>

#include <opencv2/imgproc.hpp>

#include "mv_camera_impl.h"

namespace rm
{

MvCamera::MvCamera(GrabMode grab_mode, RetrieveMode retrieve_mode, std::string_view serial, const std::vector<int> &decode_param)
{
    _impl = new MvCamera::Impl(grab_mode, retrieve_mode, serial, decode_param);
}

MvCamera::~MvCamera() { delete _impl; }
bool MvCamera::set(int propId, double value) { return _impl->set(propId, value); }
double MvCamera::get(int propId) const { return _impl->get(propId); }
bool MvCamera::isOpened() const { return _impl->isOpened(); }
bool MvCamera::read(cv::OutputArray image) { return _impl->read(image); }
bool MvCamera::reconnect() { return _impl->reconnect(); }

MvCamera::Impl::Impl(GrabMode grab_mode, RetrieveMode retrieve_mode, std::string_view serial, const std::vector<int> &decode_param) noexcept
    : _camera_list(new tSdkCameraDevInfo[_camera_counts]), _grab_mode(grab_mode), _retrieve_mode(retrieve_mode)
{
    // 需要解码参数的几种解码模式 Several decoding modes that require decoding parameters
    bool init_param_status = true;
    if (retrieve_mode & RETRIEVE_LUT)
        init_param_status = decode_param.empty() ? false : initLUT(decode_param);
    if (!init_param_status)
        WARNING_("camera decode param is empty!");
    if (!serial.empty())
        _camera_id = serial;
    open();
}

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
        PASS_("camera enum status: %s", CameraGetErrorString(_status));
    else
        INFO_("camera enum status: %s", CameraGetErrorString(_status));
    INFO_("camera quantity: %d", _camera_counts);
    // 无设备连接
    if (_camera_counts == 0)
    {
        ERROR_("could not find the camera devise.");
        return false;
    }
    // Key: 序列号, Val: 相机设备信息哈希表
    std::unordered_map<std::string, tSdkCameraDevInfo *> id_info;
    for (INT enum_idx = 0; enum_idx < _camera_counts; ++enum_idx)
        id_info[_camera_list[enum_idx].acSn] = &_camera_list[enum_idx];
    // 若无序列号，初始化第一个相机
    if (_camera_id.empty())
        _status = CameraInit(_camera_list, -1, -1, &_hCamera);
    // 根据序列号初始化相机
    else if (id_info.find(_camera_id) != id_info.end())
        _status = CameraInit(id_info[_camera_id], -1, -1, &_hCamera);
    else
    {
        ERROR_("could not find the camera according to the specific S/N");
        return false;
    }
    // 初始化失败
    if (_status == CAMERA_STATUS_SUCCESS)
        PASS_("camera initial status: %s", CameraGetErrorString(_status));
    else
    {
        ERROR_("failed to initial the camera device: %s", CameraGetErrorString(_status));
        return false;
    }

    // 设置采集模式
    CameraSetTriggerMode(_hCamera, _grab_mode);
    // 整合的设备描述信息
    tSdkCameraCapbility capability;
    CameraGetCapability(_hCamera, &capability);
    // 让SDK进入工作模式
    CameraPlay(_hCamera);
    if (capability.sIspCapacity.bMonoSensor)
    {
        _channel = 1;
        CameraSetIspOutFormat(_hCamera, CAMERA_MEDIA_TYPE_MONO8);
    }
    else
    {
        _channel = 3;
        CameraSetIspOutFormat(_hCamera, CAMERA_MEDIA_TYPE_BGR8);
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

bool MvCamera::Impl::retrieve(cv::OutputArray image, RetrieveMode flag) noexcept
{
    if (_channel != 1 && _channel != 3)
    {
        ERROR_("camera image _channel: %d.", _channel);
        image.assign(cv::Mat());
        CameraReleaseImageBuffer(_hCamera, _pbyBuffer);
        return false;
    }
    // SDK
    if (flag & RETRIEVE_SDK)
    {
        CameraImageProcess(_hCamera, _pbyBuffer, _pbyOut, &_frame_info);
        bool retflag = false;
        if (_pbyOut != nullptr)
        {
            image.assign(cv::Mat(
                cv::Size(_frame_info.iWidth, _frame_info.iHeight),
                _frame_info.uiMediaType == CAMERA_MEDIA_TYPE_MONO8 ? CV_8UC1 : CV_8UC3,
                _pbyOut));
            CameraReleaseImageBuffer(_hCamera, _pbyBuffer);
            retflag = true;
        }
        else
        {
            image.assign(cv::Mat());
            ERROR_("failed to retrieve, retrieve mode: %d.", _retrieve_mode);
            retflag = false;
        }
        return retflag;
    }
    // CV
    else if (flag & RETRIEVE_CV)
    {
        cv::Mat bayerImg(cv::Size(_frame_info.iWidth, _frame_info.iHeight), CV_8U, _pbyBuffer);
        if (_channel == 1)
            image.assign(bayerImg);
        else if (_channel == 3)
        {
            if (flag & RETRIEVE_LUT)
            {
                // use LUT to process 'bayerImg', and then convert to 3 channels
                // image, which has the same effect with less computation
                cv::LUT(bayerImg, _look_up_table, bayerImg);
            }
            cv::Mat bgrImg;
            // cvtColor has a good effect processed by multy-thread，which is
            // suitable for the device with weak single core performance
            cvtColor(bayerImg, bgrImg, mediaType2CVType(_frame_info.uiMediaType));
            image.assign(bgrImg);
        }
        CameraReleaseImageBuffer(_hCamera, _pbyBuffer);
        return true;
    }

    ERROR_("failed to retrieve, retrieve mode: %d.", _retrieve_mode);
    image.assign(cv::Mat());
    return false;
}

bool MvCamera::Impl::initLUT(const std::vector<int> &lut) noexcept
{
    if (lut.size() != 256)
    {
        ERROR_("parameters in Look-Up Table");
        return false;
    }
    uchar table1[256];
    for (int i = 0; i < 256; i++)
        table1[i] = static_cast<uchar>(lut[i]);
    _look_up_table = cv::Mat(1, 256, CV_8U, table1);
    return true;
}

bool MvCamera::Impl::reconnect() noexcept
{
    INFO_("camera device reconnect");
    release();
    sleep(1);
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

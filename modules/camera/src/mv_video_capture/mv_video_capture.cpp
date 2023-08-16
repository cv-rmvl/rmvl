/**
 * @file mv_video_capture.cpp
 * @author RoboMaster Vision Community
 *         zhaoxi
 *         黄裕炯 (961352855@qq.com)
 * @brief 工业相机驱动
 * @version 1.0
 * @date 2018-12-08
 *
 * @copyright Copyright 2021 (c), RoboMaster Vision Community
 *
 */

#include <unistd.h>

#include <opencv2/imgproc.hpp>

#include "rmvl/camera/mv_video_capture.h"
#include "rmvl/camera/camera_logging.h"

using namespace rm;
using namespace std;
using namespace cv;

MvVideoCapture::MvVideoCapture(GrabMode grab_mode, RetrieveMode retrieve_mode,
                               const string &serial, const vector<int> &decode_param)
    : _camera_list(new tSdkCameraDevInfo[_camera_counts]),
      _grab_mode(grab_mode), _retrieve_mode(retrieve_mode)
{
    // 需要解码参数的几种解码模式 Several decoding modes that require decoding parameters
    bool init_param_status = true;
    if (retrieve_mode & RETRIEVE_LUT)
        init_param_status = decode_param.empty() ? false : initLUT(decode_param);
    if (!init_param_status)
        CAM_WARNING("camera decode param is empty!");
    if (!serial.empty())
        _camera_id = serial;
    open();
}

MvVideoCapture::~MvVideoCapture()
{
    delete[] _camera_list;
    _camera_list = nullptr;
    delete[] _pbyOut;
    _pbyOut = nullptr;
    release();
}

bool MvVideoCapture::open()
{
    CameraSdkInit(1);
    // 枚举设备，并建立设备列表
    _status = CameraEnumerateDevice(_camera_list, &_camera_counts);
    if (_status == CAMERA_STATUS_SUCCESS)
        CAM_PASS("camera enum status: %s", CameraGetErrorString(_status));
    else
        CAM_INFO("camera enum status: %s", CameraGetErrorString(_status));
    CAM_INFO("camera quantity: %d", _camera_counts);
    // 无设备连接
    if (_camera_counts == 0)
    {
        CAM_ERROR("could not find the camera devise.");
        return false;
    }
    // 哈希匹配 序列号-相机设备信息哈希表
    unordered_map<string, tSdkCameraDevInfo *> id_info;
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
        CAM_ERROR("could not find the camera according to the specific S/N");
        return false;
    }
    // 初始化失败
    if (_status == CAMERA_STATUS_SUCCESS)
        CAM_PASS("camera initial status: %s", CameraGetErrorString(_status));
    else
    {
        CAM_ERROR("failed to initial the camera device: %s", CameraGetErrorString(_status));
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
inline ColorConversionCodes mediaType2CVType(UINT media_type)
{
    static std::unordered_map<UINT, ColorConversionCodes> convertion =
        {{CAMERA_MEDIA_TYPE_BAYGR8, COLOR_BayerGR2RGB},
         {CAMERA_MEDIA_TYPE_BAYRG8, COLOR_BayerRG2RGB},
         {CAMERA_MEDIA_TYPE_BAYGB8, COLOR_BayerGB2RGB},
         {CAMERA_MEDIA_TYPE_BAYBG8, COLOR_BayerBG2RGB}};
    return convertion[media_type];
}

bool MvVideoCapture::retrieve(OutputArray image, RetrieveMode flag)
{
    if (_channel != 1 && _channel != 3)
    {
        CAM_ERROR("camera image _channel: %d.", _channel);
        image.assign(Mat());
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
            image.assign(Mat(
                Size(_frame_info.iWidth, _frame_info.iHeight),
                _frame_info.uiMediaType == CAMERA_MEDIA_TYPE_MONO8 ? CV_8UC1 : CV_8UC3,
                _pbyOut));
            CameraReleaseImageBuffer(_hCamera, _pbyBuffer);
            retflag = true;
        }
        else
        {
            image.assign(Mat());
            CAM_ERROR("failed to retrieve, retrieve mode: %d.", _retrieve_mode);
            retflag = false;
        }
        return retflag;
    }
    // CV
    else if (flag & RETRIEVE_CV)
    {
        Mat bayerImg(Size(_frame_info.iWidth, _frame_info.iHeight), CV_8U, _pbyBuffer);
        if (_channel == 1)
            image.assign(bayerImg);
        else if (_channel == 3)
        {
            if (flag & RETRIEVE_LUT)
            {
                // use LUT to process 'bayerImg', and then convert to 3 channels
                // image, which has the same effect with less computation
                LUT(bayerImg, _look_up_table, bayerImg);
            }
            Mat bgrImg;
            // cvtColor has a good effect processed by multy-thread，which is
            // suitable for the device with weak single core performance
            cvtColor(bayerImg, bgrImg, mediaType2CVType(_frame_info.uiMediaType));
            image.assign(bgrImg);
        }
        CameraReleaseImageBuffer(_hCamera, _pbyBuffer);
        return true;
    }

    CAM_ERROR("failed to retrieve, retrieve mode: %d.", _retrieve_mode);
    image.assign(Mat());
    return false;
}

bool MvVideoCapture::initLUT(const vector<int> &lut)
{
    if (lut.size() != 256)
    {
        CAM_ERROR("parameters in Look-Up Table");
        return false;
    }
    uchar table1[256];
    for (int i = 0; i < 256; i++)
        table1[i] = static_cast<uchar>(lut[i]);
    _look_up_table = Mat(1, 256, CV_8U, table1);
    return true;
}

bool MvVideoCapture::reconnect()
{
    CAM_INFO("camera device reconnect");
    release();
    sleep(1);
    // 重置相机数量
    _camera_counts = 8;
    open();
    // 还原参数设置
    if (_auto_exposure)
        set(CAP_PROP_RM_AUTO_EXPOSURE);
    else
        set(CAP_PROP_RM_MANUAL_EXPOSURE);
    set(CAP_PROP_RM_EXPOSURE, _exposure);
    set(CAP_PROP_RM_GAIN, _gain);
    set(CAP_PROP_RM_WB_RGAIN, _r_gain);
    set(CAP_PROP_RM_WB_GGAIN, _g_gain);
    set(CAP_PROP_RM_WB_BGAIN, _b_gain);
    return true;
}

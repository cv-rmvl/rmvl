/**
 * @file mv_properties.cpp
 * @author RoboMaster Vision Community
 * @brief
 * @version 1.0
 * @date 2022-11-19
 *
 * @copyright Copyright 2023 (c), RoboMaster Vision Community
 *
 */

#include "rmvl/camera/mv_video_capture.h"
#include "rmvl/camera/logging.h"

using namespace rm;
using namespace std;
using namespace cv;

bool MvVideoCapture::set(int propId, double value)
{
    switch (propId)
    {
    // Properties
    case CAP_PROP_RM_AUTO_EXPOSURE:
        _auto_exposure = true;
        return CameraSetAeState(_hCamera, _auto_exposure) == CAMERA_STATUS_SUCCESS;
    case CAP_PROP_RM_MANUAL_EXPOSURE:
        _auto_exposure = false;
        return CameraSetAeState(_hCamera, _auto_exposure) == CAMERA_STATUS_SUCCESS;
    case CAP_PROP_RM_EXPOSURE:
        _exposure = value;
        return CameraSetExposureTime(_hCamera, _exposure) == CAMERA_STATUS_SUCCESS;
    case CAP_PROP_RM_GAIN:
        _gain = value;
        return CameraSetAnalogGain(_hCamera, _gain) == CAMERA_STATUS_SUCCESS;
    case CAP_PROP_RM_AUTO_WB:
        return CameraSetWbMode(_hCamera, true) == CAMERA_STATUS_SUCCESS;
    case CAP_PROP_RM_MANUAL_WB:
        return CameraSetWbMode(_hCamera, false) == CAMERA_STATUS_SUCCESS;
    case CAP_PROP_RM_WB_BGAIN:
        _b_gain = value;
        return CameraSetGain(_hCamera, _r_gain, _g_gain, _b_gain) == CAMERA_STATUS_SUCCESS;
    case CAP_PROP_RM_WB_GGAIN:
        _g_gain = value;
        return CameraSetGain(_hCamera, _r_gain, _g_gain, _b_gain) == CAMERA_STATUS_SUCCESS;
    case CAP_PROP_RM_WB_RGAIN:
        _r_gain = value;
        return CameraSetGain(_hCamera, _r_gain, _g_gain, _b_gain) == CAMERA_STATUS_SUCCESS;
    case CAP_PROP_RM_TRIGGER_COUNT:
        if (_grab_mode != GRAB_CONTINUOUS)
            return CameraSetTriggerCount(_hCamera, static_cast<INT>(value)) == CAMERA_STATUS_SUCCESS;
        else
            return false;
    case CAP_PROP_RM_TRIGGER_DELAY:
        if (_grab_mode == GRAB_HARDWARE)
            return CameraSetTriggerDelayTime(_hCamera, static_cast<UINT>(value)) == CAMERA_STATUS_SUCCESS;
        else
            return false;
    case CAP_PROP_RM_TRIGGER_PERIOD:
        if (_grab_mode != GRAB_CONTINUOUS)
            return CameraSetTriggerPeriodTime(_hCamera, static_cast<UINT>(value)) == CAMERA_STATUS_SUCCESS;
        else
            return false;
    case CAP_PROP_RM_GAMMA:
        _gamma = value;
        return CameraSetGamma(_hCamera, static_cast<int>(_gamma)) == CAMERA_STATUS_SUCCESS;
    case CAP_PROP_RM_CONTRAST:
        _contrast = value;
        return CameraSetContrast(_hCamera, static_cast<int>(_contrast)) == CAMERA_STATUS_SUCCESS;
    case CAP_PROP_RM_SATURATION:
        _saturation = value;
        return CameraSetSaturation(_hCamera, static_cast<int>(_saturation)) == CAMERA_STATUS_SUCCESS;
    case CAP_PROP_RM_SHARPNESS:
        _sharpness = value;
        return CameraSetSharpness(_hCamera, static_cast<int>(_sharpness)) == CAMERA_STATUS_SUCCESS;
    // Activities
    case CAP_ACT_RM_ONCE_WB:
        return CameraSetOnceWB(_hCamera) == CAMERA_STATUS_SUCCESS;
    case CAP_ACT_RM_SOFT_TRIGGER:
        if (_grab_mode != GRAB_CONTINUOUS)
            return CameraSoftTrigger(_hCamera) == CAMERA_STATUS_SUCCESS;
        else
            return false;
    default:
        CAM_ERROR("Try to set undefined variable, id: %d.", propId);
        return false;
    }
}

double MvVideoCapture::get(int propId) const
{
    switch (propId)
    {
    case CAP_PROP_RM_AUTO_EXPOSURE:
        return _auto_exposure;
    case CAP_PROP_RM_MANUAL_EXPOSURE:
        return !_auto_exposure;
    case CAP_PROP_RM_EXPOSURE:
        return _exposure;
    case CAP_PROP_RM_GAIN:
        return _gain;
    case CAP_PROP_RM_WB_BGAIN:
        return _b_gain;
    case CAP_PROP_RM_WB_GGAIN:
        return _g_gain;
    case CAP_PROP_RM_WB_RGAIN:
        return _r_gain;
    case CAP_PROP_RM_TRIGGER_COUNT:
        INT trigger_count;
        return CameraGetTriggerCount(_hCamera, &trigger_count) == 0 ? trigger_count : -1;
    case CAP_PROP_RM_TRIGGER_DELAY:
        UINT trigger_delay;
        return CameraGetTriggerDelayTime(_hCamera, &trigger_delay) == 0 ? trigger_delay : -1;
    case CAP_PROP_RM_TRIGGER_PERIOD:
        UINT trigger_period;
        return CameraGetTriggerPeriodTime(_hCamera, &trigger_period) == 0 ? trigger_period : -1;
    case CAP_PROP_RM_GAMMA:
        return _gamma;
    case CAP_PROP_RM_CONTRAST:
        return _contrast;
    case CAP_PROP_RM_SATURATION:
        return _saturation;
    case CAP_PROP_RM_SHARPNESS:
        return _sharpness;
    case CAP_PROP_RM_FRAME_HEIGHT:
        return static_cast<double>(_frame_info.iHeight);
    case CAP_PROP_RM_FRAME_WIDTH:
        return static_cast<double>(_frame_info.iWidth);
    default:
        CAM_ERROR("Try to get undefined variable, id: %d.", propId);
        return CAMERA_STATUS_FAILED;
    }
}

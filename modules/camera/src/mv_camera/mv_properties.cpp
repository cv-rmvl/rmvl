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

#include "rmvl/camera/mv_camera.h"

using namespace rm;
using namespace std;
using namespace cv;

bool MvCamera::set(int propId, double value)
{
    switch (propId)
    {
    // Properties
    case CAMERA_AUTO_EXPOSURE:
        _auto_exposure = true;
        return CameraSetAeState(_hCamera, _auto_exposure) == CAMERA_STATUS_SUCCESS;
    case CAMERA_MANUAL_EXPOSURE:
        _auto_exposure = false;
        return CameraSetAeState(_hCamera, _auto_exposure) == CAMERA_STATUS_SUCCESS;
    case CAMERA_EXPOSURE:
        _exposure = value;
        return CameraSetExposureTime(_hCamera, _exposure) == CAMERA_STATUS_SUCCESS;
    case CAMERA_GAIN:
        _gain = value;
        return CameraSetAnalogGain(_hCamera, _gain) == CAMERA_STATUS_SUCCESS;
    case CAMERA_AUTO_WB:
        return CameraSetWbMode(_hCamera, true) == CAMERA_STATUS_SUCCESS;
    case CAMERA_MANUAL_WB:
        return CameraSetWbMode(_hCamera, false) == CAMERA_STATUS_SUCCESS;
    case CAMERA_WB_BGAIN:
        _b_gain = value;
        return CameraSetGain(_hCamera, _r_gain, _g_gain, _b_gain) == CAMERA_STATUS_SUCCESS;
    case CAMERA_WB_GGAIN:
        _g_gain = value;
        return CameraSetGain(_hCamera, _r_gain, _g_gain, _b_gain) == CAMERA_STATUS_SUCCESS;
    case CAMERA_WB_RGAIN:
        _r_gain = value;
        return CameraSetGain(_hCamera, _r_gain, _g_gain, _b_gain) == CAMERA_STATUS_SUCCESS;
    case CAMERA_TRIGGER_COUNT:
        if (_grab_mode != GRAB_CONTINUOUS)
            return CameraSetTriggerCount(_hCamera, static_cast<INT>(value)) == CAMERA_STATUS_SUCCESS;
        else
            return false;
    case CAMERA_TRIGGER_DELAY:
        if (_grab_mode == GRAB_HARDWARE)
            return CameraSetTriggerDelayTime(_hCamera, static_cast<UINT>(value)) == CAMERA_STATUS_SUCCESS;
        else
            return false;
    case CAMERA_TRIGGER_PERIOD:
        if (_grab_mode != GRAB_CONTINUOUS)
            return CameraSetTriggerPeriodTime(_hCamera, static_cast<UINT>(value)) == CAMERA_STATUS_SUCCESS;
        else
            return false;
    case CAMERA_GAMMA:
        _gamma = value;
        return CameraSetGamma(_hCamera, static_cast<int>(_gamma)) == CAMERA_STATUS_SUCCESS;
    case CAMERA_CONTRAST:
        _contrast = value;
        return CameraSetContrast(_hCamera, static_cast<int>(_contrast)) == CAMERA_STATUS_SUCCESS;
    case CAMERA_SATURATION:
        _saturation = value;
        return CameraSetSaturation(_hCamera, static_cast<int>(_saturation)) == CAMERA_STATUS_SUCCESS;
    case CAMERA_SHARPNESS:
        _sharpness = value;
        return CameraSetSharpness(_hCamera, static_cast<int>(_sharpness)) == CAMERA_STATUS_SUCCESS;
    // Activities
    case CAMERA_ONCE_WB:
        return CameraSetOnceWB(_hCamera) == CAMERA_STATUS_SUCCESS;
    case CAMERA_SOFT_TRIGGER:
        if (_grab_mode != GRAB_CONTINUOUS)
            return CameraSoftTrigger(_hCamera) == CAMERA_STATUS_SUCCESS;
        else
            return false;
    default:
        ERROR_("Try to set undefined variable, id: %d.", propId);
        return false;
    }
}

double MvCamera::get(int propId) const
{
    switch (propId)
    {
    case CAMERA_AUTO_EXPOSURE:
        return _auto_exposure;
    case CAMERA_MANUAL_EXPOSURE:
        return !_auto_exposure;
    case CAMERA_EXPOSURE:
        return _exposure;
    case CAMERA_GAIN:
        return _gain;
    case CAMERA_WB_BGAIN:
        return _b_gain;
    case CAMERA_WB_GGAIN:
        return _g_gain;
    case CAMERA_WB_RGAIN:
        return _r_gain;
    case CAMERA_TRIGGER_COUNT:
        INT trigger_count;
        return CameraGetTriggerCount(_hCamera, &trigger_count) == 0 ? trigger_count : -1;
    case CAMERA_TRIGGER_DELAY:
        UINT trigger_delay;
        return CameraGetTriggerDelayTime(_hCamera, &trigger_delay) == 0 ? trigger_delay : -1;
    case CAMERA_TRIGGER_PERIOD:
        UINT trigger_period;
        return CameraGetTriggerPeriodTime(_hCamera, &trigger_period) == 0 ? trigger_period : -1;
    case CAMERA_GAMMA:
        return _gamma;
    case CAMERA_CONTRAST:
        return _contrast;
    case CAMERA_SATURATION:
        return _saturation;
    case CAMERA_SHARPNESS:
        return _sharpness;
    case CAMERA_FRAME_HEIGHT:
        return static_cast<double>(_frame_info.iHeight);
    case CAMERA_FRAME_WIDTH:
        return static_cast<double>(_frame_info.iWidth);
    default:
        ERROR_("Try to get undefined variable, id: %d.", propId);
        return CAMERA_STATUS_FAILED;
    }
}

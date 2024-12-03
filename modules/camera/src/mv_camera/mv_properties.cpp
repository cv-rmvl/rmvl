/**
 * @file mv_properties.cpp
 * @author zhaoxi (535394140@qq.com)
 * @brief 迈德威视相机属性设置
 * @version 1.0
 * @date 2022-11-19
 *
 * @copyright Copyright 2023 (c), RoboMaster Vision Community
 *
 */

#include "mv_camera_impl.h"

namespace rm
{

bool MvCamera::Impl::set(int propId, double value) noexcept
{
    switch (propId)
    {
    // Properties
    case CAMERA_AUTO_EXPOSURE:
        _auto_exposure = true;
        return CameraSetAeState(_handle, _auto_exposure) == CAMERA_STATUS_SUCCESS;
    case CAMERA_MANUAL_EXPOSURE:
        _auto_exposure = false;
        return CameraSetAeState(_handle, _auto_exposure) == CAMERA_STATUS_SUCCESS;
    case CAMERA_EXPOSURE:
        _exposure = value;
        return CameraSetExposureTime(_handle, _exposure) == CAMERA_STATUS_SUCCESS;
    case CAMERA_GAIN:
        _gain = static_cast<int>(value);
        return CameraSetAnalogGain(_handle, _gain) == CAMERA_STATUS_SUCCESS;
    case CAMERA_AUTO_WB:
        return CameraSetWbMode(_handle, true) == CAMERA_STATUS_SUCCESS;
    case CAMERA_MANUAL_WB:
        return CameraSetWbMode(_handle, false) == CAMERA_STATUS_SUCCESS;
    case CAMERA_WB_BGAIN:
        _b_gain = static_cast<int>(value);
        return CameraSetGain(_handle, _r_gain, _g_gain, _b_gain) == CAMERA_STATUS_SUCCESS;
    case CAMERA_WB_GGAIN:
        _g_gain = static_cast<int>(value);
        return CameraSetGain(_handle, _r_gain, _g_gain, _b_gain) == CAMERA_STATUS_SUCCESS;
    case CAMERA_WB_RGAIN:
        _r_gain = static_cast<int>(value);
        return CameraSetGain(_handle, _r_gain, _g_gain, _b_gain) == CAMERA_STATUS_SUCCESS;
    case CAMERA_TRIGGER_COUNT:
        if (_grab_mode != GrabMode::Continuous)
            return CameraSetTriggerCount(_handle, static_cast<INT>(value)) == CAMERA_STATUS_SUCCESS;
        else
            return false;
    case CAMERA_TRIGGER_DELAY:
        if (_grab_mode == GrabMode::Hardware)
            return CameraSetTriggerDelayTime(_handle, static_cast<UINT>(value)) == CAMERA_STATUS_SUCCESS;
        else
            return false;
#ifndef _WIN32
    case CAMERA_TRIGGER_PERIOD:
        if (_grab_mode != GrabMode::Continuous)
            return CameraSetTriggerPeriodTime(_handle, static_cast<UINT>(value)) == CAMERA_STATUS_SUCCESS;
        else
            return false;
#endif // _WIN32
    case CAMERA_GAMMA:
        _gamma = static_cast<int>(value);
        return CameraSetGamma(_handle, _gamma) == CAMERA_STATUS_SUCCESS;
    case CAMERA_CONTRAST:
        _contrast = static_cast<int>(value);
        return CameraSetContrast(_handle, _contrast) == CAMERA_STATUS_SUCCESS;
    case CAMERA_SATURATION:
        _saturation = static_cast<int>(value);
        return CameraSetSaturation(_handle, _saturation) == CAMERA_STATUS_SUCCESS;
    case CAMERA_SHARPNESS:
        _sharpness = static_cast<int>(value);
        return CameraSetSharpness(_handle, _sharpness) == CAMERA_STATUS_SUCCESS;
    // Activities
    case CAMERA_ONCE_WB:
        return CameraSetOnceWB(_handle) == CAMERA_STATUS_SUCCESS;
    case CAMERA_TRIGGER_SOFT:
        if (_grab_mode != GrabMode::Continuous)
            return CameraSoftTrigger(_handle) == CAMERA_STATUS_SUCCESS;
        else
            return false;
    default:
        WARNING_("(mv) Try to set undefined variable, id: %d.", propId);
        return false;
    }
}

double MvCamera::Impl::get(int propId) const noexcept
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
        return CameraGetTriggerCount(_handle, &trigger_count) == 0 ? trigger_count : -1;
    case CAMERA_TRIGGER_DELAY:
        UINT trigger_delay;
        return CameraGetTriggerDelayTime(_handle, &trigger_delay) == 0 ? trigger_delay : -1;
#ifndef _WIN32
    case CAMERA_TRIGGER_PERIOD:
        UINT trigger_period;
        return CameraGetTriggerPeriodTime(_handle, &trigger_period) == 0 ? trigger_period : -1;
#endif // _WIN32
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
        WARNING_("(mv) Try to get undefined variable, id: %d.", propId);
        return CAMERA_STATUS_FAILED;
    }
}

} // namespace rm

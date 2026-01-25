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

#include "rmvl/core/util.hpp"

#include "rmvlpara/camera/mv_camera.h"

namespace rm {

template <>
bool MvCamera::Impl::set(CameraProperties propId, bool value) noexcept {
    switch (propId) {
    case CameraProperties::auto_exposure:
        return CameraSetAeState(_handle, value) == CAMERA_STATUS_SUCCESS;
    case CameraProperties::auto_wb:
        return CameraSetWbMode(_handle, value) == CAMERA_STATUS_SUCCESS;
    default:
        WARNING_("(mv) Try to set undefined variable, id: %d.", static_cast<int>(propId));
        return false;
    }
}

template <>
bool MvCamera::Impl::set(CameraProperties propId, int value) noexcept {
    switch (propId) {
    case CameraProperties::gain:
        return CameraSetAnalogGain(_handle, value) == CAMERA_STATUS_SUCCESS;
    case CameraProperties::wb_bgain: {
        int r_gain{}, g_gain{}, _{};
        if (CameraGetGain(_handle, &r_gain, &g_gain, &_) != CAMERA_STATUS_SUCCESS)
            return false;
        return CameraSetGain(_handle, r_gain, g_gain, value) == CAMERA_STATUS_SUCCESS;
    }
    case CameraProperties::wb_ggain: {
        int r_gain{}, _{}, b_gain{};
        if (CameraGetGain(_handle, &r_gain, &_, &b_gain) != CAMERA_STATUS_SUCCESS)
            return false;
        return CameraSetGain(_handle, r_gain, value, b_gain) == CAMERA_STATUS_SUCCESS;
    }
    case CameraProperties::wb_rgain: {
        int _{}, g_gain{}, b_gain{};
        if (CameraGetGain(_handle, &_, &g_gain, &b_gain) != CAMERA_STATUS_SUCCESS)
            return false;
        return CameraSetGain(_handle, value, g_gain, b_gain) == CAMERA_STATUS_SUCCESS;
    }
    case CameraProperties::trigger_count:
        return (_grab_mode != GrabMode::Continuous) ? CameraSetTriggerCount(_handle, value) == CAMERA_STATUS_SUCCESS : false;
    case CameraProperties::trigger_delay:
        return (_grab_mode == GrabMode::Hardware) ? CameraSetTriggerDelayTime(_handle, static_cast<UINT>(value)) == CAMERA_STATUS_SUCCESS : false;
#ifndef _WIN32
    case CameraProperties::trigger_period:
        return (_grab_mode != GrabMode::Continuous) ? CameraSetTriggerPeriodTime(_handle, static_cast<UINT>(value)) == CAMERA_STATUS_SUCCESS : false;
#endif // _WIN32
    case CameraProperties::gamma:
        return CameraSetGamma(_handle, value) == CAMERA_STATUS_SUCCESS;
    case CameraProperties::contrast:
        return CameraSetContrast(_handle, value) == CAMERA_STATUS_SUCCESS;
    case CameraProperties::saturation:
        return CameraSetSaturation(_handle, value) == CAMERA_STATUS_SUCCESS;
    case CameraProperties::sharpness:
        return CameraSetSharpness(_handle, value) == CAMERA_STATUS_SUCCESS;
    default:
        WARNING_("(mv) Try to set undefined variable, id: %d.", static_cast<int>(propId));
        return false;
    }
}

template <>
bool MvCamera::Impl::set(CameraProperties propId, double value) noexcept {
    switch (propId) {
    case CameraProperties::exposure:
        return CameraSetExposureTime(_handle, value) == CAMERA_STATUS_SUCCESS;
    default:
        WARNING_("(mv) Try to set undefined variable, id: %d.", static_cast<int>(propId));
        return false;
    }
}

void MvCamera::Impl::load(const para::MvCameraParam &param) {
    this->set(CameraProperties::auto_exposure, param.auto_exposure);
    this->set(CameraProperties::exposure, param.exposure);
    this->set(CameraProperties::gain, param.gain);
    this->set(CameraProperties::auto_wb, param.auto_wb);
    this->set(CameraProperties::wb_bgain, param.b_gain);
    this->set(CameraProperties::wb_ggain, param.g_gain);
    this->set(CameraProperties::wb_rgain, param.r_gain);
    this->set(CameraProperties::gamma, param.gamma);
    this->set(CameraProperties::contrast, param.contrast);
    this->set(CameraProperties::saturation, param.saturation);
    this->set(CameraProperties::sharpness, param.sharpness);
}

double MvCamera::Impl::get(CameraProperties propId) const noexcept {
    switch (propId) {
    case CameraProperties::exposure: {
        double res{};
        return CameraGetExposureTime(_handle, &res) == CAMERA_STATUS_SUCCESS ? res : -1;
    }
    case CameraProperties::gain: {
        INT res{};
        return CameraGetAnalogGain(_handle, &res) == CAMERA_STATUS_SUCCESS ? res : -1;
    }
    case CameraProperties::wb_bgain: {
        INT _r{}, _g{}, res{};
        return CameraGetGain(_handle, &_r, &_g, &res) == CAMERA_STATUS_SUCCESS ? res : -1;
    }
    case CameraProperties::wb_ggain: {
        INT _r{}, res{}, _b{};
        return CameraGetGain(_handle, &_r, &res, &_b) == CAMERA_STATUS_SUCCESS ? res : -1;
    }
    case CameraProperties::wb_rgain: {
        INT res{}, _g{}, _b{};
        return CameraGetGain(_handle, &res, &_g, &_b) == CAMERA_STATUS_SUCCESS ? res : -1;
    }
    case CameraProperties::trigger_count: {
        INT res{};
        return CameraGetTriggerCount(_handle, &res) == 0 ? res : -1;
    }
    case CameraProperties::trigger_delay: {
        UINT res{};
        return CameraGetTriggerDelayTime(_handle, &res) == 0 ? res : -1;
    }
#ifndef _WIN32
    case CameraProperties::trigger_period: {
        UINT res{};
        return CameraGetTriggerPeriodTime(_handle, &res) == 0 ? res : -1;
    }
#endif // _WIN32
    case CameraProperties::gamma: {
        INT res{};
        return CameraGetGamma(_handle, &res) == CAMERA_STATUS_SUCCESS ? res : -1;
    }
    case CameraProperties::contrast: {
        INT res{};
        return CameraGetContrast(_handle, &res) == CAMERA_STATUS_SUCCESS ? res : -1;
    }
    case CameraProperties::saturation: {
        INT res{};
        return CameraGetSaturation(_handle, &res) == CAMERA_STATUS_SUCCESS ? res : -1;
    }
    case CameraProperties::sharpness: {
        INT res{};
        return CameraGetSharpness(_handle, &res) == CAMERA_STATUS_SUCCESS ? res : -1;
    }
    case CameraProperties::frame_height:
        return _frame_info.iHeight;
    case CameraProperties::frame_width:
        return _frame_info.iWidth;
    default:
        WARNING_("(mv) Try to get undefined variable, id: %d.", static_cast<int>(propId));
        return -1;
    }
}

bool MvCamera::Impl::trigger(CameraEvents eventId) const noexcept {
    switch (eventId) {
    case CameraEvents::once_wb:
        return CameraSetOnceWB(_handle) == CAMERA_STATUS_SUCCESS;
    case CameraEvents::software:
        return (_grab_mode != GrabMode::Continuous) ? CameraSoftTrigger(_handle) == CAMERA_STATUS_SUCCESS : false;
    default:
        WARNING_("(mv) Try to trigger undefined variable, id: %d.", static_cast<int>(eventId));
        return false;
    }
}

} // namespace rm

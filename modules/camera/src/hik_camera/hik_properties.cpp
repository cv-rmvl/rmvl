/**
 * @file hik_properties.cpp
 * @author RoboMaster Vision Community
 * @brief HikRobot 工业相机参数 I/O
 * @version 1.0
 * @date 2023-06-13
 *
 * @copyright Copyright 2023 (c), RoboMaster Vision Community
 *
 */

#include "hik_camera_impl.h"

#include "rmvl/core/util.hpp"

#include "rmvlpara/camera/hik_camera.h"

namespace rm {

template <>
bool HikCamera::Impl::set(CameraProperties propId, bool value) noexcept {
    switch (propId) {
    case CameraProperties::auto_exposure:
        return MV_CC_SetEnumValue(_handle, "ExposureAuto", value ? MV_EXPOSURE_AUTO_MODE_CONTINUOUS : MV_EXPOSURE_AUTO_MODE_OFF) == MV_OK;
    case CameraProperties::auto_wb:
        return MV_CC_SetEnumValue(_handle, "BalanceWhiteAuto", value ? MV_BALANCEWHITE_AUTO_CONTINUOUS : MV_BALANCEWHITE_AUTO_OFF) == MV_OK;
    default:
        WARNING_("(hik) Try to set undefined variable, id: %d.", static_cast<int>(propId));
        return false;
    }
}

template <>
bool HikCamera::Impl::set(CameraProperties propId, uint32_t value) noexcept {
    switch (propId) {
    case CameraProperties::wb_bgain:
        return MV_CC_SetEnumValue(_handle, "BalanceRatioSelector", 2) == MV_OK && // Blue
               MV_CC_SetIntValue(_handle, "BalanceRatio", value) == MV_OK;
    case CameraProperties::wb_ggain:
        return MV_CC_SetEnumValue(_handle, "BalanceRatioSelector", 1) == MV_OK && // Green
               MV_CC_SetIntValue(_handle, "BalanceRatio", value) == MV_OK;
    case CameraProperties::wb_rgain:
        return MV_CC_SetEnumValue(_handle, "BalanceRatioSelector", 0) == MV_OK && // Red
               MV_CC_SetIntValue(_handle, "BalanceRatio", value) == MV_OK;
    case CameraProperties::trigger_count:
        return MV_CC_SetIntValue(_handle, "AcquisitionBurstFrameCount", value) == MV_OK;
    default:
        WARNING_("(hik) Try to set undefined variable, id: %d.", static_cast<int>(propId));
        return false;
    }
}

template <>
bool HikCamera::Impl::set(CameraProperties propId, float value) noexcept {
    switch (propId) {
    case CameraProperties::exposure:
        return MV_CC_SetFloatValue(_handle, "ExposureTime", value) == MV_OK;
    case CameraProperties::gain:
        return MV_CC_SetFloatValue(_handle, "Gain", value) == MV_OK;
    case CameraProperties::trigger_delay:
        return MV_CC_SetFloatValue(_handle, "TriggerDelay", static_cast<float>(value)) == MV_OK;
    case CameraProperties::saturation:
        return MV_CC_SetBoolValue(_handle, "SaturationEnable", true) == MV_OK &&
               MV_CC_SetFloatValue(_handle, "Saturation", static_cast<float>(value)) == MV_OK;
    default:
        WARNING_("(hik) Try to set undefined variable, id: %d.", static_cast<int>(propId));
        return false;
    }
}

void HikCamera::Impl::load(const para::HikCameraParam &param) noexcept {
    this->set(CameraProperties::auto_exposure, param.auto_exposure);
    this->set(CameraProperties::exposure, param.exposure);
    this->set(CameraProperties::saturation, param.saturation);
    this->set(CameraProperties::gain, param.gain);
    this->set(CameraProperties::auto_wb, param.auto_wb);
    this->set(CameraProperties::wb_bgain, param.b_gain);
    this->set(CameraProperties::wb_ggain, param.g_gain);
    this->set(CameraProperties::wb_rgain, param.r_gain);
}

bool HikCamera::Impl::trigger(CameraEvents eventId) const noexcept {
    switch (eventId) {
    case CameraEvents::once_exposure:
        return MV_CC_SetEnumValue(_handle, "ExposureAuto", MV_EXPOSURE_AUTO_MODE_ONCE) == MV_OK;
    case CameraEvents::once_wb:
        return MV_CC_SetEnumValue(_handle, "BalanceWhiteAuto", MV_BALANCEWHITE_AUTO_ONCE) == MV_OK;
    case CameraEvents::software:
        return MV_CC_SetCommandValue(_handle, "TriggerSoftware") == MV_OK;
    default:
        WARNING_("(hik) Try to trigger undefined event, id: %d.", static_cast<int>(eventId));
        return false;
    }
}

double HikCamera::Impl::get(CameraProperties propId) const noexcept {
    MVCC_FLOATVALUE f_value = {0, 0, 0, {0}};
    MVCC_INTVALUE i_value = {0, 0, 0, 0, {0}};
    switch (propId) {
    case CameraProperties::exposure:
        return MV_CC_GetFloatValue(_handle, "ExposureTime", &f_value) == MV_OK ? f_value.fCurValue : -1.0;
    case CameraProperties::gain:
        return MV_CC_GetFloatValue(_handle, "Gain", &f_value) == MV_OK ? f_value.fCurValue : -1.0;
    case CameraProperties::wb_bgain:
        return MV_CC_SetEnumValue(_handle, "BalanceRatioSelector", 2) == MV_OK && // Blue
                       MV_CC_GetIntValue(_handle, "BalanceRatio", &i_value) == MV_OK
                   ? static_cast<double>(i_value.nCurValue)
                   : -1.0;
    case CameraProperties::wb_ggain:
        return MV_CC_SetEnumValue(_handle, "BalanceRatioSelector", 1) == MV_OK && // Green
                       MV_CC_GetIntValue(_handle, "BalanceRatio", &i_value) == MV_OK
                   ? static_cast<double>(i_value.nCurValue)
                   : -1.0;
    case CameraProperties::wb_rgain:
        return MV_CC_SetEnumValue(_handle, "BalanceRatioSelector", 0) == MV_OK && // Red
                       MV_CC_GetIntValue(_handle, "BalanceRatio", &i_value) == MV_OK
                   ? static_cast<double>(i_value.nCurValue)
                   : -1.0;
    case CameraProperties::trigger_count:
        return MV_CC_GetIntValue(_handle, "AcquisitionBurstFrameCount", &i_value) == MV_OK ? static_cast<double>(i_value.nCurValue) : -1.0;
    case CameraProperties::trigger_delay:
        return MV_CC_GetFloatValue(_handle, "TriggerDelay", &f_value) == MV_OK ? static_cast<double>(f_value.fCurValue) : -1.0;
    case CameraProperties::saturation:
        return MV_CC_GetFloatValue(_handle, "Saturation", &f_value) == MV_OK ? static_cast<double>(f_value.fCurValue) : -1.0;
    default:
        WARNING_("(hik) Try to set undefined variable, id: %d.", static_cast<int>(propId));
        return -1.0;
    }
}

} // namespace rm

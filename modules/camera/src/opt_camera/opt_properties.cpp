/**
 * @file opt_properties.cpp
 * @author zhaoxi (535394140@qq.com)
 * @brief 奥普特机器视觉相机库属性设置
 * @version 1.0
 * @date 2023-12-18
 *
 * @copyright Copyright 2023 (c), zhaoxi
 *
 */

#include "opt_camera_impl.h"

#include "rmvl/core/util.hpp"

#include "rmvlpara/camera/opt_camera.h"

namespace rm {

template <>
bool OptCamera::Impl::set(CameraProperties prop_id, bool value) noexcept {
    switch (prop_id) {
    case CameraProperties::auto_exposure:
        return OPT_SetEnumFeatureSymbol(_handle, "ExposureAuto", value ? "Continuous" : "Off") == OPT_OK;
    case CameraProperties::auto_wb:
        return OPT_SetEnumFeatureSymbol(_handle, "BalanceWhiteAuto", value ? "Continuous" : "Off") == OPT_OK;
    default:
        WARNING_("(opt) Try to set an undefined variable, prop_id: %d.", static_cast<int>(prop_id));
        return false;
    }
}

template <>
bool OptCamera::Impl::set(CameraProperties prop_id, int64_t value) noexcept {
    switch (prop_id) {
    case CameraProperties::saturation:
        return OPT_SetIntFeatureValue(_handle, "Saturation", value) == OPT_OK;
    default:
        WARNING_("(opt) Try to set an undefined variable, prop_id: %d.", static_cast<int>(prop_id));
        return false;
    }
}

template <>
bool OptCamera::Impl::set(CameraProperties prop_id, double value) noexcept {
    switch (prop_id) {
    case CameraProperties::exposure:
        return OPT_SetDoubleFeatureValue(_handle, "ExposureTime", value) == OPT_OK;
    case CameraProperties::wb_rgain:
        return OPT_SetEnumFeatureSymbol(_handle, "BalanceRatioSelector", "Red") == OPT_OK &&
               OPT_SetDoubleFeatureValue(_handle, "BalanceRatio", value) == OPT_OK;
    case CameraProperties::wb_ggain:
        return OPT_SetEnumFeatureSymbol(_handle, "BalanceRatioSelector", "Green") == OPT_OK &&
               OPT_SetDoubleFeatureValue(_handle, "BalanceRatio", value) == OPT_OK;
    case CameraProperties::wb_bgain:
        return OPT_SetEnumFeatureSymbol(_handle, "BalanceRatioSelector", "Blue") == OPT_OK &&
               OPT_SetDoubleFeatureValue(_handle, "BalanceRatio", value) == OPT_OK;
    case CameraProperties::gain:
        return OPT_SetDoubleFeatureValue(_handle, "GainRaw", value) == OPT_OK;
    case CameraProperties::gamma:
        return OPT_SetBoolFeatureValue(_handle, "EnableGamma", true) == OPT_OK &&
               OPT_SetDoubleFeatureValue(_handle, "Gamma", value) == OPT_OK;
    default:
        WARNING_("(opt) Try to set an undefined variable, prop_id: %d.", static_cast<int>(prop_id));
        return false;
    }
}

bool OptCamera::Impl::trigger(CameraEvents event_id) const noexcept {
    switch (event_id) {
    default:
        WARNING_("(opt) Try to trigger an undefined variable, prop_id: %d.", static_cast<int>(event_id));
        return false;
    }
}

void OptCamera::Impl::load(const para::OptCameraParam &param) {
    this->set(CameraProperties::exposure, param.exposure);
    this->set(CameraProperties::gain, param.gain);
    this->set(CameraProperties::wb_bgain, param.b_gain);
    this->set(CameraProperties::wb_ggain, param.g_gain);
    this->set(CameraProperties::wb_rgain, param.r_gain);
    this->set(CameraProperties::gamma, param.gamma);
    this->set(CameraProperties::contrast, param.contrast);
}

double OptCamera::Impl::get(CameraProperties prop_id) const noexcept {
    if (OPT_GetDeviceInfo(_handle, _device_list.pDevInfo) != OPT_OK)
        return -1.0;
    double dretval{};
    int64_t iretval{};
    switch (prop_id) {
    case CameraProperties::exposure:
        return OPT_GetDoubleFeatureValue(_handle, "ExposureTime", &dretval) == OPT_OK ? dretval : -1.0;
    case CameraProperties::gain:
        return OPT_GetDoubleFeatureValue(_handle, "GainRaw", &dretval) == OPT_OK ? dretval : -1.0;
    case CameraProperties::wb_bgain:
        return (OPT_SetEnumFeatureSymbol(_handle, "BalanceRatioSelector", "Blue") == OPT_OK && OPT_GetDoubleFeatureValue(_handle, "BalanceRatio", &dretval) == OPT_OK) ? dretval : -1.0;
    case CameraProperties::wb_ggain:
        return (OPT_SetEnumFeatureSymbol(_handle, "BalanceRatioSelector", "Green") == OPT_OK && OPT_GetDoubleFeatureValue(_handle, "BalanceRatio", &dretval) == OPT_OK) ? dretval : -1.0;
    case CameraProperties::wb_rgain:
        return (OPT_SetEnumFeatureSymbol(_handle, "BalanceRatioSelector", "Red") == OPT_OK && OPT_GetDoubleFeatureValue(_handle, "BalanceRatio", &dretval) == OPT_OK) ? dretval : -1.0;
    case CameraProperties::gamma:
        return OPT_GetDoubleFeatureValue(_handle, "Gamma", &dretval) == OPT_OK ? dretval : -1.0;
    case CameraProperties::saturation:
        return OPT_GetIntFeatureValue(_handle, "Saturation", &iretval) == OPT_OK ? static_cast<double>(iretval) : -1.0;
    default:
        WARNING_("(opt) Try to get an undefined variable, prop_id: %d.", static_cast<int>(prop_id));
        return -1.0;
    }
}

} // namespace rm

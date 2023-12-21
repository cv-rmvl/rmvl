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

namespace rm
{

bool OptCamera::Impl::set(int propId, double value) noexcept
{
    switch (propId)
    {
    case CAMERA_AUTO_EXPOSURE:
        return OPT_SetEnumFeatureSymbol(_handle, "ExposureAuto", "Off") == OPT_OK;
    case CAMERA_EXPOSURE:
        return OPT_SetDoubleFeatureValue(_handle, "ExposureTime", value) == OPT_OK;
    case CAMERA_AUTO_WB:
        return OPT_SetEnumFeatureSymbol(_handle, "BalanceWhiteAuto", "Continuous") == OPT_OK;
    case CAMERA_MANUAL_WB:
        return OPT_SetEnumFeatureSymbol(_handle, "BalanceWhiteAuto", "Off") == OPT_OK;
    case CAMERA_WB_RGAIN:
        return OPT_SetEnumFeatureSymbol(_handle, "BalanceRatioSelector", "Red") == OPT_OK &&
               OPT_SetDoubleFeatureValue(_handle, "BalanceRatio", value) == OPT_OK;
    case CAMERA_WB_GGAIN:
        return OPT_SetEnumFeatureSymbol(_handle, "BalanceRatioSelector", "Green") == OPT_OK &&
               OPT_SetDoubleFeatureValue(_handle, "BalanceRatio", value) == OPT_OK;
    case CAMERA_WB_BGAIN:
        return OPT_SetEnumFeatureSymbol(_handle, "BalanceRatioSelector", "Blue") == OPT_OK &&
               OPT_SetDoubleFeatureValue(_handle, "BalanceRatio", value) == OPT_OK;
    case CAMERA_GAIN:
        return OPT_SetDoubleFeatureValue(_handle, "GainRaw", value) == OPT_OK;
    case CAMERA_GAMMA:
        return OPT_SetBoolFeatureValue(_handle, "EnableGamma", true) == OPT_OK &&
               OPT_SetDoubleFeatureValue(_handle, "Gamma", value) == OPT_OK;
    case CAMERA_SATURATION:
        return OPT_SetIntFeatureValue(_handle, "Saturation", static_cast<int64_t>(value)) == OPT_OK;
    default:
        ERROR_("opt - Try to set an undefined variable, propId: %d.", propId);
        return false;
    }
}

double OptCamera::Impl::get(int propId) const noexcept
{
    if (OPT_GetDeviceInfo(_handle, _device_list.pDevInfo) != OPT_OK)
        return -1.0;
    double dretval{};
    int64_t iretval{};
    switch (propId)
    {
    case CAMERA_EXPOSURE:
        return OPT_GetDoubleFeatureValue(_handle, "ExposureTime", &dretval) == OPT_OK ? dretval : -1.0;
    case CAMERA_GAIN:
        return OPT_GetDoubleFeatureValue(_handle, "GainRaw", &dretval) == OPT_OK ? dretval : -1.0;
    case CAMERA_WB_BGAIN:
        return (OPT_SetEnumFeatureSymbol(_handle, "BalanceRatioSelector", "Blue") == OPT_OK && OPT_GetDoubleFeatureValue(_handle, "BalanceRatio", &dretval) == OPT_OK) ? dretval : -1.0;
    case CAMERA_WB_GGAIN:
        return (OPT_SetEnumFeatureSymbol(_handle, "BalanceRatioSelector", "Green") == OPT_OK && OPT_GetDoubleFeatureValue(_handle, "BalanceRatio", &dretval) == OPT_OK) ? dretval : -1.0;
    case CAMERA_WB_RGAIN:
        return (OPT_SetEnumFeatureSymbol(_handle, "BalanceRatioSelector", "Red") == OPT_OK && OPT_GetDoubleFeatureValue(_handle, "BalanceRatio", &dretval) == OPT_OK) ? dretval : -1.0;
    case CAMERA_GAMMA:
        return OPT_GetDoubleFeatureValue(_handle, "Gamma", &dretval) == OPT_OK ? dretval : -1.0;
    case CAMERA_SATURATION:
        return OPT_GetIntFeatureValue(_handle, "Saturation", &iretval) == OPT_OK ? static_cast<double>(iretval) : -1.0;
    default:
        ERROR_("opt - try to get an undefined variable, propId: %d.", propId);
        return -1.0;
    }
}

} // namespace rm

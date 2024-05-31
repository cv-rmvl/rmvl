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

namespace rm
{

bool HikCamera::Impl::set(int propId, double value) noexcept
{
    switch (propId)
    {
    // Properties
    case CAMERA_AUTO_EXPOSURE:
        return MV_CC_SetEnumValue(_handle, "ExposureAuto", MV_EXPOSURE_AUTO_MODE_CONTINUOUS) == MV_OK;
    case CAMERA_MANUAL_EXPOSURE:
        return MV_CC_SetEnumValue(_handle, "ExposureAuto", MV_EXPOSURE_AUTO_MODE_OFF) == MV_OK;
    case CAMERA_ONCE_EXPOSURE:
        return MV_CC_SetEnumValue(_handle, "ExposureAuto", MV_EXPOSURE_AUTO_MODE_ONCE) == MV_OK;
    case CAMERA_AUTO_WB:
        return MV_CC_SetEnumValue(_handle, "BalanceWhiteAuto", MV_BALANCEWHITE_AUTO_CONTINUOUS) == MV_OK;
    case CAMERA_MANUAL_WB:
        return MV_CC_SetEnumValue(_handle, "BalanceWhiteAuto", MV_BALANCEWHITE_AUTO_OFF) == MV_OK;
    case CAMERA_ONCE_WB:
        return MV_CC_SetEnumValue(_handle, "BalanceWhiteAuto", MV_BALANCEWHITE_AUTO_ONCE) == MV_OK;
    case CAMERA_EXPOSURE:
        return MV_CC_SetFloatValue(_handle, "ExposureTime", static_cast<float>(value)) == MV_OK;
    case CAMERA_GAIN:
        return MV_CC_SetFloatValue(_handle, "Gain", static_cast<float>(value)) == MV_OK;
    case CAMERA_WB_BGAIN:
        return MV_CC_SetEnumValue(_handle, "BalanceRatioSelector", 2) == MV_OK && // Blue
               MV_CC_SetIntValue(_handle, "BalanceRatio", static_cast<unsigned int>(value)) == MV_OK;
    case CAMERA_WB_GGAIN:
        return MV_CC_SetEnumValue(_handle, "BalanceRatioSelector", 1) == MV_OK && // Green
               MV_CC_SetIntValue(_handle, "BalanceRatio", static_cast<unsigned int>(value)) == MV_OK;
    case CAMERA_WB_RGAIN:
        return MV_CC_SetEnumValue(_handle, "BalanceRatioSelector", 0) == MV_OK && // Red
               MV_CC_SetIntValue(_handle, "BalanceRatio", static_cast<unsigned int>(value)) == MV_OK;
    case CAMERA_TRIGGER_COUNT:
        return MV_CC_SetIntValue(_handle, "AcquisitionBurstFrameCount", static_cast<unsigned int>(value)) == MV_OK;
    case CAMERA_TRIGGER_DELAY:
        return MV_CC_SetFloatValue(_handle, "TriggerDelay", static_cast<float>(value)) == MV_OK;
    case CAMERA_SATURATION:
        return MV_CC_SetBoolValue(_handle, "SaturationEnable", true) == MV_OK &&
               MV_CC_SetFloatValue(_handle, "Saturation", static_cast<float>(value)) == MV_OK;
    case CAMERA_TRIGGER_SOFT:
        return MV_CC_SetCommandValue(_handle, "TriggerSoftware") == MV_OK;
    default:
        ERROR_("Try to set undefined variable, id: %d.", propId);
        return false;
    }
}

double HikCamera::Impl::get(int propId) const noexcept
{
    MVCC_FLOATVALUE f_value = {0, 0, 0, {0}};
    MVCC_INTVALUE i_value = {0, 0, 0, 0, {0}};
    switch (propId)
    {
    // Properties
    case CAMERA_EXPOSURE:
        return MV_CC_GetFloatValue(_handle, "ExposureTime", &f_value) == MV_OK ? f_value.fCurValue : -1;
    case CAMERA_GAIN:
        return MV_CC_GetFloatValue(_handle, "Gain", &f_value) == MV_OK ? f_value.fCurValue : -1;
    case CAMERA_WB_BGAIN:
        return MV_CC_SetEnumValue(_handle, "BalanceRatioSelector", 2) == MV_OK && // Blue
                       MV_CC_GetIntValue(_handle, "BalanceRatio", &i_value) == MV_OK
                   ? static_cast<double>(i_value.nCurValue)
                   : -1;
    case CAMERA_WB_GGAIN:
        return MV_CC_SetEnumValue(_handle, "BalanceRatioSelector", 1) == MV_OK && // Green
                       MV_CC_GetIntValue(_handle, "BalanceRatio", &i_value) == MV_OK
                   ? static_cast<double>(i_value.nCurValue)
                   : -1;
    case CAMERA_WB_RGAIN:
        return MV_CC_SetEnumValue(_handle, "BalanceRatioSelector", 0) == MV_OK && // Red
                       MV_CC_GetIntValue(_handle, "BalanceRatio", &i_value) == MV_OK
                   ? static_cast<double>(i_value.nCurValue)
                   : -1;
    case CAMERA_TRIGGER_COUNT:
        return MV_CC_GetIntValue(_handle, "AcquisitionBurstFrameCount", &i_value) == MV_OK ? static_cast<double>(i_value.nCurValue) : -1;
    case CAMERA_TRIGGER_DELAY:
        return MV_CC_GetFloatValue(_handle, "TriggerDelay", &f_value) == MV_OK ? static_cast<double>(f_value.fCurValue) : -1;
    case CAMERA_SATURATION:
        return MV_CC_GetFloatValue(_handle, "Saturation", &f_value) == MV_OK ? static_cast<double>(f_value.fCurValue) : -1;
    default:
        ERROR_("Try to set undefined variable, id: %d.", propId);
        return false;
    }
}

} // namespace rm

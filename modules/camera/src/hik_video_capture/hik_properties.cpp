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

#include "rmvl/camera/logging.h"
#include "rmvl/camera/hik_video_capture.h"

using namespace rm;
using namespace std;
using namespace cv;

bool HikVideoCapture::set(int propId, double value)
{
    switch (propId)
    {
    // Properties
    case CAP_PROP_RM_AUTO_EXPOSURE:
        return MV_CC_SetEnumValue(_handle, "ExposureAuto", 2) == MV_OK; // Continuous
    case CAP_PROP_RM_MANUAL_EXPOSURE:
        return MV_CC_SetEnumValue(_handle, "ExposureAuto", 0) == MV_OK; // Off
    case CAP_PROP_RM_EXPOSURE:
        return MV_CC_SetFloatValue(_handle, "ExposureTime", static_cast<float>(value)) == MV_OK;
    case CAP_PROP_RM_GAIN:
        return MV_CC_SetFloatValue(_handle, "Gain", static_cast<float>(value)) == MV_OK;
    case CAP_PROP_RM_AUTO_WB:
        return MV_CC_SetEnumValue(_handle, "BalanceWhiteAuto", 1) == MV_OK; // Continuous
    case CAP_PROP_RM_MANUAL_WB:
        return MV_CC_SetEnumValue(_handle, "BalanceWhiteAuto", 0) == MV_OK; // Off
    case CAP_PROP_RM_WB_BGAIN:
        return MV_CC_SetEnumValue(_handle, "BalanceRatioSelector", 2) == MV_OK && // Blue
               MV_CC_SetIntValue(_handle, "BalanceRatio", static_cast<unsigned int>(value)) == MV_OK;
    case CAP_PROP_RM_WB_GGAIN:
        return MV_CC_SetEnumValue(_handle, "BalanceRatioSelector", 1) == MV_OK && // Green
               MV_CC_SetIntValue(_handle, "BalanceRatio", static_cast<unsigned int>(value)) == MV_OK;
    case CAP_PROP_RM_WB_RGAIN:
        return MV_CC_SetEnumValue(_handle, "BalanceRatioSelector", 0) == MV_OK && // Red
               MV_CC_SetIntValue(_handle, "BalanceRatio", static_cast<unsigned int>(value)) == MV_OK;
    case CAP_PROP_RM_TRIGGER_COUNT:
        return MV_CC_SetIntValue(_handle, "AcquisitionBurstFrameCount", static_cast<unsigned int>(value)) == MV_OK;
    case CAP_PROP_RM_TRIGGER_DELAY:
        return MV_CC_SetFloatValue(_handle, "TriggerDelay", static_cast<float>(value)) == MV_OK;
    case CAP_PROP_RM_SATURATION:
        return MV_CC_SetBoolValue(_handle, "SaturationEnable", true) == MV_OK &&
               MV_CC_SetFloatValue(_handle, "Saturation", static_cast<float>(value)) == MV_OK;
    // Activities
    case CAP_ACT_RM_ONCE_WB:
        return MV_CC_SetEnumValue(_handle, "BalanceWhiteAuto", 2) == MV_OK; // Once
    case CAP_ACT_RM_SOFT_TRIGGER:
        return MV_CC_SetCommandValue(_handle, "TriggerSoftware") == MV_OK;
    default:
        CAM_ERROR("Try to set undefined variable, id: %d.", propId);
        return false;
    }
}

double HikVideoCapture::get(int propId) const
{
    MVCC_FLOATVALUE f_value = {.fCurValue = 0, .fMax = 0, .fMin = 0, .nReserved = {0}};
    MVCC_INTVALUE i_value = {.nCurValue = 0, .nMax = 0, .nMin = 0, .nInc = 0, .nReserved = {0}};
    switch (propId)
    {
    // Properties
    case CAP_PROP_RM_EXPOSURE:
        return MV_CC_GetFloatValue(_handle, "ExposureTime", &f_value) == MV_OK ? f_value.fCurValue : -1;
    case CAP_PROP_RM_GAIN:
        return MV_CC_GetFloatValue(_handle, "Gain", &f_value) == MV_OK ? f_value.fCurValue : -1;
    case CAP_PROP_RM_WB_BGAIN:
        return MV_CC_SetEnumValue(_handle, "BalanceRatioSelector", 2) == MV_OK && // Blue
                       MV_CC_GetIntValue(_handle, "BalanceRatio", &i_value) == MV_OK
                   ? static_cast<double>(i_value.nCurValue)
                   : -1;
    case CAP_PROP_RM_WB_GGAIN:
        return MV_CC_SetEnumValue(_handle, "BalanceRatioSelector", 1) == MV_OK && // Green
                       MV_CC_GetIntValue(_handle, "BalanceRatio", &i_value) == MV_OK
                   ? static_cast<double>(i_value.nCurValue)
                   : -1;
    case CAP_PROP_RM_WB_RGAIN:
        return MV_CC_SetEnumValue(_handle, "BalanceRatioSelector", 0) == MV_OK && // Red
                       MV_CC_GetIntValue(_handle, "BalanceRatio", &i_value) == MV_OK
                   ? static_cast<double>(i_value.nCurValue)
                   : -1;
    case CAP_PROP_RM_TRIGGER_COUNT:
        return MV_CC_GetIntValue(_handle, "AcquisitionBurstFrameCount", &i_value) == MV_OK ? static_cast<double>(i_value.nCurValue) : -1;
    case CAP_PROP_RM_TRIGGER_DELAY:
        return MV_CC_GetFloatValue(_handle, "TriggerDelay", &f_value) == MV_OK ? static_cast<double>(f_value.fCurValue) : -1;
    case CAP_PROP_RM_SATURATION:
        return MV_CC_GetFloatValue(_handle, "Saturation", &f_value) == MV_OK ? static_cast<double>(f_value.fCurValue) : -1;
    default:
        CAM_ERROR("Try to set undefined variable, id: %d.", propId);
        return false;
    }
}
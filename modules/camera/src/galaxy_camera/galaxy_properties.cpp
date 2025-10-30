/**
 * @file galaxy_properties.cpp
 * @author zhaoxi (535394140@qq.com)
 * @brief
 * @version 1.0
 * @date 2024-10-28
 *
 * @copyright Copyright 2024 (c), zhaoxi
 *
 */

#include <GxIAPI.h>

#include "galaxy_camera_impl.h"

#include "rmvl/core/util.hpp"

#include "rmvlpara/camera/galaxy_camera.h"

namespace rm {

void GalaxyCamera::Impl::load(const para::GalaxyCameraParam &param) {
    this->set(CAMERA_EXPOSURE, param.exposure);
    this->set(CAMERA_SATURATION, param.saturation);
    this->set(CAMERA_GAIN, param.gain);
    this->set(CAMERA_WB_BGAIN, param.b_gain);
    this->set(CAMERA_WB_GGAIN, param.g_gain);
    this->set(CAMERA_WB_RGAIN, param.r_gain);
}

bool GalaxyCamera::Impl::set(int prop_id, double value) noexcept {
    switch (prop_id) {
    // Properties
    case CAMERA_AUTO_EXPOSURE:
        return GXSetEnum(_handle, GX_ENUM_EXPOSURE_AUTO, GX_EXPOSURE_AUTO_CONTINUOUS) == GX_STATUS_SUCCESS;
    case CAMERA_MANUAL_EXPOSURE:
        return GXSetEnum(_handle, GX_ENUM_EXPOSURE_AUTO, GX_EXPOSURE_AUTO_OFF) == GX_STATUS_SUCCESS;
    case CAMERA_ONCE_EXPOSURE:
        return GXSetEnum(_handle, GX_ENUM_EXPOSURE_AUTO, GX_EXPOSURE_AUTO_ONCE) == GX_STATUS_SUCCESS;
    case CAMERA_AUTO_WB:
        return GXSetEnum(_handle, GX_ENUM_BALANCE_WHITE_AUTO, GX_BALANCE_WHITE_AUTO_CONTINUOUS) == GX_STATUS_SUCCESS;
    case CAMERA_MANUAL_WB:
        return GXSetEnum(_handle, GX_ENUM_BALANCE_WHITE_AUTO, GX_BALANCE_WHITE_AUTO_OFF) == GX_STATUS_SUCCESS;
    case CAMERA_ONCE_WB:
        return GXSetEnum(_handle, GX_ENUM_BALANCE_WHITE_AUTO, GX_BALANCE_WHITE_AUTO_ONCE) == GX_STATUS_SUCCESS;
    case CAMERA_EXPOSURE:
        return GXSetFloat(_handle, GX_FLOAT_EXPOSURE_TIME, value) == GX_STATUS_SUCCESS;
    case CAMERA_GAIN:
        return GXSetFloat(_handle, GX_FLOAT_GAIN, value) == GX_STATUS_SUCCESS;
    case CAMERA_WB_BGAIN:
        return GXSetEnum(_handle, GX_ENUM_BALANCE_RATIO_SELECTOR, GX_BALANCE_RATIO_SELECTOR_BLUE) == GX_STATUS_SUCCESS &&
               GXSetFloat(_handle, GX_FLOAT_BALANCE_RATIO, value) == GX_STATUS_SUCCESS;
    case CAMERA_WB_GGAIN:
        return GXSetEnum(_handle, GX_ENUM_BALANCE_RATIO_SELECTOR, GX_BALANCE_RATIO_SELECTOR_GREEN) == GX_STATUS_SUCCESS &&
               GXSetFloat(_handle, GX_FLOAT_BALANCE_RATIO, value) == GX_STATUS_SUCCESS;
    case CAMERA_WB_RGAIN:
        return GXSetEnum(_handle, GX_ENUM_BALANCE_RATIO_SELECTOR, GX_BALANCE_RATIO_SELECTOR_RED) == GX_STATUS_SUCCESS &&
               GXSetFloat(_handle, GX_FLOAT_BALANCE_RATIO, value) == GX_STATUS_SUCCESS;
    case CAMERA_TRIGGER_COUNT:
        if (static_cast<int64_t>(value) == 1)
            return GXSetEnum(_handle, GX_ENUM_ACQUISITION_MODE, GX_ACQ_MODE_SINGLE_FRAME) == GX_STATUS_SUCCESS;
        else if (static_cast<int64_t>(value) > 1)
            return GXSetEnum(_handle, GX_ENUM_ACQUISITION_MODE, GX_ACQ_MODE_MULITI_FRAME) == GX_STATUS_SUCCESS &&
                   GXSetInt(_handle, GX_INT_ACQUISITION_FRAME_COUNT, static_cast<int64_t>(value)) == GX_STATUS_SUCCESS;
        else {
            ERROR_("(galaxy) Invalid trigger count, please use a positive integer.");
            return false;
        }
    case CAMERA_TRIGGER_DELAY:
        return GXSetFloat(_handle, GX_FLOAT_TRIGGER_DELAY, value) == GX_STATUS_SUCCESS;
    case CAMERA_SATURATION:
        if (value < 0)
            return GXSetEnum(_handle, GX_ENUM_SATURATION_MODE, GX_ENUM_SATURATION_OFF) == GX_STATUS_SUCCESS;
        else
            return GXSetEnum(_handle, GX_ENUM_SATURATION_MODE, GX_ENUM_SATURATION_ON) == GX_STATUS_SUCCESS &&
                   GXSetInt(_handle, GX_INT_SATURATION, static_cast<int64_t>(value)) == GX_STATUS_SUCCESS;
    case CAMERA_TRIGGER_SOFT:
        return GXSendCommand(_handle, GX_COMMAND_TRIGGER_SOFTWARE) == GX_STATUS_SUCCESS;
    default:
        WARNING_("(galaxy) try to set undefined variable, id: %d.", prop_id);
        return false;
    }
    return false;
}

double GalaxyCamera::Impl::get(int prop_id) const noexcept {
    double f_value{};
    int64_t i_value{};
    switch (prop_id) {
    // Properties
    case CAMERA_EXPOSURE:
        return GXGetFloat(_handle, GX_FLOAT_EXPOSURE_TIME, &f_value) == GX_STATUS_SUCCESS ? f_value : -1.0;
    case CAMERA_GAIN:
        return GXGetFloat(_handle, GX_FLOAT_GAIN, &f_value) == GX_STATUS_SUCCESS ? f_value : -1.0;
    case CAMERA_WB_BGAIN:
        return (GXSetEnum(_handle, GX_ENUM_BALANCE_RATIO_SELECTOR, GX_BALANCE_RATIO_SELECTOR_BLUE) == GX_STATUS_SUCCESS &&
                GXGetFloat(_handle, GX_FLOAT_BALANCE_RATIO, &f_value) == GX_STATUS_SUCCESS)
                   ? f_value
                   : -1.0;
    case CAMERA_WB_GGAIN:
        return (GXSetEnum(_handle, GX_ENUM_BALANCE_RATIO_SELECTOR, GX_BALANCE_RATIO_SELECTOR_GREEN) == GX_STATUS_SUCCESS &&
                GXGetFloat(_handle, GX_FLOAT_BALANCE_RATIO, &f_value) == GX_STATUS_SUCCESS)
                   ? f_value
                   : -1.0;
    case CAMERA_WB_RGAIN:
        return (GXSetEnum(_handle, GX_ENUM_BALANCE_RATIO_SELECTOR, GX_BALANCE_RATIO_SELECTOR_RED) == GX_STATUS_SUCCESS &&
                GXGetFloat(_handle, GX_FLOAT_BALANCE_RATIO, &f_value) == GX_STATUS_SUCCESS)
                   ? f_value
                   : -1.0;
    case CAMERA_TRIGGER_COUNT:
        return GXGetInt(_handle, GX_INT_ACQUISITION_FRAME_COUNT, &i_value) == GX_STATUS_SUCCESS ? i_value : -1.0;
    case CAMERA_TRIGGER_DELAY:
        return GXGetFloat(_handle, GX_FLOAT_TRIGGER_DELAY, &f_value) == GX_STATUS_SUCCESS ? f_value : -1.0;
    case CAMERA_SATURATION:
        return GXGetInt(_handle, GX_INT_SATURATION, &i_value) == GX_STATUS_SUCCESS ? i_value : -1.0;
    default:
        WARNING_("(galaxy) Try to set undefined variable, id: %d.", prop_id);
        return -1.0;
    }
}

} // namespace rm

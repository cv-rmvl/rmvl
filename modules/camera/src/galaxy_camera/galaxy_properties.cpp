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

template <>
bool GalaxyCamera::Impl::set(CameraProperties prop_id, bool value) noexcept {
    switch (prop_id) {
    case CameraProperties::auto_exposure:
        return GXSetEnum(_handle, GX_ENUM_EXPOSURE_AUTO, value ? GX_EXPOSURE_AUTO_CONTINUOUS : GX_EXPOSURE_AUTO_OFF) == GX_STATUS_SUCCESS;
    case CameraProperties::auto_wb:
        return GXSetEnum(_handle, GX_ENUM_BALANCE_WHITE_AUTO, value ? GX_BALANCE_WHITE_AUTO_CONTINUOUS : GX_BALANCE_WHITE_AUTO_OFF) == GX_STATUS_SUCCESS;
    default:
        WARNING_("(galaxy) try to set undefined variable, id: %d.", static_cast<int>(prop_id));
        return false;
    }
}

template <>
bool GalaxyCamera::Impl::set(CameraProperties prop_id, int64_t value) noexcept {
    switch (prop_id) {
    case CameraProperties::trigger_count:
        if (value == 1L)
            return GXSetEnum(_handle, GX_ENUM_ACQUISITION_MODE, GX_ACQ_MODE_SINGLE_FRAME) == GX_STATUS_SUCCESS;
        else if (value > 1L)
            return GXSetEnum(_handle, GX_ENUM_ACQUISITION_MODE, GX_ACQ_MODE_MULITI_FRAME) == GX_STATUS_SUCCESS &&
                   GXSetInt(_handle, GX_INT_ACQUISITION_FRAME_COUNT, static_cast<int64_t>(value)) == GX_STATUS_SUCCESS;
        else {
            ERROR_("(galaxy) Invalid trigger count, please use a positive integer.");
            return false;
        }
    case CameraProperties::saturation:
        if (value < 0)
            return GXSetEnum(_handle, GX_ENUM_SATURATION_MODE, GX_ENUM_SATURATION_OFF) == GX_STATUS_SUCCESS;
        else
            return GXSetEnum(_handle, GX_ENUM_SATURATION_MODE, GX_ENUM_SATURATION_ON) == GX_STATUS_SUCCESS &&
                   GXSetInt(_handle, GX_INT_SATURATION, static_cast<int64_t>(value)) == GX_STATUS_SUCCESS;
    default:
        WARNING_("(galaxy) try to set undefined variable, id: %d.", static_cast<int>(prop_id));
        return false;
    }
}

template <>
bool GalaxyCamera::Impl::set(CameraProperties prop_id, double value) noexcept {
    switch (prop_id) {
    case CameraProperties::exposure:
        return GXSetFloat(_handle, GX_FLOAT_EXPOSURE_TIME, value) == GX_STATUS_SUCCESS;
    case CameraProperties::gain:
        return GXSetFloat(_handle, GX_FLOAT_GAIN, value) == GX_STATUS_SUCCESS;
    case CameraProperties::wb_bgain:
        return GXSetEnum(_handle, GX_ENUM_BALANCE_RATIO_SELECTOR, GX_BALANCE_RATIO_SELECTOR_BLUE) == GX_STATUS_SUCCESS &&
               GXSetFloat(_handle, GX_FLOAT_BALANCE_RATIO, value) == GX_STATUS_SUCCESS;
    case CameraProperties::wb_ggain:
        return GXSetEnum(_handle, GX_ENUM_BALANCE_RATIO_SELECTOR, GX_BALANCE_RATIO_SELECTOR_GREEN) == GX_STATUS_SUCCESS &&
               GXSetFloat(_handle, GX_FLOAT_BALANCE_RATIO, value) == GX_STATUS_SUCCESS;
    case CameraProperties::wb_rgain:
        return GXSetEnum(_handle, GX_ENUM_BALANCE_RATIO_SELECTOR, GX_BALANCE_RATIO_SELECTOR_RED) == GX_STATUS_SUCCESS &&
               GXSetFloat(_handle, GX_FLOAT_BALANCE_RATIO, value) == GX_STATUS_SUCCESS;
    case CameraProperties::trigger_delay:
        return GXSetFloat(_handle, GX_FLOAT_TRIGGER_DELAY, value) == GX_STATUS_SUCCESS;
    default:
        WARNING_("(galaxy) try to set undefined variable, id: %d.", static_cast<int>(prop_id));
        return false;
    }
    return false;
}

void GalaxyCamera::Impl::load(const para::GalaxyCameraParam &param) {
    this->set(CameraProperties::exposure, param.exposure);
    this->set(CameraProperties::saturation, param.saturation);
    this->set(CameraProperties::gain, param.gain);
    this->set(CameraProperties::wb_bgain, param.b_gain);
    this->set(CameraProperties::wb_ggain, param.g_gain);
    this->set(CameraProperties::wb_rgain, param.r_gain);
}

double GalaxyCamera::Impl::get(CameraProperties prop_id) const noexcept {
    double f_value{};
    int64_t i_value{};
    switch (prop_id) {
    case CameraProperties::exposure:
        return GXGetFloat(_handle, GX_FLOAT_EXPOSURE_TIME, &f_value) == GX_STATUS_SUCCESS ? f_value : -1.0;
    case CameraProperties::gain:
        return GXGetFloat(_handle, GX_FLOAT_GAIN, &f_value) == GX_STATUS_SUCCESS ? f_value : -1.0;
    case CameraProperties::wb_bgain:
        return (GXSetEnum(_handle, GX_ENUM_BALANCE_RATIO_SELECTOR, GX_BALANCE_RATIO_SELECTOR_BLUE) == GX_STATUS_SUCCESS &&
                GXGetFloat(_handle, GX_FLOAT_BALANCE_RATIO, &f_value) == GX_STATUS_SUCCESS)
                   ? f_value
                   : -1.0;
    case CameraProperties::wb_ggain:
        return (GXSetEnum(_handle, GX_ENUM_BALANCE_RATIO_SELECTOR, GX_BALANCE_RATIO_SELECTOR_GREEN) == GX_STATUS_SUCCESS &&
                GXGetFloat(_handle, GX_FLOAT_BALANCE_RATIO, &f_value) == GX_STATUS_SUCCESS)
                   ? f_value
                   : -1.0;
    case CameraProperties::wb_rgain:
        return (GXSetEnum(_handle, GX_ENUM_BALANCE_RATIO_SELECTOR, GX_BALANCE_RATIO_SELECTOR_RED) == GX_STATUS_SUCCESS &&
                GXGetFloat(_handle, GX_FLOAT_BALANCE_RATIO, &f_value) == GX_STATUS_SUCCESS)
                   ? f_value
                   : -1.0;
    case CameraProperties::trigger_count:
        return GXGetInt(_handle, GX_INT_ACQUISITION_FRAME_COUNT, &i_value) == GX_STATUS_SUCCESS ? i_value : -1.0;
    case CameraProperties::trigger_delay:
        return GXGetFloat(_handle, GX_FLOAT_TRIGGER_DELAY, &f_value) == GX_STATUS_SUCCESS ? f_value : -1.0;
    case CameraProperties::saturation:
        return GXGetInt(_handle, GX_INT_SATURATION, &i_value) == GX_STATUS_SUCCESS ? i_value : -1.0;
    default:
        WARNING_("(galaxy) Try to set undefined variable, id: %d.", static_cast<int>(prop_id));
        return -1.0;
    }
}

bool GalaxyCamera::Impl::trigger(CameraEvents event_id) const noexcept {
    switch (event_id) {
    case CameraEvents::once_exposure:
        return GXSetEnum(_handle, GX_ENUM_EXPOSURE_AUTO, GX_EXPOSURE_AUTO_ONCE) == GX_STATUS_SUCCESS;
    case CameraEvents::once_wb:
        return GXSetEnum(_handle, GX_ENUM_BALANCE_WHITE_AUTO, GX_BALANCE_WHITE_AUTO_ONCE) == GX_STATUS_SUCCESS;
    case CameraEvents::software:
        return GXSendCommand(_handle, GX_COMMAND_TRIGGER_SOFTWARE) == GX_STATUS_SUCCESS;
    default:
        WARNING_("(galaxy) Try to trigger undefined variable, id: %d.", static_cast<int>(event_id));
        return false;
    }
}

} // namespace rm

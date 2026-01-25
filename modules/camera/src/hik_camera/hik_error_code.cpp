/**
 * @file hik_error_code.cpp
 * @author RoboMaster Vision Community
 * @brief HikRobot 工业相机库错误码
 * @version 1.0
 * @date 2023-06-15
 *
 * @copyright Copyright 2023 (c), RoboMaster Vision Community
 *
 */

#include "hik_camera_impl.h"

namespace rm {

const char *HikCamera::Impl::errorCode2Str(unsigned int code) noexcept {
    switch (code) {
    case MV_E_HANDLE:
        return "Error or invalid handle";
    case MV_E_SUPPORT:
        return "Not supported function";
    case MV_E_BUFOVER:
        return "Buffer overflow";
    case MV_E_CALLORDER:
        return "Function calling order error";
    case MV_E_PARAMETER:
        return "Incorrect parameter";
    case MV_E_RESOURCE:
        return "Applying resource failed";
    case MV_E_NODATA:
        return "No data";
    case MV_E_PRECONDITION:
        return "Precondition error, or running environment changed";
    case MV_E_VERSION:
        return "Version mismatches";
    case MV_E_NOENOUGH_BUF:
        return "Insufficient memory";
    case MV_E_ABNORMAL_IMAGE:
        return "Abnormal image, maybe incomplete image because of lost packet";
    case MV_E_LOAD_LIBRARY:
        return "Load library failed";
    case MV_E_NOOUTBUF:
        return "No Avaliable Buffer";
    case MV_E_UNKNOW:
        return "Unknown error";
    case MV_E_GC_GENERIC:
        return "General error";
    case MV_E_GC_ARGUMENT:
        return "Illegal parameters";
    case MV_E_GC_RANGE:
        return "The value is out of range";
    case MV_E_GC_PROPERTY:
        return "Property";
    case MV_E_GC_RUNTIME:
        return "Running environment error";
    case MV_E_GC_LOGICAL:
        return "Logical error";
    case MV_E_GC_ACCESS:
        return "Node accessing condition error";
    case MV_E_GC_TIMEOUT:
        return "Timeout";
    case MV_E_GC_DYNAMICCAST:
        return "Transformation exception";
    case MV_E_GC_UNKNOW:
        return "GenICam unknown error";
    case MV_E_NOT_IMPLEMENTED:
        return "The command is not supported by device";
    case MV_E_INVALID_ADDRESS:
        return "The target address being accessed does not exist";
    case MV_E_WRITE_PROTECT:
        return "The target address is not writable";
    case MV_E_ACCESS_DENIED:
        return "No permission";
    case MV_E_BUSY:
        return "Device is busy, or network disconnected";
    case MV_E_PACKET:
        return "Network data packet error";
    case MV_E_NETER:
        return "Network error";
    case MV_E_IP_CONFLICT:
        return "Device IP conflict";
    case MV_E_USB_READ:
        return "Reading USB error";
    case MV_E_USB_WRITE:
        return "Writing USB error";
    case MV_E_USB_DEVICE:
        return "Device exception";
    case MV_E_USB_GENICAM:
        return "GenICam error";
    case MV_E_USB_BANDWIDTH:
        return "Insufficient bandwidth, this error code is newly added";
    case MV_E_USB_DRIVER:
        return "Driver mismatch or unmounted drive";
    case MV_E_USB_UNKNOW:
        return "USB unknown error";
    case MV_E_UPG_FILE_MISMATCH:
        return "Firmware mismatches";
    case MV_E_UPG_LANGUSGE_MISMATCH:
        return "Firmware language mismatches";
    case MV_E_UPG_CONFLICT:
        return "Upgrading conflicted (repeated upgrading requests during device upgrade)";
    case MV_E_UPG_INNER_ERR:
        return "Camera internal error during upgrade";
    case MV_E_UPG_UNKNOW:
        return "Unknown error during upgrade";
    default:
        return "Successed, no error";
    }
}

} // namespace rm

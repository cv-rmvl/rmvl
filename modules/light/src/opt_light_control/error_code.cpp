/**
 * @file error_code.cpp
 * @author 赵曦 (535394140@qq.com)
 * @brief
 * @version 1.0
 * @date 2022-09-27
 *
 * @copyright Copyright (c) 2023, zhaoxi
 *
 */

#include <string>
#include <unordered_map>

#include <OPTErrorCode.h>

#include "rmvl/light/opt_light_control.h"

using namespace std;
using namespace rm;

void LightController::initErrorCode()
{
    _error_code[OPT_SUCCEED] = "operation succeed";
    _error_code[OPT_ERR_INVALIDHANDLE] = "invalid handle";
    _error_code[OPT_ERR_UNKNOWN] = "error unknown";
    _error_code[OPT_ERR_INITSERIAL_FAILED] = "failed to initialize a serial port";
    _error_code[OPT_ERR_RELEASESERIALPORT_FAILED] = "failed to release a serial port";
    _error_code[OPT_ERR_SERIALPORT_UNOPENED] = "attempt to access an unopened serial port";
    _error_code[OPT_ERR_CREATEETHECON_FAILED] = "failed to create an Ethernet connection";
    _error_code[OPT_ERR_DESTORYETHECON_FAILED] = "failed to destroy an Ethernet connection";
    _error_code[OPT_ERR_SN_NOTFOUND] = "SN is not found";
    _error_code[OPT_ERR_TURNONCH_FAILED] = "failed to turn on the specified channel(s)";
    _error_code[OPT_ERR_TURNOFFCH_FAILED] = "failed to turn off the specified channel(s)";
    _error_code[OPT_ERR_SET_INTENSITY_FAILED] = "failed to set the intensity for the specified channel(s)";
    _error_code[OPT_ERR_READ_INTENSITY_FAILED] = "failed to read the intensity for the specified channel(s)";
    _error_code[OPT_ERR_SET_TRIGGERWIDTH_FAILED] = "failed to set trigger pulse width";
    _error_code[OPT_ERR_READ_TRIGGERWIDTH_FAILED] = "failed to read trigger pulse width";
    _error_code[OPT_ERR_READ_HBTRIGGERWIDTH_FAILED] = "failed to read high brightness trigger pulse width";
    _error_code[OPT_ERR_SET_HBTRIGGERWIDTH_FAILED] = "failed to set high brightness trigger pulse width";
    _error_code[OPT_ERR_READ_SN_FAILED] = "failed to read serial number";
    _error_code[OPT_ERR_READ_IPCONFIG_FAILED] = "failed to read IP address";
    _error_code[OPT_ERR_CHINDEX_OUTRANGE] = "index(es) out of the range";
    _error_code[OPT_ERR_WRITE_FAILED] = "failed to write data";
    _error_code[OPT_ERR_PARAM_OUTRANGE] = "parameter(s) out of the range";
    _error_code[OPT_ERR_READ_MAC_FAILED] = "failed to read MAC";
    _error_code[OPT_ERR_SET_MAXCURRENT_FAILED] = "failed to set max current";
    _error_code[OPT_ERR_READ_MAXCURRENT_FAILED] = "failed to read max current";
    _error_code[OPT_ERR_SET_TRIGGERACTIVATION_FAILED] = "failed to set trigger activation";
    _error_code[OPT_ERR_READ_TRIGGERACTIVATION_FAILED] = "failed to read trigger activation";
    _error_code[OPT_ERR_SET_WORKMODE_FAILED] = "failed to set work mode";
    _error_code[OPT_ERR_READ_WORKMODE_FAILED] = "failed to read work mode";
    _error_code[OPT_ERR_SET_BAUDRATE_FAILED] = "failed to set baud rate";
    _error_code[OPT_ERR_SET_CHANNELAMOUNT_FAILED] = "failed to set channel amount";
    _error_code[OPT_ERR_SET_DETECTEDMINLOAD_FAILED] = "failed to set detected min load";
    _error_code[OPT_ERR_READ_OUTERTRIGGERFREQUENCYUPPERBOUND_FAILED] = "failed to read outer trigger frequency upper bound";
    _error_code[OPT_ERR_SET_AUTOSTROBEFREQUENCY_FAILED] = "failed to set auto-strobe frequency";
    _error_code[OPT_ERR_READ_AUTOSTROBEFREQUENCY_FAILED] = "failed to read auto-strobe frequency";
    _error_code[OPT_ERR_READ_OUTERFREQUENCYUPPERBOUND_FAILED] = "failed to read max frequency";
    _error_code[OPT_ERR_SET_INNERTRIGGERFREQUENCY_FAILED] = "failed to set inner trigger frequency";
    _error_code[OPT_ERR_READ_INNERTRIGGERFREQUENCY_FAILED] = "failed to read inner trigger frequency";
    _error_code[OPT_ERR_SET_DHCP_FAILED] = "failed to set DHCP";
    _error_code[OPT_ERR_SET_LOADMODE_FAILED] = "failed to set load mode";
    _error_code[OPT_ERR_READ_PROPERTY_FAILED] = "failed to read property";
    _error_code[OPT_ERR_CONNECTION_RESET_FAILED] = "failed to reset connection";
    _error_code[OPT_ERR_SET_HEARTBEAT_FAILED] = "failed to set ETHERNET connection heartbeat";
    _error_code[OPT_ERR_GETCONTROLLERLIST_FAILED] = "Failed to get controller(s) list";
    _error_code[OPT_ERR_SOFTWARETRIGGER_FAILED] = "Failed to software trigger";
    _error_code[OPT_ERR_GET_CHANNELSTATE_FAILED] = "Failed to get channel State";
    _error_code[OPT_ERR_SET_KEEPALIVEPARAMETERS_FAILED] = "Failed to set keepalvie parameters";
    _error_code[OPT_ERR_ENABLE_KEEPALIVE_FAILED] = "Failed to enable/disable keepalive";
    _error_code[OPT_ERR_READSTEPCOUNT_FAILED] = "Failed to read step count";
    _error_code[OPT_ERR_SETTRIGGERMODE_FAILED] = "Failed to set trigger mode";
    _error_code[OPT_ERR_READTRIGGERMODE_FAILED] = "Failed to read trigger mode";
    _error_code[OPT_ERR_SETCURRENTSTEPINDEX_FAILED] = "Failed to set current step index";
    _error_code[OPT_ERR_READCURRENTSTEPINDEX_FAILED] = "Failed to read current step index";
    _error_code[OPT_ERR_RESETSEQ_FAILED] = "Failed to reset current step index";
    _error_code[OPT_ERR_SETTRIGGERDELAY_FAILED] = "Failed to set trigger delay";
    _error_code[OPT_ERR_GET_TRIGGERDELAY_FAILED] = "Failed to get trigger delay";
    _error_code[OPT_ERR_SETMULTITRIGGERDELAY_FAILED] = "Failed to set multiple channels trigger delay";
    _error_code[OPT_ERR_SETSEQTABLEDATA_FAILED] = "Failed to set SEQ table data";
    _error_code[OPT_ERR_READSEQTABLEDATA_FAILED] = "Failed to Read SEQ table data";
    _error_code[OPT_ERR_READ_CHANNELS_FAILED] = "Failed to read controller's channel";
    _error_code[OPT_ERR_READ_KEEPALIVE_STATE_FAILED] = "Failed to read the state of keepalive";
    _error_code[OPT_ERR_READ_KEEPALIVE_CONTINUOUS_TIME_FAILED] = "Failed to read the continuous time of keepalive";
    _error_code[OPT_ERR_READ_DELIVERY_TIMES_FAILED] = "Failed to read the delivery times of prop packet";
    _error_code[OPT_ERR_READ_INTERVAL_TIME_FAILED] = "Failed to read the interval time of prop packet";
    _error_code[OPT_ERR_READ_OUTPUTBOARD_VISION_FAILED] = "Failed to read the vision of output board";
    _error_code[OPT_ERR_READ_DETECT_MODE_FAILED] = "Failed to read detect mode of load";
    _error_code[OPT_ERR_SET_BOOT_STATE_MODE_FAILED] = "Failed to set mode of boot state";
    _error_code[OPT_ERR_READ_MODEL_BOOT_MODE_FAILED] = "Failed to read the specified channel boot state";
    _error_code[OPT_ERR_SET_OUTERTRIGGERFREQUENCYUPPERBOUND_FAILED] = "Failed to set outer trigger frequency upper bound";
    _error_code[OPT_ERR_SET_IPCONFIG_FAILED] = "Failed to set IP configuration of the controller";
    _error_code[OPT_ERR_SET_VOLTAGE_FAILED] = "Failed to set voltage of specified channel voltage";
    _error_code[OPT_ERR_READ_VOLTAGE_FAILED] = "Failed to read the specified channel's voltage";
    _error_code[OPT_ERR_SET_TIMEUNIT_FAILED] = "Failed to set time unit";
    _error_code[OPT_ERR_READ_TIMEUNIT_FAILED] = "Failed to read time unit";
    _error_code[OPT_ERR_FILEEXT] = "File suffix name is wrong";
    _error_code[OPT_ERR_FILEPATH_EMPTY] = "File path is empty";
    _error_code[OPT_ERR_FILE_MAGIC_NUM] = "magic number is wrong";
    _error_code[OPT_ERR_FILE_CHECKSUM] = "Checksum is wrong";
    _error_code[OPT_ERR_SEQDATA_EQUAL] = "Current SEQ table data is different from load file data";
    _error_code[OPT_ERR_SET_HB_TIMEUNIT_FAILED] = "Failed to set highlight time unit";
    _error_code[OPT_ERR_READ_HB_TIMEUNIT_FAILED] = "Failed to read highlight time unit";
    _error_code[OPT_ERR_SET_TRIGGERDELAY_TIMEUNIT_FAILED] = "Failed to set trigger delay time unit";
    _error_code[OPT_ERR_READ_TRIGGERDELAY_TIMEUNIT_FAILED] = "Failed to read trigger delay time unit";
    _error_code[OPT_ERR_SET_PERCENT_FAILED] = "Failed to set percent of brightening current";
    _error_code[OPT_ERR_READ_PERCENT_FAILED] = "Failed to read percent of brightening current";
    _error_code[OPT_ERR_SET_HB_LIMIT_STATE_FAILED] = "Failed to set high light trigger output duty limit switch state";
    _error_code[OPT_ERR_READ_HB_LIMIT_STATE_FAILED] = "Failed to read high light trigger output duty limit switch state";
    _error_code[OPT_ERR_SET_HB_TRIGGER_OUTPUT_DUTY_RATIO_FAILED] = "Failed to set high light trigger output duty limit ratio";
    _error_code[OPT_ERR_READ_HB_TRIGGER_OUTPUT_DUTY_RATIO_FAILED] = "Failed to read high light trigger output duty limit ratio";
    _error_code[OPT_ERR_SET_DIFF_PRESURE_LIMIT_STATE_FAILED] = "Failed to set differential pressure limit function switch status";
    _error_code[OPT_ERR_READ_DIFF_PRESURE_LIMIT_STATE_FAILED] = "Failed to read differential pressure limit function switch status";
    _error_code[OPT_ERR_SEARCH_SERIALPORT_FAILED] = "failed to search serialport";
    _error_code[OPT_ERR_SET_END_ADDRESS_FAILED] = "Failed to set the specified channel's end address index";
    _error_code[OPT_ERR_SET_HARDWARE_RESET_SWITCH_STATE_FAILED] = "Failed to set the specified channel's hardware reset switch state";
    _error_code[OPT_ERR_SET_ADDRESS_SWITCH_STATE_FAILED] = "Failed to set the specified channel's hardware reset switch state";
    _error_code[OPT_ERR_READ_PROGRAMMABLE_PARAM_FAILED] = "Failed to read programmable parameters for the specified channel's";
    _error_code[OPT_ERR_SET_START_ADDRESS_FAILED] = "Failed to set the specified channel's start address index";
}

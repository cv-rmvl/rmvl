/**
 * @file light_control.cpp
 * @author 赵曦 (535394140@qq.com)
 * @brief
 * @version 1.0
 * @date 2022-09-27
 *
 * @copyright Copyright (c) 2023, zhaoxi
 *
 */

#include <thread>

#include <OPTController.h>
#include <OPTErrorCode.h>

#include "rmvl/light/opt_light_control.h"

bool rm::OPTLightController::connect(const LightIpConfig &ip_config)
{
    if (_init)
        disconnect();
    _init = true;
    return OPTController_CreateEtheConnectionByIP(const_cast<char *>(ip_config.ip.c_str()), &_handle) == OPT_SUCCEED;
}

bool rm::OPTLightController::connect(std::string_view SN)
{
    using namespace std::chrono_literals;
    if (_init)
        disconnect();
    _init = true;
    if (OPTController_CreateEtheConnectionBySN(const_cast<char *>(SN.data()), &_handle) == OPT_SUCCEED)
    {
        std::this_thread::sleep_for(1ms);
        return true;
    }
    else
        return false;
}

bool rm::OPTLightController::disconnect()
{
    if (_init)
    {
        closeAllChannels();
        _init = false;
        return OPTController_DestroyEtheConnection(_handle) == OPT_SUCCEED;
    }
    return false;
}

bool rm::OPTLightController::openChannels(const std::vector<int> &channels)
{
    return OPTController_TurnOnMultiChannel(_handle, const_cast<int *>(channels.data()), static_cast<int>(channels.size())) == OPT_SUCCEED;
}

bool rm::OPTLightController::openAllChannels() { return OPTController_TurnOnChannel(_handle, 0) == OPT_SUCCEED; }

bool rm::OPTLightController::closeChannels(const std::vector<int> &channels)
{
    std::vector<IntensityItem> intensities(channels.size());
    for (size_t i = 0; i < channels.size(); ++i)
        intensities[i] = {channels[i], 0};
    OPTController_SetMultiIntensity(_handle, intensities.data(), static_cast<int>(intensities.size()));
    return OPTController_TurnOffMultiChannel(_handle, const_cast<int *>(channels.data()),
                                             static_cast<int>(channels.size())) == OPT_SUCCEED;
}

bool rm::OPTLightController::closeAllChannels()
{
    OPTController_SetIntensity(_handle, 0, 0);
    return OPTController_TurnOffChannel(_handle, 0) == OPT_SUCCEED;
}

int rm::OPTLightController::getIntensity(int channel) const
{
    int intensity;
    return OPTController_ReadIntensity(_handle, channel, &intensity) == OPT_SUCCEED ? intensity : -1;
}

bool rm::OPTLightController::setIntensity(int channel, int intensity)
{
    return OPTController_SetIntensity(_handle, channel, intensity) == OPT_SUCCEED;
}

bool rm::OPTLightController::trigger(int channel, int time) const { return OPTController_SoftwareTrigger(_handle, channel, time) == OPT_SUCCEED; }

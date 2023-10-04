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

#include "rmvl/light/opt_light_control.h"

using namespace std;
using namespace rm;

bool LightController::connect(const IPConfig &ip_config)
{
    if (_init)
        disconnect();
    _init = true;
    _status_code = OPTController_CreateEtheConnectionByIP(const_cast<char *>(ip_config.ip), &_handle);
    if (_status_code == OPT_SUCCEED)
    {
        return true;
    }
    else
        return false;
}

bool LightController::connect(const char *SN)
{
    if (_init)
        disconnect();
    _init = true;
    _status_code = OPTController_CreateEtheConnectionBySN(const_cast<char *>(SN), &_handle);
    if (_status_code == OPT_SUCCEED)
    {
        this_thread::sleep_for(chrono::milliseconds(1));
        return true;
    }
    else
        return false;
}

bool LightController::disconnect()
{
    if (_init)
    {
        closeAllChannels();
        _init = false;
        _status_code = OPTController_DestroyEtheConnection(_handle);
        return _status_code == OPT_SUCCEED;
    }
    return false;
}

bool LightController::openChannels(const std::vector<int> &channels)
{
    _status_code = OPTController_TurnOnMultiChannel(_handle, const_cast<int *>(channels.data()),
                                                     static_cast<int>(channels.size()));
    return _status_code == OPT_SUCCEED;
}

bool LightController::openAllChannels()
{
    _status_code = OPTController_TurnOnChannel(_handle, 0);
    return _status_code == OPT_SUCCEED;
}

bool LightController::closeChannels(const std::vector<int> &channels)
{
    vector<IntensityItem> intensities(channels.size());
    for (size_t i = 0; i < channels.size(); ++i)
        intensities[i] = {channels[i], 0};
    OPTController_SetMultiIntensity(_handle, intensities.data(), static_cast<int>(intensities.size()));
    _status_code = OPTController_TurnOffMultiChannel(_handle, const_cast<int *>(channels.data()),
                                                      static_cast<int>(channels.size()));
    return _status_code == OPT_SUCCEED;
}

bool LightController::closeAllChannels()
{
    OPTController_SetIntensity(_handle, 0, 0);
    _status_code = OPTController_TurnOffChannel(_handle, 0);
    return _status_code == OPT_SUCCEED;
}

int LightController::getIntensity(int channel)
{
    int intensity;
    _status_code = OPTController_ReadIntensity(_handle, channel, &intensity);
    return _status_code == OPT_SUCCEED ? intensity : -1;
}

bool LightController::setIntensity(int channel, int intensity)
{
    _status_code = OPTController_SetIntensity(_handle, channel, intensity);
    return _status_code == OPT_SUCCEED;
}

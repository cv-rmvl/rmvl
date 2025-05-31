#include <OPTController.h>
#include <OPTErrorCode.h>

#include "opt_light_impl.hpp"
#include "rmvl/core/timer.hpp"

namespace rm
{

OPTLightController::OPTLightController(const LightConfig &cfg, std::string_view id) : _impl(new Impl(cfg, id)) {}
bool OPTLightController::isOpened() const noexcept { return _impl->isOpened(); }
bool OPTLightController::open(const std::vector<int> &channels) noexcept { return _impl->open(channels); }
bool OPTLightController::open() noexcept { return _impl->open(); }
bool OPTLightController::close(const std::vector<int> &channels) noexcept { return _impl->close(channels); }
bool OPTLightController::close() noexcept { return _impl->close(); }
int OPTLightController::getIntensity(int channel) const noexcept { return _impl->getIntensity(channel); }
bool OPTLightController::setIntensity(int channel, int intensity) noexcept { return _impl->setIntensity(channel, intensity); }
bool OPTLightController::trigger(int channel, int time) const noexcept { return _impl->trigger(channel, time); }

OPTLightController::Impl::Impl(const LightConfig &cfg, std::string_view id)
{
    if (_init)
        disconnect();
    switch (cfg.handle_mode)
    {
    case LightHandleMode::IP:
        _init = (OPTController_CreateEtheConnectionByIP(const_cast<char *>(id.data()), &_handle) == OPT_SUCCEED);
        break;
    case LightHandleMode::Key:
        if (OPTController_CreateEtheConnectionBySN(const_cast<char *>(id.data()), &_handle) == OPT_SUCCEED)
        {
            Timer::sleep_for(1000);
            _init = true;
        }
        break;
    default:
        RMVL_Error(RMVL_StsBadArg, "Unknown handle mode.");
    }
}

bool OPTLightController::Impl::disconnect() noexcept
{
    if (_init)
    {
        close();
        _init = false;
        return OPTController_DestroyEtheConnection(_handle) == OPT_SUCCEED;
    }
    return false;
}

bool OPTLightController::Impl::open(const std::vector<int> &channels) noexcept
{
    return OPTController_TurnOnMultiChannel(_handle, const_cast<int *>(channels.data()), static_cast<int>(channels.size())) == OPT_SUCCEED;
}

bool OPTLightController::Impl::open() noexcept
{
    return OPTController_TurnOnChannel(_handle, 0) == OPT_SUCCEED;
}

bool OPTLightController::Impl::close(const std::vector<int> &channels) noexcept
{
    std::vector<IntensityItem> intensities(channels.size());
    for (size_t i = 0; i < channels.size(); ++i)
        intensities[i] = {channels[i], 0};
    OPTController_SetMultiIntensity(_handle, intensities.data(), static_cast<int>(intensities.size()));
    return OPTController_TurnOffMultiChannel(_handle, const_cast<int *>(channels.data()),
                                             static_cast<int>(channels.size())) == OPT_SUCCEED;
}

bool OPTLightController::Impl::close() noexcept
{
    OPTController_SetIntensity(_handle, 0, 0);
    return OPTController_TurnOffChannel(_handle, 0) == OPT_SUCCEED;
}

int OPTLightController::Impl::getIntensity(int channel) const noexcept
{
    int intensity;
    return OPTController_ReadIntensity(_handle, channel, &intensity) == OPT_SUCCEED ? intensity : -1;
}

bool OPTLightController::Impl::setIntensity(int channel, int intensity) noexcept
{
    return OPTController_SetIntensity(_handle, channel, intensity) == OPT_SUCCEED;
}

bool OPTLightController::Impl::trigger(int channel, int time) const noexcept
{
    return OPTController_SoftwareTrigger(_handle, channel, time) == OPT_SUCCEED;
}

} // namespace rm

#include <thread>

#include "hik_light_impl.hpp"

#include "rmvlpara/light/hik_light_control.h"

namespace rm
{

RMVL_IMPL_DEF(HikLightController)

HikLightController::HikLightController(const LightConfig &cfg, std::string_view id) : _impl(new Impl(cfg, id)) {}
bool HikLightController::isOpened() const { return _impl->isOpened(); }
bool HikLightController::open() { return _impl->open(); }
bool HikLightController::close() { return _impl->close(); }
int HikLightController::get(int chn) const { return _impl->get(chn); }
bool HikLightController::set(int chn, int val) { return _impl->set(chn, val); }

using namespace std::string_literals;
constexpr const char HIK_CHN_STR[] = "ABCDEF";

HikLightController::Impl::Impl(const LightConfig &cfg, std::string_view id)
{
    if (cfg.handle_mode == LightHandleMode::Serial)
    {
        SerialPortMode mode{};
        mode.baud_rate = BaudRate::BR_19200;
        mode.read_mode = SerialReadMode::NONBLOCK;
        _sp = std::make_unique<SerialPort>(id, mode);
    }
    else
    {
        RMVL_Error(RMVL_StsBadArg, "Unsupported handle mode");
    }
}

bool HikLightController::Impl::open()
{
    RMVL_DbgAssert(isOpened());
    if (!_sp->write("SH#"))
        return false;
    std::this_thread::sleep_for(std::chrono::milliseconds(para::hik_light_control_param.DELAY_AFTER_WRITE));
    std::string buf;
    if (!_sp->read(buf))
        return false;
    return buf == "H";
}

bool HikLightController::Impl::close()
{
    RMVL_DbgAssert(isOpened());
    if (!_sp->write("SL#"))
        return false;
    std::this_thread::sleep_for(std::chrono::milliseconds(para::hik_light_control_param.DELAY_AFTER_WRITE));
    std::string buf;
    if (!_sp->read(buf))
        return false;
    return buf == "L";
}

int HikLightController::Impl::get(int chn) const
{
    RMVL_DbgAssert(isOpened());
    RMVL_Assert(chn > 0 && chn <= 6);
    if (!_sp->write("S"s + HIK_CHN_STR[chn - 1] + "#"))
        return -1;
    std::string buf;
    std::this_thread::sleep_for(std::chrono::milliseconds(para::hik_light_control_param.DELAY_AFTER_WRITE));
    if (!_sp->read(buf))
        return -1;
    if (buf == "NG")
        return -1;
    try
    {
        return std::stoi(std::string(buf.begin() + 1, buf.end()));
    }
    catch (const std::exception &)
    {
        ERROR_("Failed to obtain brightness, try to increase the 'DELAY_AFTER_WRITE' parameter");
        return -1;
    }
}

bool HikLightController::Impl::set(int chn, int val)
{
    RMVL_DbgAssert(isOpened());
    RMVL_Assert(chn > 0 && chn <= 6 && val >= 0 && val <= 255);
    char ch = HIK_CHN_STR[chn - 1];
    char command[8]{};
    snprintf(command, sizeof(command), "S%c%04d#", HIK_CHN_STR[chn - 1], val);

    if (!_sp->write(command))
        return false;
    std::this_thread::sleep_for(std::chrono::milliseconds(para::hik_light_control_param.DELAY_AFTER_WRITE));
    std::string buf;
    if (!_sp->read(buf))
        return false;
    return buf == std::string(1, ch);
}

} // namespace rm

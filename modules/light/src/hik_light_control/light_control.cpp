#include <thread>

#include "hik_light_impl.hpp"

#include "rmvlpara/light/hik_light_control.h"

namespace rm
{

RMVL_IMPL_DEF(HikLightController)

HikLightController::HikLightController(const LightConfig &cfg, HikProductID pid, std::string_view id) : _impl(new Impl(cfg, pid, id)) {}
bool HikLightController::isOpened() const { return _impl->isOpened(); }
bool HikLightController::open() { return _impl->open(); }
bool HikLightController::close() { return _impl->close(); }
int HikLightController::get(int chn) const { return _impl->get(chn); }
bool HikLightController::set(int chn, int val) { return _impl->set(chn, val); }

static std::unique_ptr<SerialPort> lead_create(std::string_view id)
{
    SerialPortMode mode{};
    mode.baud_rate = BaudRate::BR_19200;
    mode.read_mode = SerialReadMode::BLOCK;
    return std::make_unique<SerialPort>(id, mode);
}

static bool lead_open(SerialPort &sp)
{
    if (!sp.write("SH#"))
        return false;
    std::this_thread::sleep_for(std::chrono::milliseconds(para::hik_light_control_param.DELAY_AFTER_WRITE));
    std::string buf;
    if (!sp.read(buf))
        return false;
    return buf == "H";
}

static bool lead_close(SerialPort &sp)
{
    if (!sp.write("SL#"))
        return false;
    std::this_thread::sleep_for(std::chrono::milliseconds(para::hik_light_control_param.DELAY_AFTER_WRITE));
    std::string buf;
    if (!sp.read(buf))
        return false;
    return buf == "L";
}

using namespace std::string_literals;
constexpr const char HIK_CHN_STR[] = "ABCDEFGH";

static int lead_get(SerialPort &sp, int chn)
{
    RMVL_Assert(chn > 0 && chn <= 8);
    if (!sp.write("S"s + HIK_CHN_STR[chn - 1] + "#"))
        return -1;
    std::string buf;
    std::this_thread::sleep_for(std::chrono::milliseconds(para::hik_light_control_param.DELAY_AFTER_WRITE));
    if (!sp.read(buf))
        return -1;
    if (buf == "NG")
        return -1;
    return std::stoi(std::string(buf.begin() + 1, buf.end()));
}

static bool lead_set(SerialPort &sp, int chn, int val)
{
    RMVL_Assert(chn > 0 && chn <= 8 && val >= 0 && val <= 255);
    char ch = HIK_CHN_STR[chn - 1];
    char command[8]{};
    snprintf(command, sizeof(command), "S%c%04d#", HIK_CHN_STR[chn - 1], val);

    if (!sp.write(command))
        return false;
    std::this_thread::sleep_for(std::chrono::milliseconds(para::hik_light_control_param.DELAY_AFTER_WRITE));
    std::string buf;
    if (!sp.read(buf))
        return false;
    return buf == std::string(1, ch);
}

HikLightController::Impl::Impl(const LightConfig &cfg, HikProductID pid, std::string_view id) : _pid(pid)
{
    if (cfg.handle_mode == LightHandleMode::Serial)
    {
        switch (pid)
        {
        case HikProductID::LEAD:
            _sp = lead_create(id);
            break;
        default:
            break;
        }
    }
    else
    {
        RMVL_Error(RMVL_StsBadArg, "Unsupported handle mode");
    }
}

bool HikLightController::Impl::open()
{
    RMVL_DbgAssert(isOpened());
    switch (_pid)
    {
    case HikProductID::LEAD:
        return lead_open(*_sp);
    default:
        return false;
    }
}

bool HikLightController::Impl::close()
{
    RMVL_DbgAssert(isOpened());
    switch (_pid)
    {
    case HikProductID::LEAD:
        return lead_close(*_sp);
    default:
        return false;
    }
}

int HikLightController::Impl::get(int chn) const
{
    RMVL_DbgAssert(isOpened());

    switch (_pid)
    {
    case HikProductID::LEAD:
        return lead_get(*_sp, chn);
    default:
        return -1;
    }
}

bool HikLightController::Impl::set(int chn, int val)
{
    RMVL_DbgAssert(isOpened());
    switch (_pid)
    {
    case HikProductID::LEAD:
        return lead_set(*_sp, chn, val);
    default:
        return false;
    }
}

} // namespace rm

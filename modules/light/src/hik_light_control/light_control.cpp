#include <thread>

#include "hik_light_impl.hpp"

#include "rmvlpara/light/hik_light_control.h"

namespace rm {

HikLightController::HikLightController(const LightConfig &cfg, std::string_view id) : _impl(new Impl(cfg, id)) {}
HikLightController::~HikLightController() = default;
bool HikLightController::isOpened() const noexcept { return _impl->isOpened(); }
bool HikLightController::open() noexcept { return _impl->open(); }
bool HikLightController::close() noexcept { return _impl->close(); }
int HikLightController::get(int chn) const noexcept { return _impl->get(chn); }
bool HikLightController::set(int chn, int val) noexcept { return _impl->set(chn, val); }

using namespace std::string_literals;
constexpr const char HIK_CHN_STR[] = "ABCDEF";

HikLightController::Impl::Impl(const LightConfig &cfg, std::string_view id) {
    if (cfg.handle_mode == LightHandleMode::Serial)
        _sp = std::make_unique<SerialPort>(id, BaudRate::BR_19200, SerialReadMode::NONBLOCK);
    else
        RMVL_Error(RMVL_StsBadArg, "Unsupported handle mode");
}

bool HikLightController::Impl::open() noexcept {
    if (!_sp->write("SH#"))
        return false;
    std::this_thread::sleep_for(std::chrono::microseconds(para::hik_light_control_param.DELAY_AFTER_WRITE));
    std::string buf;
    if (!_sp->read(buf))
        return false;
    return buf == "H";
}

bool HikLightController::Impl::close() noexcept {
    if (!_sp->write("SL#"))
        return false;
    std::this_thread::sleep_for(std::chrono::microseconds(para::hik_light_control_param.DELAY_AFTER_WRITE));
    std::string buf{};
    if (!_sp->read(buf))
        return false;
    return buf == "L";
}

int HikLightController::Impl::get(int chn) const noexcept {
    if (chn <= 0 || chn > 6) {
        ERROR_("Invalid channel number, it should be in the range of 1 ~ 6");
        return -1;
    }
    std::string buf{};
    std::this_thread::sleep_for(std::chrono::microseconds(para::hik_light_control_param.DELAY_AFTER_WRITE));
    if (!_sp->read(buf))
        return -1;
    if (buf == "NG")
        return -1;
    try {
        return std::stoi(std::string(buf.begin() + 1, buf.end()));
    } catch (const std::exception &) {
        ERROR_("Failed to obtain brightness, try to increase the 'DELAY_AFTER_WRITE' parameter");
        return -1;
    }
}

bool HikLightController::Impl::set(int chn, int val) noexcept {
    if (chn <= 0 || chn > 6 || val < 0 || val > 255) {
        ERROR_("Invalid argument, channel number should be in the range of 1 ~ 6, and brightness value should be in the range of 0 ~ 255");
        return false;
    }
    char ch = HIK_CHN_STR[chn - 1];
    char command[8]{};
    snprintf(command, sizeof(command), "S%c%04d#", HIK_CHN_STR[chn - 1], val);

    if (!_sp->write(command))
        return false;
    std::this_thread::sleep_for(std::chrono::microseconds(para::hik_light_control_param.DELAY_AFTER_WRITE));
    std::string buf{};
    if (!_sp->read(buf))
        return false;
    return buf == std::string(1, ch);
}

} // namespace rm

/**
 * @file ctl_pid.cpp
 * @author zhaoxi (535394140@qq.com)
 * @brief
 * @version 1.0
 * @date 2026-04-23
 *
 * @copyright Copyright 2026 (c), zhaoxi
 *
 */

#include "rmvl/core/util.hpp"

#include "rmvl/lpss/ctl/pid.hpp"

namespace rm::lpss::ctl {

PID::PID(const std::vector<double> &kp, const std::vector<double> &ki, const std::vector<double> &kd,
         InSampleMapping imapping, OutSampleMapping omapping)
    : ControlLawBase(imapping, omapping), _kp(kp), _ki(ki), _kd(kd) {
    const size_t dof = _kp.size();
    if (_ki.size() != dof || _kd.size() != dof)
        RMVL_Error(RMVL_StsBadArg, "PID gain vectors must have the same size");
    _prev_out.assign(dof, 0.0);
    for (auto &prev_err : _prev_errs)
        prev_err.assign(dof, 0.0);
}

void PID::reset() noexcept {
    std::fill(_prev_out.begin(), _prev_out.end(), 0.0);
    for (auto &prev_err : _prev_errs)
        std::fill(prev_err.begin(), prev_err.end(), 0.0);
}

ControlStatus PID::do_compute(const std::vector<double> &desired, const std::vector<double> &fb, int32_t period, std::vector<double> &command) noexcept {
    const double T = static_cast<double>(period) / 1000.0;
    const size_t dof = _kp.size();
    
    // 验证输入维度
    if (desired.size() != dof || fb.size() != dof)
        return ControlStatus::InvalidInput;
    
    std::vector<double> out(dof, 0.0);
    
    // 计算误差
    std::vector<double> err(dof, 0.0);
    for (std::size_t i = 0; i < dof; ++i)
        err[i] = desired[i] - fb[i];
    
    // 应用控制律：u[k] - u[k-1] = (Kp + Kd)x[k] + (KiT - Kp - 2Kd)x[k-1] + Kd*x[k-2]
    for (std::size_t i = 0; i < dof; ++i)
        out[i] = _prev_out[i] + (_kp[i] + _kd[i]) * err[i] + (_ki[i] * T - _kp[i] - 2.0 * _kd[i]) * _prev_errs[0][i] + _kd[i] * _prev_errs[1][i];
    
    // 输出控制命令
    command = std::move(out);
    
    // 更新缓存
    _prev_out = command;
    _prev_errs[1] = std::move(_prev_errs[0]);
    _prev_errs[0] = std::move(err);

    return ControlStatus::Ok;
}

} // namespace rm::lpss::ctl


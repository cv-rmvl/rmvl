/**
 * @file ctl_ff.cpp
 * @author zhaoxi (535394140@qq.com)
 * @brief
 * @version 1.0
 * @date 2026-04-25
 *
 * @copyright Copyright 2026 (c), zhaoxi
 *
 */

#include "rmvl/core/util.hpp"

#include "rmvl/lpss/ctl/ff.hpp"

namespace rm::lpss::ctl {

FeedForward::FeedForward(const std::vector<double> &a0, const std::vector<double> &a1, const std::vector<double> &a2, InSampleMapping imapping, OutSampleMapping omapping)
    : ControlLawBase(imapping, omapping), _a0(a0), _a1(a1), _a2(a2) {
    const size_t dof = _a0.size();
    if (_a1.size() != dof || _a2.size() != dof)
        RMVL_Error(RMVL_StsBadArg, "FeedForward coefficient vectors must have the same size");
    for (auto &prev_d : _prev_ds)
        prev_d.assign(dof, 0.0);
}

void FeedForward::reset() noexcept {
    for (auto &prev_d : _prev_ds)
        std::fill(prev_d.begin(), prev_d.end(), 0.0);
}

ControlStatus FeedForward::do_compute(const std::vector<double> &desired, const std::vector<double> &, int32_t period, std::vector<double> &command) noexcept {
    const double T = static_cast<double>(period) / 1000.0;
    const size_t dof = _a0.size();

    // 验证输入维度
    if (desired.size() != dof)
        return ControlStatus::InvalidInput;

    std::vector<double> out(dof, 0.0);

    // 应用差分方程
    for (std::size_t i = 0; i < dof; ++i)
        out[i] = (_a0[i] + _a1[i] / T + _a2[i] / (T * T)) * desired[i] - (_a1[i] / T + 2.0 * _a2[i] / (T * T)) * _prev_ds[0][i] + _a2[i] / (T * T) * _prev_ds[1][i];

    // 输出控制命令
    command = std::move(out);

    // 更新缓存
    _prev_ds[1] = std::move(_prev_ds[0]);
    _prev_ds[0] = desired;

    return ControlStatus::Ok;
}

} // namespace rm::lpss::ctl

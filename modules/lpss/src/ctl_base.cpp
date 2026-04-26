/**
 * @file ctl_base.cpp
 * @author zhaoxi (535394140@qq.com)
 * @brief
 * @version 1.0
 * @date 2026-04-28
 *
 * @copyright Copyright 2026 (c), zhaoxi
 *
 */

#include "rmvl/lpss/ctl/base.hpp"

namespace rm::lpss::ctl {

// ============================================================================
// 输入采样映射实现
// ============================================================================

void basic_pos_imapping(const msg::JointState &d_in, const msg::JointState &fb_in,
                        std::vector<double> &d_out, std::vector<double> &fb_out) noexcept {
    d_out = d_in.position;
    fb_out = fb_in.position;
}

void basic_vel_imapping(const msg::JointState &d_in, const msg::JointState &fb_in,
                        std::vector<double> &d_out, std::vector<double> &fb_out) noexcept {
    d_out = d_in.velocity;
    fb_out = fb_in.velocity;
}

void basic_eff_imapping(const msg::JointState &d_in, const msg::JointState &fb_in,
                        std::vector<double> &d_out, std::vector<double> &fb_out) noexcept {
    d_out = d_in.effort;
    fb_out = fb_in.effort;
}

void basic_pos_omapping(std::vector<double> cmd_in, msg::JointState &cmd_out) noexcept {
    const size_t dof = cmd_in.size();
    cmd_out.position = std::move(cmd_in);
    cmd_out.velocity.assign(dof, 0.0);
    cmd_out.effort.assign(dof, 0.0);
}

void basic_vel_omapping(std::vector<double> cmd_in, msg::JointState &cmd_out) noexcept {
    const size_t dof = cmd_in.size();
    cmd_out.position.assign(dof, 0.0);
    cmd_out.velocity = std::move(cmd_in);
    cmd_out.effort.assign(dof, 0.0);
}

void basic_eff_omapping(std::vector<double> cmd_in, msg::JointState &cmd_out) noexcept {
    const size_t dof = cmd_in.size();
    cmd_out.position.assign(dof, 0.0);
    cmd_out.velocity.assign(dof, 0.0);
    cmd_out.effort = std::move(cmd_in);
}

ControlStatus ControlLawBase::compute(const msg::JointState &desired, const msg::JointState &fb, int32_t period, msg::JointState &command) noexcept {
    // 验证输入维度
    const size_t dof = desired.name.size();
    if (fb.name.size() != dof)
        return ControlStatus::InvalidInput;

    // 调用输入映射函数提取向量对
    std::vector<double> desired_vec{}, fb_vec{};
    _input_fn(desired, fb, desired_vec, fb_vec);

    // 验证提取的向量维度
    if (desired_vec.size() != dof || fb_vec.size() != dof)
        return ControlStatus::InvalidInput;

    // 调用虚函数进行实际的脉冲传递函数计算
    std::vector<double> cmd_vec(dof, 0.0);
    auto res = do_compute(desired_vec, fb_vec, period, cmd_vec);
    if (res != ControlStatus::Ok)
        return res;

    // 调用输出映射函数将结果写回 JointState
    command.header.stamp = desired.header.stamp;
    command.name = desired.name;
    _output_fn(cmd_vec, command);

    return ControlStatus::Ok;
}

} // namespace rm::lpss::ctl

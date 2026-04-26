/**
 * @file pid.hpp
 * @author zhaoxi (535394140@qq.com)
 * @brief LPSS 机器人扩展：控制律组件 - 离散 PID 控制律
 * @version 1.0
 * @date 2026-04-17
 *
 * @copyright Copyright 2026 (c), zhaoxi
 *
 */

#pragma once

#include "base.hpp"

namespace rm::lpss::ctl {

//! @addtogroup lpss_robot
//! @{

/**
 * @brief 【控制律组件】单环离散比例-积分-微分控制器
 * @details 对于连续系统中，PID 的传递函数表示为\f[G(s)=K_p+\frac{K_i}s+K_ds\tag1\f]将上式与零阶保持器 ZOH 进行串联，可以得到在离散系统中的脉冲传递函数：
 *          \f[\begin{aligned}G(z)&=\mathcal{Z}\left\{\frac{1-e^{sT}}s\left(K_p+\frac{K_i}s+K_ds\right)\right\}\\&=K_p+\frac{K_iTz^{-1}}{1-z^{-1}}+K_d
 *          \left(1-z^{-1}\right)\end{aligned}\tag2\f]转换为差分方程的形式为\f[u[k]-u[k-1]=(K_p+K_d)x[k]+(K_iT-K_p-2K_d)x[k-1]+K_dx[k-2]\tag3\f]其中，
 *          \f$u[k]\f$ 是控制输出，\f$x[k]\f$ 是系统的误差输入，\f$T\f$ 是采样周期。PID 控制器的设计参数为比例增益 \f$K_p\f$、积分增益 \f$K_i\f$
 *          和微分增益 \f$K_d\f$，通过调整它们可以实现对系统动态性能的优化。
 * @note 若要实现多环控制（如外环位置、内环速度），请自行继承 ControlLawBase 并组合多个 PID 实例
 */
class PID final : public ControlLawBase {
public:
    //! @cond
    PID(const std::vector<double> &kp, const std::vector<double> &ki, const std::vector<double> &kd,
        InSampleMapping imapping, OutSampleMapping omapping);
    //! @endcond

    /**
     * @brief 创建单环离散 PID 控制器实例
     *
     * @param[in] kp 比例增益
     * @param[in] ki 积分增益
     * @param[in] kd 微分增益
     * @param[in] imapping 输入采样映射
     * @param[in] omapping 输出采样映射
     *
     * @return PID 控制器独占指针
     */
    static ControlLawBase::ptr create(const std::vector<double> &kp, const std::vector<double> &ki, const std::vector<double> &kd,
                                      InSampleMapping imapping = basic_pos_imapping, OutSampleMapping omapping = basic_pos_omapping) noexcept {
        return std::make_unique<PID>(kp, ki, kd, imapping, omapping);
    }

    //! 重置状态
    void reset() noexcept override;

    /**
     * @brief 执行一次控制计算
     *
     * @param[in] desired 当前时刻期望状态向量
     * @param[in] fb 当前反馈状态向量
     * @param[in] period 控制周期（毫秒）
     * @param[out] command 控制命令输出向量
     * @return 控制计算状态
     */
    ControlStatus do_compute(const std::vector<double> &desired, const std::vector<double> &fb, int32_t period, std::vector<double> &command) noexcept override;

private:
    const std::vector<double> _kp{}; //!< 比例增益
    const std::vector<double> _ki{}; //!< 积分增益
    const std::vector<double> _kd{}; //!< 微分增益

    std::vector<double> _prev_errs[2]{}; //!< 误差缓存
    std::vector<double> _prev_out{};     //!< 输出缓存
};

//! @} lpss_robot

} // namespace rm::lpss::ctl

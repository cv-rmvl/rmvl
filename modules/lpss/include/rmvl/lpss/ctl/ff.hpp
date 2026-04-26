/**
 * @file ff.hpp
 * @author zhaoxi (535394140@qq.com)
 * @brief LPSS 机器人扩展：控制律组件 - 离散前馈环节
 * @version 1.0
 * @date 2026-04-25
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
 * @brief 【控制律组件】离散动态全补偿的前馈环节，适合在已知系统模型或有外部扰动补偿需求时使用
 * @details <center><img src="ff.png" alt="ff" width="40%" /><div style="margin-top: 0.4em;">图1：带前馈环节的反馈控制系统</div></center>
 *          上图所示，\f$G_f(s)\f$表示前馈环节，其设计通常基于系统的模型或经验数据，理论上动态全补偿的前馈传递函数 \f$G_f(s)\f$ 具有如下表达式\f[G_f(s)=\frac1{G_s(s)
 *          G_b(s)}\tag1\f]其中，\f$G_s(s)\f$ 是系统的传递函数，\f$G_b(s)\f$ 是反馈环节的传递函数。
 *
 *          下面给出一个典型的例子：对于一般的电机模型，令电流 \f$i\f$ 为输入 \f$x_i(t)\f$，位置 \f$\theta\f$ 为输出 \f$x_o(t)\f$，那么由 FOC 保证，电机的输出力矩 \f$M=k_Mi
 *          \f$，其中 \f$k_M\f$ 是电机的力矩常数。假设电机的转动惯量为 \f$J\f$，阻尼系数为 \f$c\f$，那么系统的动力学方程为\f[J\ddot{x_i}(t)+c\dot{x_i}(t)=M=k_Mx_o(t)\tag2\f]
 *          对其进行拉普拉斯变换后得到系统开环传递函数为\f[G_s(s)=\frac{k_M}{Js^2+cs}\tag3\f]一般我们会使用双环反馈控制器来控制电机的位置，内环为速度环，外环为位置环，如下图所示：
 *          <center><img src="motor.png" alt="motor" width="70%" /><div style="margin-top: 0.4em;">图2：带前馈的双环控制电机调速模型</div></center>
 *          其中，\f$G_{c1}(s)\f$ 和 \f$G_{c2}(s)\f$ 分别为位置环和速度环的控制器传递函数，\f$G_{f1}(s)\f$ 和 \f$G_{f2}(s)\f$ 分别为位置环和速度环的前馈传递函数。对于内环速度环，
 *          理论上全补偿的前馈传递函数具有如下形式\f[\begin{aligned}G_{f1}(s)&=s\\G_{f2}(s)&=\frac{Js+c}{k_M}\end{aligned}\tag4\f]如果对于单环控制，则前馈传递函数的理论补偿形式为
 *          \f[G_f(s)=\frac{Js^2+cs}{k_M}\tag5\f]一般的，可以令前馈传递函数为\f[G_f(s)=a_0+a_1s+a_2s^2\tag6\f]其中 \f$a_0\f$、\f$a_1\f$ 和 \f$a_2\f$ 是前馈环节的设计参数，
 *          通过调整它们可以实现对系统动态性能的优化。式中不存在真分式，且前馈输入一般为给定的无噪声信号，因此求微分不会引入噪声，可以使用后向欧拉对前馈环节进行近似，
 *          得到前馈环节的脉冲传递函数为\f[G_f(z)=a_0+a_1\frac{1-z^{-1}}T+a_2\left(\frac{1-z^{-1}}T\right)^2\tag7\f]转换为如下的差分方程形式\f[u[k]=\left(a_0+\frac{a_1}T+\frac{a_2}
 *          {T^2}\right)x[k]-\frac1T\left(a_1+\frac{2a_2}T\right)x[k-1]+\frac{a_2}{T^2}x[k-2]\tag8\f]
 */
class FeedForward final : public ControlLawBase {
public:
    //! @cond
    FeedForward(const std::vector<double> &a0, const std::vector<double> &a1, const std::vector<double> &a2, InSampleMapping imapping, OutSampleMapping omapping);
    //! @endcond

    /**
     * @brief 创建离散前馈环节实例
     *
     * @param[in] a0 前馈零阶项系数
     * @param[in] a1 前馈一阶项系数
     * @param[in] a2 前馈二阶项系数
     * @param[in] imapping 输入采样映射，默认为基础位置映射
     * @param[in] omapping 输出采样映射，默认为基础位置映射
     *
     * @return 前馈环节独占指针
     */
    static ControlLawBase::ptr create(const std::vector<double> &a0, const std::vector<double> &a1, const std::vector<double> &a2,
                                      InSampleMapping imapping = basic_pos_imapping, OutSampleMapping omapping = basic_pos_omapping) noexcept {
        return std::make_unique<FeedForward>(a0, a1, a2, imapping, omapping);
    }

    //! 重置状态
    void reset() noexcept override;

    /**
     * @brief 执行一次控制计算
     *
     * @param[in] desired 当前时刻期望状态向量
     * @param[in] fb 当前反馈状态向量（此接口中未使用）
     * @param[in] period 控制周期（毫秒）
     * @param[out] command 控制命令输出向量
     * @return 控制计算状态
     */
    ControlStatus do_compute(const std::vector<double> &desired, const std::vector<double> &fb, int32_t period, std::vector<double> &command) noexcept override;

private:
    const std::vector<double> _a0{}; //!< 前馈零阶项系数
    const std::vector<double> _a1{}; //!< 前馈一阶项系数
    const std::vector<double> _a2{}; //!< 前馈二阶项系数

    std::vector<double> _prev_ds[2]{}; //!< 期望输入缓存
};

//! @} lpss_robot

} // namespace rm::lpss::ctl

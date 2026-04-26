/**
 * @file base.hpp
 * @author zhaoxi (535394140@qq.com)
 * @brief LPSS 机器人扩展：控制律组件 - 基类与单位传递函数
 * @version 1.0
 * @date 2026-04-16
 *
 * @copyright Copyright 2026 (c), zhaoxi
 *
 */

#pragma once

#include <memory>
#include <vector>

#include "rmvlmsg/sensor/joint_state.hpp"

//! @brief 运动控制相关的基础组件，供具体控制律实现继承使用
namespace rm::lpss::ctl {

//! @addtogroup lpss_robot
//! @{

//! 控制律计算状态
enum class ControlStatus : uint8_t {
    Ok = 0,       //!< 本周期计算成功
    InvalidInput, //!< 输入维度或数据非法
    Diverged,     //!< 计算过程发散
    Failed        //!< 其他失败
};

/**
 * @brief 输入采样映射，定义了如何从 JointState 中提取向量供控制律计算使用
 * @param[in] d_in msg::JointState 表示的期望值
 * @param[in] fb_in msg::JointState 表示的反馈值
 * @param[out] d_out std::vector 表示的期望值，供控制律计算使用
 * @param[out] fb_out std::vector 表示的反馈值，供控制律计算使用
 * @details 用户可自定义此类函数实现复杂的提取逻辑，如多字段融合等
 */
using InSampleMapping = void (*)(const msg::JointState &d_in, const msg::JointState &fb_in, std::vector<double> &d_out, std::vector<double> &fb_out) noexcept;

/**
 * @brief 输出采样映射，定义了如何将控制律计算结果写回 JointState 输出
 * @param[in] cmd_in std::vector 表示的控制量，这是控制律的直接输出
 * @param[out] cmd_out msg::JointState 表示的控制量，供用户使用
 */
using OutSampleMapping = void (*)(std::vector<double> cmd_in, msg::JointState &cmd_out) noexcept;

//! 基础位置采样输入映射，从 position 字段提取
void basic_pos_imapping(const msg::JointState &d_in, const msg::JointState &fb_in, std::vector<double> &d_out, std::vector<double> &fb_out) noexcept;

//! 基础速度采样输入映射，从 velocity 字段提取
void basic_vel_imapping(const msg::JointState &d_in, const msg::JointState &fb_in, std::vector<double> &d_out, std::vector<double> &fb_out) noexcept;

//! 基础力矩采样输入映射，从 effort 字段提取
void basic_eff_imapping(const msg::JointState &d_in, const msg::JointState &fb_in, std::vector<double> &d_out, std::vector<double> &fb_out) noexcept;

//! 基础位置采样输出映射，将结果写回 position 字段
void basic_pos_omapping(std::vector<double> cmd_in, msg::JointState &cmd_out) noexcept;

//! 基础速度采样输出映射，将结果写回 velocity 字段
void basic_vel_omapping(std::vector<double> cmd_in, msg::JointState &cmd_out) noexcept;

//! 基础力矩采样输出映射，将结果写回 effort 字段
void basic_eff_omapping(std::vector<double> cmd_in, msg::JointState &cmd_out) noexcept;

/**
 * @brief 【控制律组件】控制律基类，供 RobotController 组合调用
 * @details 控制律组件主要描述了如下图所示的控制计算逻辑
 *          <center><img src="ctllaw.png" alt="控制律组件框图" width="25%" /></center>
 */
class ControlLawBase {
public:
    //! @cond
    ControlLawBase(InSampleMapping input_fn, OutSampleMapping output_fn) noexcept
        : _input_fn(input_fn), _output_fn(output_fn) {}
    ControlLawBase(const ControlLawBase &) = delete;
    ControlLawBase &operator=(const ControlLawBase &) = delete;
    ControlLawBase(ControlLawBase &&) = delete;
    ControlLawBase &operator=(ControlLawBase &&) = delete;
    virtual ~ControlLawBase() = default;
    //! @endcond

    using ptr = std::unique_ptr<ControlLawBase>;

    /**
     * @brief 重置内部状态
     * @details 常用于轨迹切换、故障恢复或重新使能时清空积分器/观测器状态
     */
    virtual void reset() noexcept = 0;

    /**
     * @brief 执行一次控制计算
     * @details
     * - 调用输入映射函数从 @p desired 和 @p fb 中提取向量对
     * - 将提取的向量传递给虚函数 do_compute 进行实际脉冲传递函数计算
     * - 调用输出映射函数将计算结果写回 @p command 的对应字段
     *
     * @param[in] desired 当前时刻期望状态
     * @param[in] fb 当前反馈状态
     * @param[in] period 控制周期（毫秒）
     * @param[out] command 控制命令输出
     * @return 控制计算状态
     */
    ControlStatus compute(const msg::JointState &desired, const msg::JointState &fb, int32_t period, msg::JointState &command) noexcept;

protected:
    /**
     * @brief 纯虚的向量计算接口，由具体的控制律实现
     * @details 派生类需要实现此函数来执行实际的向量级脉冲传递函数计算
     *
     * @param[in] desired 期望状态向量
     * @param[in] fb 反馈状态向量
     * @param[in] period 控制周期（毫秒）
     * @param[out] command 控制命令输出向量
     * @return 控制计算状态
     */
    virtual ControlStatus do_compute(const std::vector<double> &desired, const std::vector<double> &fb, int32_t period, std::vector<double> &command) noexcept = 0;

private:
    InSampleMapping _input_fn;   //!< 输入采样映射函数
    OutSampleMapping _output_fn; //!< 输出采样映射函数
};

//! 【控制律组件】单位传递函数，\f$G(s)=1\f$
class UnitTF final : public ControlLawBase {
public:
    /**
     * @brief 构造单位传递函数控制律
     *
     * @param[in] imapping 输入采样映射
     * @param[in] omapping 输出采样映射
     */
    UnitTF(InSampleMapping imapping, OutSampleMapping omapping) noexcept : ControlLawBase(imapping, omapping) {}

    //! @cond
    void reset() noexcept override {}
    //! @endcond

    //! 创建单位传递函数控制律对象
    static ControlLawBase::ptr create(InSampleMapping imapping, OutSampleMapping omapping) noexcept {
        return std::make_unique<UnitTF>(imapping, omapping);
    }

private:
    ControlStatus do_compute(const std::vector<double> &desired, const std::vector<double> &, int32_t, std::vector<double> &command) noexcept override {
        command = desired;
        return ControlStatus::Ok;
    }
};

//! @} lpss_robot

} // namespace rm::lpss::ctl

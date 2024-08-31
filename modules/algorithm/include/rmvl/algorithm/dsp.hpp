/**
 * @file dsp.hpp
 * @author zhaoxi (535394140@qq.com)
 * @brief 数字信号处理库
 * @version 1.0
 * @date 2024-08-31
 *
 * @copyright Copyright 2024 (c), zhaoxi
 *
 */

#pragma once

#include <deque>
#include <complex>

#ifdef HAVE_OPENCV
#include <opencv2/core/mat.hpp>
#endif // HAVE_OPENCV

namespace rm
{

//! @addtogroup algorithm
//! @{
//! @defgroup algorithm_dsp 数字信号处理
//! @{
//! @brief 包含数字信号处理的相关函数
//! @} algorithm_dsp
//! @} algorithm

//! @addtogroup algorithm_dsp
//! @{

//! 实数信号
using RealSignal = std::deque<double>;
//! 复数信号
using ComplexSignal = std::deque<std::complex<double>>;

//! 信号谱类型
enum class GxType
{
    Amp,      //!< 幅度谱
    Phase,    //!< 相位谱
    Power,    //!< 功率谱
    LogPower, //!< 对数功率谱
};

/**
 * @brief 计算离散傅里叶变换
 * 
 * @param[in] xt 时域复信号
 * @return 频域复信号
 */
ComplexSignal dft(const ComplexSignal &xt);

/**
 * @brief 计算离散傅里叶逆变换
 * 
 * @param[in] Xf 频域复信号
 * @return 时域复信号
 */
ComplexSignal idft(const ComplexSignal &Xf);

/**
 * @brief 计算信号谱
 * 
 * @param[in] x 复信号
 * @param[in] type 谱类型
 * @return 实数信号谱
 */
RealSignal Gx(const ComplexSignal &x, GxType type);

#ifdef HAVE_OPENCV

/**
 * @brief 绘制信号
 * 
 * @param[in] datas 信号数据
 * @param[in] color 颜色
 * @return `cv::Mat` 表示的绘制图像
 */
cv::Mat draw(const RealSignal &datas, const cv::Scalar &color);

#endif // HAVE_OPENCV

//! @} algorithm_dsp

} // namespace rm

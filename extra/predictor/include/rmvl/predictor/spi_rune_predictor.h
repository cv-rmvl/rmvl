/**
 * @file spi_rune_predictor.h
 * @author RoboMaster Vision Community
 * @brief 系统参数辨识神符预测头文件
 * @version 1.0
 * @date 2023-06-08
 *
 * @copyright Copyright 2023 (c), RoboMaster Vision Community
 *
 */

#pragma once

#include <vector>

#include "predictor.h"

namespace rm
{

//! @addtogroup spi_rune_predictor
//! @{

//! 系统参数辨识神符预测类
class SpiRunePredictor final : public predictor
{
    const size_t _n; //!< 模型阶数
    const double _T; //!< 采样间隔
    cv::Mat _Pm;     //!< 协方差矩阵
    cv::Mat _xm;     //!< 待求解系数向量

public:
    //! 构造函数
    SpiRunePredictor();

    /**
     * @brief 系统参数辨识神符预测核心函数
     * @note
     * - 静态响应预测量 `B` 生效: `ANG_Z`
     * @note
     * - 动态响应预测量 `Kt` 生效: `ANG_Z`
     *
     * @param[in] groups 序列组列表
     * @param[in] tof 每个追踪器对应的子弹飞行时间
     * @return 预测模块信息
     */
    PredictInfo predict(const std::vector<group::ptr> &groups,
                        const std::unordered_map<tracker::ptr, double> &tof) override;

    //! 构建 SpiRunePredictor
    static inline std::unique_ptr<SpiRunePredictor> make_predictor() { return std::make_unique<SpiRunePredictor>(); }

private:
    /**
     * @brief 从单个追踪器中计算静态预测增量
     *
     * @param[in] p_tracker 单个追踪器
     * @return 静态预测增量
     */
    float staticPredict(tracker::ptr p_tracker);

    /**
     * @brief 角度预测，获取增量
     *
     * @param[in] rawdatas 原始数据队列
     * @param[in] tf 子弹飞行时间（单位 s）
     * @return 角度预测增量 \f$\Delta\theta\f$
     */
    double anglePredict(const std::deque<double> &rawdatas, double tf);

    /**
     * @brief 递推最小二乘的系统参数辨识过程
     *
     * @param[in] rawdatas 原始数据队列
     */
    void identifier(const std::deque<double> &rawdatas);
};

//! @} spi_rune_predictor

} // namespace rm

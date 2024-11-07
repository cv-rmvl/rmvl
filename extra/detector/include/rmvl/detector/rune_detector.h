/**
 * @file rune_detector.h
 * @author RoboMaster Vision Community
 * @brief 神符识别派生类头文件
 * @version 1.0
 * @date 2022-07-20
 *
 * @copyright Copyright 2023 (c), RoboMaster Vision Community
 *
 */

#pragma once

#include "detector.h"
#include "rmvl/tracker/rune_tracker.h"

namespace rm
{

//! @addtogroup rune_detector
//! @{

//! 能量机关识别模块（包含已激活、未激活）
class RuneDetector final : public detector
{
public:
    RuneDetector() = default;

    /**
     * @brief 神符识别核心函数
     *
     * @param[in out] groups 所有序列组
     * @param[in] src 原图像
     * @param[in] color 待识别的颜色
     * @param[in] imu_data IMU 数据
     * @param[in] tick 当前时间点
     * @return 识别信息结构体
     */
    DetectInfo detect(std::vector<group::ptr> &groups, const cv::Mat &src, PixChannel color, const ImuData &imu_data, double tick) override;

    //! 构建 RuneDetector
    static inline std::unique_ptr<RuneDetector> make_detector() { return std::make_unique<RuneDetector>(); }

private:
    /**
     * @brief 找出所有神符目标
     *
     * @param[in] src 预处理之后的图像
     * @param[out] features 找到的所有特征
     * @param[out] combos 找到的所有神符组合体
     */
    void find(cv::Mat src, std::vector<feature::ptr> &features, std::vector<combo::ptr> &combos);

    /**
     * @brief 匹配、更新时间序列
     *
     * @param[in] rune_trackers 神符追踪器
     * @param[in] combos 每一帧的所有目标
     */
    void match(std::vector<tracker::ptr> &rune_trackers, const std::vector<combo::ptr> &combos);

    /**
     * @brief 获取最佳的神符中心点
     * @note 在已知的所有中心点中寻找到最适合作为中心点的一个特征
     *
     * @param[in] p_targets 所有神符靶心
     * @param[in] p_centers 所有神符中心点
     * @return RuneCenter::ptr
     */
    RuneCenter::ptr getBestCenter(const std::vector<RuneTarget::ptr> &p_targets, const std::vector<RuneCenter::ptr> &p_centers);

    /**
     * @brief 获取未激活的神符
     *
     * @param[in] p_targets 所有神符靶心
     * @param[in] p_center 最佳神符中心点
     * @return 未激活的神符
     */
    std::vector<Rune::ptr> getRune(const std::vector<RuneTarget::ptr> &p_targets, RuneCenter::ptr p_center);

    /**
     * @brief 神符匹配至时间序列
     *
     * @param[in out] trackers 神符追踪器列表
     * @param[in] combos 每一帧的所有目标
     */
    void matchRunes(std::vector<tracker::ptr> &trackers, const std::vector<combo::ptr> &combos);
};

//! @} rune_detector

} // namespace rm

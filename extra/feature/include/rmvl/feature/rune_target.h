/**
 * @file rune_target.h
 * @author RoboMaster Vision Community
 * @brief
 * @version 0.1
 * @date 2023-04-09
 *
 * @copyright Copyright 2023 (c), RoboMaster Vision Community
 *
 */

#pragma once

#include "feature.h"

namespace rm
{

//! @addtogroup rune_target
//! @{

//! 神符靶心特征
class RuneTarget : public feature
{
    struct ContourPoint
    {
        cv::Point contour_point;
        float distance;
        bool is_positive = true;
    };

public:
    using ptr = std::shared_ptr<RuneTarget>;
    using const_ptr = std::shared_ptr<const RuneTarget>;

    //! @cond
    RuneTarget() = default;
    RuneTarget(const std::vector<cv::Point> &, const cv::RotatedRect &, bool is_active);
    RuneTarget(const cv::Point &center, bool is_active);
    //! @endcond

    /**
     * @brief 使用轮廓和层次结构构造 RuneTarget 的构造接口
     *
     * @param[in] contour 轮廓
     * @param[in] is_active 是否激活？
     * @return 如果成功，返回 RuneTarget 的共享指针，否则返回 nullptr
     */
    static std::shared_ptr<RuneTarget> make_feature(const std::vector<cv::Point> &contour, bool is_active);

    /**
     * @brief 使用特征中心点构造 RuneTarget 的构造接口
     *
     * @param[in] center 特征中心点
     * @param[in] is_active 是否激活？
     * @return 如果成功，返回 RuneTarget 的共享指针，否则返回 nullptr
     */
    static std::shared_ptr<RuneTarget> make_feature(const cv::Point &center, bool is_active);

    /**
     * @brief 从另一个特征进行构造
     * 
     * @return 指向新特征的共享指针
     */
    feature::ptr clone() override { return std::make_shared<RuneTarget>(*this); }

    RMVL_FEATURE_CAST(RuneTarget)

    //! 获取长宽比
    inline float getRatio() const { return _ratio; }
    //! 是否激活标志位
    inline bool isActive() const { return _is_active; }
    //! 获取半径
    inline float getRadius() const { return _radius; }
    //! 获取轮廓
    inline const std::vector<cv::Point> &getContours() { return _contour; }

private:
    std::vector<cv::Point> _contour; //!< 轮廓点集
    cv::RotatedRect _rotated_rect;   //!< 旋转矩形
    float _ratio{};                  //!< 长宽比
    bool _is_active{};               //!< 是否处于激活状态
    float _radius{};                 //!< 半径
};

//! @} rune_target

} // namespace rm

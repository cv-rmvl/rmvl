/**
 * @file rune_center.h
 * @author RoboMaster Vision Community
 * @brief
 * @version 1.0
 * @date 2021-09-10
 *
 * @copyright Copyright 2021 (c), RoboMaster Vision Community
 *
 */

#pragma once

#include "feature.h"

namespace rm
{

//! @addtogroup rune_center
//! @{

//! 神符中心特征
class RMVL_EXPORTS_W_DES RuneCenter : public feature
{
private:
    std::vector<cv::Point> _contour; //!< 轮廓点集
    cv::RotatedRect _rotated_rect;   //!< 旋转矩形
    float _ratio{};                  //!< 长宽比

public:
    using ptr = std::shared_ptr<RuneCenter>;
    using const_ptr = std::shared_ptr<const RuneCenter>;

    //! @cond
    RuneCenter(const std::vector<cv::Point> &, cv::RotatedRect &);
    RuneCenter(const cv::Point2f &);
    //! @endcond

    /**
     * @brief 使用特征中心点构造 RuneCenter 的构造接口
     *
     * @param[in] center 特征中心点
     * @return 如果成功，返回 RuneCenter 的共享指针，否则返回 nullptr
     */
    RMVL_W static inline ptr make_feature(const cv::Point2f &center) { return std::make_shared<RuneCenter>(center); }

    /**
     * @brief 使用轮廓和层次结构构造 RuneCenter 的构造接口
     *
     * @param[in] contour 轮廓
     * @return 如果成功，返回 RuneCenter 的共享指针，否则返回 nullptr
     */
    RMVL_W static ptr make_feature(const std::vector<cv::Point> &contour);

    /**
     * @brief 从另一个特征进行构造
     *
     * @return 指向新特征的共享指针
     */
    RMVL_W feature::ptr clone() override { return std::make_shared<RuneCenter>(*this); }

    RMVL_FEATURE_CAST(RuneCenter)
};

//! @} rune_center

} // namespace rm

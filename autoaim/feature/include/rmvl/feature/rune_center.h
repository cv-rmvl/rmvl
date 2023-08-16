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
class RuneCenter : public feature
{
private:
    std::vector<cv::Point> _contour; //!< 轮廓点集
    cv::RotatedRect _rotated_rect;   //!< 旋转矩形
    float _ratio = 0.f;              //!< 长宽比

public:
    RuneCenter(const RuneCenter &) = delete;
    RuneCenter(RuneCenter &&) = delete;
    RuneCenter(std::vector<cv::Point> &contour, cv::RotatedRect &rotated_rect);
    RuneCenter(const cv::Point2f &);

    /**
     * @brief 使用特征中心点构造 RuneCenter 的构造接口
     *
     * @param[in] center 特征中心点
     * @return 如果成功，返回 RuneCenter 的共享指针，否则返回 nullptr
     */
    static std::shared_ptr<RuneCenter> make_feature(const cv::Point2f &center);

    /**
     * @brief 使用轮廓和层次结构构造 RuneCenter 的构造接口
     *
     * @param[in] contour 轮廓
     * @return 如果成功，返回 RuneCenter 的共享指针，否则返回 nullptr
     */
    static std::shared_ptr<RuneCenter> make_feature(std::vector<cv::Point> &contour);

    /**
     * @brief 动态类型转换
     *
     * @param[in] p_feature feature_ptr 抽象指针
     * @return 派生对象指针
     */
    static inline std::shared_ptr<RuneCenter> cast(feature_ptr p_feature)
    {
        return std::dynamic_pointer_cast<RuneCenter>(p_feature);
    }

    //! 获取长宽比
    inline float getRatio() { return _ratio; }
    //! 获取轮廓点集
    inline const std::vector<cv::Point> &getContour() { return _contour; }
};

//! 神符中心特征共享指针
using rune_center_ptr = std::shared_ptr<RuneCenter>;

//! @} rune_center

} // namespace rm

/**
 * @file pilot.h
 * @author RoboMaster Vision Community
 * @brief 引导灯类头文件
 * @version 1.0
 * @date 2021-12-14
 *
 * @copyright Copyright 2021 (c), RoboMaster Vision Community
 *
 */

#pragma once

#include "feature.h"

namespace rm
{

//! @addtogroup pilot
//! @{

/**
 * @brief 前哨、基地的引导灯
 * @note 角点包括左顶点、右顶点
 */
class RMVL_EXPORTS_W_DES Pilot : public feature
{
private:
    cv::RotatedRect _rotated_rect; //!< 旋转矩形
    cv::Point2f _left;             //!< 左顶点
    cv::Point2f _right;            //!< 右顶点

public:
    using ptr = std::shared_ptr<Pilot>;
    using const_ptr = std::shared_ptr<const Pilot>;

    //! @cond
    Pilot(const std::vector<cv::Point> &, cv::RotatedRect &, float, float);
    Pilot(const float &, const float &, const cv::Point2f &, const float &, const std::vector<cv::Point2f> &);
    //! @endcond

    /**
     * @brief 构造接口
     *
     * @param contour 引导灯轮廓点
     * @param tri 包含引导灯颜色信息的灰度图像（三值图: 0, 125, 255）
     * @return 如果能够被成功构造，将返回引导灯的共享指针，否则返回空
     */
    RMVL_W static ptr make_feature(const std::vector<cv::Point> &contour, cv::Mat &tri);

    /**
     * @brief 构造 Pilot
     *
     * @param[in] ref_width 参考宽度
     * @param[in] ref_height 参考高度
     * @param[in] ref_center 参考中心点
     * @param[in] ref_angle 参考角度
     * @param[in] ref_corners 参考角点
     * @return 返回 Pilot 的共享指针
     */
    RMVL_W static inline ptr make_feature(float ref_width, float ref_height, const cv::Point2f &ref_center, float ref_angle, const std::vector<cv::Point2f> &ref_corners)
    {
        return std::make_shared<Pilot>(ref_width, ref_height, ref_center, ref_angle, ref_corners);
    }

    /**
     * @brief 从另一个特征进行构造
     *
     * @return 指向新特征的共享指针
     */
    RMVL_W feature::ptr clone() override { return std::make_shared<Pilot>(*this); }

    RMVL_FEATURE_CAST(Pilot)
};

//! @} pilot

} // namespace rm

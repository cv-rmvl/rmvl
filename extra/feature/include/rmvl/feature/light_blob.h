/**
 * @file light_blob.h
 * @author RoboMaster Vision Community
 * @brief 灯条类头文件
 * @version 1.0
 * @date 2021-08-11
 *
 * @copyright Copyright 2021 (c), RoboMaster Vision Community
 *
 */

#pragma once

#include "feature.h"

namespace rm
{

//! @addtogroup light_blob
//! @{

/**
 * @brief 装甲板灯条
 * @note "/" 状态角度为正
 */
class RMVL_EXPORTS_W_DES LightBlob : public feature
{
    cv::RotatedRect _rotated_rect; //!< 旋转矩形
    cv::Point2f _top;              //!< 上顶点
    cv::Point2f _bottom;           //!< 下顶点

public:
    using ptr = std::shared_ptr<LightBlob>;
    using const_ptr = std::shared_ptr<const LightBlob>;

    //! @cond
    LightBlob() = default;
    LightBlob(const cv::Point2f &, const cv::Point2f &, float);
    LightBlob(const std::vector<cv::Point> &, cv::RotatedRect &, float, float);
    //! @endcond

    //! 获取灯条顶端点
    RMVL_W inline const cv::Point2f &getTopPoint() const { return _top; }
    //! 获取灯条底端点
    RMVL_W inline const cv::Point2f &getBottomPoint() const { return _bottom; }

    /**
     * @brief LightBlob 构造接口
     *
     * @param[in] contour 特征轮廓
     * @return 若构造成功则返回 LightBlob 的共享指针，否则返回 nullptr
     */
    RMVL_W static ptr make_feature(const std::vector<cv::Point> &contour);

    /**
     * @brief LightBlob 构造接口，使用若干参数
     *
     * @param[in] top 上顶点
     * @param[in] bottom 下顶点
     * @param[in] width 灯条宽度
     * @return 若构造成功则返回 LightBlob 的共享指针，否则返回 nullptr
     */
    RMVL_W static inline ptr make_feature(const cv::Point2f &top, const cv::Point2f &bottom, float width) { return std::make_shared<LightBlob>(top, bottom, width); }

    /**
     * @brief 从另一个特征进行构造
     *
     * @return 指向新特征的共享指针
     */
    RMVL_W feature::ptr clone() override { return std::make_shared<LightBlob>(*this); }

    RMVL_FEATURE_CAST(LightBlob)

private:
    /**
     * @brief 计算准确的特征信息
     *
     * @param[in] lw_ratio 长宽比
     * @param[in] contour 轮廓点
     */
    void calcAccurateInfo(float lw_ratio, const std::vector<cv::Point> &contour);
};

//! @} light_blob

} // namespace rm

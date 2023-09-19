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
class LightBlob : public feature
{
    cv::RotatedRect _rotated_rect; //!< 旋转矩形
    cv::Point2f _top;              //!< 上顶点
    cv::Point2f _bottom;           //!< 下顶点

public:
    LightBlob() = default;
    LightBlob(LightBlob &&) = delete;
    LightBlob(const LightBlob &) = delete;
    LightBlob(const cv::Point2f &, const cv::Point2f &, float);
    LightBlob(const std::vector<cv::Point> &, cv::RotatedRect &, float, float);

    //! 获取灯条顶端点
    inline cv::Point2f getTopPoint() { return _top; }
    //! 获取灯条底端点
    inline cv::Point2f getBottomPoint() { return _bottom; }

    /**
     * @brief LightBlob 构造接口
     *
     * @param[in] contour 特征轮廓
     * @return 若构造成功则返回 LightBlob 的共享指针，否则返回 nullptr
     */
    static std::shared_ptr<LightBlob> make_feature(const std::vector<cv::Point> &contour);

    /**
     * @brief LightBlob 构造接口，使用若干参数
     *
     * @param[in] top 上顶点
     * @param[in] bottom 下顶点
     * @param[in] width 灯条宽度
     * @return 若构造成功则返回 LightBlob 的共享指针，否则返回 nullptr
     */
    static inline std::shared_ptr<LightBlob> make_feature(const cv::Point2f &top, const cv::Point2f &bottom, float width)
    {
        return std::make_shared<LightBlob>(top, bottom, width);
    }

    /**
     * @brief 动态类型转换
     *
     * @param[in] p_feature feature_ptr 抽象指针
     * @return 派生对象指针
     */
    static inline std::shared_ptr<LightBlob> cast(feature_ptr p_feature)
    {
        return std::dynamic_pointer_cast<LightBlob>(p_feature);
    }

private:
    /**
     * @brief 计算准确的特征信息
     *
     * @param[in] lw_ratio 长宽比
     * @param[in] contour 轮廓点
     */
    void calcAccurateInfo(float lw_ratio, const std::vector<cv::Point> &contour);
};

using light_blob_ptr = std::shared_ptr<LightBlob>;

//! @} light_blob

} // namespace rm

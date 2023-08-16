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
class Pilot : public feature
{
private:
    cv::RotatedRect _rotated_rect; //!< 旋转矩形
    cv::Point2f _left;             //!< 左顶点
    cv::Point2f _right;            //!< 右顶点
    bool _is_matched = false;      //!< 是否匹配标志位

public:
    Pilot(const Pilot &) = delete;
    Pilot(Pilot &&) = delete;

    Pilot(std::vector<cv::Point> &contour, cv::RotatedRect &rotated_rect, float width, float height);
    Pilot(const float &ref_width, const float &ref_height, const cv::Point2f &ref_center, const float &ref_angle, const std::vector<cv::Point2f> &ref_corners);

    /**
     * @brief 构造接口
     *
     * @param contour 引导灯轮廓点
     * @param tri 包含引导灯颜色信息的灰度图像（三值图: 0, 125, 255）
     * @return 如果能够被成功构造，将返回引导灯的共享指针，否则返回空
     */
    static std::shared_ptr<Pilot> make_feature(std::vector<cv::Point> &contour, cv::Mat &tri);

    /**
     * @brief 构造 Pilot
     *
     * @param[in] ref_width 参考宽度
     * @param[in] ref_height 参考高度
     * @param[in] ref_center 参考中心点
     * @param[in] ref_angle 参考角度
     * @param[in] ref_corners 参考角点
     * @return std::shared_ptr<Pilot>
     */
    static inline std::shared_ptr<Pilot> make_feature(float ref_width, float ref_height, const cv::Point2f &ref_center, float ref_angle, const std::vector<cv::Point2f> &ref_corners)
    {
        return std::make_shared<Pilot>(ref_width, ref_height, ref_center, ref_angle, ref_corners);
    }

    /**
     * @brief 动态类型转换
     *
     * @param[in] p_feature feature_ptr 抽象指针
     * @return 派生对象指针
     */
    static inline std::shared_ptr<Pilot> cast(feature_ptr p_feature)
    {
        return std::dynamic_pointer_cast<Pilot>(p_feature);
    }

    //! 设置匹配信息
    inline void setMatchMessage(bool match) { _is_matched = match; }
    //! 获取匹配信息
    inline bool getMatchMessage() { return _is_matched; }
    //! 获取灯条左端点
    inline cv::Point2f getLeftPoint() { return _left; }
    //! 获取灯条右端点
    inline cv::Point2f getRightPoint() { return _right; }

private:
    /**
     * @brief 提取准确的左右顶点
     *
     * @param contour 轮廓信息
     */
    void getTruePoint(std::vector<cv::Point> &);
};

using pilot_ptr = std::shared_ptr<Pilot>;

//! @} pilot

} // namespace rm

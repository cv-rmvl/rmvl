/**
 * @file feature.h
 * @author RoboMaster Vision Community
 * @brief feature class header
 * @version 1.0
 * @date 2021-08-10
 *
 * @copyright Copyright 2021 (c), RoboMaster Vision Community
 *
 */

#pragma once

#include <memory>

#include <opencv2/core/types.hpp>

#include "rmvl/types.hpp"

namespace rm
{

//! @addtogroup feature
//! @{

//! 图像中的轮廓特征
class RMVL_EXPORTS_W_ABS feature
{
protected:
    float _width{};                    //!< 特征宽度
    float _height{};                   //!< 特征高度
    float _angle{};                    //!< 特征角度
    cv::Point2f _center;               //!< 特征中心点
    std::vector<cv::Point2f> _corners; //!< 特征角点

    rm::RMStatus _type; //!< 状态类型信息

public:
    using ptr = std::shared_ptr<feature>;
    using const_ptr = std::shared_ptr<const feature>;

    /**
     * @brief 从另一个特征进行构造
     *
     * @return 指向新特征的共享指针
     */
    RMVL_W virtual ptr clone() = 0;

    //! 获取特征面积
    RMVL_W inline float area() const { return _height * _width; }
    //! 获取特征中心点
    RMVL_W inline const cv::Point2f &center() const { return _center; }
    //! 获取特征宽度
    RMVL_W inline float width() const { return _width; }
    //! 获取特征高度
    RMVL_W inline float height() const { return _height; }
    //! 获取特征角度
    RMVL_W inline float angle() const { return _angle; }
    //! 获取特征角点
    RMVL_W inline const std::vector<cv::Point2f> &corners() const { return _corners; }
    //! 获取状态信息
    RMVL_W inline const RMStatus &type() const { return _type; }
};

#define RMVL_FEATURE_CAST(name)                                                                           \
    static inline ptr cast(feature::ptr p_feature) { return std::dynamic_pointer_cast<name>(p_feature); } \
    static inline const_ptr cast(feature::const_ptr p_feature) { return std::dynamic_pointer_cast<const name>(p_feature); }

//! 默认图像特征，仅表示一个孤立的点 `cv::Point2f`
class RMVL_EXPORTS_W_DES DefaultFeature final : public feature
{
public:
    using ptr = std::shared_ptr<DefaultFeature>;
    using const_ptr = std::shared_ptr<const DefaultFeature>;

    DefaultFeature() = default;
    DefaultFeature(const cv::Point2f &p) : feature() { _center = p, _corners = {p}; }

    /**
     * @brief DefaultFeature 构造接口
     *
     * @param[in] p 孤立的二维点
     * @return DefaultFeature 共享指针
     */
    RMVL_W static inline ptr make_feature(const cv::Point2f &p) { return std::make_shared<DefaultFeature>(p); }

    /**
     * @brief 从另一个特征进行构造
     *
     * @return 指向新特征的共享指针
     */
    RMVL_W feature::ptr clone() override { return std::make_shared<DefaultFeature>(*this); }

    RMVL_FEATURE_CAST(DefaultFeature)
};

//! @} feature

} // namespace rm

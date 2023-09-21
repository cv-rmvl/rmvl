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
class feature
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

    virtual ~feature() = 0;
    //! 获取特征面积
    inline float getArea() const { return _height * _width; }
    //! 获取特征中心点
    inline const cv::Point2f &getCenter() const { return _center; }
    //! 获取特征宽度
    inline float getWidth() const { return _width; }
    //! 获取特征高度
    inline float getHeight() const { return _height; }
    //! 获取特征角度
    inline float getAngle() const { return _angle; }
    //! 获取特征角点
    inline const auto &getCorners() const { return _corners; }
    //! 获取状态信息
    inline const RMStatus &getType() const { return _type; }
};

inline feature::~feature() = default;

//! 默认图像特征，仅表示一个孤立的点 `cv::Point2f`
class DefaultFeature final : public feature
{
public:
    using ptr = std::shared_ptr<DefaultFeature>;
    using const_ptr = std::shared_ptr<const DefaultFeature>;

    DefaultFeature(const cv::Point2f &p) : feature() { _center = p, _corners = {p}; }

    /**
     * @brief DefaultFeature 构造接口
     *
     * @param[in] p 孤立的二维点
     * @return DefaultFeature 共享指针
     */
    inline static std::shared_ptr<DefaultFeature> make_feature(const cv::Point2f &p) { return std::make_shared<DefaultFeature>(p); }

    /**
     * @brief 动态类型转换
     *
     * @param[in] p_feature feature::ptr 抽象指针
     * @return 派生对象指针
     */
    static inline DefaultFeature::ptr cast(feature::ptr p_feature) { return std::dynamic_pointer_cast<DefaultFeature>(p_feature); }

    /**
     * @brief 动态类型转换
     *
     * @param[in] p_feature feature::const_ptr 抽象指针
     * @return 派生对象指针
     */
    static inline DefaultFeature::const_ptr cast(feature::const_ptr p_feature) { return std::dynamic_pointer_cast<const DefaultFeature>(p_feature); }
};

//! @} feature

} // namespace rm

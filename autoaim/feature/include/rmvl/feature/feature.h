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

namespace rm
{

//! @addtogroup feature
//! @{

//! 图像中的轮廓特征
class feature
{
protected:
    float _width = 0.f;                //!< 特征宽度
    float _height = 0.f;               //!< 特征高度
    float _angle = 0.f;                //!< 特征角度
    cv::Point2f _center;               //!< 特征中心点
    std::vector<cv::Point2f> _corners; //!< 特征角点

public:
    virtual ~feature() = 0;
    //! 获取特征面积
    inline virtual float getArea() { return _height * _width; }
    //! 获取特征中心点
    inline virtual cv::Point2f getCenter() { return _center; }
    //! 获取特征宽度
    inline float getWidth() const { return _width; }
    //! 获取特征高度
    inline float getHeight() const { return _height; }
    //! 获取特征角度
    inline float getAngle() const { return _angle; }
    //! 获取特征角点
    inline const std::vector<cv::Point2f> &getCorners() const { return _corners; }
};

inline feature::~feature() = default;

//! 特征共享指针
using feature_ptr = std::shared_ptr<feature>;

//! @} feature

} // namespace rm

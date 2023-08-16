/**
 * @file combo.h
 * @author RoboMaster Vision Community
 * @brief the combination of the features header file
 * @version 1.0
 * @date 2021-08-10
 *
 * @copyright Copyright 2021 (c), RoboMaster Vision Community
 *
 */
#pragma once

#include "rmvl/rmath/result_pnp.hpp"
#include "rmvl/types.hpp"

#include "rmvl/core/dataio.hpp"
#include "rmvl/feature/feature.h"

namespace rm
{

//! @addtogroup combo
//! @{

//! 特征组合体
class combo
{
protected:
    std::vector<feature_ptr> _features; //!< 特征列表
    float _height = 0.f;                //!< 高度
    float _width = 0.f;                 //!< 宽度
    float _angle = 0.f;                 //!< 角度
    RMStatus _type;                     //!< 类型
    cv::Point2f _center;                //!< 中心点
    cv::Point2f _relative_angle;        //!< 相对目标转角
    GyroData _gyro_data;                //!< 当前陀螺仪数据
    std::vector<cv::Point2f> _corners;  //!< 角点
    ResultPnP<float> _pnp_data;         //!< PNP 数据
    int64 _tick;                        //!< 捕获该组合体时的时间戳

public:
    virtual ~combo() = 0;

    //! 获取组合体高度
    inline float getHeight() const { return _height; }
    //! 获取组合体宽度
    inline float getWidth() const { return _width; }
    //! 获取组合体角度
    inline float getAngle() const { return _angle; }
    //! 获取组合体中心点
    inline cv::Point2f getCenter() const { return _center; }
    //! 获取组合体角点
    inline const std::vector<cv::Point2f> &getCorners() { return _corners; }
    //! 获取组合体 PNP 数据
    inline const ResultPnP<float> &getPNP() const { return _pnp_data; }
    //! 获取组合体类型
    inline RMStatus getType() const { return _type; }
    //! 获取捕获该组合体时的时间戳
    inline int64 getTick() const { return _tick; }
    //! 获取组合体的相对目标转角
    inline cv::Point2f getRelativeAngle() const { return _relative_angle; }
    //! 获取组合体当前的陀螺仪数据
    inline const GyroData &getGyroData() const { return _gyro_data; }

    /**
     * @brief 获取指定特征
     *
     * @param[in] idx 下标
     * @return 指定特征
     */
    inline feature_ptr at(size_t idx) const { return _features.at(idx); }
    //! 获取特征列表数据
    inline auto &data() { return _features; }
    //! 获取特征列表大小
    inline size_t size() const { return _features.size(); }
    //! 判断特征列表是否为空
    inline bool empty() const { return _features.empty(); }
};

inline combo::~combo() = default;

using combo_ptr = std::shared_ptr<combo>; //!< 组合体共享指针

//! @} combo

} // namespace rm

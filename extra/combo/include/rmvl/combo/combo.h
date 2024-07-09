/**
 * @file combo.h
 * @author RoboMaster Vision Community
 * @brief the combination of the features header file
 * @version 2.0
 * @date 2021-08-10
 *
 * @copyright Copyright 2021 (c), RoboMaster Vision Community
 *
 */
#pragma once

#include "rmvl/camera/camutils.hpp"
#include "rmvl/algorithm/math.hpp"
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
    std::vector<feature::ptr> _features; //!< 特征列表

    float _height{};                   //!< 高度
    float _width{};                    //!< 宽度
    float _angle{};                    //!< 角度
    RMStatus _type{};                  //!< 类型
    cv::Point2f _center;               //!< 中心点
    cv::Point2f _relative_angle;       //!< 相对目标转角
    GyroData _gyro_data;               //!< 当前陀螺仪数据
    std::vector<cv::Point2f> _corners; //!< 角点
    CameraExtrinsics _extrinsic;       //!< 相机外参
    double _tick{};                    //!< 捕获该组合体时的时间点

public:
    using ptr = std::shared_ptr<combo>;
    using const_ptr = std::shared_ptr<const combo>;

    /**
     * @brief 从另一个组合体进行构造
     *
     * @param[in] tick 当前时间点，可用 `rm::Timer::now()` 获取
     * @return 指向新组合体的共享指针
     */
    virtual ptr clone(double tick) = 0;

    //! 获取组合体高度
    inline float getHeight() const { return _height; }
    //! 获取组合体宽度
    inline float getWidth() const { return _width; }
    //! 获取组合体角度
    inline float getAngle() const { return _angle; }
    //! 获取组合体中心点
    inline cv::Point2f getCenter() const { return _center; }
    //! 获取组合体角点
    inline const std::vector<cv::Point2f> &getCorners() const { return _corners; }
    //! 获取组合体相机外参
    inline const CameraExtrinsics &getExtrinsics() const { return _extrinsic; }
    //! 获取组合体类型
    inline RMStatus getType() const { return _type; }
    //! 获取捕获该组合体的时间点
    inline double getTick() const { return _tick; }
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
    inline feature::ptr at(size_t idx) { return _features.at(idx); }

    /**
     * @brief 获取指定特征
     *
     * @param[in] idx 下标
     * @return 指定特征
     */
    inline const feature::const_ptr at(size_t idx) const { return _features.at(idx); }

    //! 获取特征列表数据
    inline const auto &data() const { return _features; }
    //! 获取特征列表大小
    inline size_t size() const { return _features.size(); }
    //! 判断特征列表是否为空
    inline bool empty() const { return _features.empty(); }
};

//! 默认组合体，包含一个固定的特征，退化为 `feature` 使用
class DefaultCombo final : public combo
{
public:
    using ptr = std::shared_ptr<DefaultCombo>;
    using const_ptr = std::shared_ptr<const DefaultCombo>;

    //! @warning 构造函数不直接使用
    DefaultCombo(feature::ptr, double);

    /**
     * @brief 构造 DefaultCombo
     *
     * @param[in] p_feature 特征 `feature` 共享指针
     * @param[in] tick 当前时间点，可用 `rm::Timer::now()` 获取
     * @return DefaultCombo 共享指针
     */
    static inline ptr make_combo(feature::ptr p_feature, double tick) { return std::make_shared<DefaultCombo>(p_feature, tick); }

    /**
     * @brief 从另一个组合体进行构造
     *
     * @param[in] tick 当前时间点，可用 `rm::Timer::now()` 获取
     * @return 指向新组合体的共享指针
     */
    combo::ptr clone(double tick) override
    {
        auto retval = std::make_shared<DefaultCombo>(*this);
        retval->_tick = tick;
        return retval;
    }

    /**
     * @brief 动态类型转换
     *
     * @param[in] p_combo combo::ptr 抽象指针
     * @return 派生对象指针
     */
    static inline ptr cast(combo::ptr p_combo) { return std::dynamic_pointer_cast<DefaultCombo>(p_combo); }

    /**
     * @brief 动态类型转换
     *
     * @param[in] p_combo combo::const_ptr 抽象指针
     * @return 派生对象指针
     */
    static inline const_ptr cast(combo::const_ptr p_combo) { return std::dynamic_pointer_cast<const DefaultCombo>(p_combo); }
};

//! @} combo

} // namespace rm

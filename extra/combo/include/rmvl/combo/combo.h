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

#include "rmvl/algorithm/math.hpp"
#include "rmvl/camera/camutils.hpp"

#include "rmvl/core/io.hpp"
#include "rmvl/feature/feature.h"

namespace rm
{

//! @addtogroup combo
//! @{

//! 特征组合体
class RMVL_EXPORTS_W_ABS combo
{
protected:
    std::vector<feature::ptr> _features; //!< 特征列表

    float _height{};                   //!< 高度
    float _width{};                    //!< 宽度
    float _angle{};                    //!< 角度
    RMStatus _type{};                  //!< 类型
    cv::Point2f _center;               //!< 中心点
    cv::Point2f _relative_angle;       //!< 相对目标转角
    ImuData _imu_data;                 //!< 当前 IMU 数据
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
    RMVL_W virtual ptr clone(double tick) = 0;

    //! 获取组合体高度
    RMVL_W inline float height() const { return _height; }
    //! 获取组合体宽度
    RMVL_W inline float width() const { return _width; }
    //! 获取组合体角度
    RMVL_W inline float angle() const { return _angle; }
    //! 获取组合体中心点
    RMVL_W inline cv::Point2f center() const { return _center; }
    //! 获取组合体角点
    RMVL_W inline const std::vector<cv::Point2f> &corners() const { return _corners; }

    /**
     * @brief 获取指定角点
     *
     * @param[in] idx 下标
     * @return 指定角点
     */
    RMVL_W inline cv::Point2f corner(int idx) const { return _corners[idx]; }
    //! 获取组合体相机外参
    RMVL_W inline const CameraExtrinsics &extrinsic() const { return _extrinsic; }
    //! 获取组合体类型
    RMVL_W inline RMStatus type() const { return _type; }
    //! 获取捕获该组合体的时间点
    RMVL_W inline double tick() const { return _tick; }
    //! 获取组合体的相对目标转角
    RMVL_W inline cv::Point2f getRelativeAngle() const { return _relative_angle; }
    //! 获取组合体当前的 IMU 数据
    RMVL_W inline const ImuData &imu() const { return _imu_data; }

    /**
     * @brief 获取指定特征
     *
     * @param[in] idx 下标
     * @return 指定特征
     */
    RMVL_W inline feature::ptr at(std::size_t idx) { return _features.at(idx); }

    /**
     * @brief 获取指定特征
     *
     * @param[in] idx 下标
     * @return 指定特征
     */
    inline const feature::const_ptr at(std::size_t idx) const { return _features.at(idx); }

    //! 获取特征列表数据
    RMVL_W inline const std::vector<feature::ptr> &data() const { return _features; }
    //! 获取特征列表大小
    RMVL_W inline std::size_t size() const { return _features.size(); }
    //! 判断特征列表是否为空
    RMVL_W inline bool empty() const { return _features.empty(); }
};

#define RMVL_COMBO_CAST(name)                                                                       \
    static inline ptr cast(combo::ptr p_combo) { return std::dynamic_pointer_cast<name>(p_combo); } \
    static inline const_ptr cast(combo::const_ptr p_combo) { return std::dynamic_pointer_cast<const name>(p_combo); }

//! 默认组合体，包含一个固定的特征，退化为 `feature` 使用
class RMVL_EXPORTS_W_DES DefaultCombo final : public combo
{
public:
    using ptr = std::shared_ptr<DefaultCombo>;
    using const_ptr = std::shared_ptr<const DefaultCombo>;

    //! @cond
    DefaultCombo(feature::ptr, double);
    //! @endcond

    /**
     * @brief 构造 DefaultCombo
     *
     * @param[in] p_feature 特征 `feature` 共享指针
     * @param[in] tick 当前时间点，可用 `rm::Timer::now()` 获取
     * @return DefaultCombo 共享指针
     */
    RMVL_W static inline ptr make_combo(feature::ptr p_feature, double tick) { return std::make_shared<DefaultCombo>(p_feature, tick); }

    /**
     * @brief 从另一个组合体进行构造
     *
     * @param[in] tick 当前时间点，可用 `rm::Timer::now()` 获取
     * @return 指向新组合体的共享指针
     */
    RMVL_W combo::ptr clone(double tick) override;

    RMVL_COMBO_CAST(DefaultCombo)
};

//! @} combo

} // namespace rm

/**
 * @file rune.h
 * @author RoboMaster Vision Community
 * @brief 神符类头文件
 * @version 1.0
 * @date 2021-09-11
 *
 * @copyright Copyright 2021 (c), RoboMaster Vision Community
 *
 */

#pragma once

#include "combo.h"

#include "rmvl/feature/rune_center.h"
#include "rmvl/feature/rune_target.h"

namespace rm {

//! @addtogroup combo_rune
//! @{

/**
 * @brief 神符（能量机关）
 * - 特征信息 `[0]`: 神符靶心，`[1]`: 神符旋转中心（R 标）
 * - 角度信息 `angle()` 获取的角度范围是 `[-180,180)`
 * - 角点信息 `corners()` 获取的内容是 `[0]`: 神符靶心特征中心点，`[1]`: 神符中心特征中心点
 * - 相机外参 `extrinsic()` 数据仅直线距离 `distance` 有效
 * - 中心点 `center()` 为神符靶心的中心点，要获取神符旋转中心请访问对应特征，例如使用 `at(1)`
 */
class RMVL_EXPORTS_W_DES Rune final : public combo {
public:
    using ptr = std::shared_ptr<Rune>;
    using const_ptr = std::shared_ptr<const Rune>;

    //! @cond
    Rune() = default;
    Rune(RuneTarget::ptr p_target, RuneCenter::ptr p_center, const ImuData &imu_data, double tick);
    //! @endcond

    /**
     * @brief Rune 构造接口
     *
     * @param[in] p_target 神符靶心
     * @param[in] p_center 神符中心
     * @param[in] imu_data IMU 数据
     * @param[in] tick 捕获特征的时间点
     * @param[in] force 是否为强制构造
     * @return 神符共享指针
     */
    RMVL_W static ptr make_combo(RuneTarget::ptr p_target, RuneCenter::ptr p_center, const ImuData &imu_data, int64_t tick, bool force = false);

    /**
     * @brief 从另一个组合体进行构造
     *
     * @param[in] tick 当前时间点，可用 `rm::Time::now()` 获取
     * @return 指向新组合体的共享指针
     */
    RMVL_W combo::ptr clone(int64_t tick) override;

    RMVL_COMBO_CAST(Rune)

    /**
     * @brief 相机平面中将角度值映射至垂直平面
     * @note 为保证域的连续性，角度使用从原点出发的向量表示
     *
     * @param[in] angle_vec 相机平面中的角度值（起点指向终点的向量）
     * @param[in] diff_theta 两个平面之间的夹角（角度制）
     * @return 垂直平面中的角度值
     */
    static cv::Vec2f cameraConvertToVertical(const cv::Vec2f &angle_vec, float diff_theta);

    /**
     * @brief 垂直平面中将角度值映射至相机平面
     * @note 为保证域的连续性，角度使用从原点出发的向量表示
     *
     * @param[in] angle_vec 垂直平面中的角度值（起点指向终点的向量）
     * @param[in] diff_theta 两个平面之间的夹角（角度制）
     * @return 相机平面中的角度值
     */
    static cv::Vec2f verticalConvertToCamera(const cv::Vec2f &angle_vec, float diff_theta);

    //! 3D 空间下神符特征间距
    RMVL_W inline float getFeatureDis() const { return _feature_dis; }
    //! 是否激活
    RMVL_W inline bool isActive() { return _is_active; }

private:
    float _feature_dis{}; //!< 特征间距
    bool _is_active{};    //!< 是否激活
};

//! @} combo_rune

} // namespace rm

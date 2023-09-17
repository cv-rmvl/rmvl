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

namespace rm
{

//! @addtogroup combo_rune
//! @{

//! 能量机关神符
class Rune final : public combo
{
    float _feature_dis = 0.f; //!< 特征间距
    bool _is_active = false;  //!< 是否激活

public:
    Rune(const rune_target_ptr &p_target, const rune_center_ptr &p_center, const GyroData &gyro_data, int64 tick);
    Rune(const Rune &) = delete;
    Rune(Rune &&) = delete;

    /**
     * @brief Rune 构造接口
     *
     * @param[in] p_target 神符靶心
     * @param[in] p_center 神符中心
     * @param[in] gyro_data 陀螺仪数据
     * @param[in] tick 捕获特征的时间戳
     * @param[in] force 是否为强制构造
     * @return std::shared_ptr<Rune>
     */
    static std::shared_ptr<Rune> make_combo(const rune_target_ptr &p_target, const rune_center_ptr &p_center,
                                            const GyroData &gyro_data, int64 tick, bool force = false);

    /**
     * @brief 动态类型转换
     *
     * @param[in] p_combo combo_ptr 抽象指针
     * @return 派生对象指针
     */
    static inline std::shared_ptr<Rune> cast(combo_ptr p_combo)
    {
        return std::dynamic_pointer_cast<Rune>(p_combo);
    }

    /**
     * @brief 相机平面中将角度值映射至垂直平面
     * @note 为保证域的连续性，角度使用从原点出发的向量表示
     *
     * @param[in] angle_vec 相机平面中的角度值（起点指向终点的向量）
     * @param[in] diff_theta 两个平面之间的夹角（角度制）
     * @return 垂直平面中的角度值
     */
    static inline cv::Vec2f cameraConvertToVertical(const cv::Vec2f &angle_vec, float diff_theta)
    {
        return {angle_vec(0), angle_vec(1) * sec(deg2rad(diff_theta))};
    }

    /**
     * @brief 垂直平面中将角度值映射至相机平面
     * @note 为保证域的连续性，角度使用从原点出发的向量表示
     *
     * @param[in] angle_vec 垂直平面中的角度值（起点指向终点的向量）
     * @param[in] diff_theta 两个平面之间的夹角（角度制）
     * @return 相机平面中的角度值
     */
    static inline cv::Vec2f verticalConvertToCamera(const cv::Vec2f &angle_vec, float diff_theta)
    {
        return {angle_vec(0), angle_vec(1) * cos(deg2rad(diff_theta))};
    }

    //! 3D 空间下神符特征间距
    inline float getFeatureDis() const { return _feature_dis; }
    //! 是否激活
    inline bool isActive() { return _is_active; }

private:
    /**
     * @brief 计算神符装甲板的四个角点
     *
     * @param[in] p_target 神符靶心
     * @param[in] p_center 神符中心
     * @return 按照左下，左上，右上，右下的顺序排列的角点向量
     */
    static std::vector<cv::Point2f> calculatePoints(rune_target_ptr p_target, rune_center_ptr p_center);
};

/**
 * @brief 神符组合特征共享指针
 * @note
 * - `at(0)`: 神符靶心，`at(1)`: 神符旋转中心（R 标）
 * - `getAngle()` 获取的角度范围是 `[-180,180)`
 * - `getCorners()`
 *   - `[0]`: 靶心环最靠近神符中心的点
 *   - `[1]`: 靶心环与角点[0][2]的中垂线的交点（顺时针）
 *   - `[2]`: 靶心环最远离神符中心的点
 *   - `[3]`: 靶心环与角点[0][2]的中垂线的交点（顺时针）
 *   - `[4]`: 神符中心点
 * - `getExtrinsics` 数据仅直线距离 `distance` 有效
 * - `getCenter` 为神符靶心的中心点，要获取神符旋转中心请访问对应特征 `at(1)`
 */
using rune_ptr = std::shared_ptr<Rune>;

//! @} combo_rune

} // namespace rm

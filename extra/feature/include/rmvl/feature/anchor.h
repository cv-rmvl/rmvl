/**
 * @file anchor.h
 * @author zhaoxi (535394140@qq.com)
 * @brief 定位点特征
 * @version 1.0
 * @date 2025-02-06
 *
 * @copyright Copyright 2025 (c), zhaoxi
 *
 */

#pragma once

#include <rmvl/feature/feature.h>

namespace rm
{

//! 定位点类型
enum class AnchorType
{
    Unknown, //!< 未知
    Circle,  //!< 圆形
    Square,  //!< 正方形
    Cross,   //!< 十字交叉
};

/**
 * @brief 定位点特征，具有以下状态类型
 * - "anchor": "circle" 或 "square" 或 "triangle" 或 "cross" 均为字符串状态，标识定位点类型
 * - 当 anchor 为 "circle" 时，具有
 *   - "e": <number of eccentricity> 为数值状态，表示图像中测量的离心率
 */
class Anchor final : public rm::feature
{
public:
    using ptr = std::shared_ptr<Anchor>;
    using const_ptr = std::shared_ptr<const Anchor>;

    Anchor() = default;

    /**
     * @brief Anchor 构造接口
     *
     * @param[in] contour 定位点轮廓
     * @param[in] type 定位点类型
     * @return Anchor 共享指针
     */
    static ptr make_feature(const std::vector<cv::Point> &contour, AnchorType type);

    /**
     * @brief 从另一个特征进行构造
     *
     * @return 指向新特征的共享指针
     */
    feature::ptr clone() override { return std::make_shared<Anchor>(*this); }

    RMVL_FEATURE_CAST(Anchor)

    //! 获取定位点类型的字符串表示
    static std::string_view to_string(AnchorType type);

    //! 从字符串类型获取定位点类型
    static AnchorType from_string(std::string_view type);
};

} // namespace rm

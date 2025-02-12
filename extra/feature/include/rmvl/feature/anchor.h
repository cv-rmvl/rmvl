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

//! @addtogroup anchor
//! @{
//! @brief 包含圆形、方形、十字交叉三种类型的定位点特征
//! @details ![anchor](extra/anchor.png)

//! 定位点类型
enum class AnchorType
{
    Unknown, //!< 未知
    Circle,  //!< 圆形
    Square,  //!< 方形
    Cross,   //!< 十字交叉
};

/**
 * @brief 定位点特征，具有以下状态类型
 * - `anchor`，可包含 `"circle"` 或 `"square"` 或 `"cross"` 其一的字符串状态，表示定位点类型
 * - 当 `anchor` 为 `"circle"` 时，状态类型还具有
 *   - `e`，为数值状态，表示图像中测量的离心率
 */
class RMVL_EXPORTS_W_DES Anchor final : public feature
{
public:
    using ptr = std::shared_ptr<Anchor>;
    using const_ptr = std::shared_ptr<const Anchor>;

    /**
     * @brief Anchor 构造接口
     *
     * @param[in] contour 定位点轮廓
     * @param[in] type 定位点类型
     * @return Anchor 共享指针
     */
    RMVL_W static ptr make_feature(const std::vector<cv::Point> &contour, AnchorType type);

    /**
     * @brief 从另一个特征进行构造
     *
     * @return 指向新特征的共享指针
     */
    RMVL_W feature::ptr clone() override { return std::make_shared<Anchor>(*this); }

    RMVL_FEATURE_CAST(Anchor)

    //! 获取定位点类型的字符串表示
    RMVL_W static std::string_view to_string(AnchorType type);

    //! 从字符串类型获取定位点类型
    RMVL_W static AnchorType from_string(std::string_view type);
};

//! @} anchor

} // namespace rm

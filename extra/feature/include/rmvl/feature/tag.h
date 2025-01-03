/**
 * @file tag.h
 * @author zhaoxi (535394140@qq.com)
 * @brief AprilTag 视觉标签特征类
 * @version 1.0
 * @date 2023-09-18
 *
 * @copyright Copyright 2023 (c), zhaoxi
 *
 */

#pragma once

#include "feature.h"

namespace rm
{

//! @addtogroup tag
//! @{

//! AprilTag 视觉标签特征类
class RMVL_EXPORTS_W_DES Tag final : public feature
{
public:
    using ptr = std::shared_ptr<Tag>;
    using const_ptr = std::shared_ptr<const Tag>;

    //! @cond
    Tag(const std::vector<cv::Point2f> &corners, char type);
    //! @endcond

    /**
     * @brief 构造 Tag 对象
     *
     * @param[in] corners 角点列表
     * @param[in] type AprilTag 视觉标签类型，包含 `A` 到 `Z` 和 `0` 到 `9`
     * @return 构造成功返回 Tag 共享指针，否则返回 `nullptr`
     */
    RMVL_W static inline ptr make_feature(const std::vector<cv::Point2f> &corners, char type)
    {
        if ((type >= 'A' && type <= 'Z') || (type >= '0' && type <= '9'))
            return std::make_shared<Tag>(corners, type);
        else
            return nullptr;
    }

    /**
     * @brief 从另一个特征进行构造
     *
     * @return 指向新特征的共享指针
     */
    RMVL_W feature::ptr clone() override { return std::make_shared<Tag>(*this); }

    RMVL_FEATURE_CAST(Tag)
};

//! @} tag

} // namespace rm

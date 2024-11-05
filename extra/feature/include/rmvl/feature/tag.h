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
class Tag final : public feature
{
public:
    using ptr = std::shared_ptr<Tag>;
    using const_ptr = std::shared_ptr<const Tag>;

    //! @cond
    Tag(const std::array<cv::Point2f, 4> &corners, TagType type);
    //! @endcond

    /**
     * @brief 构造 Tag 对象
     *
     * @param[in] corners 角点列表
     * @param[in] type AprilTag 视觉标签类型，可参考 @ref rm::TagType
     * @return 构造成功返回 Tag 共享指针，否则返回 `nullptr`
     */
    static inline ptr make_feature(const std::array<cv::Point2f, 4> &corners, TagType type) { return std::make_shared<Tag>(corners, type); }

    /**
     * @brief 从另一个特征进行构造
     * 
     * @return 指向新特征的共享指针
     */
    feature::ptr clone() override { return std::make_shared<Tag>(*this); }

    RMVL_FEATURE_CAST(Tag)
};

//! @} tag

} // namespace rm

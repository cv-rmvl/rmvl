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

    //! @warning 不直接使用构造函数
    Tag(const std::array<cv::Point2f, 4> &corners, TagType type);

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

    /**
     * @brief 动态类型转换
     *
     * @param[in] p_feature feature::ptr 抽象指针
     * @return 派生对象指针
     */
    static inline ptr cast(feature::ptr p_feature) { return std::dynamic_pointer_cast<Tag>(p_feature); }

    /**
     * @brief 动态类型转换
     *
     * @param[in] p_feature feature::const_ptr 抽象指针
     * @return 派生对象指针
     */
    static inline const_ptr cast(feature::const_ptr p_feature) { return std::dynamic_pointer_cast<const Tag>(p_feature); }
};

//! @} tag

} // namespace rm

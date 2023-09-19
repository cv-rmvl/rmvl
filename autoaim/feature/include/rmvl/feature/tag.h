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
    //! @warning 不直接使用构造函数
    Tag(const std::vector<cv::Point2f> &corners, TagType type);

    /**
     * @brief 构造 Tag 对象
     *
     * @param[in] corners 角点列表（必须包含 `4` 个角点，否则构造失败）
     * @param[in] type AprilTag 视觉标签类型，可参考 @ref rm::TagType
     * @return 构造成功返回 Tag 共享指针，否则返回 `nullptr`
     */
    static std::shared_ptr<Tag> make_feature(const std::vector<cv::Point2f> &corners, TagType type)
    {
        if (corners.size() != 4)
            return nullptr;
        return std::make_shared<Tag>(corners, type);
    }
};

using tag_ptr = std::shared_ptr<Tag>;

} // namespace rm

//! @} tag

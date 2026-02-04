/**
 * @file tag_detector.h
 * @author zhaoxi (535394140@qq.com)
 * @brief AprilTag 识别模块
 * @version 1.0
 * @date 2023-09-17
 *
 * @copyright Copyright 2023 (c), zhaoxi
 *
 */

#pragma once

#include "rmvl/feature/tag.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

struct apriltag_family;
typedef struct apriltag_family apriltag_family_t;
struct apriltag_detector;
typedef struct apriltag_detector apriltag_detector_t;

#ifdef __cplusplus
}
#endif

namespace rm {

//! @addtogroup tag_detector
//! @{

/**
 * @brief AprilTag 识别器
 * @note 仅支持 Tag25h9 格式
 * @cite AprilRobotics23
 */
class RMVL_EXPORTS_W_DEU TagDetector final {
    apriltag_family_t *_tf{};
    apriltag_detector_t *_td{};

public:
    using ptr = std::unique_ptr<TagDetector>;

    //! @cond
    TagDetector();
    ~TagDetector();

    TagDetector(const TagDetector &) = delete;
    TagDetector(TagDetector &&) = default;
    TagDetector &operator=(const TagDetector &) = delete;
    TagDetector &operator=(TagDetector &&) = default;
    //! @endcond

    //! 构造 TagDetector
    RMVL_W static inline ptr make_detector() { return std::make_unique<TagDetector>(); }

    /**
     * @brief 识别接口
     * @note 提取出所有角点以及对应的类型，通过 `type` 和 `corners` 方法可以获取
     *
     * @param[in] src 原图像
     * @return 识别到的 Tag 列表
     */
    RMVL_W std::vector<Tag::const_ptr> detect(cv::Mat src);
};

//! @} tag_detector

} // namespace rm

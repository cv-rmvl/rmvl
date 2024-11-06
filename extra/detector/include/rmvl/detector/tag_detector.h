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

#include "detector.h"

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

namespace rm
{

//! @addtogroup tag_detector
//! @{

/**
 * @brief AprilTag 识别器
 * @note 仅支持 Tag25h9 格式
 * @cite AprilRobotics23
 */
class TagDetector final : public detector
{
    apriltag_family_t *_tf;
    apriltag_detector_t *_td;

public:
    TagDetector();
    ~TagDetector();

    TagDetector(const TagDetector &) = delete;
    TagDetector(TagDetector &&) = delete;

    //! 构造 TagDetector
    static inline std::unique_ptr<TagDetector> make_detector() { return std::make_unique<TagDetector>(); }

    /**
     * @brief 识别接口
     * @note 提取出所有角点以及对应的类型，通过 `type` 和 `corners` 方法可以获取
     *
     * @param[in out] groups 所有序列组
     * @param[in] src 原图像
     * @param[in] color 待处理颜色
     * @param[in] imu_data 当前 IMU 数据
     * @param[in] tick 当前时间点
     */
    DetectInfo detect(std::vector<group::ptr> &groups, cv::Mat &src, PixChannel color,
                      const ImuData &imu_data, double tick) override;

private:
    /**
     * @brief Tag 特征组合体匹配至时间序列
     *
     * @param[in out] trackers 所有追踪器序列
     * @param[in] combos 每一帧的所有目标
     */
    void match(std::vector<tracker::ptr> &trackers, const std::vector<combo::ptr> &combos);

    /**
     * @brief 及时删除多帧为空的序列
     *
     * @param[in out] trackers 所有追踪器序列
     */
    void eraseNullTracker(std::vector<tracker::ptr> &trackers);
};

//! @} tag_detector

} // namespace rm

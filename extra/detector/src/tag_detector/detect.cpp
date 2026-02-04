/**
 * @file detect.cpp
 * @author zhaoxi (535394140@qq.com)
 * @brief
 * @version 1.0
 * @date 2023-05-08
 *
 * @copyright Copyright 2023 (c), zhaoxi
 *
 */

#include <unordered_set>

#include <opencv2/imgproc.hpp>

#include "rmvl/core/util.hpp"
#include "rmvl/detector/tag_detector.h"
#include "rmvl/feature/tag.h"

#include "tag25h9.h"

namespace rm {

TagDetector::TagDetector() {
    _tf = tag25h9_create();
    _td = apriltag_detector_create();
    // 初始化 AprilTag 检测器
    apriltag_detector_add_family(_td, _tf);
}

TagDetector::~TagDetector() {
    apriltag_detector_destroy(_td);
    tag25h9_destroy(_tf);
}

std::vector<Tag::const_ptr> TagDetector::detect(cv::Mat src) {
    cv::Mat gray{};
    cvtColor(src, gray, cv::COLOR_BGR2GRAY);
    std::vector<std::vector<cv::Point2f>> corners;
    std::vector<char> types;

    // 格式转换
    image_u8_t apriltag_img = {gray.cols, gray.rows, gray.cols, gray.data};

    // 检测 AprilTag
    zarray_t *detections = apriltag_detector_detect(_td, &apriltag_img);
    int target_size = zarray_size(detections);
    corners.resize(target_size);
    for (int i = 0; i < target_size; i++)
        corners[i].resize(4);
    types.resize(target_size);
    for (int i = 0; i < target_size; i++) {
        apriltag_detection_t *det = nullptr;
        zarray_get(detections, i, &det);
        for (int j = 0; j < 4; j++)
            corners[i][j] = cv::Point2f(static_cast<float>(det->p[j][0]),
                                        static_cast<float>(det->p[j][1]));
        if (det->id >= 0 && det->id <= 9)
            types[i] = '0' + det->id;
        else if (det->id >= 10 && det->id <= 35)
            types[i] = 'A' + det->id - 10;
        else
            types[i] = '-';
    }

    // 释放资源
    apriltag_detections_destroy(detections);

    if (corners.size() != types.size())
        RMVL_Error(RMVL_StsBadSize, "Size of the corners and type are not equal");
    size_t feature_size = corners.size();

    std::vector<Tag::const_ptr> res{};
    res.reserve(feature_size);
    for (size_t i = 0; i < feature_size; ++i) {
        auto tag = Tag::make_feature(corners[i], types[i]);
        if (tag == nullptr)
            continue;
        res.push_back(tag);
    }
    return res;
}

} // namespace rm

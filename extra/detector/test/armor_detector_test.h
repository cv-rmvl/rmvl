/**
 * @file armor_detector_test.h
 * @author RoboMaster Vision Community
 * @brief 装甲板识别单元测试头文件
 * @version 1.0
 * @date 2022-08-30
 *
 * @copyright Copyright 2023 (c), RoboMaster Vision Community
 *
 */

#pragma once

#include <gtest/gtest.h>

#include <opencv2/imgproc.hpp>

#include "rmvl/algorithm/math.hpp"
#include "rmvl/detector/armor_detector.h"

namespace rm_test
{

// 装甲板识别单元测试组件
class ArmorDetectorTest : public testing::Test
{
public:
    cv::Mat src;
    rm::detector::ptr p_detector;
    std::vector<rm::group::ptr> groups;

    void SetUp() override
    {
        resetImg();
        resetDetector();
    }
    void TearDown() override { p_detector.reset(); }

    /**
     * @brief 重置图像
     */
    inline void resetImg() { src = cv::Mat::zeros(cv::Size(1280, 1024), CV_8UC3); }

    /**
     * @brief 重置识别器
     */
    inline void resetDetector() { p_detector = rm::ArmorDetector::make_detector(); }

    /**
     * @brief 构建 Armor
     *
     * @param center 装甲板中心点
     * @param angle 装甲板倾角
     * @return
     */
    void buildArmorImg(cv::Point center, float angle)
    {
        buildBlobImg(angle, center - cv::Point(125 * cos(rm::deg2rad(angle)), 125 * sin(rm::deg2rad(angle))));
        buildBlobImg(angle, center + cv::Point(125 * cos(rm::deg2rad(angle)), 125 * sin(rm::deg2rad(angle))));
    }

    /**
     * @brief 构造 LightBlob
     *
     * @param angle 倾斜角
     * @param center 中心点
     * @return
     */
    void buildBlobImg(float angle, cv::Point center)
    {
        cv::Point base_bias(static_cast<int>(-110 * sin(rm::deg2rad(angle))),
                            static_cast<int>(110 * cos(rm::deg2rad(angle))));
        cv::line(src, center - base_bias / 2, center + base_bias / 2, cv::Scalar(0, 0, 255), 12);
    }
};

} // namespace rm_test

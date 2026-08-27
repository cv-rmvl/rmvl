/**
 * @file test_cv.cpp
 * @author zhaoxi (535394140@qq.com)
 * @brief OpenCV 与 LPSS 消息转换单元测试
 * @version 1.0
 * @date 2026-08-24
 *
 * @copyright Copyright 2026 (c), zhaoxi
 *
 */

#include "rmvl/lpss/cv.hpp"

#ifdef HAVE_OPENCV

#include <cstdint>

#include <gtest/gtest.h>

namespace rm_test {

using namespace rm;

TEST(LPSS_cvmsg, converts_depth_images) {
    cv::Mat depth16(2, 3, CV_16UC1);
    depth16.at<uint16_t>(0, 0) = 1234U;
    depth16.at<uint16_t>(1, 2) = 5678U;
    const auto message16 = cvmsg::to_msg(depth16, msg::Image::encoding_16uc1);
    EXPECT_EQ(message16.data.size(), 12U);
    const auto restored16 = cvmsg::from_msg(message16);
    ASSERT_EQ(restored16.type(), CV_16UC1);
    EXPECT_EQ(restored16.at<uint16_t>(0, 0), 1234U);
    EXPECT_EQ(restored16.at<uint16_t>(1, 2), 5678U);

    cv::Mat depth32(2, 3, CV_32FC1);
    depth32.at<float>(0, 0) = 1.25F;
    depth32.at<float>(1, 2) = 4.5F;
    const auto message32 = cvmsg::to_msg(depth32, msg::Image::encoding_32fc1);
    EXPECT_EQ(message32.data.size(), 24U);
    const auto restored32 = cvmsg::from_msg(message32);
    ASSERT_EQ(restored32.type(), CV_32FC1);
    EXPECT_FLOAT_EQ(restored32.at<float>(0, 0), 1.25F);
    EXPECT_FLOAT_EQ(restored32.at<float>(1, 2), 4.5F);
}

} // namespace rm_test

#endif // HAVE_OPENCV

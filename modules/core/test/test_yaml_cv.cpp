/**
 * @file test_yaml_cv.cpp
 * @brief YAML 模块 OpenCV 类型兼容测试
 */

#include <rmvl/rmvl_modules.hpp>

#ifdef HAVE_OPENCV

#include <string>
#include <vector>

#include <gtest/gtest.h>

#include <opencv2/core/persistence.hpp>

#include "rmvl/core/yaml.hpp"

namespace rm_test {

using namespace rm::yaml;

TEST(YAML_OpenCV, parse_opencv_matrix) {
    const auto result = parse(R"(%YAML:1.0
---
camera_matrix: !!opencv-matrix
   rows: 3
   cols: 3
   dt: d
   data: [ 1250., 0., 640., 0., 1250., 512., 0., 0., 1. ]
)");
    ASSERT_TRUE(result) << result.error.message;
    EXPECT_NE((*result)["camera_matrix"].tag().find("opencv-matrix"), std::string_view::npos);

    const auto matrix = (*result)["camera_matrix"].as<cv::Matx33d>();
    ASSERT_TRUE(matrix.has_value());
    EXPECT_DOUBLE_EQ((*matrix)(0, 0), 1250.0);
    EXPECT_DOUBLE_EQ((*matrix)(1, 2), 512.0);
}

TEST(YAML_OpenCV, parse_opencv_matrix_without_header) {
    const auto result = parse(R"(camera_matrix: !!opencv-matrix
  rows: 2
  cols: 2
  dt: f
  data: [1., 2., 3., 4.]
)");
    ASSERT_TRUE(result) << result.error.message;

    const auto matrix = (*result)["camera_matrix"].as<cv::Matx22f>();
    ASSERT_TRUE(matrix.has_value());
    EXPECT_EQ(*matrix, (cv::Matx22f{1.F, 2.F, 3.F, 4.F}));
}

TEST(YAML_OpenCV, common_type_round_trip) {
    auto root = Node::createMap();
    ASSERT_TRUE(root.set("point", cv::Point2f{1.5F, -2.0F}));
    ASSERT_TRUE(root.set("point3", cv::Point3d{1.0, 2.0, 3.0}));
    ASSERT_TRUE(root.set("size", cv::Size{640, 480}));
    ASSERT_TRUE(root.set("rect", cv::Rect2d{1.0, 2.0, 3.0, 4.0}));
    ASSERT_TRUE(root.set("vec", cv::Vec3f{4.0F, 5.0F, 6.0F}));
    ASSERT_TRUE(root.set("scalar", cv::Scalar{1.0, 2.0, 3.0, 4.0}));
    ASSERT_TRUE(root.set("range", cv::Range{2, 9}));
    ASSERT_TRUE(root.set("points", std::vector<cv::Point>{cv::Point{1, 2}, cv::Point{3, 4}}));

    EXPECT_EQ(root["point"].as<cv::Point2f>(), cv::Point2f(1.5F, -2.0F));
    EXPECT_EQ(root["point3"].as<cv::Point3d>(), cv::Point3d(1.0, 2.0, 3.0));
    EXPECT_EQ(root["size"].as<cv::Size>(), cv::Size(640, 480));
    EXPECT_EQ(root["rect"].as<cv::Rect2d>(), cv::Rect2d(1.0, 2.0, 3.0, 4.0));
    EXPECT_EQ(root["vec"].as<cv::Vec3f>(), cv::Vec3f(4.0F, 5.0F, 6.0F));
    EXPECT_EQ(root["range"].as<cv::Range>(), cv::Range(2, 9));
    EXPECT_EQ(root["points"].as<std::vector<cv::Point>>(),
              (std::vector<cv::Point>{cv::Point{1, 2}, cv::Point{3, 4}}));
}

TEST(YAML_OpenCV, feature_type_round_trip) {
    auto root = Node::createMap();
    const cv::KeyPoint keypoint{cv::Point2f{12.0F, 34.0F}, 7.0F, 45.0F, 0.8F, 2, 9};
    const cv::DMatch match{1, 2, 3, 0.25F};
    ASSERT_TRUE(root.set("keypoint", keypoint));
    ASSERT_TRUE(root.set("match", match));

    const auto decoded_keypoint = root["keypoint"].as<cv::KeyPoint>();
    const auto decoded_match = root["match"].as<cv::DMatch>();
    ASSERT_TRUE(decoded_keypoint.has_value());
    ASSERT_TRUE(decoded_match.has_value());
    EXPECT_EQ(decoded_keypoint->pt, keypoint.pt);
    EXPECT_FLOAT_EQ(decoded_keypoint->response, keypoint.response);
    EXPECT_EQ(decoded_keypoint->class_id, keypoint.class_id);
    EXPECT_EQ(decoded_match->queryIdx, match.queryIdx);
    EXPECT_FLOAT_EQ(decoded_match->distance, match.distance);
}

TEST(YAML_OpenCV, matrix_round_trip) {
    cv::Mat matrix(2, 3, CV_32FC2);
    for (std::size_t i = 0; i < matrix.total() * matrix.channels(); ++i)
        matrix.ptr<float>()[i] = static_cast<float>(i) * 0.5F;

    auto root = Node::createMap();
    ASSERT_TRUE(root.set("matrix", matrix));
    EXPECT_NE(root["matrix"].tag().find("opencv-matrix"), std::string_view::npos);
    EXPECT_EQ(root["matrix"]["dt"].as<std::string>(), "2f");

    const auto decoded = root["matrix"].as<cv::Mat>();
    ASSERT_TRUE(decoded.has_value());
    ASSERT_EQ(decoded->type(), matrix.type());
    ASSERT_EQ(decoded->rows, matrix.rows);
    ASSERT_EQ(decoded->cols, matrix.cols);
    for (std::size_t i = 0; i < matrix.total() * matrix.channels(); ++i)
        EXPECT_FLOAT_EQ(decoded->ptr<float>()[i], matrix.ptr<float>()[i]);
}

TEST(YAML_OpenCV, empty_and_nd_matrix_round_trip) {
    auto root = Node::createMap();
    ASSERT_TRUE(root.set("empty", cv::Mat{}));

    const int sizes[]{2, 3, 4};
    cv::Mat matrix(3, sizes, CV_16SC2);
    for (std::size_t i = 0; i < matrix.total() * matrix.channels(); ++i)
        matrix.ptr<short>()[i] = static_cast<short>(static_cast<int>(i) - 12);
    ASSERT_TRUE(root.set("nd", matrix));
    EXPECT_NE(root["nd"].tag().find("opencv-nd-matrix"), std::string_view::npos);

    const auto empty = root["empty"].as<cv::Mat>();
    const auto decoded = root["nd"].as<cv::Mat>();
    ASSERT_TRUE(empty.has_value());
    ASSERT_TRUE(empty->empty());
    ASSERT_TRUE(decoded.has_value());
    ASSERT_EQ(decoded->dims, 3);
    ASSERT_EQ(decoded->type(), CV_16SC2);
    for (int i = 0; i < decoded->dims; ++i)
        EXPECT_EQ(decoded->size[i], sizes[i]);
    for (std::size_t i = 0; i < matrix.total() * matrix.channels(); ++i)
        EXPECT_EQ(decoded->ptr<short>()[i], matrix.ptr<short>()[i]);
}

TEST(YAML_OpenCV, float16_matrix_round_trip) {
    const cv::Mat source = (cv::Mat_<float>(1, 4) << 0.5F, -1.25F, 3.0F, 10.5F);
    cv::Mat matrix;
    source.convertTo(matrix, CV_16F);

    auto root = Node::createMap();
    ASSERT_TRUE(root.set("matrix", matrix));
    const auto decoded = root["matrix"].as<cv::Mat>();
    ASSERT_TRUE(decoded.has_value());
    ASSERT_EQ(decoded->type(), CV_16F);

    cv::Mat restored;
    decoded->convertTo(restored, CV_32F);
    for (int i = 0; i < restored.cols; ++i)
        EXPECT_NEAR(restored.at<float>(0, i), source.at<float>(0, i), 1e-3F);
}

TEST(YAML_OpenCV, read_filestorage_output) {
    cv::FileStorage storage("", cv::FileStorage::WRITE | cv::FileStorage::MEMORY | cv::FileStorage::FORMAT_YAML);
    storage << "point" << cv::Point2f(1.25F, 2.5F);
    storage << "matrix" << cv::Mat(cv::Matx22d{1.0, 2.0, 3.0, 4.0});
    const std::string source = storage.releaseAndGetString();

    const auto result = parse(source);
    ASSERT_TRUE(result) << result.error.message << '\n' << source;
    EXPECT_EQ((*result)["point"].as<cv::Point2f>(), cv::Point2f(1.25F, 2.5F));
    EXPECT_EQ((*result)["matrix"].as<cv::Matx22d>(), (cv::Matx22d{1.0, 2.0, 3.0, 4.0}));
}

TEST(YAML_OpenCV, filestorage_reads_output) {
    auto root = Node::createMap();
    ASSERT_TRUE(root.set("point", cv::Point2f{1.25F, 2.5F}));
    ASSERT_TRUE(root.set("matrix", cv::Matx22d{1.0, 2.0, 3.0, 4.0}));
    const auto source = dumpOpenCv(root);

    cv::FileStorage storage(source, cv::FileStorage::READ | cv::FileStorage::MEMORY | cv::FileStorage::FORMAT_YAML);
    ASSERT_TRUE(storage.isOpened()) << source;
    cv::Point2f point;
    cv::Matx22d matrix;
    storage["point"] >> point;
    storage["matrix"] >> matrix;
    EXPECT_EQ(point, cv::Point2f(1.25F, 2.5F));
    EXPECT_EQ(matrix, (cv::Matx22d{1.0, 2.0, 3.0, 4.0}));
}

} // namespace rm_test

#endif // HAVE_OPENCV

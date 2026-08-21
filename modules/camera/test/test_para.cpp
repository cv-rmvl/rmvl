/**
 * @file test_para.cpp
 * @brief 相机参数读写测试
 */

#include <cstdio>

#include <gtest/gtest.h>

#include <opencv2/core/persistence.hpp>

#include "rmvlpara/camera/camera.h"

namespace rm_test {

TEST(CameraPara, compatible_with_opencv_filestorage) {
    constexpr auto input_path = "rmvl_camera_para_input.yml";
    constexpr auto output_path = "rmvl_camera_para_output.yml";
    const cv::Matx33f camera_matrix{1200.F, 0.F, 640.F, 0.F, 1190.F, 512.F, 0.F, 0.F, 1.F};
    const cv::Matx<float, 5, 1> distortion{0.1F, -0.2F, 0.01F, 0.02F, 0.3F};
    {
        cv::FileStorage storage(input_path, cv::FileStorage::WRITE | cv::FileStorage::FORMAT_YAML);
        ASSERT_TRUE(storage.isOpened());
        storage << "EULER_0" << 2;
        storage << "EULER_1" << 1;
        storage << "EULER_2" << 0;
        storage << "cameraMatrix" << cv::Mat(camera_matrix);
        storage << "distCoeffs" << cv::Mat(distortion);
    }

    rm::para::CameraParam param;
    ASSERT_TRUE(param.read(input_path));
    EXPECT_EQ(param.EULER_0, 2);
    EXPECT_EQ(param.EULER_1, 1);
    EXPECT_EQ(param.EULER_2, 0);
    EXPECT_EQ(param.cameraMatrix, camera_matrix);
    EXPECT_EQ(param.distCoeffs, distortion);
    ASSERT_TRUE(param.write(output_path));

    cv::FileStorage storage(output_path, cv::FileStorage::READ);
    ASSERT_TRUE(storage.isOpened());
    cv::Matx33f decoded_camera;
    cv::Matx<float, 5, 1> decoded_distortion;
    storage["cameraMatrix"] >> decoded_camera;
    storage["distCoeffs"] >> decoded_distortion;
    EXPECT_EQ(decoded_camera, camera_matrix);
    EXPECT_EQ(decoded_distortion, distortion);

    storage.release();
    std::remove(input_path);
    std::remove(output_path);
}

} // namespace rm_test

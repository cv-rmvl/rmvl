/**
 * @file test_gyro_group.cpp
 * @author RoboMaster Vision Community
 * @brief
 * @version 1.0
 * @date 2023-07-08
 *
 * @copyright Copyright 2023 (c), RoboMaster Vision Community
 *
 */

#include <gtest/gtest.h>

#include <opencv2/imgproc.hpp>

#include "rmvl/group/gyro_group.h"
#include "rmvl/rmath/transform.h"
#include "rmvl/tracker/gyro_tracker.h"

#include "rmvlpara/combo/armor.h"

namespace rm_test
{

class GyroGroupTest : public testing::Test
{
    std::vector<cv::Point3f> world_points;
    cv::Matx33f cameraMatrix;
    cv::Matx51f distCoeffs;

    void SetUp() override
    {
        world_points = {{-67, 27, -7},
                        {-67, -27, 7},
                        {67, -27, 7},
                        {67, 27, -7}};
        cameraMatrix = {1250, 0, 640,
                        0, 1250, 512,
                        0, 0, 1};
        distCoeffs = cv::Matx51f::zeros();
        para::armor_param.SMALL_ARMOR = world_points;
    }

public:
    /**
     * @brief 构建装甲板
     *
     * @param[in] tvec 平移向量
     * @param[in] angle 绕 Y 轴旋转的角度
     * @return combo::ptr
     */
    rm::combo::ptr createArmor(cv::Vec3f tvec, float angle)
    {
        auto rmat = rm::euler2Mat(angle, rm::Y);
        cv::Vec3f rvec;
        cv::Rodrigues(rmat, rvec);
        std::vector<cv::Point2f> imagePoints;
        cv::projectPoints(world_points, rvec, tvec, cameraMatrix, distCoeffs, imagePoints);
        auto p_left = rm::LightBlob::make_feature(imagePoints[1], imagePoints[0], 10);
        auto p_right = rm::LightBlob::make_feature(imagePoints[2], imagePoints[3], 10);
        return rm::Armor::make_combo(p_left, p_right, rm::GyroData(), cv::getTickCount(), rm::ArmorSizeType::SMALL);
    }
};

TEST_F(GyroGroupTest, build_4_from_1_armor)
{
    auto p_armor = createArmor(cv::Vec3f(-50, -50, 1000), -20);
    auto p_group = rm::GyroGroup::make_group({p_armor}, 4);
    const auto &p_trackers = p_group->data();
    EXPECT_EQ(p_trackers.size(), 4);
}

} // namespace rm_test
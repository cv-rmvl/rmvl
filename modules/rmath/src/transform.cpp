/**
 * @file transform.cpp
 * @author RoboMaster Vision Community
 * @brief 额外数据函数库
 * @version 1.0
 * @date 2021-06-14
 *
 * @copyright Copyright 2021 (c), RoboMaster Vision Community
 *
 */

#include <opencv2/calib3d.hpp>

#include "rmvl/rmath/transform.h"

cv::Point2f rm::calculateRelativeAngle(const cv::Matx33f &cameraMatrix, cv::Point2f center)
{
    cv::Matx31f tf_point;
    cv::Matx33f cameraMatrix_inverse = cameraMatrix.inv();
    tf_point(0) = center.x;
    tf_point(1) = center.y;
    tf_point(2) = 1;
    // 得到tan角矩阵
    cv::Matx31f tf_result = cameraMatrix_inverse * tf_point;
    // 从像素坐标系转换成相机坐标系角度
    return {rad2deg(atanf(tf_result(0))),
            rad2deg(atanf(tf_result(1)))};
}

cv::Point2f rm::calculateRelativeCenter(const cv::Matx33f &cameraMatrix, cv::Point2f angle)
{
    float yaw = tanf(deg2rad(angle.x));
    float pitch = tanf(deg2rad(angle.y));
    cv::Matx31f center_vector;
    center_vector(0) = yaw;
    center_vector(1) = pitch;
    center_vector(2) = 1;
    // 得到tan角矩阵
    cv::Matx31f result = cameraMatrix * center_vector;
    return {result(0), result(1)};
}

cv::Vec2f rm::cameraConvertToPixel(const cv::Matx33f &cameraMatrix, const cv::Matx51f &distCoeffs, const cv::Vec3f &center3d)
{
    std::vector<cv::Point3f> world_center3ds(1);
    cv::Vec3f center_tvec = center3d;
    cv::Vec3f center_rvec = {0, 0, 1}; // (1, 0, 0) 或 (0, 1, 0) 均可
    std::vector<cv::Point2f> center2ds;
    cv::projectPoints(world_center3ds, center_rvec, center_tvec, cameraMatrix, distCoeffs, center2ds);
    return center2ds.front();
}

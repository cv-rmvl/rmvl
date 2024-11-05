/**
 * @file Armor_cal.cpp
 * @author RoboMaster Vision Community
 * @brief 装甲板类中有关计算内容的成员方法实现
 * @version 1.0
 * @date 2021-08-17
 *
 * @copyright Copyright 2021 (c), RoboMaster Vision Community
 *
 */

#include <opencv2/imgproc.hpp>

#include "rmvl/combo/armor.h"
#include "rmvl/algorithm/transform.hpp"

#include "rmvlpara/combo/armor.h"

namespace rm
{

ArmorSizeType Armor::matchArmorType()
{
    if (_svm != nullptr)
    {
        cv::Matx15f test_sample = {_combo_ratio,   // 装甲板长宽比
                                   _width_ratio,   // 灯条宽度比
                                   _length_ratio}; // 灯条长度比
        auto res = _svm->predict(test_sample);
        return res == 1 ? ArmorSizeType::SMALL : ArmorSizeType::BIG;
    }
    // 装甲板长宽比例符合
    if (_combo_ratio < para::armor_param.MAX_SMALL_COMBO_RATIO)
        return ArmorSizeType::SMALL;
    else if (_combo_ratio > para::armor_param.MIN_BIG_COMBO_RATIO)
        return ArmorSizeType::BIG;
    // 错位角判断
    if (abs(_corner_angle) < para::armor_param.MAX_SMALL_CORNER_ANGLE)
        return ArmorSizeType::SMALL;
    else if (abs(_corner_angle) > para::armor_param.MIN_BIG_CORNER_ANGLE)
        return ArmorSizeType::BIG;
    // 宽度比例判断
    if (_width_ratio < para::armor_param.BIG_SMALL_WIDTH_RATIO)
        return ArmorSizeType::SMALL; // 装甲板长宽比例较小
    else
        return ArmorSizeType::BIG;
}

void Armor::gyroConvertToCamera(const cv::Matx33f &gyro_rmat, const cv::Vec3f &gyro_tvec,
                                const GyroData &gyro_data, cv::Matx33f &cam_rmat, cv::Vec3f &cam_tvec)
{
    auto rot = euler2Mat(deg2rad(gyro_data.rotation.yaw), Y) *
               euler2Mat(deg2rad(-gyro_data.rotation.pitch), X);
    rot = rot.inv();
    cam_rmat = rot * gyro_rmat;
    cam_tvec = rot * gyro_tvec;
}

void Armor::cameraConvertToGyro(const cv::Matx33f &cam_rmat, const cv::Vec3f &cam_tvec,
                                const GyroData &gyro_data, cv::Matx33f &gyro_rmat, cv::Vec3f &gyro_tvec)
{
    auto rot = euler2Mat(deg2rad(gyro_data.rotation.yaw), Y) *   // yaw 方向与 Y 轴欧拉角方向相同
               euler2Mat(deg2rad(-gyro_data.rotation.pitch), X); // pitch 方向与 X 轴欧拉角方向相反
    gyro_rmat = rot * cam_rmat;
    gyro_tvec = rot * cam_tvec;
}

bool Armor::isContainBlob(LightBlob::ptr blob, Armor::ptr armor)
{
    // Get the four corner points of the area where the armor is located: (a, b, c, d)
    const auto &points = armor->corners();
    cv::Vec2f ab = {points[1].x - points[0].x, points[1].y - points[0].y};
    cv::Vec2f cd = {points[3].x - points[2].x, points[3].y - points[2].y};
    // Define the real corner points of the armor
    cv::Vec2f A = {armor->at(0)->center().x - ab(0) / 2, armor->at(0)->center().y - ab(1) / 2};
    cv::Vec2f B = {armor->at(0)->center().x + ab(0) / 2, armor->at(0)->center().y + ab(1) / 2};
    cv::Vec2f C = {armor->at(1)->center().x - cd(0) / 2, armor->at(1)->center().y - cd(1) / 2};
    cv::Vec2f D = {armor->at(1)->center().x + cd(0) / 2, armor->at(1)->center().y + cd(1) / 2};
    // Define the real corner vectors of the armor
    cv::Vec2f AB = B - A;
    cv::Vec2f BC = C - B;
    cv::Vec2f CD = D - C;
    cv::Vec2f DA = A - D;
    // Repeated ?
    if (blob == armor->at(0) || blob == armor->at(1))
        return false;
    // Calculate
    cv::Vec2f P = blob->center();
    cv::Vec2f AP = P - A;
    cv::Vec2f BP = P - B;
    cv::Vec2f CP = P - C;
    cv::Vec2f DP = P - D;
    if (cross2D(AB, AP) > 0.f && cross2D(BC, BP) > 0.f &&
        cross2D(CD, CP) > 0.f && cross2D(DA, DP) > 0.f)
        return true;
    else
        return false;
}

cv::Mat Armor::getNumberROI(cv::Mat src, const_ptr p_armor)
{
    // 计算装甲板之间距离,该距离为获得的roi的边长
    double h_dis = p_armor->height() * para::armor_param.ROI_HEIGHT_RATIO;
    double w_dis = p_armor->width() * para::armor_param.ROI_WIDTH_RATIO;

    // 图像中的 4 个角点
    std::vector<cv::Point2f> corners = p_armor->corners();
    if (corners.size() != 4)
        return cv::Mat();

    // 从左灯条上顶点指向下顶点的单位向量
    cv::Point2f e_ver_left = (corners[0] - corners[1]) / getDistance(corners[0], corners[1]);
    // 从右灯条上顶点指向下顶点的单位向量
    cv::Point2f e_ver_right = (corners[3] - corners[2]) / getDistance(corners[3], corners[2]);
    // 从上顶点右灯条指向左灯条的单位向量
    cv::Point2f e_hor_top = (corners[1] - corners[2]) / getDistance(corners[1], corners[2]);
    // 从下顶点右灯条指向左灯条的单位向量
    cv::Point2f e_hor_bottom = (corners[0] - corners[3]) / getDistance(corners[1], corners[2]);

    cv::Point2f src_corners[4];
    src_corners[0] = cv::Point2f(p_armor->center() - e_ver_left * h_dis / 2 + e_hor_top * w_dis / 2);     // 左上
    src_corners[1] = cv::Point2f(p_armor->center() + e_ver_left * h_dis / 2 + e_hor_bottom * w_dis / 2);  // 左下
    src_corners[2] = cv::Point2f(p_armor->center() - e_ver_right * h_dis / 2 - e_hor_top * w_dis / 2);    // 右上
    src_corners[3] = cv::Point2f(p_armor->center() + e_ver_right * h_dis / 2 - e_hor_bottom * w_dis / 2); // 右下

    // 边界修正
    src_corners[0].y = src_corners[0].y < 0 ? 0 : src_corners[0].y;
    src_corners[1].y = src_corners[1].y > src.rows ? src.rows : src_corners[1].y;
    src_corners[2].y = src_corners[2].y < 0 ? 0 : src_corners[2].y;
    src_corners[3].y = src_corners[3].y > src.rows ? src.rows : src_corners[3].y;

    src_corners[0].x = src_corners[0].x < 0 ? 0 : src_corners[0].x;
    src_corners[1].x = src_corners[1].x > src.cols ? src.cols : src_corners[1].x;
    src_corners[2].x = src_corners[2].x < 0 ? 0 : src_corners[2].x;
    src_corners[3].x = src_corners[3].x > src.cols ? src.cols : src_corners[3].x;
    // 目标角点
    cv::Point2f goal_corners[4] = {cv::Point2f(0, 0),          // 左上
                                   cv::Point2f(0, h_dis),      // 左下
                                   cv::Point2f(w_dis, 0),      // 右上
                                   cv::Point2f(w_dis, h_dis)}; // 右下

    cv::Mat roi;
    // 进行透视变换
    cv::warpPerspective(src, roi, cv::getPerspectiveTransform(src_corners, goal_corners), cv::Size(w_dis, h_dis));
    cv::cvtColor(roi, roi, cv::COLOR_BGR2GRAY);
    cv::resize(roi, roi, cv::Size(para::armor_param.ROI_SIZE, para::armor_param.ROI_SIZE));
    cv::threshold(roi, roi, 0, 255, cv::THRESH_OTSU);
    return roi;
}

} // namespace rm

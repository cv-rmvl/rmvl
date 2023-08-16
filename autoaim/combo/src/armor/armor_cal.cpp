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

#include <opencv2/calib3d.hpp>
#include <opencv2/imgproc.hpp>

#include "rmvl/combo/armor.h"
#include "rmvl/rmath/transform.h"

#include "rmvlpara/combo/armor.h"

using namespace rm;
using namespace para;
using namespace std;
using namespace cv;

ArmorSizeType Armor::matchArmorType()
{
    if (_svm != nullptr)
    {
        Matx15f test_sample = {_combo_ratio,   // 装甲板长宽比
                               _width_ratio,   // 灯条宽度比
                               _length_ratio}; // 灯条长度比
        auto res = _svm->predict(test_sample);
        return res == 1 ? ArmorSizeType::SMALL : ArmorSizeType::BIG;
    }
    // 装甲板长宽比例符合
    if (_combo_ratio < armor_param.MAX_SMALL_COMBO_RATIO)
        return ArmorSizeType::SMALL;
    else if (_combo_ratio > armor_param.MIN_BIG_COMBO_RATIO)
        return ArmorSizeType::BIG;
    // 错位角判断
    if (abs(_corner_angle) < armor_param.MAX_SMALL_CORNER_ANGLE)
        return ArmorSizeType::SMALL;
    else if (abs(_corner_angle) > armor_param.MIN_BIG_CORNER_ANGLE)
        return ArmorSizeType::BIG;
    // 宽度比例判断
    if (_width_ratio < armor_param.BIG_SMALL_WIDTH_RATIO)
        return ArmorSizeType::SMALL; // 装甲板长宽比例较小
    else
        return ArmorSizeType::BIG;
}

ResultPnP<float> Armor::calculatePnPData(const Matx33f &cameraMatrix, const Matx51f &distcoeff, const GyroData &gyro_data)
{
    Vec3f rvec;                   // 旋转向量
    Vec3f tvec;                   // 平移向量
    ResultPnP<float> pnp_message; // 存储 PNP 信息
    // DEBUG_INFO_("Armor: %s, armor_type: %s", _func_, _type.ArmorSizeTypeID == ArmorSizeType::SMALL ? "SMALL" : "BIG");
    _type.ArmorSizeTypeID == ArmorSizeType::SMALL
        ? solvePnP(armor_param.SMALL_ARMOR, _corners, cameraMatrix, distcoeff, rvec, tvec, false, SOLVEPNP_IPPE)
        : solvePnP(armor_param.BIG_ARMOR, _corners, cameraMatrix, distcoeff, rvec, tvec, false, SOLVEPNP_IPPE);
    // 变换为陀螺仪坐标系下
    Matx33f rmat;
    Rodrigues(rvec, rmat);
    Matx33f gyro_rmat;
    Vec3f gyro_tvec;
    Armor::cameraConvertToGyro(rmat, tvec, gyro_data, gyro_rmat, gyro_tvec);
    pnp_message.R(gyro_rmat);
    pnp_message.tvec(gyro_tvec);

    return pnp_message;
}

void Armor::gyroConvertToCamera(const Matx33f &gyro_rmat, const Vec3f &gyro_tvec,
                                const GyroData &gyro_data, Matx33f &cam_rmat, Vec3f &cam_tvec)
{
    auto rot = euler2Mat(deg2rad(gyro_data.rotation.yaw), Y) *
               euler2Mat(deg2rad(-gyro_data.rotation.pitch), X);
    rot = rot.inv();
    cam_rmat = rot * gyro_rmat;
    cam_tvec = rot * gyro_tvec;
}

void Armor::cameraConvertToGyro(const Matx33f &cam_rmat, const Vec3f &cam_tvec,
                                const GyroData &gyro_data, Matx33f &gyro_rmat, Vec3f &gyro_tvec)
{
    auto rot = euler2Mat(deg2rad(gyro_data.rotation.yaw), Y) *   // yaw 方向与 Y 轴欧拉角方向相同
               euler2Mat(deg2rad(-gyro_data.rotation.pitch), X); // pitch 方向与 X 轴欧拉角方向相反
    gyro_rmat = rot * cam_rmat;
    gyro_tvec = rot * cam_tvec;
}

bool Armor::isContainBlob(light_blob_ptr blob, std::shared_ptr<Armor> armor)
{
    // Get the four corner points of the area where the armor is located: (a, b, c, d)
    const auto &points = armor->getCorners();
    Vec2f ab = {points[1].x - points[0].x, points[1].y - points[0].y};
    Vec2f cd = {points[3].x - points[2].x, points[3].y - points[2].y};
    // Define the real corner points of the armor
    Vec2f A = {armor->at(0)->getCenter().x - ab(0) / 2, armor->at(0)->getCenter().y - ab(1) / 2};
    Vec2f B = {armor->at(0)->getCenter().x + ab(0) / 2, armor->at(0)->getCenter().y + ab(1) / 2};
    Vec2f C = {armor->at(1)->getCenter().x - cd(0) / 2, armor->at(1)->getCenter().y - cd(1) / 2};
    Vec2f D = {armor->at(1)->getCenter().x + cd(0) / 2, armor->at(1)->getCenter().y + cd(1) / 2};
    // Define the real corner vectors of the armor
    Vec2f AB = B - A;
    Vec2f BC = C - B;
    Vec2f CD = D - C;
    Vec2f DA = A - D;
    // Repeated ?
    if (blob == armor->at(0) || blob == armor->at(1))
        return false;
    // Calculate
    Vec2f P = blob->getCenter();
    Vec2f AP = P - A;
    Vec2f BP = P - B;
    Vec2f CP = P - C;
    Vec2f DP = P - D;
    if (cross2D(AB, AP) > 0.f && cross2D(BC, BP) > 0.f &&
        cross2D(CD, CP) > 0.f && cross2D(DA, DP) > 0.f)
        return true;
    else
        return false;
}

Mat Armor::getNumberROI(Mat src, combo_ptr p_combo)
{
    // 计算装甲板之间距离,该距离为获得的roi的边长
    double h_dis = p_combo->getHeight() * armor_param.ROI_HEIGHT_RATIO;
    double w_dis = p_combo->getWidth() * armor_param.ROI_WIDTH_RATIO;

    // 图像中的 4 个角点
    vector<Point2f> corners = p_combo->getCorners();

    // 从左灯条上顶点指向下顶点的单位向量
    Point2f e_ver_left = (corners[0] - corners[1]) / getDistance(corners[0], corners[1]);
    // 从右灯条上顶点指向下顶点的单位向量
    Point2f e_ver_right = (corners[3] - corners[2]) / getDistance(corners[3], corners[2]);
    // 从上顶点右灯条指向左灯条的单位向量
    Point2f e_hor_top = (corners[1] - corners[2]) / getDistance(corners[1], corners[2]);
    // 从下顶点右灯条指向左灯条的单位向量
    Point2f e_hor_bottom = (corners[0] - corners[3]) / getDistance(corners[1], corners[2]);

    Point2f src_corners[4];
    src_corners[0] = Point2f(p_combo->getCenter() - e_ver_left * h_dis / 2 + e_hor_top * w_dis / 2);     // 左上
    src_corners[1] = Point2f(p_combo->getCenter() + e_ver_left * h_dis / 2 + e_hor_bottom * w_dis / 2);  // 左下
    src_corners[2] = Point2f(p_combo->getCenter() - e_ver_right * h_dis / 2 - e_hor_top * w_dis / 2);    // 右上
    src_corners[3] = Point2f(p_combo->getCenter() + e_ver_right * h_dis / 2 - e_hor_bottom * w_dis / 2); // 右下

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
    Point2f goal_corners[4] = {Point2f(0, 0),          // 左上
                               Point2f(0, h_dis),      // 左下
                               Point2f(w_dis, 0),      // 右上
                               Point2f(w_dis, h_dis)}; // 右下

    Mat roi;
    // 进行透视变换
    warpPerspective(src, roi, getPerspectiveTransform(src_corners, goal_corners), Size(w_dis, h_dis));
    cvtColor(roi, roi, COLOR_BGR2GRAY);
    resize(roi, roi, Size(armor_param.ROI_SIZE, armor_param.ROI_SIZE));
    threshold(roi, roi, 0, 255, THRESH_OTSU);
    cvtColor(roi, roi, COLOR_GRAY2BGR);
    return roi;
}

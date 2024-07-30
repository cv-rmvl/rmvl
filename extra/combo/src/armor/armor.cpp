/**
 * @file Armor.cpp
 * @author RoboMaster Vision Community
 * @brief 装甲板类
 * @version 1.0
 * @date 2021-08-13
 *
 * @copyright Copyright 2021 (c), RoboMaster Vision Community
 *
 */

#include <opencv2/calib3d.hpp>

#include "rmvl/combo/armor.h"
#include "rmvl/core/transform.hpp"

#include "rmvlpara/camera/camera.h"
#include "rmvlpara/combo/armor.h"

namespace rm
{

std::shared_ptr<Armor> Armor::make_combo(LightBlob::ptr p_left, LightBlob::ptr p_right, const GyroData &gyro_data,
                                         double tick, ArmorSizeType armor_size_type)
{
    // 判空
    if (p_left == nullptr || p_right == nullptr)
        return nullptr;

    // ------------------------【左右灯条长度比值】------------------------
    float length_l = p_left->getHeight();
    float length_r = p_right->getHeight();
    // 获取长度偏差
    float length_ratio = (length_l / length_r >= 1) ? length_l / length_r : length_r / length_l;

    // ------------------------【左右灯条宽度比值】------------------------
    float width_l = p_left->getWidth();
    float width_r = p_right->getWidth();
    // 获取宽度偏差
    float width_ratio = (width_l / width_r >= 1) ? width_l / width_r : width_r / width_l;

    // -----------------------【装甲板倾角是否符合】-----------------------
    float top_angle = getHAngle(p_left->getTopPoint(), p_right->getTopPoint(), DEG);
    float bottom_angle = getHAngle(p_left->getBottomPoint(), p_right->getBottomPoint(), DEG);
    float corner_angle = (top_angle + bottom_angle) / 2.f;
    corner_angle = abs((p_left->getAngle() + p_right->getAngle()) / 2.f + corner_angle);
    // 获取角度偏差
    float angle_diff = abs(p_left->getAngle() - p_right->getAngle());

    // -------------------------【装甲板长宽比值】-------------------------
    float combo_height = (length_l + length_r) / 2.f;
    // 灯条间距 + 一个即为装甲板的宽
    float combo_width = getDistance(p_left->getCenter(), p_right->getCenter()) + (width_l + width_r / 2.f);
    float combo_ratio = combo_width / combo_height;

    // --------------------------【更新匹配误差】--------------------------
    float match_error = para::armor_param.ERROR_LENGTH_SCALE_RATIO * length_ratio +
                        para::armor_param.ERROR_WIDTH_SCALE_RATIO * width_ratio +
                        para::armor_param.ERROR_TILT_ANGLE_RATIO * corner_angle +
                        para::armor_param.ERROR_ANGLE_SCALE_RATIO * angle_diff;
    if (armor_size_type == ArmorSizeType::UNKNOWN)
    {
        // 判断是否构造
        if (length_ratio >= para::armor_param.MAX_LENGTH_RATIO) // 长度偏差判断
            return nullptr;
        if (width_ratio >= para::armor_param.MAX_WIDTH_RATIO) // 宽度偏差判断
            return nullptr;
        if (corner_angle >= para::armor_param.MAX_CORNER_ANGLE) // 装甲板错位角判断
            return nullptr;
        if (angle_diff >= para::armor_param.MAX_DELTA_ANGLE) // 角度差值判断
            return nullptr;
        if (combo_ratio <= para::armor_param.MIN_COMBO_RATIO ||
            combo_ratio >= para::armor_param.MAX_COMBO_RATIO) // 装甲板长宽比值判断
            return nullptr;
    }

    return std::make_shared<Armor>(p_left, p_right, gyro_data, tick,
                                   width_ratio, length_ratio, corner_angle, match_error,
                                   combo_height, combo_width, combo_ratio, armor_size_type);
}

/**
 * @brief 获取装甲板的位姿
 *
 * @param[in] cam_matrix 相机内参，用于解算相机外参
 * @param[in] distCoeffs 相机畸变参数，用于解算相机外参
 * @param[in] gyro_data 陀螺仪数据
 * @param[in] type 装甲板类型
 * @param[in] corners 装甲板角点
 * @return CameraExtrinsics - 相机外参
 */
static CameraExtrinsics calculateExtrinsic(const cv::Matx33f &cameraMatrix, const cv::Matx51f &distCoeffs, const GyroData &gyro_data,
                                           ArmorSizeType type, const std::vector<cv::Point2f> &corners)
{
    cv::Vec3f rvec;             // 旋转向量
    cv::Vec3f tvec;             // 平移向量
    CameraExtrinsics extrinsic; // 存储相机外参
    type == ArmorSizeType::SMALL
        ? cv::solvePnP(para::armor_param.SMALL_ARMOR, corners, cameraMatrix, distCoeffs, rvec, tvec, false, cv::SOLVEPNP_IPPE)
        : cv::solvePnP(para::armor_param.BIG_ARMOR, corners, cameraMatrix, distCoeffs, rvec, tvec, false, cv::SOLVEPNP_IPPE);
    // 变换为陀螺仪坐标系下
    cv::Matx33f rmat;
    cv::Rodrigues(rvec, rmat);
    cv::Matx33f gyro_rmat;
    cv::Vec3f gyro_tvec;
    Armor::cameraConvertToGyro(rmat, tvec, gyro_data, gyro_rmat, gyro_tvec);
    extrinsic.R(gyro_rmat);
    extrinsic.tvec(gyro_tvec);

    return extrinsic;
}

Armor::Armor(LightBlob::ptr p_left, LightBlob::ptr p_right, const GyroData &gyro_data, double tick,
             float width_r, float length_r, float corner_angle, float match_error,
             float combo_h, float combo_w, float combo_r, ArmorSizeType armor_size_type)
    : _width_ratio(width_r), _length_ratio(length_r), _corner_angle(corner_angle), _match_error(match_error)
{
    _height = combo_h;
    _width = combo_w;
    _combo_ratio = combo_r;
    // 获取装甲板中心
    _center = (p_left->getCenter() + p_right->getCenter()) / 2.f;
    // 获取相对角度
    _relative_angle = calculateRelativeAngle(para::camera_param.cameraMatrix, _center);
    // 获取当前装甲板对应的陀螺仪位置信息
    _gyro_data = gyro_data;
    // 装甲板角度
    _angle = (p_left->getAngle() + p_right->getAngle()) / 2.f;
    // 匹配大小装甲板
    if (armor_size_type == ArmorSizeType::UNKNOWN)
        _type.ArmorSizeTypeID = matchArmorType();
    else
        _type.ArmorSizeTypeID = armor_size_type;
    // 更新角点
    _corners = {p_left->getBottomPoint(),   // 左下
                p_left->getTopPoint(),      // 左上
                p_right->getTopPoint(),     // 右上
                p_right->getBottomPoint()}; // 右下
    // 计算相机外参
    _extrinsic = calculateExtrinsic(para::camera_param.cameraMatrix, para::camera_param.distCoeffs,
                                    gyro_data, _type.ArmorSizeTypeID, _corners);
    const auto &rmat = _extrinsic.R();
    _pose = cv::normalize(cv::Vec2f(rmat(0, 2), rmat(2, 2)));
    // 设置组合体的特征容器
    _features = {p_left, p_right};
    _tick = tick;
}

combo::ptr Armor::clone(double tick)
{
    auto retval = std::make_shared<Armor>(*this);
    // 更新内部所有特征
    for (auto &p_feature : retval->_features)
        p_feature = p_feature->clone();
    // 更新时间戳
    retval->_tick = tick;
    return retval;
}

} // namespace rm

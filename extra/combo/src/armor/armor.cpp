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

#include "rmvl/algorithm/transform.hpp"
#include "rmvl/combo/armor.h"

#include "rmvlpara/camera/camera.h"
#include "rmvlpara/combo/armor.h"

namespace rm {

const char *to_string(ArmorSizeType armor_size) {
    switch (armor_size) {
    case ArmorSizeType::SMALL:
        return "small";
    case ArmorSizeType::BIG:
        return "big";
    default:
        return "unknown";
    }
}

ArmorSizeType to_armor_size_type(const StateType &tp) {
    if (tp.index() == 0)
        return ArmorSizeType::UNKNOWN;

    const auto &str = std::get<std::string>(tp);
    if (str == "small")
        return ArmorSizeType::SMALL;
    else if (str == "big")
        return ArmorSizeType::BIG;
    return ArmorSizeType::UNKNOWN;
}

const char *to_string(RobotType robot) {
    switch (robot) {
    case RobotType::HERO:
        return "hero";
    case RobotType::ENGINEER:
        return "engineer";
    case RobotType::INFANTRY_3:
        return "infantry_3";
    case RobotType::INFANTRY_4:
        return "infantry_4";
    case RobotType::INFANTRY_5:
        return "infantry_5";
    case RobotType::OUTPOST:
        return "outpost";
    case RobotType::BASE:
        return "base";
    case RobotType::SENTRY:
        return "sentry";
    default:
        return "unknown";
    }
}

RobotType to_robot_type(const StateType &type) {
    if (type.index() == 0)
        return RobotType::UNKNOWN;

    const auto &str = std::get<std::string>(type);
    if (str == "hero")
        return RobotType::HERO;
    else if (str == "engineer")
        return RobotType::ENGINEER;
    else if (str == "infantry_3")
        return RobotType::INFANTRY_3;
    else if (str == "infantry_4")
        return RobotType::INFANTRY_4;
    else if (str == "infantry_5")
        return RobotType::INFANTRY_5;
    else if (str == "outpost")
        return RobotType::OUTPOST;
    else if (str == "base")
        return RobotType::BASE;
    else if (str == "sentry")
        return RobotType::SENTRY;
    return RobotType::UNKNOWN;
}

/**
 * @brief 获取装甲板的位姿
 *
 * @param[in] cam_matrix 相机内参，用于解算相机外参
 * @param[in] distCoeffs 相机畸变参数，用于解算相机外参
 * @param[in] imu_data IMU 数据
 * @param[in] type 装甲板类型
 * @param[in] corners 装甲板角点
 * @return CameraExtrinsics - 相机外参
 */
static CameraExtrinsics calculateExtrinsic(const cv::Matx33f &cameraMatrix, const cv::Matx51f &distCoeffs, const ImuData &imu_data,
                                           ArmorSizeType type, const std::vector<cv::Point2f> &corners) {
    cv::Vec3f rvec;             // 旋转向量
    cv::Vec3f tvec;             // 平移向量
    CameraExtrinsics extrinsic; // 存储相机外参
    type == ArmorSizeType::SMALL
        ? cv::solvePnP(para::armor_param.SMALL_ARMOR, corners, cameraMatrix, distCoeffs, rvec, tvec, false, cv::SOLVEPNP_IPPE)
        : cv::solvePnP(para::armor_param.BIG_ARMOR, corners, cameraMatrix, distCoeffs, rvec, tvec, false, cv::SOLVEPNP_IPPE);
    // 变换为 IMU 坐标系下
    cv::Matx33f rmat;
    cv::Rodrigues(rvec, rmat);
    cv::Matx33f gyro_rmat;
    cv::Vec3f gyro_tvec;
    Armor::cameraConvertToImu(rmat, tvec, imu_data, gyro_rmat, gyro_tvec);
    extrinsic.R(gyro_rmat);
    extrinsic.tvec(gyro_tvec);

    return extrinsic;
}

std::shared_ptr<Armor> Armor::make_combo(LightBlob::ptr p_left, LightBlob::ptr p_right, const ImuData &imu_data, int64_t tick, ArmorSizeType armor_size_type) {
    // 判空
    if (p_left == nullptr || p_right == nullptr)
        return nullptr;

    // ------------------------【左右灯条长度比值】------------------------
    float length_l = p_left->height();
    float length_r = p_right->height();
    // 获取长度偏差
    float length_ratio = (length_l / length_r >= 1) ? length_l / length_r : length_r / length_l;

    // ------------------------【左右灯条宽度比值】------------------------
    float width_l = p_left->width();
    float width_r = p_right->width();
    // 获取宽度偏差
    float width_ratio = (width_l / width_r >= 1) ? width_l / width_r : width_r / width_l;

    // -----------------------【装甲板倾角是否符合】-----------------------
    float top_angle = getHAngle(p_left->getTopPoint(), p_right->getTopPoint(), DEG);
    float bottom_angle = getHAngle(p_left->getBottomPoint(), p_right->getBottomPoint(), DEG);
    float corner_angle = (top_angle + bottom_angle) / 2.f;
    corner_angle = abs((p_left->angle() + p_right->angle()) / 2.f + corner_angle);
    // 获取角度偏差
    float angle_diff = abs(p_left->angle() - p_right->angle());

    // -------------------------【装甲板长宽比值】-------------------------
    float combo_height = (length_l + length_r) / 2.f;
    // 灯条间距 + 一个即为装甲板的宽
    float combo_width = getDistance(p_left->center(), p_right->center()) + (width_l + width_r / 2.f);
    float combo_ratio = combo_width / combo_height;

    // --------------------------【更新匹配误差】--------------------------
    float match_error = para::armor_param.ERROR_LENGTH_SCALE_RATIO * length_ratio +
                        para::armor_param.ERROR_WIDTH_SCALE_RATIO * width_ratio +
                        para::armor_param.ERROR_TILT_ANGLE_RATIO * corner_angle +
                        para::armor_param.ERROR_ANGLE_SCALE_RATIO * angle_diff;
    if (armor_size_type == ArmorSizeType::UNKNOWN) {
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

    auto retval = std::make_shared<Armor>(width_r, length_r, corner_angle, match_error);

    retval->_height = combo_height;
    retval->_width = combo_width;
    retval->_combo_ratio = combo_ratio;
    // 获取装甲板中心
    retval->_center = (p_left->center() + p_right->center()) / 2.f;
    // 获取相对角度
    retval->_relative_angle = calculateRelativeAngle(para::camera_param.cameraMatrix, retval->_center);
    // 获取当前装甲板对应的 IMU 位置信息
    retval->_imu_data = imu_data;
    // 装甲板角度
    retval->_angle = (p_left->angle() + p_right->angle()) / 2.f;
    // 匹配大小装甲板
    ArmorSizeType armor_size{};
    if (armor_size_type == ArmorSizeType::UNKNOWN) {
        if (_svm != nullptr) {
            cv::Matx15f test_sample = {combo_ratio,   // 装甲板长宽比
                                       width_ratio,   // 灯条宽度比
                                       length_ratio}; // 灯条长度比
            auto res = _svm->predict(test_sample);
            armor_size = (res == 1) ? ArmorSizeType::SMALL : ArmorSizeType::BIG;
        }
        // 装甲板长宽比例符合
        if (combo_ratio < para::armor_param.MAX_SMALL_COMBO_RATIO)
            armor_size = ArmorSizeType::SMALL;
        else if (combo_ratio > para::armor_param.MIN_BIG_COMBO_RATIO)
            armor_size = ArmorSizeType::BIG;
        // 错位角判断
        if (abs(corner_angle) < para::armor_param.MAX_SMALL_CORNER_ANGLE)
            armor_size = ArmorSizeType::SMALL;
        else if (abs(corner_angle) > para::armor_param.MIN_BIG_CORNER_ANGLE)
            armor_size = ArmorSizeType::BIG;
        // 宽度比例判断
        if (width_ratio < para::armor_param.BIG_SMALL_WIDTH_RATIO)
            armor_size = ArmorSizeType::SMALL; // 装甲板长宽比例较小
        else
            armor_size = ArmorSizeType::BIG;
    } else
        armor_size = armor_size_type;
    retval->_state["armor_size"] = to_string(armor_size);
    // 更新角点
    retval->_corners = {p_left->getBottomPoint(),   // 左下
                        p_left->getTopPoint(),      // 左上
                        p_right->getTopPoint(),     // 右上
                        p_right->getBottomPoint()}; // 右下
    // 计算相机外参
    retval->_extrinsic = calculateExtrinsic(para::camera_param.cameraMatrix, para::camera_param.distCoeffs, imu_data, armor_size, retval->_corners);
    const auto &rmat = retval->_extrinsic.R();
    retval->_pose = cv::normalize(cv::Vec2f(rmat(0, 2), rmat(2, 2)));
    // 设置组合体的特征容器
    retval->_features = {p_left, p_right};
    retval->_tick = tick;

    return retval;
}

void Armor::setType(RobotType stat) { _state["robot"] = to_string(stat); }

Armor::Armor(float width_r, float length_r, float corner_angle, float match_error)
    : _width_ratio(width_r), _length_ratio(length_r), _corner_angle(corner_angle), _match_error(match_error) {}

combo::ptr Armor::clone(int64_t tick) {
    auto retval = std::make_shared<Armor>(*this);
    // 更新内部所有特征
    for (auto &p_feature : retval->_features)
        p_feature = p_feature->clone();
    // 更新时间戳
    retval->_tick = tick;
    return retval;
}

} // namespace rm

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

#include "rmvl/combo/armor.h"
#include "rmvl/rmath/transform.h"

#include "rmvlpara/camera.hpp"
#include "rmvlpara/combo/armor.h"

using namespace rm;
using namespace para;
using namespace std;
using namespace cv;

shared_ptr<Armor> Armor::make_combo(LightBlob::ptr p_left, LightBlob::ptr p_right, const GyroData &gyro_data,
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
    float match_error = armor_param.ERROR_LENGTH_SCALE_RATIO * length_ratio +
                        armor_param.ERROR_WIDTH_SCALE_RATIO * width_ratio +
                        armor_param.ERROR_TILT_ANGLE_RATIO * corner_angle +
                        armor_param.ERROR_ANGLE_SCALE_RATIO * angle_diff;
    if (armor_size_type == ArmorSizeType::UNKNOWN)
    {
        // 判断是否构造
        if (length_ratio >= armor_param.MAX_LENGTH_RATIO) // 长度偏差判断
            return nullptr;
        if (width_ratio >= armor_param.MAX_WIDTH_RATIO) // 宽度偏差判断
            return nullptr;
        if (corner_angle >= armor_param.MAX_CORNER_ANGLE) // 装甲板错位角判断
            return nullptr;
        if (angle_diff >= armor_param.MAX_DELTA_ANGLE) // 角度差值判断
            return nullptr;
        if (combo_ratio <= armor_param.MIN_COMBO_RATIO ||
            combo_ratio >= armor_param.MAX_COMBO_RATIO) // 装甲板长宽比值判断
            return nullptr;
    }

    auto ret = make_shared<Armor>(p_left, p_right, gyro_data, tick,
                                  width_ratio, length_ratio, corner_angle, match_error,
                                  combo_height, combo_width, combo_ratio, armor_size_type);
    return ret;
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
    _relative_angle = calculateRelativeAngle(camera_param.cameraMatrix, _center);
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
    _extrinsic = calculateExtrinsic(camera_param.cameraMatrix, camera_param.distCoeff, gyro_data);
    const auto &rmat = _extrinsic.R();
    _pose = normalize(Vec2f(rmat(0, 2), rmat(2, 2)));
    // 设置组合体的特征容器
    _features = {p_left, p_right};
    _tick = tick;
}

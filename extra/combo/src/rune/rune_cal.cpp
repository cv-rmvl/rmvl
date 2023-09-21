/**
 * @file Rune_cal.cpp
 * @author RoboMaster Vision Community
 * @brief 神符类中有关计算内容的成员方法实现
 * @version 1.0
 * @date 2021-09-13
 *
 * @copyright Copyright 2021 (c), RoboMaster Vision Community
 *
 */

#include <opencv2/calib3d.hpp>

#include "rmvl/combo/rune.h"

#include "rmvlpara/combo/rune.h"

using namespace cv;
using namespace std;
using namespace para;
using namespace rm;

vector<Point2f> Rune::calculatePoints(rune_target_ptr p_target, rune_center_ptr p_center)
{
    // -------------------------------【获取神符五个角点】-------------------------------
    // 中心点
    Point2f center = p_center->getCenter();

    Point2f target_center = p_target->getCenter();
    float radius = p_target->getRadius();

    Point2f direction = (target_center - center) / getDistance(target_center, center);
    // 逆时针旋转90度
    Point2f vertical_direction = Matx22f(0, -1, 1, 0) * direction;

    // Matx33f y_rotate;
    // Rodrigues(Vec3f(0, (PI_2) * index_delta, 0), y_rotate);
    // 四个角点
    Point2f close_point = target_center - direction * radius;
    Point2f left_point = target_center + vertical_direction * radius;
    Point2f far_point = target_center + direction * radius;
    Point2f right_point = target_center - vertical_direction * radius;

    // 4 神符装甲板角点 + 1 神符中心点, 方向是顺时针
    return {close_point, left_point, far_point, right_point, center};
}

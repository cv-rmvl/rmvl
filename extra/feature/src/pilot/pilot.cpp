/**
 * @file Pilot.cpp
 * @author RoboMaster Vision Community
 * @brief 引导灯类源文件
 * @version 1.0
 * @date 2021-12-14
 *
 * @copyright Copyright 2021 (c), RoboMaster Vision Community
 *
 */

#include <opencv2/imgproc.hpp>

#include "rmvl/algorithm/math.hpp"
#include "rmvl/feature/pilot.h"

#include "rmvlpara/feature/pilot.h"

namespace rm
{

/**
 * @brief 获取准确的特征信息
 *
 * @param[in] contour 轮廓点集
 * @param[in] center 中心点
 * @param[in out] width 宽度
 * @param[in out] height 高度
 * @param[out] left 左顶点
 * @param[out] right 右顶点
 * @param[out] angle 角度
 */
static void correct(const std::vector<cv::Point> &contour, const cv::Point2f &center, float &width,
                    float &height, cv::Point2f &left, cv::Point2f &right, float &angle)
{
    cv::Point pl = center;
    cv::Point pr = center;
    // 遍历每一个轮廓点
    for (const auto &point : contour)
    {
        if (abs(point.y - center.y) > height / para::pilot_param.VERTEX_K)
            continue;
        // 小于或大于一定值时才对其进行计算比较，节省运算
        if (point.x < center.x) // 找出轮廓中最左边的点作为左极点
            pl = (point.x < pl.x) ? point : pl;
        else // 找出轮廓中最右边的点作为右极点
            pr = (point.x > pr.x) ? point : pr;
    }
    // 提取轮廓点集中离拟合椭圆左右顶点最近的点作修正后的数据
    if (pl.x != center.x && pr.x != center.x)
    {
        // 更新左右顶点
        left = pl;
        right = pr;
        float tmpw = getDistance(left, right);
        height = height * tmpw / width;
        width = tmpw;
        angle = getHAngle(left, right);
    }
}

Pilot::ptr Pilot::make_feature(const std::vector<cv::Point> &contour, cv::Mat &bin)
{
    cv::RotatedRect rotated_rect = cv::fitEllipse(contour);
    cv::Point2f center = rotated_rect.center;
    if (center.x < 0 || center.x > bin.cols ||
        center.y < 0 || center.y > bin.rows)
        return nullptr;
    float width = std::max(rotated_rect.size.width, rotated_rect.size.height);
    if (std::isnan(width) || width < 0 || width > 100000.f)
        return nullptr;
    float height = std::min(rotated_rect.size.width, rotated_rect.size.height);
    if (std::isnan(height) || height < 0 || height > 100000.f)
        return nullptr;
    // 仅对二值图中绿色部分拟合形状进行筛选
    int row = center.y;
    int col_l = center.x - width / 3;
    int col_r = center.x + width / 3;
    if (bin.ptr<uchar>(row)[col_l] == 125 ||
        bin.ptr<uchar>(row)[col_r] == 125)
    {
        if (contour.empty())
            return nullptr;
        float wh_ratio = width / height;
        if (wh_ratio > para::pilot_param.MAX_RATIO ||
            wh_ratio < para::pilot_param.MIN_RATIO)
            return nullptr;
    }
    else
        return nullptr;

    cv::Point2f left{}, right{};
    correct(contour, center, width, height, left, right, rotated_rect.angle);

    auto retval = std::make_shared<Pilot>();

    retval->_width = width;
    retval->_height = height;
    retval->_center = center;
    retval->_angle = rotated_rect.angle;
    retval->_corners = {left, right};

    return retval;
}

Pilot::ptr Pilot::make_feature(float w, float h, const cv::Point2f &center, float angle, const std::vector<cv::Point2f> &corners)
{
    if (std::isnan(w) || w < 0 || w > 100000.f)
        return nullptr;
    if (std::isnan(h) || h < 0 || h > 100000.f)
        return nullptr;

    auto retval = std::make_shared<Pilot>();
    retval->_width = w;
    retval->_height = h;
    retval->_center = center;
    retval->_angle = angle;
    retval->_corners = corners;

    return retval;
}

} // namespace rm

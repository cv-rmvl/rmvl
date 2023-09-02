/**
 * @file find.cpp
 * @author RoboMaster Vision Community
 * @brief find
 * @version 1.0
 * @date 2021-09-21
 *
 * @copyright Copyright 2021 (c), RoboMaster Vision Community
 *
 */

#include <unordered_set>

#include <opencv2/imgproc.hpp>

#include "rmvl/detector/rune_detector.h"
#include "rmvl/feature/rune/rune_logging.h"

#include "rmvlpara/combo/rune.h"
#include "rmvlpara/detector/rune_detector.h"

using namespace rm;
using namespace para;
using namespace std;
using namespace cv;

/**
 * @brief 未激活神符靶心等级向量判断
 *
 * @param[in] hierarchy 所有的等级向量
 * @param[in] idx 指定的等级向量的下标
 * @return 等级结构是否满足要求
 */
inline bool isHierarchyInactive(const vector<Vec4i> &hierarchy, size_t idx)
{
    bool is_inactive = 0;
    vector<int> bro_hierarchy;
    int index = idx;
    while (hierarchy[index][0] != -1)
    {
        bro_hierarchy.push_back(hierarchy[index][0]);
        index = hierarchy[index][0];
    }
    while (hierarchy[index][1] != -1)
    {
        bro_hierarchy.push_back(hierarchy[index][1]);
        index = hierarchy[index][1];
    }
    if (bro_hierarchy.size() > 2)
        is_inactive = 1;

    return is_inactive &&                                            // h[idx] 有大于两个并列轮廓
           hierarchy[idx][2] != -1 &&                                // h[idx] 有子轮廓，记为 hs[idx]
           ((hierarchy[hierarchy[idx][2]][2] != -1 &&                // hs[idx] 有子轮廓，记为 hss[idx]
             hierarchy[hierarchy[hierarchy[idx][2]][2]][2] == -1) || // hss[idx] 无子轮廓
            hierarchy[hierarchy[idx][2]][2] == -1) &&                // hs[idx] 无子轮廓
           hierarchy[idx][3] != -1 &&                                // h[idx] 有父轮廓，记为 hf[idx]
           hierarchy[hierarchy[idx][3]][3] == -1;                    // hf[idx] 无父轮廓
}

/**
 * @brief 已激活神符靶心等级向量判断
 *
 * @param[in] contours 所有轮廓
 * @param[in] hierarchy 所有的等级向量
 * @param[in] idx 指定的等级向量的下标
 * @return 等级结构是否满足要求
 */
inline bool isHierarchyActive(const vector<vector<Point>> &contours, const vector<Vec4i> &hierarchy, size_t idx)
{
    if (
        // h[i] 无子轮廓，但有父轮廓，记为 hf[i]
        hierarchy[idx][2] == -1 && hierarchy[idx][3] != -1 &&
        // hf[i] 无父轮廓
        hierarchy[hierarchy[idx][3]][3] == -1)
    {
        if (contourArea(contours[idx]) > rune_detector_param.BIG_TARGET_AREA)
            return true;
        RotatedRect inner = fitEllipse(contours[idx]);
        RotatedRect outer = fitEllipse(contours[hierarchy[idx][3]]);
        Point2f inner_center = inner.center;
        Point2f outer_center = outer.center;
        auto dis = getDistance(inner_center, outer_center);
        auto size = (outer.size.width + outer.size.height) / 2.;
        // 偏移与最大直径的比值
        DEBUG_RUNE_INFO_("target 0.ratio : %f", dis / size);
        if (dis / size < rune_detector_param.CONCENTRICITY_RATIO)
        {
            DEBUG_RUNE_PASS_("target 0.ratio : pass");
            return true;
        }
        DEBUG_RUNE_WARNING_("target 0.ratio : fail");
    }
    return false;
}

/**
 * @brief 神符中心等级向量判断
 *
 * @param[in] hierarchy 所有的等级向量
 * @param[in] idx 指定的等级向量的下标
 * @return 等级结构是否满足要求
 */
inline bool isHierarchyCenter(const vector<vector<Point>> &contours, const vector<Vec4i> &hierarchy, size_t idx)
{
    // h[idx] 必须存在若干并列轮廓，并且无父轮廓
    if ((hierarchy[idx][0] == -1 && hierarchy[idx][1] == -1) || hierarchy[idx][3] != -1)
        return false;
    // h[idx] 无子轮廓
    if (hierarchy[idx][2] == -1)
        return true;
    // h[idx] 有子轮廓，记为 hs[idx] 轮廓，hs[idx] 无子轮廓
    else if (hierarchy[hierarchy[idx][2]][2] == -1)
    {
        RotatedRect outer = fitEllipse(contours[idx]);
        Point2f outer_center = outer.center;
        Point2f inner_center;
        for (const auto &contour_point : contours[hierarchy[idx][2]])
        {
            inner_center += (Point2f)contour_point;
        }
        int contours_num = contours[hierarchy[idx][2]].size();
        inner_center /= (float)contours[hierarchy[idx][2]].size();
        auto dis = getDistance(inner_center, outer_center);
        auto size = (outer.size.width + outer.size.height) / 2.;
        // 偏移与最大直径的比值
        DEBUG_RUNE_INFO_("center 0.ratio : %f", dis / size);
        if (dis / size > rune_detector_param.CONCENTRICITY_RATIO)
        {
            DEBUG_RUNE_PASS_("center 0.ratio : pass");
            return true;
        }
        DEBUG_RUNE_WARNING_("center 0.ratio : fail");
        if (contours_num < 10)
            return true;
        return false;
    }
    return false;
}

void RuneDetector::find(Mat src, vector<feature_ptr> &features, vector<combo_ptr> &combos)
{
    vector<vector<Point>> contours; // 轮廓二维向量
    vector<Vec4i> hierarchy;        // 轮廓等级向量
    // 神符轮廓识别
    findContours(src, contours, hierarchy, RETR_TREE, CHAIN_APPROX_NONE);
    // 神符旋转中心向量
    vector<rune_center_ptr> rune_centers;
    vector<rune_target_ptr> rune_targets;
    // 遍历轮廓构建特征
    for (size_t i = 0; i < contours.size(); i++)
    {
        // ======================== 轮廓面积筛选 ========================
        double rune_area = contourArea(contours[i]);
        // 极小面积杂光跳过匹配
        if (rune_area < rune_detector_param.MIN_CONTOUR_AREA ||
            rune_area > rune_detector_param.MAX_CONTOUR_AREA)
            continue;
        DEBUG_RUNE_INFO_("--------------------------------------");
        DEBUG_RUNE_INFO_("rune_feature_area %ld : %f", i, rune_area);
        // ========================== 轮廓筛选 ==========================
        // 未激活神符靶心
        if (isHierarchyInactive(hierarchy, i))
        {
            DEBUG_RUNE_INFO_("--------------------------------------");
            DEBUG_RUNE_INFO_("target unactive");
            rune_target_ptr p_target = RuneTarget::make_feature(contours[i], false);
            if (p_target != nullptr)
            {
                DEBUG_RUNE_PASS_("target pass");
                rune_targets.push_back(p_target);
                continue;
            }
        }
        // 已激活神符靶心
        if (isHierarchyActive(contours, hierarchy, i))
        {
            DEBUG_RUNE_INFO_("--------------------------------------");
            DEBUG_RUNE_INFO_("target active");
            rune_target_ptr p_target = RuneTarget::make_feature(contours[i], true);
            if (p_target != nullptr)
            {
                DEBUG_RUNE_PASS_("target pass");
                rune_targets.push_back(p_target);
                continue;
            }
        }
        // 神符中心
        if (isHierarchyCenter(contours, hierarchy, i))
        {
            DEBUG_RUNE_INFO_("--------------------------------------");
            DEBUG_RUNE_INFO_("center");
            rune_center_ptr p_center = RuneCenter::make_feature(contours[i]);
            if (p_center != nullptr)
            {
                DEBUG_RUNE_PASS_("center pass");
                rune_centers.push_back(p_center);
                continue;
            }
        }
    }
    DEBUG_RUNE_INFO_("--------------------------------------");
    DEBUG_RUNE_INFO_("rune rune_targets_size : %ld", rune_targets.size());
    DEBUG_RUNE_INFO_("rune rune_centers_size : %ld", rune_centers.size());
    // 判空
    if (rune_targets.empty() || rune_centers.empty())
        return;
    // 最优神符中心点
    rune_center_ptr best_center = nullptr;
    best_center = getBestCenter(rune_targets, rune_centers);
    if (best_center == nullptr && rune_centers.size() == 1)
        best_center = rune_centers.front();
    // 判空
    if (best_center == nullptr)
        return;
    // 获取尚未激活的神符
    vector<rune_ptr> runes = getRune(rune_targets, best_center);
    // 更新特征、组合体集合
    for (const auto &p_rune_target : rune_targets)
        features.emplace_back(p_rune_target);
    features.emplace_back(best_center);
    for (const auto &p_combo : runes)
        combos.emplace_back(p_combo);
}

rune_center_ptr RuneDetector::getBestCenter(const vector<rune_target_ptr> &rune_targets, const vector<rune_center_ptr> &rune_centers)
{
    // 神符中心集合
    unordered_set<rune_center_ptr> center_set(rune_centers.begin(), rune_centers.end());
    // 筛除离神符连线中垂线过远的中心
    if (rune_targets.size() > 1)
    {
        for (auto &p_center : rune_centers)
        {
            for (size_t i = 0; i + 1 < rune_targets.size(); i++)
            {
                // 靶心连线方向
                Vec2f dvec = rune_targets[i]->getCenter() - rune_targets[i + 1]->getCenter();
                Matx22f rotate = {0, 1,
                                  -1, 0};
                // 靶心连线垂线
                Vec2f vvec = rotate * dvec;
                // 靶心连线中点
                Vec2f midp = (rune_targets[i]->getCenter() + rune_targets[i + 1]->getCenter()) / 2;
                // 与中垂线的距离比值
                float distance_ratio = getDistance(Vec4f(vvec(0), vvec(1), midp(0), midp(1)),
                                                   p_center->getCenter(), false) /
                                       p_center->getHeight();
                if (distance_ratio > rune_param.MAX_MID_LINE_RATIO)
                {
                    center_set.erase(p_center);
                    break;
                }
            }
        }
    }
    // 判空
    if (center_set.empty())
        return nullptr;
    // [神符中心 : 距离比值差]
    unordered_map<rune_center_ptr, float> center_ratio_differences;
    center_ratio_differences.reserve(center_set.size());
    for (auto &p_center : center_set)
    {
        center_ratio_differences[p_center] = FLOAT_MAX;
        // 距离比值差序列
        vector<float> dratio;
        dratio.reserve(rune_targets.size());
        for (auto &p_target : rune_targets)
        {
            // target 到 center 的距离与 center 尺寸的比值
            float radius_ratio = getDistance(p_target->getCenter(), p_center->getCenter()) / p_center->getHeight();
            // 扇叶为未激活且距离比值过大或过小，跳过
            if (!p_target->isActive() && (radius_ratio < rune_param.MIN_RADIUS_RATIO ||
                                          radius_ratio > rune_param.MAX_RADIUS_RATIO))
                continue;
            dratio.push_back(abs(rune_param.BEST_RADIUS_RATIO - radius_ratio));
        }
        if (!dratio.empty())
            center_ratio_differences[p_center] = *min_element(dratio.begin(), dratio.end());
    }

    // 选择距离比值差最小的中心
    return min_element(center_ratio_differences.begin(), center_ratio_differences.end(),
                       [&](const auto &lhs, const auto &rhs)
                       {
                           return lhs.second < rhs.second;
                       })
        ->first;
}

vector<rune_ptr> RuneDetector::getRune(const vector<rune_target_ptr> &rune_targets, const rune_center_ptr &p_center)
{
    if (rune_targets.empty())
        return {};
    if (!p_center)
        return {};

    vector<rune_ptr> retval;
    // 神符外侧、神符支架两两匹配
    for (const auto &p_target : rune_targets)
    {
        DEBUG_RUNE_INFO_("--------------------------------------");
        rune_ptr rune = Rune::make_combo(p_target, p_center, _gyro_data, _tick);
        if (rune != nullptr)
        {
            DEBUG_RUNE_PASS_("rune pass");
            retval.push_back(rune);
        }
    }
    return retval;
}
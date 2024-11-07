/**
 * @file match.cpp
 * @author RoboMaster Vision Community
 * @brief match to tracker and group
 * @version 0.1
 * @date 2022-12-10
 *
 * @copyright Copyright 2022 (c), RoboMaster Vision Community
 *
 */

#include "rmvl/detector/gyro_detector.h"
#include "rmvl/group/gyro_group.h"
#include "rmvl/algorithm/datastruct.hpp"

#include "rmvlpara/camera/camera.h"
#include "rmvlpara/detector/gyro_detector.h"

namespace rm
{

/**
 * @brief 判断能否合并
 *
 * @param[in] lhs 左装甲板
 * @param[in] rhs 右装甲板
 * @return 能否合并
 */
static inline bool canBeUnion(const combo::ptr &lhs, const combo::ptr &rhs)
{
    // 宽度比值
    float width_ratio = (lhs->width() > rhs->width()) ? lhs->width() / rhs->width()
                                                            : rhs->width() / lhs->width();
    if (width_ratio > para::gyro_detector_param.MAX_WIDTH_RATIO)
    {
        DEBUG_INFO_("can't be union: width_ratio = %f", width_ratio);
        return false;
    }
    // 高度比值
    float height_ratio = (lhs->height() > rhs->height()) ? lhs->height() / rhs->height()
                                                               : rhs->height() / lhs->height();
    if (height_ratio > para::gyro_detector_param.MAX_HEIGHT_RATIO)
    {
        DEBUG_INFO_("can't be union: height_ratio = %f", height_ratio);
        return false;
    }
    float avg_height = std::max(lhs->height(), rhs->height());
    // 水平距离及与高度的比值
    float dx = std::abs(lhs->center().x - rhs->center().x);
    float dx_ratio = dx / avg_height;
    if (dx_ratio > para::gyro_detector_param.MAX_X_DISTANCE_RATIO ||
        dx_ratio < para::gyro_detector_param.MIN_X_DISTANCE_RATIO)
    {
        DEBUG_INFO_("can't be union: dx_ratio = %f", dx_ratio);
        return false;
    }
    // 垂直距离及与高度的比值
    float dy = std::abs(lhs->center().y - rhs->center().y);
    float dy_ratio = dy / avg_height;
    if (dy_ratio > para::gyro_detector_param.MAX_Y_DISTANCE_RATIO)
    {
        DEBUG_INFO_("can't be union: dy_ratio = %f", dy_ratio);
        return false;
    }
    // 角度需要呈 "/  \" 样
    if (lhs->angle() < 0 || rhs->angle() > 0)
        return false;
    // 角度差距要稍大
    float delta_angle = lhs->angle() * rhs->angle();
    if (delta_angle < para::gyro_detector_param.MIN_MULTIPLE_ANGLE ||
        delta_angle > para::gyro_detector_param.MAX_MULTIPLE_ANGLE)
    {
        DEBUG_INFO_("can't be union: delta_angle = %f", delta_angle);
        return false;
    }
    return true;
}

void GyroDetector::match(std::vector<group::ptr> &groups, std::vector<combo::ptr> &combos)
{
    std::unordered_map<combo::ptr, std::vector<combo::ptr>> combo_maps; // [代表元素:同组装甲板的列表]
    // 用并查集进行同组装甲板合并
    if (!combos.empty())
    {
        // 从左到右排序得到有序的装甲板序列
        sort(combos.begin(), combos.end(), [](combo::const_ptr lhs, combo::const_ptr rhs) {
            return lhs->center().x < rhs->center().x;
        });
        UnionFind<combo::ptr> uf(combos.begin(), combos.end());
        for (size_t i = 0; i + 1 < combos.size(); ++i)
            for (size_t j = i + 1; j < combos.size(); ++j)
                if (canBeUnion(combos[i], combos[j]))
                    uf.merge(combos[i], combos[j]);
        combo_maps = uf.extract();
    }
    // groups 为空，进行初始化
    if (groups.empty())
    {
        if (combo_maps.empty())
            return;
        // 为每一个装甲板组初始化序列组
        for (const auto &map_pair : combo_maps)
        {
            auto p_group = GyroGroup::make_group(map_pair.second, _armor_num);
            if (p_group != nullptr)
                groups.emplace_back(p_group);
        }
    }
    // groups 非空，进行更新操作
    else
    {
        // 装甲板集合代表元素对应的序列组中轴线坐标 [代表元素:旋转中心点]
        std::unordered_map<combo::const_ptr, cv::Point3f> groups_center;
        // 遍历装甲板组，构建 groups_center
        for (const auto &map_pair : combo_maps)
        {
            combo::const_ptr represent = map_pair.first;
            const auto &combo_vec = map_pair.second;
            // 设置装甲板的中心、姿态、半径集合
            size_t combo_size = combo_vec.size();
            std::vector<cv::Vec3f> combos_t(combo_size); // t: 每个集合中的装甲板的平移向量
            std::vector<cv::Vec2f> combos_P(combo_size); // P: 每个集合中的装甲板的姿态法向量
            std::vector<float> combos_r(combo_size);     // r: 每个集合中的装甲板在整车模型中的旋转半径
            // 为 combo_vec 的每一个装甲板设置 t, R, r 三个数据量
            for (size_t i = 0; i < combo_size; i++)
            {
                combo::const_ptr p_combo = combo_vec[i];
                combos_t[i] = p_combo->extrinsics().tvec();
                combos_P[i] = Armor::cast(p_combo)->getPose();
                combos_r[i] = para::gyro_group_param.INIT_RADIUS;
            }
            // 解算序列组中轴线坐标
            cv::Vec3f group_center;
            GyroGroup::calcGroupFrom3DMessage(combos_P, combos_t, combos_r, group_center);
            groups_center[represent] = group_center;
        }

        // combo_maps > groups
        if (combo_maps.size() > groups.size())
        {
            // 初始化代表装甲板集合
            std::unordered_set<combo::ptr> represent_set;
            for (const auto &p : combo_maps)
                represent_set.insert(p.first); // represent
            // 距离最近的装甲板组匹配到相应的序列组中
            for (auto &p_group : groups)
            {
                auto p_gyro_group = GyroGroup::cast(p_group);
                // 离 p_group 最近的 group_center 及其距离
                auto min_it = *min_element(represent_set.begin(), represent_set.end(), [&](combo::const_ptr lhs, combo::const_ptr rhs) {
                    return getDistance(groups_center[lhs], cv::Point3f(p_gyro_group->getCenter3D())) <
                           getDistance(groups_center[rhs], cv::Point3f(p_gyro_group->getCenter3D()));
                });
                matchOneGroup(p_group, combo_maps[min_it]);
                represent_set.erase(min_it);
            }
            // 没有匹配到的装甲板组作为新的序列
            for (const auto &represent : represent_set)
                groups.emplace_back(GyroGroup::make_group(combo_maps[represent], _armor_num));
        }
        // combo_maps < groups
        else if (combo_maps.size() < groups.size())
        {
            // 初始化序列组集合
            std::unordered_set<GyroGroup::ptr> group_set(groups.size());
            for (const auto &p_group : groups)
            {
                auto p_gyro_group = GyroGroup::cast(p_group);
                group_set.insert(p_gyro_group);
            }
            // 距离最近的装甲板组匹配到相应的序列组中
            for (const auto &combo_map : combo_maps)
            {
                // 离 group_center 最近的 group 及其距离
                auto min_it = min_element(group_set.begin(), group_set.end(), [&](GyroGroup::ptr lhs, GyroGroup::ptr rhs) {
                    return getDistance(groups_center[combo_map.first], cv::Point3f(lhs->getCenter3D())) <
                           getDistance(groups_center[combo_map.first], cv::Point3f(rhs->getCenter3D()));
                });
                matchOneGroup(*min_it, combo_map.second);
                group_set.erase(*min_it);
            }
            // 没有匹配到的序列组传入空集合
            for (const auto &p_group : group_set)
                matchOneGroup(p_group, {});
        }
        // combo_maps = groups
        else
        {
            // 初始化代表装甲板集合
            std::unordered_set<combo::ptr> represent_set;
            for (const auto &p : combo_maps)
                represent_set.insert(p.first); // represent
            size_t before_size = groups.size();
            for (size_t i = 0; i < before_size; i++)
            {
                auto p_gyro_group = GyroGroup::cast(groups[i]);
                if (p_gyro_group == nullptr)
                    RMVL_Error(RMVL_BadDynamicType, "Dynamic type of p_gyro_group isn\'t equal to \"GyroGroup\"");
                // 离 group 最近的 group_center
                auto min_it = *min_element(represent_set.begin(), represent_set.end(), [&](combo::const_ptr lhs, combo::const_ptr rhs) {
                    return getDistance(groups_center[lhs], cv::Point3f(p_gyro_group->getCenter3D())) <
                           getDistance(groups_center[rhs], cv::Point3f(p_gyro_group->getCenter3D()));
                });
                // 最短距离
                float min_dis = getDistance(groups_center[min_it], cv::Point3f(p_gyro_group->getCenter3D()));
                // 判断是否突变
                if (min_dis > para::gyro_detector_param.MAX_GROUP_DELTA_DIS)
                {
                    // 创建新序列，原来的序列组打入 nullptr
                    matchOneGroup(p_gyro_group, {});
                    // 没有匹配到的装甲板作为新的序列组
                    groups.emplace_back(GyroGroup::make_group(combo_maps.at(min_it), _armor_num));
                }
                else
                    matchOneGroup(p_gyro_group, combo_maps[min_it]);
                represent_set.erase(min_it);
            }
        }
        // 为所有序列组进行同步，并记录出现异常的 group
        std::unordered_set<group::const_ptr> remove_set;
        for (const auto &p_group : groups)
        {
            try
            {
                p_group->sync(_imu_data, _tick);
            }
            catch (const rm::Exception &e)
            {
                ERROR_("Occurred an exception! %s", e.err.c_str());
                remove_set.insert(p_group);
            }
        }
        // 删除异常 group
        groups.erase(remove_if(groups.begin(), groups.end(), [&remove_set](group::const_ptr val) {
                         return remove_set.find(val) != remove_set.end();
                     }),
                     groups.end());
    }
    // 删除四个 tracker 同时消失数量过多的 group
    groups.erase(remove_if(groups.begin(), groups.end(), [](group::const_ptr p_group) {
                     return p_group->getVanishNumber() > para::gyro_group_param.TRACK_FRAMES;
                 }),
                 groups.end());
    // 删除 2D 中心点相距过近的 group
    sort(groups.begin(), groups.end(), [](group::const_ptr lhs, group::const_ptr rhs) {
        return lhs->center().x < rhs->center().x;
    });
    std::unordered_set<group::const_ptr> erase_group_set;
    for (size_t i = 1; i < groups.size(); ++i)
        if (getDistance(groups[i]->center(), groups[i - 1]->center()) < para::gyro_detector_param.MIN_CENTER_DIS)
            erase_group_set.insert(groups[i]);
    groups.erase(remove_if(groups.begin(), groups.end(), [&erase_group_set](group::const_ptr p_group) {
                     return erase_group_set.find(p_group) != erase_group_set.end();
                 }),
                 groups.end());
}

void GyroDetector::matchOneGroup(group::ptr group, const std::vector<combo::ptr> &combos)
{
    // 在所有追踪器
    std::vector<tracker::ptr> trackers = group->data();
    std::unordered_set<tracker::ptr> tracker_set(trackers.begin(), trackers.end());
    // 根据距离 distance 排序，选出待匹配的 trackers
    sort(trackers.begin(), trackers.end(), [](tracker::const_ptr lhs, tracker::const_ptr rhs) {
        return lhs->extrinsics().distance() < rhs->extrinsics().distance();
    });
    // 待匹配参考 tracker 的大小
    size_t ref_size = trackers.size();
    if (ref_size > 2)
    {
        ref_size = ref_size >> 1;
        trackers.erase(trackers.begin() + ref_size, trackers.end());
    }
    // combos 大小判断
    if (combos.size() > ref_size)
        RMVL_Error_(RMVL_StsBadArg, "Size of the argument \"combos\" is greater than %zu. (size = %zu)", ref_size, combos.size());
    for (auto p_combo : combos)
    {
        // 当前捕获到的新装甲板角点
        const auto &cur_p = p_combo->corners();
        auto min_tracker = *min_element(trackers.begin(), trackers.end(), [&](tracker::const_ptr lhs, tracker::const_ptr rhs) {
            const auto &lhs_p = lhs->front()->corners();
            const auto &rhs_p = rhs->front()->corners();
            return (getDistance(cur_p[0], lhs_p[0]) + getDistance(cur_p[1], lhs_p[1]) +
                    getDistance(cur_p[2], lhs_p[2]) + getDistance(cur_p[3], lhs_p[3])) <
                   (getDistance(cur_p[0], rhs_p[0]) + getDistance(cur_p[1], rhs_p[1]) +
                    getDistance(cur_p[2], rhs_p[2]) + getDistance(cur_p[3], rhs_p[3]));
        });
        min_tracker->update(p_combo);   // 更新追踪器
        tracker_set.erase(min_tracker); // 移出待匹配追踪器序列
        auto p_gyro_tracker = GyroTracker::cast(min_tracker);
        p_gyro_tracker->updateVanishState(GyroTracker::APPEAR); // 设置为可见
    }
    // 没有匹配到的序列设置为不可见
    for (auto p_tracker : tracker_set)
        GyroTracker::cast(p_tracker)->updateVanishState(GyroTracker::VANISH);
}

void GyroDetector::eraseFakeTracker(std::vector<tracker::ptr> &trackers)
{
    // 删除
    trackers.erase(remove_if(trackers.begin(), trackers.end(), [](tracker::const_ptr p_tracker) {
                       return p_tracker->type().RobotTypeID == RobotType::UNKNOWN;
                   }),
                   trackers.end());
}

} // namespace rm

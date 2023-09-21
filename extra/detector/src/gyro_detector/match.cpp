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
#include "rmvl/rmath/union_find.hpp"

#include "rmvlpara/camera.hpp"
#include "rmvlpara/detector/gyro_detector.h"

using namespace rm;
using namespace para;
using namespace std;
using namespace cv;

/**
 * @brief 判断能否合并
 *
 * @param[in] lhs 左装甲板
 * @param[in] rhs 右装甲板
 * @return 能否合并
 */
static inline bool canBeUnion(const combo_ptr &lhs, const combo_ptr &rhs)
{
    // 宽度比值
    float width_ratio = (lhs->getWidth() > rhs->getWidth()) ? lhs->getWidth() / rhs->getWidth()
                                                            : rhs->getWidth() / lhs->getWidth();
    if (width_ratio > gyro_detector_param.MAX_WIDTH_RATIO)
    {
        DEBUG_INFO_("can't be union: width_ratio = %f", width_ratio);
        return false;
    }
    // 高度比值
    float height_ratio = (lhs->getHeight() > rhs->getHeight()) ? lhs->getHeight() / rhs->getHeight()
                                                               : rhs->getHeight() / lhs->getHeight();
    if (height_ratio > gyro_detector_param.MAX_HEIGHT_RATIO)
    {
        DEBUG_INFO_("can't be union: height_ratio = %f", height_ratio);
        return false;
    }
    float avg_height = max(lhs->getHeight(), rhs->getHeight());
    // 水平距离及与高度的比值
    float dx = abs(lhs->getCenter().x - rhs->getCenter().x);
    float dx_ratio = dx / avg_height;
    if (dx_ratio > gyro_detector_param.MAX_X_DISTANCE_RATIO ||
        dx_ratio < gyro_detector_param.MIN_X_DISTANCE_RATIO)
    {
        DEBUG_INFO_("can't be union: dx_ratio = %f", dx_ratio);
        return false;
    }
    // 垂直距离及与高度的比值
    float dy = abs(lhs->getCenter().y - rhs->getCenter().y);
    float dy_ratio = dy / avg_height;
    if (dy_ratio > gyro_detector_param.MAX_Y_DISTANCE_RATIO)
    {
        DEBUG_INFO_("can't be union: dy_ratio = %f", dy_ratio);
        return false;
    }
    // 角度需要呈 "/  \" 样
    if (lhs->getAngle() < 0 || rhs->getAngle() > 0)
        return false;
    // 角度差距要稍大
    float delta_angle = lhs->getAngle() * rhs->getAngle();
    if (delta_angle < gyro_detector_param.MIN_MULTIPLE_ANGLE ||
        delta_angle > gyro_detector_param.MAX_MULTIPLE_ANGLE)
    {
        DEBUG_INFO_("can't be union: delta_angle = %f", delta_angle);
        return false;
    }
    return true;
}

void GyroDetector::match(vector<group_ptr> &groups, vector<combo_ptr> &combos)
{
    unordered_map<combo_ptr, vector<combo_ptr>> combo_maps; // [代表元素:同组装甲板的列表]
    // 用并查集进行同组装甲板合并
    if (!combos.empty())
    {
        // 从左到右排序得到有序的装甲板序列
        sort(combos.begin(), combos.end(),
             [](const combo_ptr &lhs, const combo_ptr &rhs)
             {
                 return lhs->getCenter().x < rhs->getCenter().x;
             });
        UnionFind<combo_ptr> uf(combos.begin(), combos.end());
        for (size_t i = 0; i + 1 < combos.size(); ++i)
            for (size_t j = i + 1; j < combos.size(); ++j)
                if (canBeUnion(combos[i], combos[j]))
                    uf.unionSet(combos[i], combos[j]);
        combo_maps = uf.exportData();
    }
    // groups 为空，进行初始化
    if (groups.empty())
    {
        if (combo_maps.empty())
            return;
        // 为每一个装甲板组初始化序列组
        for (auto &map_pair : combo_maps)
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
        unordered_map<combo_ptr, Point3f> groups_center;
        // 遍历装甲板组，构建 groups_center
        for (const auto &map_pair : combo_maps)
        {
            combo_ptr represent = map_pair.first;
            const auto &combo_vec = map_pair.second;
            // 设置装甲板的中心、姿态、半径集合
            size_t combo_size = combo_vec.size();
            vector<Vec3f> combos_t(combo_size); // t: 每个集合中的装甲板的平移向量
            vector<Vec2f> combos_P(combo_size); // P: 每个集合中的装甲板的姿态法向量
            vector<float> combos_r(combo_size); // r: 每个集合中的装甲板在整车模型中的旋转半径
            // 为 combo_vec 的每一个装甲板设置 t, R, r 三个数据量
            for (size_t i = 0; i < combo_size; i++)
            {
                combo_ptr p_combo = combo_vec[i];
                combos_t[i] = p_combo->getExtrinsics().tvec();
                combos_P[i] = Armor::cast(p_combo)->getPose();
                combos_r[i] = gyro_group_param.INIT_RADIUS;
            }
            // 解算序列组中轴线坐标
            Vec3f group_center;
            GyroGroup::calcGroupFrom3DMessage(combos_P, combos_t, combos_r, group_center);
            groups_center[represent] = group_center;
        }

        // combo_maps > groups
        if (combo_maps.size() > groups.size())
        {
            // 初始化代表装甲板集合
            unordered_set<combo_ptr> represent_set;
            for (const auto &[represent, combo_set] : combo_maps)
                represent_set.insert(represent);
            // 距离最近的装甲板组匹配到相应的序列组中
            for (auto &p_group : groups)
            {
                auto p_gyro_group = GyroGroup::cast(p_group);
                // 离 p_group 最近的 group_center 及其距离
                auto min_it = *min_element(represent_set.begin(), represent_set.end(),
                                           [&](const auto &lhs, const auto &rhs)
                                           {
                                               return getDistance(groups_center[lhs], Point3f(p_gyro_group->getCenter3D())) <
                                                      getDistance(groups_center[rhs], Point3f(p_gyro_group->getCenter3D()));
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
            unordered_set<gyro_group_ptr> group_set(groups.size());
            for (const auto &p_group : groups)
            {
                auto p_gyro_group = GyroGroup::cast(p_group);
                group_set.insert(p_gyro_group);
            }
            // 距离最近的装甲板组匹配到相应的序列组中
            for (const auto &combo_map : combo_maps)
            {
                // 离 group_center 最近的 group 及其距离
                auto min_it = min_element(group_set.begin(), group_set.end(),
                                          [&combo_map, &groups_center](const auto &lhs, const auto &rhs)
                                          {
                                              return getDistance(groups_center[combo_map.first], Point3f(lhs->getCenter3D())) <
                                                     getDistance(groups_center[combo_map.first], Point3f(rhs->getCenter3D()));
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
            unordered_set<combo_ptr> represent_set;
            for (const auto &[represent, combo_set] : combo_maps)
                represent_set.insert(represent);
            size_t before_size = groups.size();
            for (size_t i = 0; i < before_size; i++)
            {
                auto p_gyro_group = GyroGroup::cast(groups[i]);
                if (p_gyro_group == nullptr)
                    RMVL_Error(RMVL_BadDynamicType, "Dynamic type of p_gyro_group isn\'t equal to \"GyroGroup\"");
                // 离 group 最近的 group_center
                auto min_it = *min_element(represent_set.begin(), represent_set.end(),
                                           [&](const auto &lhs, const auto &rhs)
                                           {
                                               return getDistance(groups_center[lhs], Point3f(p_gyro_group->getCenter3D())) <
                                                      getDistance(groups_center[rhs], Point3f(p_gyro_group->getCenter3D()));
                                           });
                // 最短距离
                float min_dis = getDistance(groups_center[min_it], Point3f(p_gyro_group->getCenter3D()));
                // 判断是否突变
                if (min_dis > gyro_detector_param.MAX_GROUP_DELTA_DIS)
                {
                    // 创建新序列，原来的序列组打入 nullptr
                    matchOneGroup(p_gyro_group, {});
                    // 没有匹配到的装甲板作为新的序列组
                    groups.emplace_back(GyroGroup::make_group(combo_maps[min_it], _armor_num));
                }
                else
                    matchOneGroup(p_gyro_group, combo_maps[min_it]);
                represent_set.erase(min_it);
            }
        }
        // 为所有序列组进行同步，并记录出现异常的 group
        unordered_set<group_ptr> remove_set;
        for (const auto &p_group : groups)
        {
            try
            {
                p_group->sync(_gyro_data, _tick);
            }
            catch (const rm::Exception &e)
            {
                ERROR_("Occurred an exception! %s", e.err.c_str());
                remove_set.insert(p_group);
            }
        }
        // 删除异常 group
        groups.erase(remove_if(groups.begin(), groups.end(),
                               [&remove_set](const group_ptr &val)
                               {
                                   return remove_set.find(val) != remove_set.end();
                               }),
                     groups.end());
    }
    // 删除四个 tracker 同时消失数量过多的 group
    groups.erase(remove_if(groups.begin(), groups.end(),
                           [](group_ptr &p_group)
                           {
                               return p_group->getVanishNumber() > gyro_group_param.TRACK_FRAMES;
                           }),
                 groups.end());
    // 删除 2D 中心点相距过近的 group
    sort(groups.begin(), groups.end(),
         [](const group_ptr &lhs, const group_ptr &rhs)
         {
             return lhs->getCenter().x < rhs->getCenter().x;
         });
    unordered_set<group_ptr> erase_group_set;
    for (size_t i = 1; i < groups.size(); ++i)
        if (getDistance(groups[i]->getCenter(), groups[i - 1]->getCenter()) < gyro_detector_param.MIN_CENTER_DIS)
            erase_group_set.insert(groups[i]);
    groups.erase(remove_if(groups.begin(), groups.end(),
                           [&erase_group_set](group_ptr &p_group)
                           {
                               return erase_group_set.find(p_group) != erase_group_set.end();
                           }),
                 groups.end());
}

void GyroDetector::matchOneGroup(group_ptr group, const std::vector<combo_ptr> &combos)
{
    // 在所有追踪器
    vector<tracker_ptr> trackers = group->data();
    unordered_set<tracker_ptr> tracker_set(trackers.begin(), trackers.end());
    // 根据距离 distance 排序，选出待匹配的 trackers
    sort(trackers.begin(), trackers.end(),
         [](const tracker_ptr &lhs, const tracker_ptr &rhs)
         {
             return lhs->getExtrinsics().distance() < rhs->getExtrinsics().distance();
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
        const auto &cur_p = p_combo->getCorners();
        auto min_tracker = *min_element(trackers.begin(), trackers.end(),
                                        [&cur_p](const tracker_ptr &lhs, const tracker_ptr &rhs)
                                        {
                                            const auto &lhs_p = lhs->front()->getCorners();
                                            const auto &rhs_p = rhs->front()->getCorners();
                                            return (getDistance(cur_p[0], lhs_p[0]) + getDistance(cur_p[1], lhs_p[1]) +
                                                    getDistance(cur_p[2], lhs_p[2]) + getDistance(cur_p[3], lhs_p[3])) <
                                                   (getDistance(cur_p[0], rhs_p[0]) + getDistance(cur_p[1], rhs_p[1]) +
                                                    getDistance(cur_p[2], rhs_p[2]) + getDistance(cur_p[3], rhs_p[3]));
                                        });
        min_tracker->update(p_combo, _tick, _gyro_data); // 更新追踪器
        tracker_set.erase(min_tracker);                  // 移出待匹配追踪器序列
        auto p_gyro_tracker = GyroTracker::cast(min_tracker);
        p_gyro_tracker->updateVanishState(GyroTracker::APPEAR); // 设置为可见
    }
    // 没有匹配到的序列设置为不可见
    for (const auto &p_tracker : tracker_set)
        GyroTracker::cast(p_tracker)->updateVanishState(GyroTracker::VANISH);
}

void GyroDetector::eraseFakeTracker(vector<tracker_ptr> &trackers)
{
    // 删除
    trackers.erase(remove_if(trackers.begin(), trackers.end(),
                             [](tracker_ptr &p_tracker)
                             {
                                 return p_tracker->getType().RobotTypeID == RobotType::UNKNOWN;
                             }),
                   trackers.end());
}

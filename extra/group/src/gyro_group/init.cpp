/**
 * @file init.cpp
 * @author RoboMaster Vision Community
 * @brief 装甲序列组初始化源文件
 * @version 0.1
 * @date 2023-02-10
 *
 * @copyright Copyright 2023 (c), RoboMaster Vision Community
 *
 */

#include "rmvl/group/gyro_group.h"
#include "rmvl/rmath/transform.h"
#include "rmvl/tracker/gyro_tracker.h"

#include "rmvlpara/camera/camera.h"
#include "rmvlpara/combo/armor.h"

namespace rm
{

GyroGroup::GyroGroup(const std::vector<combo::ptr> &first_combos, int armor_num)
{
    // 确定装甲板 armor_num 数目
    _armor_num = armor_num < 1 ? calcArmorNum(first_combos) : armor_num;
    std::vector<TrackerState> state_vec; // 追踪器信息列表
    std::vector<combo::ptr> combo_vec;   // 整车模型中的组合体列表
    cv::Vec3f group_center3d;            // 旋转中心点相机坐标
    cv::Point2f group_center2d;          // 旋转中心点像素坐标
    _gyro_data = first_combos.front()->getGyroData();
    _tick = first_combos.front()->getTick();
    // 获取 RobotType
    std::vector<RobotType> robot_type_vec;
    for_each(first_combos.begin(), first_combos.end(), [&robot_type_vec](combo::const_ptr val) {
        robot_type_vec.push_back(val->getType().RobotTypeID);
    });
    _type.RobotTypeID = calculateModeNum(robot_type_vec.begin(), robot_type_vec.end());
    // 获取序列组信息
    getGroupInfo(first_combos, state_vec, combo_vec, group_center3d, group_center2d);
    // 初始化序列组
    std::unordered_set<combo::ptr> combo_set(first_combos.begin(), first_combos.end());
    initGroup(combo_set, state_vec, combo_vec, group_center3d, group_center2d);
    // 初始化滤波器
    initFilter();
}

void GyroGroup::initGroup(const std::unordered_set<combo::ptr> &visible_combo_set, const std::vector<TrackerState> &state_vec,
                          const std::vector<combo::ptr> &combo_vec, const cv::Vec3f &group_center3d, const cv::Point2f &group_center2d)
{
    if (static_cast<int>(state_vec.size()) != _armor_num || static_cast<int>(combo_vec.size()) != _armor_num)
        RMVL_Error_(RMVL_StsBadArg, "Bad size of the \"state_vec\" (size = %zu) or \"combo_vec\" (size = %zu)",
                    state_vec.size(), combo_vec.size());
    // 按组合体顺序初始化追踪器
    std::vector<tracker::ptr> tracker_vec;
    for (const auto &p_combo : combo_vec)
        tracker_vec.push_back(GyroTracker::make_tracker(p_combo));
    _trackers = tracker_vec;
    for (int i = 0; i < _armor_num; i++)
        _tracker_state[tracker_vec[i]] = state_vec[i];
    // 旋转中心点坐标初始化
    _center3d_deq.emplace_front(group_center3d);
    _center3d = group_center3d;
    _center = group_center2d;
    // 序列可见性初始化
    for (auto p_tracker : _trackers)
    {
        if (visible_combo_set.find(p_tracker->front()) == visible_combo_set.end())
            std::const_pointer_cast<GyroTracker>(GyroTracker::cast(p_tracker))->updateVanishState(GyroTracker::VANISH);
        else
            std::const_pointer_cast<GyroTracker>(GyroTracker::cast(p_tracker))->updateVanishState(GyroTracker::APPEAR);
    }
}

void GyroGroup::initFilter()
{
    // 初始化旋转中心点位置滤波器
    _center3d_filter.setQ(para::gyro_group_param.CENTER3D_Q);
    _center3d_filter.setR(para::gyro_group_param.CENTER3D_R);
    _center3d_filter.init(cv::Matx61f(_center3d(0), _center3d(1), _center3d(2),
                                      _speed3d(0), _speed3d(1), _speed3d(2)),
                          1e-2);
}

} // namespace rm

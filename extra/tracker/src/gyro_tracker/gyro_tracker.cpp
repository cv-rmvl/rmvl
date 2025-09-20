/**
 * @file gyro_tracker.cpp
 * @author RoboMaster Vision Community
 * @brief 装甲板追踪器
 * @version 0.1
 * @date 2022-12-10
 *
 * @copyright Copyright 2022 (c), RoboMaster Vision Community
 *
 */

#include "rmvl/algorithm/math.hpp"

#include "rmvl/tracker/gyro_tracker.h"
#include "rmvlpara/tracker/gyro_tracker.h"

namespace rm {

void GyroTracker::updateFromCombo(combo::ptr p_combo) {
    _height = p_combo->height();
    _width = p_combo->width();
    _angle = p_combo->angle();
    _center = p_combo->center();
    _relative_angle = p_combo->getRelativeAngle();
    _corners = p_combo->corners();
    _extrinsic = p_combo->extrinsic();
    _pose = Armor::cast(p_combo)->getPose();
}

GyroTracker::GyroTracker(combo::ptr p_armor) {
    if (p_armor == nullptr)
        RMVL_Error(RMVL_StsBadArg, "Input argument \"p_armor\" is nullptr.");
    updateFromCombo(p_armor);

    _extrinsic = p_armor->extrinsic();
    _state = p_armor->state();
    _combo_deque.push_back(p_armor);
    _type_deque.push_back(to_robot_type(_state["robot"]));
    _duration = para::gyro_tracker_param.SAMPLE_INTERVAL;
    initFilter();
}

tracker::ptr GyroTracker::clone() {
    auto retval = std::make_shared<GyroTracker>(*this);
    // 更新内部所有组合体
    for (auto &p_combo : retval->_combo_deque)
        p_combo = p_combo->clone(p_combo->tick());
    return retval;
}

void GyroTracker::update(combo::ptr p_armor) {
    if (p_armor == nullptr)
        RMVL_Error(RMVL_StsBadArg, "Input argument \"p_armor\" is nullptr.");
    // Reset the vanish number
    updateFromCombo(p_armor);
    _combo_deque.emplace_front(p_armor);
    // 更新装甲板类型
    const auto &armor_state = p_armor->state();
    if (armor_state.contains("robot"))
        updateType(to_robot_type(armor_state.at("robot")));
    else
        updateType(RobotType::UNKNOWN);
    // 帧差时间计算
    if (_combo_deque.empty())
        RMVL_Error(RMVL_StsBadSize, "\"_combo_deque\" is empty");
    _duration = (_combo_deque.size() >= 2)
                    ? (_combo_deque.front()->tick() - _combo_deque.back()->tick()) / static_cast<double>(_combo_deque.size() - 1)
                    : para::gyro_tracker_param.SAMPLE_INTERVAL;
    if (std::isnan(_duration))
        RMVL_Error(RMVL_StsDivByZero, "\"t\" is nan");
    // 更新滤波器
    updatePositionFilter();
    updatePoseFilter();
    // 计算旋转角速度
    _rotspeed = calcRotationSpeed();
    // 容量上限 32
    if (_combo_deque.size() > 32U)
        _combo_deque.pop_back();
}

void GyroTracker::updateType(RobotType robot) {
    if (robot != RobotType::UNKNOWN)
        _type_deque.push_back(robot);
    if (_type_deque.size() > 32)
        _type_deque.pop_back();
    if (_type_deque.size() < 32)
        _state["robot"] = to_string(calculateModeNum(_type_deque.begin(), _type_deque.end()));
}

/**
 * @brief 计算两个向量在 `xOz` 平面的夹角
 *
 * @param[in] start 起始向量
 * @param[in] end 终止向量
 * @return 夹角（按照叉乘方向区分正负，俯视图顺时针为正，弧度）
 */
static inline float calcAngleFrom2Vec(const cv::Vec2f &start, const cv::Vec2f &end) {
    if (start == cv::Vec2f{})
        RMVL_Error(RMVL_StsBadArg, "\"start\" is (0, 0)");
    if (end == cv::Vec2f{})
        RMVL_Error(RMVL_StsBadArg, "\"end\" is (0, 0)");
    cv::Vec3f start3f{start(0), 0, start(1)}, end3f{end(0), 0, end(1)};
    return asin(start3f.cross(end3f)(1) / (norm(start3f) * norm(end3f)));
}

float GyroTracker::calcRotationSpeed() {
    // 容量判断
    size_t pose_num = size() >= 4 ? 4 : size();
    if (pose_num < 2)
        return 0.f;
    // 逐差计算速度，pose 为 combo 指向 center，现需要取反
    float rotspeed = calcAngleFrom2Vec(-Armor::cast(at(pose_num - 1))->getPose(),
                                       -Armor::cast(at(0))->getPose()) /
                     (static_cast<float>(pose_num - 1) * _duration);
    // 速度限幅
    float abs_rotspeed = abs(rotspeed);
    if (abs_rotspeed > para::gyro_tracker_param.MAX_ROTSPEED)
        return sgn(rotspeed) * para::gyro_tracker_param.MAX_ROTSPEED;
    else if (abs_rotspeed < para::gyro_tracker_param.MIN_ROTSPEED)
        return sgn(rotspeed) * para::gyro_tracker_param.MIN_ROTSPEED;
    else
        return rotspeed;
}

} // namespace rm

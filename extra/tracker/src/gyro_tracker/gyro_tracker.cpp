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

#include "rmvl/tracker/gyro_tracker.h"
#include "rmvlpara/tracker/gyro_tracker.h"

using namespace cv;
using namespace std;
using namespace para;
using namespace rm;

void GyroTracker::updateFromCombo(combo::ptr p_combo)
{
    _height = p_combo->getHeight();
    _width = p_combo->getWidth();
    _angle = p_combo->getAngle();
    _center = p_combo->getCenter();
    _relative_angle = p_combo->getRelativeAngle();
    _corners = p_combo->getCorners();
    _extrinsic = p_combo->getExtrinsics();
    _pose = Armor::cast(p_combo)->getPose();
}

GyroTracker::GyroTracker(combo::ptr p_armor)
{
    if (p_armor == nullptr)
        RMVL_Error(RMVL_StsBadArg, "Input argument \"p_armor\" is nullptr.");
    updateFromCombo(p_armor);

    _extrinsic = p_armor->getExtrinsics();
    _type = p_armor->getType();
    _combo_deque.emplace_front(p_armor);
    _type_deque.emplace_front(_type.RobotTypeID);
    _sample_time = gyro_tracker_param.SAMPLE_INTERVAL / 1000.f;
    initFilter();
}

void GyroTracker::update(combo::ptr p_armor, int64, const GyroData &)
{
    if (p_armor == nullptr)
        RMVL_Error(RMVL_StsBadArg, "Input argument \"p_armor\" is nullptr.");
    // Reset the vanish number
    updateFromCombo(p_armor);
    _combo_deque.emplace_front(p_armor);
    // 更新装甲板类型
    updateType(p_armor->getType());
    // 帧差时间计算
    if (_combo_deque.empty())
        RMVL_Error(RMVL_StsBadSize, "\"_combo_deque\" is empty");
    _sample_time = 0.f;
    if (_combo_deque.size() >= 2)
        _sample_time = (_combo_deque.front()->getTick() - _combo_deque.back()->getTick()) /
                        static_cast<double>(_combo_deque.size() - 1) / getTickFrequency();
    else
        _sample_time = gyro_tracker_param.SAMPLE_INTERVAL / 1000.f;
    if (isnan(_sample_time))
        RMVL_Error(RMVL_StsDivByZero, "\"t\" is nan");
    // 更新滤波器
    updateMotionFilter();
    updatePositionFilter();
    updatePoseFilter();
    // 计算旋转角速度
    _rotspeed = calcRotationSpeed();
    // 容量上限 32
    if (_combo_deque.size() > 32U)
        _combo_deque.pop_back();
}

void GyroTracker::updateType(RMStatus stat)
{
    if (_type.RobotTypeID == RobotType::UNKNOWN || stat.RobotTypeID != RobotType::UNKNOWN)
        _type_deque.emplace_front(stat.RobotTypeID);
    if (_type_deque.size() > 32)
        _type_deque.pop_back();
    if (_type_deque.size() < 32)
        _type.RobotTypeID = calculateModeNum(_type_deque.begin(), _type_deque.end());
}

/**
 * @brief 计算两个向量在 `xOz` 平面的夹角
 *
 * @param[in] start 起始向量
 * @param[in] end 终止向量
 * @return 夹角（按照叉乘方向区分正负，俯视图顺时针为正，弧度）
 */
static inline float calcAngleFrom2Vec(const Vec2f &start, const Vec2f &end)
{
    if (start == Vec2f{})
        RMVL_Error(RMVL_StsBadArg, "\"start\" is (0, 0)");
    if (end == Vec2f{})
        RMVL_Error(RMVL_StsBadArg, "\"end\" is (0, 0)");
    Vec3f start3f{start(0), 0, start(1)}, end3f{end(0), 0, end(1)};
    return asin(start3f.cross(end3f)(1) / (norm(start3f) * norm(end3f)));
}

float GyroTracker::calcRotationSpeed()
{
    // 容量判断
    size_t pose_num = size() >= 4 ? 4 : size();
    if (pose_num < 2)
        return 0.f;
    // 逐差计算速度，pose 为 combo 指向 center，现需要取反
    float rotspeed = calcAngleFrom2Vec(-Armor::cast(at(pose_num - 1))->getPose(),
                                       -Armor::cast(at(0))->getPose()) /
                     (static_cast<float>(pose_num - 1) * _sample_time);
    // 速度限幅
    float abs_rotspeed = abs(rotspeed);
    if (abs_rotspeed > gyro_tracker_param.MAX_ROTSPEED)
        return sgn(rotspeed) * gyro_tracker_param.MAX_ROTSPEED;
    else if (abs_rotspeed < gyro_tracker_param.MIN_ROTSPEED)
        return sgn(rotspeed) * gyro_tracker_param.MIN_ROTSPEED;
    else
        return rotspeed;
}

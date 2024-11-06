/**
 * @file sync.cpp
 * @author RoboMaster Vision Community
 * @brief 装甲序列组同步源文件
 * @version 0.1
 * @date 2023-02-10
 *
 * @copyright Copyright 2023 (c), RoboMaster Vision Community
 *
 */

#include "rmvl/group/gyro_group.h"
#include "rmvl/core/transform.hpp"
#include "rmvl/tracker/gyro_tracker.h"

#include "rmvlpara/camera/camera.h"
#include "rmvlpara/tracker/gyro_tracker.h"

namespace rm
{

using namespace rm::numeric_literals;

void GyroGroup::sync(const ImuData &imu_data, double tick)
{
    // -----------------【前置错误与边界条件判断】-----------------
    if (static_cast<int>(_trackers.size()) != _armor_num)
        RMVL_Error_(RMVL_StsBadArg, "Bad size of the \"_trackers\", size = %zu", _trackers.size());
    for (auto &p_tracker : _trackers)
        if (p_tracker->getVanishNumber() == 0 && p_tracker->size() < 2)
            return;
    // ------------ 更新消失帧数，取序列消失帧数最少者 ------------
    _vanish_num = (*min_element(_trackers.begin(), _trackers.end(), [](tracker::const_ptr lhs, tracker::const_ptr rhs) {
                      return lhs->getVanishNumber() < rhs->getVanishNumber();
                  }))->getVanishNumber();
    // ---------------------【提取可见的追踪器】---------------------
    std::vector<tracker::ptr> visible_trackers; // 可见追踪器
    for (auto &p_tracker : _trackers)
        if (p_tracker->getVanishNumber() == 0)
            visible_trackers.push_back(p_tracker);
    // --------------【更新序列组的时间点IMU 数据】--------------
    _tick = tick;
    _imu_data = imu_data;
    // ----【计算出待更新至序列组整体状态的必要信息，并完成同步】----
    size_t visible_num = visible_trackers.size();
    if (visible_num > 2)
        RMVL_Error_(RMVL_StsBadSize, "Bad size of the visible trackers, size = %zu", visible_num);
    if (_armor_num < 4 && visible_num > 1)
        RMVL_Error_(RMVL_StsBadArg, "Bad size of the augument \"_armor_num\" (size = %d) or \"visible_num\" (size = %zu)",
                    _armor_num, visible_num);
    // 获取可见追踪器的中心点
    std::vector<cv::Matx33f> Rs(visible_num); // 旋转矩阵
    std::vector<cv::Vec2f> Ps(visible_num);   // 姿态法向量
    std::vector<cv::Vec3f> ts(visible_num);   // 平移向量
    std::vector<float> rs(visible_num);       // 旋转半径
    for (size_t i = 0; i < visible_num; ++i)
    {
        Rs[i] = visible_trackers[i]->extrinsics().R();
        Ps[i] = GyroTracker::cast(visible_trackers[i])->getPose();
        ts[i] = visible_trackers[i]->extrinsics().tvec();
        rs[i] = _tracker_state[visible_trackers[i]].radius();
    }
    if (visible_num != 0)
    {
        calcGroupFrom3DMessage(Ps, ts, rs, _center3d);
        // 考虑 delta_y 修正 center3d
        if (visible_num == 1)
            _center3d(1) -= _tracker_state[visible_trackers.front()].delta_y();
    }
    // 获取旋转角速度与平均采样时间
    float rotation_speed{};
    float sample_time{};
    for (const auto &p_tracker : _trackers)
    {
        auto p_gyro_tracker = GyroTracker::cast(p_tracker);
        rotation_speed += p_gyro_tracker->getRotatedSpeed();
        sample_time += p_gyro_tracker->getDuration();
    }
    if (std::isnan(rotation_speed))
        RMVL_Error(RMVL_StsNotaNumber, "\"rotation_speed\" is not a number");
    _rotspeed = rotation_speed / static_cast<float>(_trackers.size());
    sample_time /= static_cast<float>(_trackers.size());
    // 中心点滤波，并同步二维点 center
    updateCenter3DFilter(_center3d, sample_time, _center3d, _speed3d);
    auto I = cv::Matx33f::eye();
    cv::Vec3f cam_tvec;
    Armor::imuConvertToCamera(I, _center3d, _imu_data, I, cam_tvec);
    _center = cameraConvertToPixel(para::camera_param.cameraMatrix, para::camera_param.distCoeffs, cam_tvec);
    // 旋转角速度滤波
    updateRotationFilter(_rotspeed, _rotspeed);
    // 更新旋转状态
    updateRotStatus();
    // --------------------【同步不可见的追踪器】--------------------
    if (visible_num == 0)
    {
        // 掉帧处理，利用自身历史信息完成强制构造与更新操作
        float theta = _rotspeed * sample_time;
        for (auto &[p_tracker, current_state] : _tracker_state)
        {
            auto p_gyro_tracker = GyroTracker::cast(p_tracker);
            // 绕 y 轴旋转
            auto rot = euler2Mat(theta, Y);
            // 旋转中心到组合体的线段向量
            cv::Vec2f tmp = -p_gyro_tracker->getPose() * current_state.radius();
            cv::Vec3f center2combo(tmp(0), 0, tmp(1));
            cv::Matx33f new_rmat = rot * p_tracker->extrinsics().R();                                    // 新的旋转矩阵
            cv::Vec3f new_tvec = _center3d + rot * center2combo + cv::Vec3f(0, current_state.delta_y(), 0); // 新的平移向量
            auto p_armor = constructComboForced(p_tracker->front(), _imu_data, new_rmat, new_tvec, _tick);
            p_tracker->update(p_armor);
        }
    }
    // 利用可见追踪器信息，完成强制构造与不可见追踪器的更新
    else if (visible_num == 1)
    {
        tracker::ptr visible_tracker = visible_trackers.front();
        auto &visible_state = _tracker_state[visible_tracker];
        cv::Vec2f tmp = -GyroTracker::cast(visible_tracker)->getPose() * _tracker_state[visible_tracker].radius();
        cv::Vec3f center2combo(tmp(0), 0, tmp(1));
        for (int i = 0; i < _armor_num - 1; ++i)
        {
            // 寻找当前不可见追踪器的下标
            auto p_tracker = _trackers[(visible_state.index() + i + 1) % _armor_num];
            if (p_tracker == visible_tracker)
                RMVL_Error(RMVL_StsError, "The \"visible_tracker\" is equal to \"p_tracker\"");
            auto &current_state = _tracker_state[p_tracker];
            // 绕 y 轴旋转
            auto rot = euler2Mat(static_cast<float>(2_PI / _armor_num * static_cast<double>((i + 1))), Y);
            cv::Matx33f new_rmat = rot * visible_tracker->extrinsics().R();                              // 新的旋转矩阵
            cv::Vec3f new_tvec = _center3d + rot * center2combo + cv::Vec3f(0, current_state.delta_y(), 0); // 新的平移向量
            auto p_armor = constructComboForced(visible_tracker->front(), _imu_data, new_rmat, new_tvec, _tick);
            p_tracker->update(p_armor);
        }
    }
    else // visible_num == 2
    {
        // 判断是否相邻，不相邻则抛出异常
        size_t idx1 = _tracker_state[visible_trackers.front()].index();
        size_t idx2 = _tracker_state[visible_trackers.back()].index();
        if ((idx1 + 1) % 4 != idx2 && (idx2 + 1) % 4 != idx1)
            RMVL_Error_(RMVL_StsError, "The indexes of the 2 visible trackers are not adjacent. It needs to "
                                       "satisfy the formula \"(idx1 + 1) % 4 == idx2\", but idx1 = %zu, idx2 = %zu.",
                        idx1, idx2);
        // 计算高度差：当前与 3D 中心点 y 轴高度差
        for (size_t i = 0; i < visible_num; ++i)
            _tracker_state[visible_trackers[i]].delta_y(ts[i](1) - _center3d(1));
        std::vector<cv::Vec3f> center2combo(visible_num);
        for (size_t i = 0; i < visible_num; ++i)
        {
            cv::Vec2f tmp = -GyroTracker::cast(visible_trackers[i])->getPose() *
                            _tracker_state[visible_trackers[i]].radius();
            center2combo[i] = cv::Vec3f(tmp(0), 0, tmp(1));
        }
        for (size_t i = 0; i < visible_num; ++i)
        {
            auto p_tracker = _trackers[(_tracker_state[visible_trackers[i]].index() + 2) % 4];
            auto &current_state = _tracker_state[p_tracker];
            // 绕 y 轴旋转
            auto rot = euler2Mat(static_cast<float>(PI), Y);
            // 平移向量的旋转增量
            cv::Matx33f new_rmat = rot * visible_trackers[i]->extrinsics().R();                             // 新旋转矩阵
            cv::Vec3f new_tvec = _center3d + rot * center2combo[i] + cv::Vec3f(0, current_state.delta_y(), 0); // 新平移向量
            auto p_armor = constructComboForced(visible_trackers[i]->front(), _imu_data, new_rmat, new_tvec, _tick);
            // 同步高度差
            current_state.delta_y(_tracker_state[visible_trackers[i]].delta_y());
            p_tracker->update(p_armor);
        }
    }
    // ----------------------【更新 RobotType】----------------------
    std::vector<RobotType> robot_type_vec;
    for (auto p_tracker : _trackers)
        if (p_tracker->type().RobotTypeID != RobotType::UNKNOWN)
            robot_type_vec.push_back(p_tracker->type().RobotTypeID);
    _type.RobotTypeID = robot_type_vec.empty() ? RobotType::UNKNOWN : calculateModeNum(robot_type_vec.begin(), robot_type_vec.end());
}

} // namespace rm

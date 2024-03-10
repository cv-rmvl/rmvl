/**
 * @file gyro_decider.cpp
 * @author RoboMaster Vision Community
 * @brief 装甲板模块决策器
 * @version 0.1
 * @date 2023-01-11
 *
 * @copyright Copyright 2023 (c), RoboMaster Vision Community
 *
 */

#include "rmvl/decider/gyro_decider.h"
#include "rmvl/group/gyro_group.h"
#include "rmvl/rmath/transform.h"
#include "rmvl/tracker/gyro_tracker.h"

#include "rmvlpara/camera/camera.h"
#include "rmvlpara/decider/gyro_decider.h"

namespace rm
{

GyroDecider::GyroDecider()
{
    std::string type_priority = para::gyro_decider_param.TYPE_PRIORITY;
    if (type_priority.length() != 9)
        RMVL_Error_(RMVL_StsBadArg, "The length of the TYPE_PRIORITY is unequal to 9, but %zu", type_priority.length());
    _priority = {{RobotType::UNKNOWN, type_priority.at(0) - '0'},
                 {RobotType::HERO, type_priority.at(1) - '0'},
                 {RobotType::ENGINEER, type_priority.at(2) - '0'},
                 {RobotType::INFANTRY_3, type_priority.at(3) - '0'},
                 {RobotType::INFANTRY_4, type_priority.at(4) - '0'},
                 {RobotType::INFANTRY_5, type_priority.at(5) - '0'},
                 {RobotType::OUTPOST, type_priority.at(6) - '0'},
                 {RobotType::BASE, type_priority.at(7) - '0'},
                 {RobotType::SENTRY, type_priority.at(8) - '0'}};
}

/**
 * @brief 计算预测量
 *
 * @param[in] p_group 参考的 group
 * @param[in] p_tracker 参考的 tracker
 * @param[in] translation_dp 平移预测增量
 * @param[in] rotangle 旋转预测角度增量
 * @param[out] point_p2d 目标预测点 (2D)
 * @param[out] point_p3d 目标预测点 (3D)
 * @param[out] point_dp 目标转角预测增量
 */
static void calcPrediction(group::ptr p_group, tracker::const_ptr p_tracker, cv::Vec3f translation_dp, float rotangle,
                           cv::Point2f &point_p2d, cv::Point3f &point_p3d, cv::Point2f &point_dp)
{
    GyroGroup::ptr p_gyro_group = GyroGroup::cast(p_group);
    if (p_gyro_group == nullptr)
        RMVL_Error(RMVL_BadDynamicType, "Fail to cast the type of \"p_group\" to \"GyroGroup::ptr\"");
    // 旋转预测增量的分量计算
    float c = cos(rotangle), s = sin(rotangle);
    const auto &tvec = p_tracker->getExtrinsics().tvec();
    cv::Vec3f center2combo_pose = tvec - p_gyro_group->getCenter3D(); // 旋转中心到 combo 的向量
    cv::Matx33f rot = {c, 0, s,
                       0, 1, 0,
                       -s, 0, c};
    cv::Vec3f rotation_dp = rot * center2combo_pose - center2combo_pose;
    // 运动合成，计算 3D 预测增量
    cv::Vec3f motion_dp = translation_dp + rotation_dp;
    cv::Vec3f motion_p = tvec + motion_dp;
    // 陀螺仪坐标系转相机坐标系
    cv::Matx33f I = cv::Matx33f::eye();
    Armor::gyroConvertToCamera(I, motion_p, p_gyro_group->getGyroData(), I, motion_p);
    // 解算 3D 预测值
    point_p3d = motion_p;
    // 解算 2D 预测值
    point_p2d = cameraConvertToPixel(para::camera_param.cameraMatrix,
                                     para::camera_param.distCoeffs, point_p3d);
    // 解算预测角度增量
    point_dp = calculateRelativeAngle(para::camera_param.cameraMatrix, point_p2d) -
               p_tracker->getRelativeAngle();
}

/**
 * @brief 计算高速状态下的基础响应
 *
 * @param[in] target_group 目标序列组
 * @param[in] target_tracker 目标追踪器
 * @param[in] predict_center3d 目标序列组的 3D 预测点
 * @return 高速状态下的基础响应
 */
static cv::Point2f calculateHighSpeedBasicResponse(group::ptr target_group, tracker::ptr target_tracker,
                                                   const cv::Point3f &predict_center3d)
{
    // pitch 最大振幅角度
    float pitch_max = target_tracker->getRelativeAngle().y;
    // pitch 最小振幅角度
    cv::Point3f min_pitch3d;
    min_pitch3d.x = GyroGroup::cast(target_group)->getCenter3D()(0);
    for (const auto &p_target : target_group->data())
        min_pitch3d.y += p_target->getExtrinsics().tvec()(1);
    min_pitch3d.y /= static_cast<float>(target_group->data().size());
    min_pitch3d.z = predict_center3d.z;
    auto min_pitch_target2d = cameraConvertToPixel(para::camera_param.cameraMatrix, para::camera_param.distCoeffs, min_pitch3d);
    auto angle_tmp = calculateRelativeAngle(para::camera_param.cameraMatrix, min_pitch_target2d);
    float pitch_min = angle_tmp.y;
    // pitch 目标角度
    return {angle_tmp.x, para::gyro_decider_param.PITCH_RESPONSE_DELAY * (pitch_max * para::gyro_decider_param.PITCH_RESPONSE_AMP +
                                                                          pitch_min * (1 - para::gyro_decider_param.PITCH_RESPONSE_AMP))};
}

DecideInfo GyroDecider::decide(const std::vector<group::ptr> &groups, RMStatus,
                               tracker::ptr last_target, const DetectInfo &,
                               const CompensateInfo &compensate_info, const PredictInfo &predict_info)
{
    // 决策信息
    DecideInfo info{};
    // 若没有序列组，则返回
    if (groups.empty() || groups.front()->data().empty())
        return info;
    // -------------------------【选择目标序列组】-------------------------
    // 初始化目标序列组
    group::ptr target_group = nullptr;
    // 若上一帧有目标序列组，将上一帧目标序列组作为目标
    if (last_target != nullptr)
        for (const auto &p_group : groups)
            for (const auto &p_tracker : p_group->data())
                if (p_tracker == last_target)
                    target_group = p_group;
    // 当目标没有被锁定或目标序列组不存在，按数字决策顺序选择目标
    if (!target_group)
        target_group = getHighestPriorityGroup(groups);

    // ------------------------【选择目标追踪器】------------------------
    // 目标追踪器
    info.target = getClosestTracker(target_group, predict_info);
    if (info.target)
    {
        // -------------------------【决策数据解算】-------------------------
        RotStatus rot_status = GyroGroup::cast(target_group)->getRotStatus();
        auto p_gyro_tracker = GyroTracker::cast(info.target);

        const auto &pKt = predict_info.dynamic_prediction.at(info.target);
        const auto &pB = predict_info.static_prediction.at(info.target);
        const auto &pBs = predict_info.shoot_delay_prediction.at(info.target);
        auto tdKt = cv::Vec3f(pKt(POS_X), pKt(POS_Y), pKt(POS_Z));
        auto tdB = cv::Vec3f(pB(POS_X), pB(POS_Y), pB(POS_Z));
        auto comp = compensate_info.compensation.at(info.target);

        // 低速状态跟随装甲板
        if (rot_status == RotStatus::LOW_ROT_SPEED)
        {
            // --------------------【提取用于响应的预测量】--------------------
            cv::Point2f resp_p2d; // 预测值 (2D)
            cv::Point3f resp_p3d; // 预测值 (3D)
            cv::Point2f resp_dp;  // 预测增量

            calcPrediction(target_group, info.target, tdKt + tdB, pKt(ANG_Y) + pB(ANG_Y),
                           resp_p2d, resp_p3d, resp_dp);
            // --------------------【提取用于发弹的预测量】--------------------
            cv::Point2f shoot_p2d; // 预测值 (2D)
            cv::Point3f shoot_p3d; // 预测值 (3D)
            cv::Point2f shoot_dp;  // 预测增量
            calcPrediction(target_group, info.target, tdKt, pKt(ANG_Y), shoot_p2d, shoot_p3d, shoot_dp);
            // 更新决策信息
            info.exp_angle = info.target->getRelativeAngle();
            info.exp_angle += resp_dp; // 考虑预测
            info.exp_angle += comp;    // 考虑补偿
            info.exp_center2d = shoot_p2d;
            info.exp_center3d = shoot_p3d;
        }
        // 高速状态瞄准中轴线
        else
        {
            // --------------------【提取用于响应的预测量】--------------------
            cv::Point2f resp_x_p2d; // yaw 预测值 (2D)
            cv::Point3f resp_x_p3d; // yaw 预测值 (3D)
            cv::Point2f resp_x_dp;  // yaw 预测增量
            calcPrediction(target_group, info.target, tdKt + tdB, 0, resp_x_p2d, resp_x_p3d, resp_x_dp);

            cv::Point2f resp_y_p2d; // pitch 预测值 (2D)
            cv::Point3f resp_y_p3d; // pitch 预测值 (3D)
            cv::Point2f resp_y_dp;  // pitch 预测增量
            calcPrediction(target_group, info.target, tdKt + tdB, pKt(ANG_Y) + pB(ANG_Y) + pBs(ANG_Y),
                           resp_y_p2d, resp_y_p3d, resp_y_dp);

            // --------------------【提取用于发弹的预测量】--------------------
            cv::Point2f shoot_p2d; // 动态预测值 (2D)
            cv::Point3f shoot_p3d; // 动态预测值 (3D)
            cv::Point2f shoot_dp;  // 动态预测增量
            calcPrediction(target_group, info.target, tdKt, pKt(ANG_Y) + pBs(ANG_Y), shoot_p2d, shoot_p3d, shoot_dp);

            info.exp_angle = calculateHighSpeedBasicResponse(target_group, info.target, shoot_p3d);
            info.exp_angle += cv::Point2f(resp_x_dp.x, resp_y_dp.y); // 考虑预测
            info.exp_angle += comp;                                  // 考虑补偿
            info.exp_center2d = shoot_p2d;
            info.exp_center3d = shoot_p3d;
        }

        // 判断能否进行射击
        if (info.target != nullptr)
        {
            // 将补偿的角度换算为坐标点
            info.shoot_center = calculateRelativeCenter(para::camera_param.cameraMatrix, -comp);
            if (getDistance(info.exp_center2d, info.shoot_center) <=
                para::gyro_decider_param.NORMAL_RADIUS_RATIO * info.target->front()->getHeight())
                info.can_shoot = true;
        }
    }
    return info;
}

tracker::ptr GyroDecider::getClosestTracker(group::ptr p_group, const PredictInfo &info)
{
    if (p_group == nullptr)
        return nullptr;
    if (p_group->empty())
        return nullptr;
    const auto &trackers = p_group->data();

    // 无用数据
    cv::Point2f unused_pa, unused_pb;

    return *min_element(trackers.begin(), trackers.end(), [&](tracker::const_ptr lhs, tracker::const_ptr rhs) {
        cv::Point3f lhs_preaim_p3d; // 左操作数预测值 (3D)
        float lhs_preaim_angle = info.dynamic_prediction.at(lhs)(ANG_Y) +
                                 info.shoot_delay_prediction.at(lhs)(ANG_Y) +
                                 para::gyro_decider_param.PRE_AIM_ANGLE * sgn(GyroGroup::cast(p_group)->getRotatedSpeed());
        calcPrediction(p_group, lhs, {}, lhs_preaim_angle, unused_pa, lhs_preaim_p3d, unused_pb);

        cv::Point3f rhs_preaim_p3d; // 右操作数预测值 (3D)
        float rhs_preaim_angle = info.dynamic_prediction.at(rhs)(ANG_Y) +
                                 info.shoot_delay_prediction.at(rhs)(ANG_Y) +
                                 para::gyro_decider_param.PRE_AIM_ANGLE * sgn(GyroGroup::cast(p_group)->getRotatedSpeed());
        calcPrediction(p_group, rhs, {}, rhs_preaim_angle, unused_pa, rhs_preaim_p3d, unused_pb);

        return getDistance(lhs_preaim_p3d, cv::Point3f(), CalPlane::xOz) <
               getDistance(rhs_preaim_p3d, cv::Point3f(), CalPlane::xOz);
    });
}

group::ptr GyroDecider::getHighestPriorityGroup(const std::vector<group::ptr> &groups)
{
    if (groups.empty())
        return nullptr;
    // 获取最优 group
    auto max_group = *max_element(groups.begin(), groups.end(), [&](group::const_ptr g1, group::ptr g2) {
        GyroGroup::const_ptr lhs = GyroGroup::cast(g1);
        GyroGroup::const_ptr rhs = GyroGroup::cast(g2);
        auto type_lhs = g1->getType().RobotTypeID;
        auto type_rhs = g1->getType().RobotTypeID;
        // 需要满足: 优先级 rhs > lhs
        if (_priority.find(type_lhs) != _priority.end() &&
            _priority.find(type_rhs) != _priority.end() &&
            _priority[type_lhs] != _priority[type_rhs])
            return _priority[type_lhs] < _priority[type_rhs];
        else
            return getDistance(lhs->getCenter3D(), cv::Vec3f(), CalPlane::xOz) >
                   getDistance(rhs->getCenter3D(), cv::Vec3f(), CalPlane::xOz);
    });
    // 当前最高优先级
    if (_priority.find(max_group->getType().RobotTypeID) != _priority.end() &&
        _priority[max_group->getType().RobotTypeID] <= 0)
        return nullptr;
    else
        return max_group;
}

} // namespace rm

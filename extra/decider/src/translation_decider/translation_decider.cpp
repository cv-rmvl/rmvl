/**
 * @file translation_decider.cpp
 * @author RoboMaster Vision Community
 * @brief 平移目标决策器
 * @version 2.0
 * @date 2022-03-18
 *
 * @copyright Copyright 2023 (c), RoboMaster Vision Community
 *
 */

#include "rmvl/decider/translation_decider.h"
#include "rmvl/rmath.hpp"

#include "rmvlpara/camera.hpp"
#include "rmvlpara/decider/translation_decider.h"

using namespace rm;
using namespace para;
using namespace std;
using namespace cv;

DecideInfo TranslationDecider::decide(const vector<group_ptr> &groups, RMStatus, const tracker_ptr &last_target,
                                      const DetectInfo &detect_info, const CompensateInfo &compensate_info,
                                      const PredictInfo &predict_info)
{
    // 决策信息
    DecideInfo info{};
    vector<tracker_ptr> trackers;
    for (auto &p_group : groups)
    {
        const auto &p_trackers = p_group->data();
        trackers.insert(trackers.end(), p_trackers.begin(), p_trackers.end());
    }
    if (trackers.empty())
        return info;
    // 获取、更新目标追踪器
    if (last_target != nullptr)
        for (const auto &p_tracker : trackers)
            if (p_tracker == last_target)
                info.target = p_tracker;
    // 未右键时，每帧更新，实时选择离图像中心最近的追踪器
    if (info.target == nullptr)
        info.target = getClosestTracker(detect_info.src, trackers);
    // 判断是否正在追踪
    if (info.target)
    {
        const auto &dKt = predict_info.dynamic_prediction.at(info.target);
        const auto &dB = predict_info.static_prediction.at(info.target);
        const auto &comp = compensate_info.compensation.at(info.target);

        auto angle_dKt = Point2f(dKt(YAW), dKt(PITCH));
        auto angle_dB = Point2f(dB(YAW), dB(PITCH));

        info.exp_angle = info.target->getRelativeAngle();
        info.exp_angle += angle_dKt + angle_dB; // 加入预测
        info.exp_angle += comp;                 // 加入补偿
        info.exp_center2d = calculateRelativeCenter(camera_param.cameraMatrix, angle_dKt);

        // 判断能否进行射击
        if (info.target != nullptr)
        {
            // 将补偿的角度换算为坐标点
            info.shoot_center = calculateRelativeCenter(camera_param.cameraMatrix, -comp);

            // 判断是否满足发弹条件
            if (getDistance(info.exp_center2d, info.shoot_center) <=
                translation_decider_param.NORMAL_RADIUS_RATIO * info.target->front()->getHeight())
                info.can_shoot = true;
        }
    }
    return info;
}

tracker_ptr TranslationDecider::getClosestTracker(Mat img, const vector<tracker_ptr> &trackers)
{
    auto center = Point2f(img.size() / 2);
    return *min_element(trackers.begin(), trackers.end(),
                        [&](const tracker_ptr &lhs, const tracker_ptr &rhs)
                        {
                            Point2f t1_delta_point = lhs->front()->getCenter() - center;
                            Point2f t2_delta_point = rhs->front()->getCenter() - center;
                            // 目标相对角距图像坐标系中心点距离
                            return (t1_delta_point.x * t1_delta_point.x + t1_delta_point.y * t1_delta_point.y) <
                                   (t2_delta_point.x * t2_delta_point.x + t2_delta_point.y * t2_delta_point.y);
                        });
}

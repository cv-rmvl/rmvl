/**
 * @file ArmorPredictor.cpp
 * @author RoboMaster Vision Community
 * @brief 装甲板预测派生类
 * @version 2.0
 * @date 2022-08-24
 *
 * @copyright Copyright 2021 (c), RoboMaster Vision Community
 *
 */

#include "rmvl/predictor/armor_predictor.h"

#include "rmvlpara/camera/camera.h"
#include "rmvlpara/predictor/armor_predictor.h"

namespace rm
{

PredictInfo ArmorPredictor::predict(const std::vector<group::ptr> &groups, const std::unordered_map<tracker::ptr, double> &tof)
{
    // 预测信息
    PredictInfo info{};
    for (auto p_group : groups)
    {
        for (auto p_tracker : p_group->data())
        {
            double tf = (tof.find(p_tracker) == tof.end()) ? 0. : tof.at(p_tracker);
            // 获取陀螺仪角速度
            cv::Point2f gyro_speed = {p_tracker->front()->getGyroData().rotation.yaw_speed,
                                      p_tracker->front()->getGyroData().rotation.pitch_speed};
            // 计算用于预测的合成角速度
            cv::Point2f speed = p_tracker->getSpeed() + gyro_speed;
            double dB_yaw = speed.x * para::armor_predictor_param.YAW_B;
            double dB_pitch = speed.y * para::armor_predictor_param.PITCH_B;
            double dKt_yaw = speed.x * para::armor_predictor_param.YAW_K * tf;
            double dKt_pitch = speed.y * para::armor_predictor_param.PITCH_K * tf;

            // 更新预测信息
            info.static_prediction[p_tracker][YAW] = dB_yaw;
            info.static_prediction[p_tracker][PITCH] = dB_pitch;
            info.dynamic_prediction[p_tracker][YAW] = dKt_yaw;
            info.dynamic_prediction[p_tracker][PITCH] = dKt_pitch;
        }
    }
    return info;
}

} // namespace rm

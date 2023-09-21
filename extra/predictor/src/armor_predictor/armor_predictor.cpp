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

#include "rmvlpara/camera.hpp"
#include "rmvlpara/predictor/armor_predictor.h"

using namespace rm;
using namespace para;
using namespace std;
using namespace cv;

PredictInfo ArmorPredictor::predict(const vector<group_ptr> &groups,
                                    const unordered_map<tracker_ptr, double> &tof)
{
    // 预测信息
    PredictInfo info{};
    for (auto p_group : groups)
    {
        for (auto p_tracker : p_group->data())
        {
            double tf = (tof.find(p_tracker) == tof.end()) ? 0. : tof.at(p_tracker);

            double dB_yaw = p_tracker->getSpeed().x * armor_predictor_param.YAW_B;
            double dB_pitch = p_tracker->getSpeed().y * armor_predictor_param.PITCH_B;
            double dKt_yaw = p_tracker->getSpeed().x * armor_predictor_param.YAW_K * tf;
            double dKt_pitch = p_tracker->getSpeed().y * armor_predictor_param.PITCH_K * tf;

            // 更新预测信息
            info.static_prediction[p_tracker][YAW] = dB_yaw;
            info.static_prediction[p_tracker][PITCH] = dB_pitch;
            info.dynamic_prediction[p_tracker][YAW] = dKt_yaw;
            info.dynamic_prediction[p_tracker][PITCH] = dKt_pitch;
        }
    }
    return info;
}
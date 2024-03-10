/**
 * @file gyro_predictor.cpp
 * @author RoboMaster Vision Community
 * @brief 装甲板预测派生类
 * @version 0.1
 * @date 2023-01-09
 *
 * @copyright Copyright 2023 (c), RoboMaster Vision Community
 *
 */

#include "rmvl/predictor/gyro_predictor.h"
#include "rmvl/group/gyro_group.h"
#include "rmvl/tracker/gyro_tracker.h"

#include "rmvlpara/predictor/gyro_predictor.h"

namespace rm
{

PredictInfo GyroPredictor::predict(const std::vector<group::ptr> &groups,
                                   const std::unordered_map<tracker::ptr, double> &tof)
{
    // 预测信息
    PredictInfo info{};
    for (auto p_group : groups)
    {
        auto p_gyro_group = GyroGroup::cast(p_group);
        for (auto p_tracker : p_group->data())
        {
            double tf = (tof.find(p_tracker) == tof.end()) ? 0. : tof.at(p_tracker);
            auto p_gyro_tracker = GyroTracker::cast(p_tracker);
            // 平移
            auto dKt_T = p_gyro_group->getSpeed3D() * para::gyro_predictor_param.K * tf;
            auto dB_T = p_gyro_group->getSpeed3D() * para::gyro_predictor_param.B;
            // 旋转
            auto dKt_R = p_gyro_group->getRotatedSpeed() * para::gyro_predictor_param.K * tf;
            auto dB_R = p_gyro_group->getRotatedSpeed() * para::gyro_predictor_param.B;
            auto dBs_R = p_gyro_group->getRotatedSpeed() * para::gyro_predictor_param.SHOOT_B;
            // 更新预测量
            auto &dynamic_vec = info.dynamic_prediction[p_tracker];
            dynamic_vec(POS_X) = dKt_T(0);
            dynamic_vec(POS_Y) = dKt_T(1);
            dynamic_vec(POS_Z) = dKt_T(2);
            dynamic_vec(ANG_Y) = dKt_R;
            auto &static_vec = info.static_prediction[p_tracker];
            static_vec(POS_X) = dB_T(0);
            static_vec(POS_Y) = dB_T(1);
            static_vec(POS_Z) = dB_T(2);
            static_vec(ANG_Y) = dB_R;
            auto &shoot_delay_vec = info.shoot_delay_prediction[p_tracker];
            shoot_delay_vec(ANG_Y) = dBs_R;
        }
    }
    return info;
}

} // namespace rm

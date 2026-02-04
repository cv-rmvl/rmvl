/**
 * @file RunePredictor.cpp
 * @author RoboMaster Vision Community
 * @brief 神符预测派生类
 * @version 2.0
 * @date 2022-08-28
 *
 * @copyright Copyright 2021 (c), RoboMaster Vision Community
 *
 */

#include "rmvl/predictor/rune_predictor.h"
#include "rmvl/tracker/rune_tracker.h"

#include "rmvlpara/predictor/rune_predictor.h"

namespace rm {

RunePredictorInfo RunePredictor::predict(const std::vector<RuneTracker::ptr> &trackers, const std::unordered_map<tracker::ptr, double> &tof) {
    // 预测信息
    RunePredictorInfo info{};
    for (auto p_tracker : trackers) {
        double tf = (tof.find(p_tracker) == tof.end()) ? 0. : tof.at(p_tracker);
        // Kt + B 预测模型
        double dKt = p_tracker->getRotatedSpeed() * para::rune_predictor_param.PREDICT_K * tf;
        double dB = p_tracker->getRotatedSpeed() * para::rune_predictor_param.PREDICT_B;
        // 更新预测量
        info.dynamic_prediction.at(p_tracker) = dKt;
        info.static_prediction.at(p_tracker) = dB;
    }
    return info;
}

} // namespace rm

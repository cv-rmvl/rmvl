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
#include "rmvl/group/rune_group.h"
#include "rmvl/tracker/rune_tracker.h"

#include "rmvlpara/camera/camera.h"
#include "rmvlpara/predictor/rune_predictor.h"
#include "rmvlpara/tracker/rune_tracker.h"

namespace rm
{

PredictInfo RunePredictor::predict(const std::vector<group::ptr> &groups, const std::unordered_map<tracker::ptr, double> &tof)
{
    if (groups.size() != 1)
        RMVL_Error(RMVL_StsBadSize, "Size of the groups is not equal to \'1\'");
    auto rune_group = RuneGroup::cast(groups.front()); // 转换为神符 group 子类
    if (rune_group == nullptr)
        RMVL_Error(RMVL_BadDynamicType, "Dynamic type of the group::ptr is not equal to Runegroup::ptr");
    // 预测信息
    PredictInfo info{};
    for (auto p_tracker : rune_group->data())
    {
        auto p_rune_tracker = RuneTracker::cast(p_tracker);
        double tf = (tof.find(p_tracker) == tof.end()) ? 0. : tof.at(p_tracker);
        // Kt + B 预测模型
        double dKt = p_rune_tracker->getRotatedSpeed() * para::rune_predictor_param.PREDICT_K * tf;
        double dB = p_rune_tracker->getRotatedSpeed() * para::rune_predictor_param.PREDICT_B;
        // 更新预测量
        auto &dynamic_vec = info.dynamic_prediction[p_tracker];
        dynamic_vec[ANG_Z] = dKt;
        auto &static_vec = info.static_prediction[p_tracker];
        static_vec[ANG_Z] = dB;
    }
    return info;
}

} // namespace rm

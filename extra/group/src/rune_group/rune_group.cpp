/**
 * @file rune_group.h
 * @author RoboMaster Vision Community
 * @brief 神符序列组源文件
 * @version 1.0
 * @date 2022-08-27
 *
 * @copyright Copyright 2023 (c), RoboMaster Vision Community
 *
 */

#include "rmvl/group/rune_group.h"
#include "rmvl/core/util.hpp"

#include "rmvlpara/group/rune_group.h"

namespace rm {

void RuneGroup::sync(const ImuData &, double) {
    if (_trackers.empty())
        RMVL_Error(RMVL_StsBadSize, "trackers of the \"rune_group\" is empty!");
    double raw_data{};
    if (_datas.empty())
        raw_data = static_cast<double>(_trackers[0]->angle());
    else {
        size_t trackers_num = _trackers.size();
        // ----------------- 角度连续化处理 -----------------
        double last_data = _datas.front();
        std::vector<double> cur_angles(trackers_num);
        cur_angles[0] = _trackers[0]->angle();
        // 角度向 cur_angles[0] 对齐
        for (size_t i = 1; i < trackers_num; ++i) {
            double cur_angle = _trackers[i]->angle();
            double n = round((cur_angle - cur_angles[0]) / para::rune_group_param.INTERVAL_ANGLE);
            cur_angles[i] = cur_angle - n * para::rune_group_param.INTERVAL_ANGLE;
        }
        // 取平均得到 cur_data
        double cur_data{};
        for (auto cur_angle : cur_angles)
            cur_data += cur_angle;
        cur_data /= static_cast<double>(trackers_num);
        // cur_data 向 last_data 对齐
        double n = round((cur_data - last_data) / para::rune_group_param.INTERVAL_ANGLE);
        raw_data = cur_data - n * para::rune_group_param.INTERVAL_ANGLE;
    }
    _datas.push_front(raw_data);
    if (_datas.size() > para::rune_group_param.RAW_DATAS_SIZE)
        _datas.pop_back();
}

group::ptr RuneGroup::clone() {
    auto retval = std::make_shared<RuneGroup>(*this);
    // 更新内部所有追踪器
    for (auto &p_tracker : retval->_trackers)
        p_tracker = p_tracker->clone();
    return retval;
}

} // namespace rm

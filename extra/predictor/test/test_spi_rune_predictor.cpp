/**
 * @file test_spi_rune_predictor.cpp
 * @author RoboMaster Vision Community
 * @brief
 * @version 1.0
 * @date 2023-06-23
 *
 * @copyright Copyright 2023 (c), RoboMaster Vision Community
 *
 */

#include "rmvl/rmvl_modules.hpp"

#ifdef HAVE_RMVL_SPI_RUNE_PREDICTOR

#include <gtest/gtest.h>

#define private public
#define protected public

#include "rmvl/predictor/spi_rune_predictor.h"

#undef private
#undef protected

#include "rmvl/algorithm/math.hpp"
#include "rmvl/core/timer.hpp"
#include "rmvl/group/rune_group.h"
#include "rmvl/tracker/rune_tracker.h"

#include "rmvlpara/predictor/spi_rune_predictor.h"

using namespace rm;

namespace rm_test {

TEST(Run_Accuracy, data_from_0_300) {
    auto p_predictor = SpiRunePredictor::make_predictor();
    auto p_center = RuneCenter::make_feature(cv::Point(500, 500));
    auto p_target = RuneTarget::make_feature(cv::Point(600, 500), false);
    auto p_rune = Rune::make_combo(p_target, p_center, ImuData(), Time::now(), true);
    auto p_tracker = RuneTracker::make_tracker(p_rune);
    std::unordered_map<tracker::ptr, double> tof;
    tof.emplace(p_tracker, 0.02);
    auto p_group = RuneGroup::make_group();
    p_group->add(p_tracker);

    for (int i = 0; i < 300; ++i) {
        cv::Point target_point(500 + 100 * cos(deg2rad(i)),
                               500 - 100 * sin(deg2rad(i)));
        auto new_target = RuneTarget::make_feature(cv::Point(600, 500), false);
        auto imu_data = ImuData{};
        double tick = Time::now();

        auto new_rune = Rune::make_combo(new_target, p_center, imu_data, tick, true);
        p_tracker->update(new_rune);
        p_group->sync(imu_data, tick);

        // predict
        p_predictor->predict(p_group, tof);
    }
}

TEST(PredictModel, uniform_data_from_0_600) {
    auto p_predictor = SpiRunePredictor::make_predictor();
    std::deque<double> datas;
    for (int i = 0; i < 600; ++i) {
        // 获取原始数据
        datas.push_front(static_cast<double>(i) + 0.1);
        if (datas.size() > para::spi_rune_predictor_param.MAX_NF + para::spi_rune_predictor_param.DIFF_ORDER + 2)
            datas.pop_back();
        // 递推
        p_predictor->identifier(datas);
        // 预测（必须满足 x * SAMPLE_INTERVAL < MAX_NF，当前 x = 0.2）
        double pre = p_predictor->anglePredict(datas, 0.2);
        if (datas.size() == 1) {
            EXPECT_EQ(pre, 0);
        } else if (datas.size() < para::spi_rune_predictor_param.MAX_NF + para::spi_rune_predictor_param.DIFF_ORDER) {
            EXPECT_EQ(pre, para::spi_rune_predictor_param.FIXED_ANGLE);
        }
        if (i > 300) {
            EXPECT_GE(pre, para::spi_rune_predictor_param.FIXED_ANGLE / 2);
        }
    }
}

TEST(PredictModel, sin_data_from_0_600) {
}

} // namespace rm_test

#endif // HAVE_RMVL_SPI_RUNE_PREDICTOR

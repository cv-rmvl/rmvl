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

#include <gtest/gtest.h>

#define private public
#define protected public

#include "rmvl/predictor/spi_rune_predictor.h"

#undef private
#undef protected

#include "rmvl/group/rune_group.h"
#include "rmvl/tracker/rune_tracker.h"

#include "rmvlpara/predictor/spi_rune_predictor.h"

using namespace rm;
using namespace para;
using namespace std;
using namespace cv;

namespace rm_test
{

TEST(Run_Accuracy, data_from_0_300)
{
    auto p_predictor = SpiRunePredictor::make_predictor();
    auto p_center = RuneCenter::make_feature(Point(500, 500));
    auto p_target = RuneTarget::make_feature(Point(600, 500), false);
    auto p_rune = Rune::make_combo(p_target, p_center, GyroData(), getTickCount(), true);
    auto p_tracker = RuneTracker::make_tracker(p_rune);
    unordered_map<tracker::ptr, double> tof;
    tof.emplace(p_tracker, 0.02);
    auto p_group = RuneGroup::make_group();
    p_group->add(p_tracker);
    vector<group::ptr> groups = {p_group};

    for (int i = 0; i < 300; ++i)
    {
        Point target_point(500 + 100 * cos(deg2rad(i)),
                           500 - 100 * sin(deg2rad(i)));
        auto new_target = RuneTarget::make_feature(Point(600, 500), false);
        auto gyro_data = GyroData{};
        int64_t tick = getTickCount();

        auto new_rune = Rune::make_combo(new_target, p_center, gyro_data, tick, true);
        p_tracker->update(new_rune, tick, gyro_data);
        p_group->sync(gyro_data, tick);

        // predict
        p_predictor->predict(groups, tof);
    }
}

TEST(PredictModel, uniform_data_from_0_600)
{
    auto p_predictor = SpiRunePredictor::make_predictor();
    deque<double> datas;
    for (int i = 0; i < 600; ++i)
    {
        // 获取原始数据
        datas.push_front(static_cast<double>(i) + 0.1);
        if (datas.size() > spi_rune_predictor_param.MAX_NF + spi_rune_predictor_param.DIFF_ORDER + 2)
            datas.pop_back();
        // 递推
        p_predictor->identifier(datas);
        // 预测（必须满足 x * SAMPLE_INTERVAL < MAX_NF，当前 x = 0.2）
        double pre = p_predictor->anglePredict(datas, 0.2);
        if (datas.size() == 1)
        {
            EXPECT_EQ(pre, 0);
        }
        else if (datas.size() < spi_rune_predictor_param.MAX_NF + spi_rune_predictor_param.DIFF_ORDER)
        {
            EXPECT_EQ(pre, spi_rune_predictor_param.FIXED_ANGLE);
        }
        if (i > 300)
        {
            EXPECT_GE(pre, spi_rune_predictor_param.FIXED_ANGLE / 2);
        }
    }
}

TEST(PredictModel, sin_data_from_0_600)
{
}

} // namespace rm_test
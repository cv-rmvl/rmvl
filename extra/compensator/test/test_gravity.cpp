/**
 * @file test_gravity.cpp
 * @author zhaoxi (535394140@qq.com)
 * @brief 重力补偿单元测试
 * @version 1.0
 * @date 2024-03-03
 *
 * @copyright Copyright 2024 (c), zhaoxi
 *
 */

#include <gtest/gtest.h>

#define private public
#define protected public

#include "../src/gravity_compensator/gravity_impl.h"

#undef private
#undef protected

#include "rmvl/core/math.hpp"
#include "rmvlpara/compensator/gravity_compensator.h"

using namespace rm::numeric_literals;

namespace rm_test
{

TEST(GravityCompensator, bulletModel)
{
    rm::GravityCompensator::Impl impl;
    auto [y_fric, t_fric] = impl.bulletModel(10, 16, 45_to_rad);
    // 无阻力情况下
    double t = 10 / (16 * cos(rm::PI_4));
    double y = 16 * sin(rm::PI_4) * t - 0.5 * rm::para::gravity_compensator_param.g * t * t;
    EXPECT_NEAR(y, y_fric, 5e-2);
    EXPECT_NEAR(t, t_fric, 5e-2);
}

} // namespace rm_test

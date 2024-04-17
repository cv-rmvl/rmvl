/**
 * @file test_kalman.cpp
 * @author zhaoxi (535394140@qq.com)
 * @brief kalman 模块单元测试
 * @version 1.0
 * @date 2024-04-18
 *
 * @copyright Copyright 2024 (c), zhaoxi
 *
 */

#include <gtest/gtest.h>

#include "rmvl/core/kalman.hpp"

#include <random>

namespace rm_test
{

// 一维匀速运动 KF 测试
TEST(KalmanTest, kf)
{
    // 状态量   x  = [ x  v ]^T
    // 状态方程 Fa = ┌ x + vt     A = ┌ 1  T ┐
    //               └ v              └ 0  1 ┘
    // 观测量   z  = x
    // 观测方程 Fh = x            H = [ 1  0 ]

    rm::KF21d kf;
    kf.init({50, 0}, 1e5);
    kf.setQ(1e-1 * cv::Matx22d::eye());
    kf.setR({1e-3});
    double t{0.01};

    cv::Matx21d x{};
    for (int i = 0; i <= 100; i++)
    {
        // 预测部分，并获取先验状态估计（弃值）
        kf.setA({1, t,
                 0, 1});
        kf.predict();
        // 更新部分，并获取后验状态估计
        kf.setH({1, 0});
        x = kf.correct({50 + 0.3 * i}); // 以 0.3/T 的速度运动
    }
    EXPECT_NEAR(x(0), 80, 1e-2); // 50 + 0.3 * 100
    EXPECT_NEAR(x(1), 30, 1e-2); // 0.3 / 0.01
}

// 二维匀速直线运动 KF 测试
TEST(KalmanTest, kf2)
{
    // 状态量   x  = [ x, y, u, v ]^T
    //               ┌ x + uT        ┌ 1  0  T  0 ┐
    // 状态方程 Fa = │ y + vT    A = │ 0  1  0  T │
    //               │ u             │ 0  0  1  0 │
    //               └ v             └ 0  0  0  1 ┘
    // 观测量   z  = [ x, y ]^T
    // 观测方程 Fh = ┌ x         H = ┌ 1  0  0  0 ┐
    //               └ y             └ 0  1  0  0 ┘

    rm::KF42d kf;
    kf.init({50, 50, 0, 0}, 1e5);
    kf.setQ(1e-1 * cv::Matx44d::eye());
    kf.setR({1e-3, 1e-3});
    double t{0.01};

    cv::Matx41d x{};
    for (int i = 0; i <= 100; i++)
    {
        // 预测部分，并获取先验状态估计（弃值）
        kf.setA({1, 0, t, 0,
                 0, 1, 0, t,
                 0, 0, 1, 0,
                 0, 0, 0, 1});
        kf.predict();
        // 更新部分，并获取后验状态估计
        kf.setH({1, 0, 0, 0,
                 0, 1, 0, 0});
        x = kf.correct({50 + 0.4 * i, 50 + 0.2 * i}); // 以 (0.4/T, 0.2/T) 的速度运动
    }
    EXPECT_NEAR(x(0), 90, 1e-2); // 50 + 0.4 * 100
    EXPECT_NEAR(x(1), 70, 1e-2); // 50 + 0.2 * 100
    EXPECT_NEAR(x(2), 40, 1e-2); // 0.4 / 0.01
    EXPECT_NEAR(x(3), 20, 1e-2); // 0.2 / 0.01
}

// 二维匀速圆周运动 EKF 测试
TEST(KalmanTest, ekf)
{
    // 状态量   x = [ cx, cy, θ, ω, r ]^T
    //              ┌ cx                  ┌ 1  0  0  0  0 ┐
    //              │ cy                  │ 0  1  0  0  0 │
    // 状态方程 F = │ θ + ωT         Ja = │ 0  0  1  T  0 │ = A
    //              │ ω                   │ 0  0  0  1  0 │
    //              └ r                   └ 0  0  0  0  1 ┘
    // 观测量   z = [ px, py, θ ]^T
    //              ┌ cx + rcosθ          ┌ 1  0 -rsinθ  0  cosθ ┐
    // 观测方程 H = │ cy + rsinθ     Jh = │ 0  1  rcosθ  0  sinθ │
    //              └ θ                   └ 0  0    1    0    0  ┘

    // 正态分布噪声
    std::default_random_engine ng;
    std::normal_distribution<double> err{0, 1};

    rm::EKF53d ekf;
    ekf.init({0, 0, 0, 0, 150}, 1e5);
    ekf.setQ(1e-1 * cv::Matx<double, 5, 5>::eye());
    ekf.setR(cv::Matx33d::diag({1e-3, 1e-3, 1e-3}));
    double t{0.01};
    ekf.setFa([=](const cv::Matx<double, 5, 1> &x) -> cv::Matx<double, 5, 1> {
        return {x(0),
                x(1),
                x(2) + x(3) * t,
                x(3),
                x(4)};
    });
    ekf.setFh([=](const cv::Matx<double, 5, 1> &x) -> cv::Matx<double, 3, 1> {
        return {x(0) + x(4) * std::cos(x(2)),
                x(1) + x(4) * std::sin(x(2)),
                x(2)};
    });

    cv::Matx<double, 5, 1> x{};

    for (int i = 0; i <= 200; i++)
    {
        // 预测部分，并获取先验状态估计
        ekf.setJa({1, 0, 0, 0, 0,
                   0, 1, 0, 0, 0,
                   0, 0, 1, t, 0,
                   0, 0, 0, 1, 0,
                   0, 0, 0, 0, 1});
        auto x_ = ekf.predict();
        // 更新部分，并获取后验状态估计
        ekf.setJh({1, 0, -x_(4) * std::sin(x_(2)), 0, std::cos(x_(2)),
                   0, 1, x_(4) * std::cos(x_(2)), 0, std::sin(x_(2)),
                   0, 0, 1, 0, 0});
        x = ekf.correct({500 + 200 * std::cos(0.02 * i) + err(ng), // 以 20 为半径，0.02/T 的角速度运动（图像上是顺时针），并加上噪声
                         500 + 200 * std::sin(0.02 * i) + err(ng),
                         0.02 * i + 0.01 * err(ng)});
    }
    EXPECT_NEAR(x(0), 500, 1);
    EXPECT_NEAR(x(1), 500, 1);
    EXPECT_NEAR(x(2), 4, 0.1); // 0.02 * 200
    EXPECT_NEAR(x(3), 2, 0.1); // 0.02 / 0.01
}

} // namespace rm_test

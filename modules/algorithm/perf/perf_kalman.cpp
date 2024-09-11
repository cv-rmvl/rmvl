/**
 * @file perf_kalman.cpp
 * @author zhaoxi (535394140@qq.com)
 * @brief 卡尔曼滤波器基准测试
 * @version 1.0
 * @date 2024-04-26
 *
 * @copyright Copyright 2024 (c), zhaoxi
 *
 */

#ifdef HAVE_OPENCV
#include <fstream>
#include <benchmark/benchmark.h>
#include <opencv2/video/tracking.hpp>

#include "rmvl/algorithm/kalman.hpp"

namespace rm_test
{

// 4 状态量 2 观测量的 rm::KalmanFilter
void kalman42_rmvl(benchmark::State &state)
{
    for (auto _ : state)
    {
        rm::KalmanFilter<float, 4, 2> kf;
        kf.setQ(cv::Matx44f::diag({0.1, 0.1, 0.1, 0.1}));
        kf.setR(cv::Matx22f::diag({1e-3f, 1e-3f}));
        kf.init(cv::Matx41f::zeros(), 1e5);
        cv::Matx41f xk;
        std::ofstream ofs("kalman42_rmvl.txt");
        for (int i = 0; i < 1000; i++)
        {
            float t{0.01f};
            // 预测
            kf.setA({1, 0, t, 0,
                     0, 1, 0, t,
                     0, 0, 1, 0,
                     0, 0, 0, 1});
            kf.predict();
            // 更新
            cv::Matx21f z(1.f * i, 2.f * i);
            xk = kf.correct(z);
        }
        ofs << xk << std::endl;
    }
}

// 4 状态量 2 观测量的 cv::KalmanFilter
void kalman42_opencv(benchmark::State &state)
{
    for (auto _ : state)
    {
        cv::KalmanFilter kf(4, 2);
        kf.processNoiseCov = cv::Mat::eye(4, 4, CV_32F) * 0.1;
        kf.measurementNoiseCov = cv::Mat::eye(2, 2, CV_32F) * 1e-3;
        kf.errorCovPost = cv::Mat::eye(4, 4, CV_32F) * 1e5;
        cv::Mat xk;
        std::fstream ofs("kalman42_opencv.txt");
        for (int i = 0; i < 1000; i++)
        {
            float t{0.01f};
            // 预测
            kf.transitionMatrix = (cv::Mat_<float>(4, 4) << 1, 0, t, 0,
                                   0, 1, 0, t,
                                   0, 0, 1, 0,
                                   0, 0, 0, 1);
            kf.predict();
            // 更新
            cv::Mat z = (cv::Mat_<float>(2, 1) << 1.f * i, 2.f * i);
            kf.measurementMatrix = cv::Mat::eye(2, 4, CV_32F);
            xk = kf.correct(z);
        }
        ofs << xk << std::endl;
    }
}

BENCHMARK(kalman42_rmvl)->Name("kf (x_dim: 4, z_dim: 2) - by rmvl  ")->Iterations(10);
BENCHMARK(kalman42_opencv)->Name("kf (x_dim: 4, z_dim: 2) - by opencv")->Iterations(10);

} // namespace rm_test

#endif // HAVE_OPENCV

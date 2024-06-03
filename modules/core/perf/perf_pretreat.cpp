/**
 * @file perf_pretreat.cpp
 * @author zhaoxi (535394140@qq.com)
 * @brief 
 * @version 1.0
 * @date 2024-06-05
 * 
 * @copyright Copyright 2024 (c), zhaoxi
 * 
 */

#include <benchmark/benchmark.h>

#include <opencv2/imgproc.hpp>

#include "rmvl/core/pretreat.hpp"

namespace rm_test
{

// binary 方法通道相减
void binary_BGR2Binary(benchmark::State &state)
{
    cv::Mat channel[3];
    channel[0] = cv::Mat(cv::Size(400, 1024), CV_8UC3, cv::Scalar(255, 0, 0));
    channel[1] = cv::Mat(cv::Size(400, 1024), CV_8UC3, cv::Scalar(0, 255, 0));
    channel[2] = cv::Mat(cv::Size(400, 1024), CV_8UC3, cv::Scalar(0, 0, 255));
    cv::Mat src;
    hconcat(channel, 3, src);
    for (auto _ : state)
        cv::Mat bin = binary(src, rm::BLUE, rm::RED, 80);
}

// threshold 方法通道相减
void threshold_BGR2Binary(benchmark::State &state)
{
    cv::Mat channel[3];
    channel[0] = cv::Mat(1024, 400, CV_8UC3, {255, 0, 0});
    channel[1] = cv::Mat(1024, 400, CV_8UC3, {0, 255, 0});
    channel[2] = cv::Mat(1024, 400, CV_8UC3, {0, 0, 255});
    cv::Mat src;
    hconcat(channel, 3, src);
    for (auto _ : state)
    {
        cv::Mat chs[3];
        split(src, chs);
        cv::Mat bin = chs[0] - chs[2];
        threshold(bin, bin, 80, 255, cv::THRESH_BINARY);
    }
}

// binary 方法三通道亮度二值化
void binary_brightness(benchmark::State &state)
{
    cv::Mat channel[3];
    channel[0] = cv::Mat(1024, 400, CV_8UC3, cv::Scalar(255, 0, 0));
    channel[1] = cv::Mat(1024, 400, CV_8UC3, cv::Scalar(0, 255, 0));
    channel[2] = cv::Mat(1024, 400, CV_8UC3, cv::Scalar(0, 0, 255));
    cv::Mat src;
    hconcat(channel, 3, src);
    for (auto _ : state)
        cv::Mat bin = rm::binary(src, 80);
}

// threshold 方法单通道亮度二值化
void threshold_brightness(benchmark::State &state)
{
    cv::Mat channel[3];
    channel[0] = cv::Mat(cv::Size(400, 1024), CV_8UC3, cv::Scalar(255, 0, 0));
    channel[1] = cv::Mat(cv::Size(400, 1024), CV_8UC3, cv::Scalar(0, 255, 0));
    channel[2] = cv::Mat(cv::Size(400, 1024), CV_8UC3, cv::Scalar(0, 0, 255));
    cv::Mat src;
    hconcat(channel, 3, src);
    for (auto _ : state)
    {
        cv::Mat bin;
        cvtColor(src, bin, cv::COLOR_BGR2GRAY);
        threshold(bin, bin, 80, 255, cv::THRESH_BINARY);
    }
}

BENCHMARK(binary_BGR2Binary)->Name("Chns Minus -   RMVL")->Iterations(20);
BENCHMARK(threshold_BGR2Binary)->Name("Chns Minus - OpenCV")->Iterations(20);
BENCHMARK(binary_brightness)->Name("Brightness -   RMVL")->Iterations(20);
BENCHMARK(threshold_brightness)->Name("Brightness - OpenCV")->Iterations(20);

} // namespace rm_test

/**
 * @file perf_pretreat.cpp
 * @author RoboMaster Vision Community
 * @brief
 * @version 1.0
 * @date 2022-11-24
 *
 * @copyright Copyright 2023 (c), RoboMaster Vision Community
 *
 */

#include <benchmark/benchmark.h>
#include <iostream>
#include <tuple>

#include <opencv2/imgproc.hpp>

#include "rmvl/imgproc/pretreat.h"

using namespace rm;
using namespace std;
using namespace cv;

namespace rm_test
{

// binary 方法通道相减
void binary_BGR2Binary(benchmark::State &state)
{
    Mat channel[3];
    channel[0] = Mat(Size(400, 1024), CV_8UC3, Scalar(255, 0, 0));
    channel[1] = Mat(Size(400, 1024), CV_8UC3, Scalar(0, 255, 0));
    channel[2] = Mat(Size(400, 1024), CV_8UC3, Scalar(0, 0, 255));
    Mat src;
    hconcat(channel, 3, src);
    for (auto _ : state)
    {
        Mat bin = binary(src, BLUE, RED, 80);
    }
}

// threshold 方法通道相减
void threshold_BGR2Binary(benchmark::State &state)
{
    Mat channel[3];
    channel[0] = Mat(Size(400, 1024), CV_8UC3, Scalar(255, 0, 0));
    channel[1] = Mat(Size(400, 1024), CV_8UC3, Scalar(0, 255, 0));
    channel[2] = Mat(Size(400, 1024), CV_8UC3, Scalar(0, 0, 255));
    Mat src;
    hconcat(channel, 3, src);
    for (auto _ : state)
    {
        Mat chs[3];
        split(src, chs);
        Mat bin = chs[0] - chs[2];
        threshold(bin, bin, 80, 255, THRESH_BINARY);
    }
}

// binary 方法三通道亮度二值化
void binary_brightness(benchmark::State &state)
{
    Mat channel[3];
    channel[0] = Mat(Size(400, 1024), CV_8UC3, Scalar(255, 0, 0));
    channel[1] = Mat(Size(400, 1024), CV_8UC3, Scalar(0, 255, 0));
    channel[2] = Mat(Size(400, 1024), CV_8UC3, Scalar(0, 0, 255));
    Mat src;
    hconcat(channel, 3, src);
    for (auto _ : state)
    {
        Mat bin = binary(src, 80);
    }
}

// threshold 方法单通道亮度二值化
void threshold_brightness(benchmark::State &state)
{
    Mat channel[3];
    channel[0] = Mat(Size(400, 1024), CV_8UC3, Scalar(255, 0, 0));
    channel[1] = Mat(Size(400, 1024), CV_8UC3, Scalar(0, 255, 0));
    channel[2] = Mat(Size(400, 1024), CV_8UC3, Scalar(0, 0, 255));
    Mat src;
    hconcat(channel, 3, src);
    for (auto _ : state)
    {
        Mat bin;
        cvtColor(src, bin, COLOR_BGR2GRAY);
        threshold(bin, bin, 80, 255, THRESH_BINARY);
    }
}

BENCHMARK(binary_BGR2Binary)->Iterations(100);
BENCHMARK(threshold_BGR2Binary)->Iterations(100);
BENCHMARK(binary_brightness)->Iterations(100);
BENCHMARK(threshold_brightness)->Iterations(100);

} // namespace rm_test

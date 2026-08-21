/**
 * @file perf_para.cpp
 * @brief rm::yaml 与 OpenCV FileStorage 参数加载性能对比
 */

#include <cstdio>

#include <benchmark/benchmark.h>

#include <opencv2/core/persistence.hpp>

#include "rmvlpara/camera/camera.h"

namespace rm_test {

namespace {

constexpr auto para_path = "rmvl_camera_para_benchmark.yml";

struct ParaFile {
    ParaFile() {
        cv::FileStorage storage(para_path, cv::FileStorage::WRITE | cv::FileStorage::FORMAT_YAML);
        storage << "EULER_0" << 1 << "EULER_1" << 0 << "EULER_2" << 2;
        storage << "cameraMatrix" << cv::Mat(cv::Matx33f{1250.F, 0.F, 640.F, 0.F, 1250.F, 512.F, 0.F, 0.F, 1.F});
        storage << "distCoeffs" << cv::Mat(cv::Matx<float, 5, 1>::zeros());
    }

    ~ParaFile() { std::remove(para_path); }
};

void prepareParaFile() {
    static const ParaFile file;
    benchmark::DoNotOptimize(&file);
}

bool loadWithFileStorage(rm::para::CameraParam &param) {
    cv::FileStorage storage(para_path, cv::FileStorage::READ);
    if (!storage.isOpened())
        return false;
    storage["EULER_0"] >> param.EULER_0;
    storage["EULER_1"] >> param.EULER_1;
    storage["EULER_2"] >> param.EULER_2;
    storage["cameraMatrix"] >> param.cameraMatrix;
    storage["distCoeffs"] >> param.distCoeffs;
    return true;
}

} // namespace

static void BM_ParaLoad_RmYaml(benchmark::State &state) {
    prepareParaFile();
    rm::para::CameraParam param;
    for (auto _ : state)
        benchmark::DoNotOptimize(param.read(para_path));
}

BENCHMARK(BM_ParaLoad_RmYaml);

static void BM_ParaLoad_OpenCvFileStorage(benchmark::State &state) {
    prepareParaFile();
    rm::para::CameraParam param;
    for (auto _ : state)
        benchmark::DoNotOptimize(loadWithFileStorage(param));
}

BENCHMARK(BM_ParaLoad_OpenCvFileStorage);

} // namespace rm_test

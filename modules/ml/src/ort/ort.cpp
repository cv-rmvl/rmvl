/**
 * @file ort.cpp
 * @author RoboMaster Vision Community
 * @brief the deployment library of the ONNXruntime (Ort)
 * @version 1.0
 * @date 2022-02-04
 *
 * @copyright Copyright 2023 (c), RoboMaster Vision Community
 *
 */

#include <algorithm>
#include <numeric>

#include <opencv2/imgproc.hpp>

#include "rmvl/core/util.hpp"
#include "rmvl/ml/ort.h"

namespace rm
{

// 默认配置的内存分配器
static Ort::AllocatorWithDefaultOptions alloc;

OnnxNet::OnnxNet(std::string_view model_path, OrtProvider prov) : _memory_info(Ort::MemoryInfo::CreateCpu(OrtAllocatorType::OrtArenaAllocator, OrtMemType::OrtMemTypeDefault))
{
    if (model_path.empty())
        RMVL_Error(RMVL_StsBadArg, "Model path empty!");

    switch (prov)
    {
    case OrtProvider::CUDA:
        _session_options.AppendExecutionProvider_CUDA({});
        break;
    case OrtProvider::TensorRT:
        _session_options.AppendExecutionProvider_TensorRT({});
        break;
    case OrtProvider::OpenVINO: {
        OrtOpenVINOProviderOptions options;
        options.device_type = "CPU_FP32";
        _session_options.AppendExecutionProvider_OpenVINO(options);
        break;
    }
    default:
        break;
    }

    _session_options.SetGraphOptimizationLevel(GraphOptimizationLevel::ORT_ENABLE_ALL);
    _session = std::make_unique<Ort::Session>(_env, model_path.data(), _session_options);

    // 定义输入输出节点的名称
#if ORT_API_VERSION < 12
    for (std::size_t i = 0; i < _session->GetInputCount(); i++)
        _inames.emplace_back(_session->GetInputName(i, alloc));
    for (std::size_t i = 0; i < _session->GetOutputCount(); i++)
        _onames.emplace_back(_session->GetOutputName(i, alloc));
#else
    for (std::size_t i = 0; i < _session->GetInputCount(); i++)
        _inames.emplace_back(_session->GetInputNameAllocated(i, alloc));
    for (std::size_t i = 0; i < _session->GetOutputCount(); i++)
        _onames.emplace_back(_session->GetOutputNameAllocated(i, alloc));
#endif
}

std::any OnnxNet::inference(const std::vector<cv::Mat> &images, const PreprocessOptions &preop, const PostprocessOptions &postop)
{
    auto itensors = preProcess(images, preop);
#if ORT_API_VERSION < 12
    return postProcess(_session->Run(Ort::RunOptions{nullptr}, _inames.data(), itensors.data(), itensors.size(), _onames.data(), _onames.size()), postop);
#else
    std::vector<const char *> input_names(_inames.size());
    for (std::size_t i = 0; i < _inames.size(); i++)
        input_names[i] = _inames[i].get();
    std::vector<const char *> output_names(_onames.size());
    for (std::size_t i = 0; i < _onames.size(); i++)
        output_names[i] = _onames[i].get();
    return postProcess(_session->Run(Ort::RunOptions{nullptr}, input_names.data(), itensors.data(), itensors.size(), output_names.data(), output_names.size()), postop);
#endif
}

void OnnxNet::printEnvInfo() noexcept
{
    printf("----------------- Build -----------------\n");
#if ORT_API_VERSION < 15
    printf("version: 1.%d (< 1.15.0)\n", ORT_API_VERSION);
#else
    printf("version: %s\n", Ort::GetVersionString().c_str());
#endif
    auto providers = Ort::GetAvailableProviders();
    printf("\n--------------- Providers ---------------\n");
    for (std::size_t i = 0; i < providers.size(); i++)
        printf("  [%zu] %s\n", i, providers[i].c_str());
}

constexpr const char *element_data_type[] = {
    "undefined", "float", "uint8", "int8", "uint16", "int16",
    "int32", "int64", "string", "bool", "float16", "double",
    "uint32", "uint64", "complex64", "complex128", "bfloat16"};

void OnnxNet::printModelInfo() noexcept
{
    if (_session == nullptr)
    {
        printf("the model is empty.\n");
        return;
    }
    printf("-------------- Input Layer --------------\n");
    std::size_t input_node = _session->GetInputCount();
    printf("input node: n = %zu\n", input_node);
    for (size_t i = 0; i < input_node; i++)
    {
#if ORT_API_VERSION < 12
        printf("[%zu]\t┬ name: %s\n", i, _session->GetInputName(i, alloc));
#else
        printf("[%zu]\t┬ name: %s\n", i, _session->GetInputNameAllocated(i, alloc).get());
#endif
        auto input_dims = _session->GetInputTypeInfo(i).GetTensorTypeAndShapeInfo().GetShape();
        printf("\t│ dim: [");
        for (auto dim : input_dims)
            printf("%ld, ", dim);
        printf("\b\b]\n");
        printf("\t└ element type: %s\n", element_data_type[_session->GetInputTypeInfo(i).GetTensorTypeAndShapeInfo().GetElementType()]);
    }

    printf("\n------------- Output  Layer -------------\n");
    std::size_t output_node = _session->GetOutputCount();
    std::printf("output node: n = %zu\n", _session->GetOutputCount());
    for (std::size_t i = 0; i < output_node; i++)
    {
#if ORT_API_VERSION < 12
        printf("[%zu]\t┬ name: %s\n", i, _session->GetInputName(i, alloc));
#else
        printf("[%zu]\t┬ name: %s\n", i, _session->GetOutputNameAllocated(i, alloc).get());
#endif
        auto output_dims = _session->GetOutputTypeInfo(i).GetTensorTypeAndShapeInfo().GetShape();
        printf("\t│ dim: [");
        for (auto dim : output_dims)
            printf("%ld, ", dim);
        printf("\b\b]\n");
        printf("\t└ element type: %s\n", element_data_type[_session->GetOutputTypeInfo(i).GetTensorTypeAndShapeInfo().GetElementType()]);
    }
}

} // namespace rm

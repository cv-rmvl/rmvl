/**
 * @file ort_impl.h
 * @author zhaoxi (535394140@qq.com)
 * @brief the deployment library of the ONNXruntime (Ort)
 * @version 1.0
 * @date 2024-01-25
 *
 * @copyright Copyright 2023 (c), zhaoxi
 *
 */

#pragma once

#include <opencv2/core/mat.hpp>

#include <onnxruntime_cxx_api.h>

#include "rmvl/ml/ort.h"

namespace rm
{

//! ONNX-Runtime (Ort) 部署库 \cite onnx-rt
class OnnxRT::Impl
{
    using session_ptr = std::unique_ptr<Ort::Session>;

    Ort::Env _env;                               //!< 环境配置
    Ort::SessionOptions _session_options;        //!< Session 配置
    Ort::MemoryInfo _memory_info;                //!< Tensor 内存分配信息
    Ort::AllocatorWithDefaultOptions _allocator; //!< 默认配置的内存分配器
    session_ptr _p_session;

    std::vector<std::vector<float>> _input_arrays; //!< 输入数组
    std::vector<const char *> _input_names;        //!< 输入名
    std::vector<const char *> _output_names;       //!< 输出名

public:
    Impl(std::string_view model_path);
    ~Impl() = default;

    void printModelInfo() noexcept;

    std::vector<size_t> inference(const std::vector<cv::Mat> &images, const std::vector<float> &means, const std::vector<float> &stds);

private:
    //! 初始化 Ort 引擎
    void setupEngine(std::string_view model_path) noexcept;

    //! 分配内存，将图像平展为 NCHW 格式的一维数组，同时将数组归一化
    void imageToVector(cv::Mat &input_image, std::vector<float> &input_array,
                       const std::vector<float> &means, const std::vector<float> &stds);

    //! 预处理
    std::vector<Ort::Value> preProcess(const std::vector<cv::Mat> &images, const std::vector<float> &means, const std::vector<float> &stds);

    //! 后处理
    std::vector<size_t> postProcess(const std::vector<Ort::Value> &output_tensors) noexcept;

    //! 推理并返回输出 Tensors
    inline std::vector<Ort::Value> doInference(const std::vector<Ort::Value> &input_tensors) noexcept
    {
        return _p_session->Run(Ort::RunOptions{nullptr}, _input_names.data(), input_tensors.data(),
                               input_tensors.size(), _output_names.data(), _output_names.size());
    }
};

} // namespace rm

/**
 * @file ort.h
 * @author RoboMaster Vision Community
 * @brief the deployment library header file of the ONNXruntime (Ort)
 * @version 1.0
 * @date 2022-02-04
 *
 * @copyright Copyright 2023 (c), RoboMaster Vision Community
 *
 */

#pragma once

#include <opencv2/core/mat.hpp>

#include <onnxruntime_cxx_api.h>

namespace rm
{

//! @addtogroup ml_ort
//! @{

//! ONNX-Runtime (Ort) 部署库 \cite onnx-rt
class OnnxRT
{
    using session_ptr = std::unique_ptr<Ort::Session>;

    Ort::Env _env;                               //!< 环境配置
    Ort::SessionOptions _session_options;        //!< Session 配置
    Ort::MemoryInfo _memory_info;                //!< Tensor 内存分配信息
    Ort::AllocatorWithDefaultOptions _allocator; //!< 默认配置的内存分配器
    session_ptr _pSession;

    std::vector<std::vector<float>> _input_arrays; //!< 输入数组
    std::vector<const char *> _input_names;        //!< 输入名
    std::vector<const char *> _output_names;       //!< 输出名

public:
    /**
     * @brief Construct the OnnxRT object
     *
     * @param[in] model_path 模型路径，如果该路径不存在，则程序将因错误而退出
     */
    OnnxRT(const std::string &model_path);
    ~OnnxRT() = default;

    void printModelInfo();

    /**
     * @brief 预处理，推理和后处理
     *
     * @param[in] images 所有的输入图像
     * @return 与概率最高的值对应的索引向量
     */
    std::vector<size_t> inference(const std::vector<cv::Mat> &images);

private:
    /**
     * @brief 初始化 Ort 引擎
     *
     * @param[in] model_path 模型路径
     */
    void setupEngine(const std::string &model_path);

    /**
     * @brief 分配内存，将图像平展为 NCHW 格式的一维数组，同时将数组归一化
     * @note 参数 input_array 需要被初始化过，其长度需与 input_image 的大小一致
     *
     * @param[in] input_image 输入图像
     * @param[out] input_array 从输入图像输入数组
     */
    void imageToVector(cv::Mat &input_image, std::vector<float> &input_array);

    /**
     * @brief 预处理
     *
     * @param[in] images 所有的输入图像
     * @return 用于网络输入的 Tensors
     */
    std::vector<Ort::Value> preProcess(const std::vector<cv::Mat> &images);

    /**
     * @brief 后处理
     *
     * @param[in] output_tensors 网络输出的 Tensors
     * @return 具有最高可信度的 index 或值
     */
    std::vector<size_t> postProcess(std::vector<Ort::Value> &output_tensors);

    /**
     * @brief 推理并返回输出 Tensors
     *
     * @param[in] input_tensors 输入 Tensors
     * @return 输出 Tensors
     */
    inline std::vector<Ort::Value> doInference(std::vector<Ort::Value> &input_tensors)
    {
        return _pSession->Run(Ort::RunOptions{nullptr}, _input_names.data(), input_tensors.data(),
                              input_tensors.size(), _output_names.data(), _output_names.size());
    }
};

//! @} ml_ort

} // namespace rm

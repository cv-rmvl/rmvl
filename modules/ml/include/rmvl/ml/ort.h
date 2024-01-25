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
public:
    /**
     * @brief 创建 OnnxRT 对象
     *
     * @param[in] model_path 模型路径，如果该路径不存在，则程序将因错误而退出
     */
    OnnxRT(std::string_view model_path);
    ~OnnxRT();

    void printModelInfo();

    /**
     * @brief 预处理，推理和后处理
     *
     * @param[in] images 所有的输入图像
     * @param[in] means 网络模型各通道的均值
     * @param[in] stds 网络模型各通道的标准差
     * @return 与概率最高的值对应的索引向量
     */
    std::vector<size_t> inference(const std::vector<cv::Mat> &images, const std::vector<float> &means, const std::vector<float> &stds);

private:
    class Impl;
    Impl *_pimpl;
};

//! @} ml_ort

} // namespace rm

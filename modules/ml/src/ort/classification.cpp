/**
 * @file classification.cpp
 * @author zhaoxi (535394140@qq.com)
 * @brief
 * @version 1.0
 * @date 2024-06-03
 *
 * @copyright Copyright 2024 (c), zhaoxi
 *
 */

#include "rmvl/core/util.hpp"
#include "rmvl/ml/ort.h"

namespace rm
{

ClassificationNet::ClassificationNet(std::string_view model_path, OrtProvider prov) : OnnxNet(model_path, prov)
{
    // 初始化输入数组
    _iarrays.resize(_session->GetInputCount());
    for (size_t i = 0; i < _session->GetInputCount(); i++)
    {
        auto shape = _session->GetInputTypeInfo(i).GetTensorTypeAndShapeInfo().GetShape();
        _iarrays[i].resize(shape[1] * shape[2] * shape[3]);
    }
}

/**
 * @brief 从 `NHWC` 格式的 `BGR` 图像生成 `NCHW` 格式的输入 `Tensor`
 *
 * @param[in] input_image 输入图像
 * @param[in] means 3 通道各自的均值
 * @param[in] stds 3 通道各自的标准差
 * @param[out] iarray `NCHW` 格式的输入 `Tensor`
 */
static void imageToVector(const cv::Mat &input_image, const std::vector<float> &means, const std::vector<float> &stds, std::vector<float> &iarray)
{
    int H{input_image.rows}, W{input_image.cols};
    RMVL_DbgAssert(static_cast<std::size_t>(3 * H * W) == iarray.size());
    RMVL_Assert(means.size() == 3 && stds.size() == 3);
    // 转 Tensor 的 NCHW 格式，做归一化和标准化
    float *p_input_array = iarray.data();
    for (int c = 0; c < 3; c++)
        for (int h = 0; h < H; h++)
            for (int w = 0; w < W; w++)
                p_input_array[c * H * W + h * W + w] = (input_image.at<uchar>(h, w * 3 + 2 - c) / 255.f - means[c]) / stds[c];
}

/**
 * @brief 从 `NHWC` 格式的灰度图像生成 `NCHW` 格式的输入 `Tensor`
 *
 * @param[in] input_image 输入图像
 * @param[in] mean 均值
 * @param[in] std 标准差
 * @param[out] iarray `NCHW` 格式的输入 `Tensor`
 */
static void imageToVector(const cv::Mat &input_image, float mean, float std, std::vector<float> &iarray)
{
    int H{input_image.rows}, W{input_image.cols};
    RMVL_DbgAssert(static_cast<std::size_t>(H * W) == iarray.size());
    // 转 Tensor 的 NCHW 格式，做归一化和标准化
    float *p_input_array = iarray.data();
    for (int h = 0; h < H; h++)
        for (int w = 0; w < W; w++)
            p_input_array[h * W + w] = (input_image.at<uchar>(h, w) / 255.f - mean) / std;
}

std::vector<Ort::Value> ClassificationNet::preProcess(const std::vector<cv::Mat> &images, const PreprocessOptions &options)
{
    std::size_t input_count = _session->GetInputCount();
    RMVL_Assert(input_count == 1 && images.size() == 1);
    // 获取输入层 Tensor
    std::vector<Ort::Value> input_tensors;
    const cv::Mat &img = images.front();
    // 合法性检查
    auto shape = _session->GetInputTypeInfo(0).GetTensorTypeAndShapeInfo().GetShape();
    RMVL_Assert(shape.size() == 4);
    if (shape[1] != img.channels() || shape[2] != img.rows || shape[3] != img.cols)
        RMVL_Error_(RMVL_StsBadSize, "image (%d, %d, %d) unequal to shape (%d, %d, %d).",
                    img.channels(), img.rows, img.cols, shape[1], shape[2], shape[3]);
    RMVL_Assert(shape[1] == 3 || shape[1] == 1);
    shape[0] = 1;
    // img -> iarray
    RMVL_Assert(!options.means.empty() && !options.stds.empty());
    if (shape[1] == 3)
        imageToVector(img, options.means, options.stds, _iarrays.front());
    else
        imageToVector(img, options.means.front(), options.stds.front(), _iarrays.front());
    // 更新每个输入层的数据
    input_tensors.emplace_back(Ort::Value::CreateTensor<float>(
        _memory_info, _iarrays.front().data(), _iarrays.front().size(), shape.data(), shape.size()));

    return input_tensors;
}

std::any ClassificationNet::postProcess(const std::vector<Ort::Value> &output_tensors, const PostprocessOptions &)
{
    RMVL_Assert(output_tensors.size() == 1);
    auto &output_tensor = output_tensors.front();
    const float *output = output_tensor.GetTensorData<float>();
    std::size_t size{output_tensor.GetTensorTypeAndShapeInfo().GetElementCount()};
    auto maxit = std::max_element(output, output + size);
    return std::make_pair(maxit - output, *maxit);
}

} // namespace rm

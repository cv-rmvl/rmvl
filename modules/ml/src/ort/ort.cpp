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

#include "rmvlpara/ml/ort.h"

using namespace cv;
using namespace std;
using namespace Ort;
using namespace para;

rm::OnnxRT::OnnxRT(const string &model_path) : _env(OrtLoggingLevel::ORT_LOGGING_LEVEL_WARNING, "OnnxDeployment"),
                                                 _memory_info(MemoryInfo::CreateCpu(OrtAllocatorType::OrtArenaAllocator,
                                                                                    OrtMemType::OrtMemTypeDefault))
{
    if (model_path.empty())
        RMVL_Error(RMVL_StsBadArg, "Model path is empty!");
    setupEngine(model_path);
}

void rm::OnnxRT::setupEngine(const string &model_path)
{
#ifdef WITH_ORT_CUDA
    OrtSessionOptionsAppendExecutionProvider_CUDA(session_options, 0);
#endif // WITH_ORT_CUDA

#ifdef WITH_ORT_TensorRT
    OrtSessionOptionsAppendExecutionProvider_Tensorrt(session_options, 0);
#endif // WITH_ORT_TensorRT

    _session_options.SetGraphOptimizationLevel(GraphOptimizationLevel::ORT_ENABLE_ALL);
    _pSession = make_unique<Session>(_env, model_path.c_str(), _session_options);

    // define the names of the I/O nodes
    for (size_t i = 0; i < _pSession->GetInputCount(); i++)
        _input_names.emplace_back(_pSession->GetInputName(i, _allocator));
    for (size_t i = 0; i < _pSession->GetOutputCount(); i++)
        _output_names.emplace_back(_pSession->GetOutputName(i, _allocator));
    // setup input array
    _input_arrays.resize(_pSession->GetInputCount());
    for (size_t i = 0; i < _pSession->GetInputCount(); i++)
    {
        auto shape = _pSession->GetInputTypeInfo(i).GetTensorTypeAndShapeInfo().GetShape();
        _input_arrays[i].resize(shape[1] * shape[2] * shape[3]);
    }
}

vector<size_t> rm::OnnxRT::inference(const vector<Mat> &images)
{
    vector<Value> input_tensors = preProcess(images);
    vector<Value> output_tensors = doInference(input_tensors);
    return postProcess(output_tensors);
}

vector<Value> rm::OnnxRT::preProcess(const vector<Mat> &images)
{
    size_t input_count = _pSession->GetInputCount();
    if (input_count != images.size())
        CV_Error(RMVL_StsBadArg, "Size of the \"images\" are not equal to the model input_count.");
    // get the correct data of each input layer
    vector<Value> input_tensors;
    for (size_t i = 0; i < input_count; i++)
    {
        auto input_shape = _pSession->GetInputTypeInfo(i).GetTensorTypeAndShapeInfo().GetShape();
        // [2], [3] are the correct size of the image
        if (input_shape.size() != 4)
            RMVL_Error(RMVL_StsBadSize, "Size of the input_shape of the model is not equal to \'4\'");
        if (input_shape[2] != input_shape[3])
            RMVL_Error(RMVL_StsError, "Shape of the input_shape[2] and input_shape[3] of the model is unequal");
        input_shape[0] = 1;
        // update the size of each input layer
        Mat input_image;
        resize(images[i], input_image, Size(input_shape[2], input_shape[3]));
        // allocate memory and normalization
        imageToVector(input_image, _input_arrays[i]);
        input_tensors.emplace_back(Value::CreateTensor<float>(_memory_info,
                                                              _input_arrays[i].data(), _input_arrays[i].size(),
                                                              input_shape.data(), input_shape.size()));
    }
    return input_tensors;
}

vector<size_t> rm::OnnxRT::postProcess(vector<Value> &output_tensors)
{
    // 所有输出对应的置信度最高的索引
    vector<size_t> output_indexs;
    for (auto &output_tensor : output_tensors)
    {
        // 获取置信度最高的索引
        const float *output = output_tensor.GetTensorData<float>();
        vector<size_t> indexs(output_tensor.GetTensorTypeAndShapeInfo().GetElementCount());
        iota(indexs.begin(), indexs.end(), 0);
        auto it = max_element(indexs.begin(), indexs.end(),
                              [&output](size_t lhs, size_t rhs)
                              {
                                  return output[lhs] < output[rhs];
                              });
        output_indexs.emplace_back(*it);
    }
    return output_indexs;
}

void rm::OnnxRT::imageToVector(Mat &input_image, vector<float> &input_array)
{
    // CHW
    int C = input_image.channels();
    if (C != 1 && C != 3)
        RMVL_Error_(RMVL_StsBadArg, "Bad channel of the input argument: \"input_image\", chn = %d", C);
    int H = input_image.rows;
    int W = input_image.cols;
    size_t pixels = C * H * W;
    if (pixels != input_array.size())
        RMVL_Error(RMVL_StsBadArg, "The size of the arguments: \"input_image\" and \"input_array\" are not equal");

    vector<double> means(C);
    vector<double> stds(C);
    if (C == 1)
    {
        means[0] = ort_param.MONO_MEANS;
        stds[0] = ort_param.MONO_STDS;
    }
    else
    {
        for (int i = 0; i < C; ++i)
        {
            means[i] = ort_param.RGB_MEANS[i];
            stds[i] = ort_param.RGB_STDS[i];
        }
    }
    // 转 Tensor 的 NCHW 格式，做归一化和标准化
    float *p_input_array = input_array.data();
    for (int c = 0; c < C; c++)
    {
        for (int h = 0; h < H; h++)
        {
            for (int w = 0; w < W; w++)
            {
                p_input_array[c * H * W + h * W + w] = input_image.ptr<uchar>(h)[w * C + 2 - c] / 255.f;
                p_input_array[c * H * W + h * W + w] = (p_input_array[c * H * W + h * W + w] - means[c]) / stds[c];
            }
        }
    }
}

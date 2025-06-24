/**
 * @file ort.h
 * @author RMVL Community
 * @brief the deployment library header file of the ONNXruntime (Ort)
 * @version 1.0
 * @date 2022-02-04
 *
 * @copyright Copyright 2023 (c), RMVL Community
 *
 */

#pragma once

#include <any>
#include <onnxruntime_cxx_api.h>

#include <opencv2/core/mat.hpp>

#include "rmvl/core/rmvldef.hpp"

namespace rm
{

//! @addtogroup ml_ort
//! @{

//! Ort 提供者
enum class OrtProvider : uint8_t
{
    CPU,      //!< 由 `CPU` 执行
    CUDA,     //!< 由 `CUDA` 执行
    TensorRT, //!< 由 `TensorRT` 执行
    OpenVINO  //!< 由 `OpenVINO` 执行
};

//! 预处理选项
struct RMVL_EXPORTS_W_AG PreprocessOptions
{
    RMVL_W_RW std::vector<float> means; //!< 均值
    RMVL_W_RW std::vector<float> stds;  //!< 标准差
};

//! 后处理选项
struct RMVL_EXPORTS_W_AG PostprocessOptions
{
    RMVL_W_RW uint8_t color{};             //!< 颜色通道
    RMVL_W_RW std::vector<float> thresh{}; //!< 阈值向量
};

//! ONNX-Runtime (Ort) 部署库基类 \cite microsoft23ort
class RMVL_EXPORTS_W OnnxNet
{
public:
    /**
     * @brief 创建 OnnxNet 对象
     *
     * @param[in] model_path 模型路径，如果该路径不存在，则程序将因错误而退出
     * @param[in] prov Ort 提供者
     */
    RMVL_W OnnxNet(std::string_view model_path, OrtProvider prov);

    //! 打印环境信息
    RMVL_W static void printEnvInfo() noexcept;
    //! 打印模型信息
    RMVL_W void printModelInfo() noexcept;

    /**
     * @brief 推理
     *
     * @param[in] images 所有输入图像
     * @param[in] preop 预处理选项，不同网络有不同的预处理选项
     * @param[in] postop 后处理选项，不同网络有不同的后处理选项
     * @return 使用 `std::any` 表示的所有推理结果，需要根据具体的网络进行解析
     * @note 可使用 `<class-type>::cast` 函数对返回类型进行转换
     */
    RMVL_W std::any inference(const std::vector<cv::Mat> &images, const PreprocessOptions &preop, const PostprocessOptions &postop);

    virtual ~OnnxNet() = default;

private:
    /**
     * @brief 预处理
     *
     * @param[in] images 所有输入图像
     * @param[in] preop 预处理选项，不同网络有不同的预处理选项
     * @return 模型的输入 Tensors
     */
    virtual std::vector<Ort::Value> preProcess(const std::vector<cv::Mat> &images, const PreprocessOptions &preop);

    /**
     * @brief 后处理
     *
     * @param[in] output_tensors 模型的输出 Tensors
     * @param[in] postop 后处理选项，不同网络有不同的后处理选项
     * @return 使用 `std::any` 表示的所有推理结果，需要根据具体的网络进行解析
     */
    virtual std::any postProcess(const std::vector<Ort::Value> &output_tensors, const PostprocessOptions &postop);

protected:
    Ort::MemoryInfo _memory_info;           //!< 内存分配信息
    Ort::Env _env;                          //!< 环境配置
    Ort::SessionOptions _session_options;   //!< 会话选项
    std::unique_ptr<Ort::Session> _session; //!< 会话
#if ORT_API_VERSION < 12
    std::vector<const char *> _inames; //!< 输入名称
    std::vector<const char *> _onames; //!< 输出名称
#else
    std::vector<Ort::AllocatedStringPtr> _inames; //!< 输入名称
    std::vector<Ort::AllocatedStringPtr> _onames; //!< 输出名称
#endif
};

/**
 * @brief 分类网络推理类
 * @note 需满足
 * @note
 * - 输入层为 `[1, c, h, w]`，其中 `c` 为输入图像的通道数，可以是 `1` 或者 `3`，`h` 为高度，`w` 为宽度
 * @note
 * - 输出层为 `[1, n]`，其中 `n` 为类别数
 */
class RMVL_EXPORTS_W ClassificationNet : public OnnxNet
{
public:
    /**
     * @brief 推理结果转换
     *
     * @param[in] result 使用 `std::any` 表示的推理结果
     * @return 转换后的推理结果，为 `std::pair<int, float>` 类型，表示分类结果及其置信度
     */
    RMVL_W static std::pair<int, float> cast(const std::any &result) { return std::any_cast<std::pair<int, float>>(result); }

    /**
     * @brief 创建分类网络对象
     *
     * @param[in] model_path 模型路径，如果该路径不存在，则程序将因错误而退出
     * @param[in] prov Ort 提供者，默认为 `OrtProvider::CPU`
     */
    RMVL_W ClassificationNet(std::string_view model_path, OrtProvider prov = OrtProvider::CPU);

private:
    /**
     * @brief 预处理
     *
     * @param[in] images 所有输入图像
     * @param[in] options 预处理选项，包含各通道的均值和标准差
     * @return 模型的输入 Tensors
     */
    std::vector<Ort::Value> preProcess(const std::vector<cv::Mat> &images, const PreprocessOptions &options) override;

    /**
     * @brief 后处理
     *
     * @param[in] output_tensors 模型的输出 Tensors
     * @param[in] postop 无需后处理选项，传入 `{}` 即可
     * @return 用 `std::any` 表示的分类结果及其置信度，可使用 `rm::ClassificationNet::cast` 函数对返回类型进行转换
     */
    std::any postProcess(const std::vector<Ort::Value> &output_tensors, const PostprocessOptions &postop) override;

    std::vector<std::vector<float>> _iarrays; //!< 输入数组
};

//! @} ml_ort

} // namespace rm

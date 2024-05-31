/**
 * @file ort_print.cpp
 * @author RoboMaster Vision Community
 * @brief
 * @version 1.0
 * @date 2022-02-04
 *
 * @copyright Copyright 2023 (c), RoboMaster Vision Community
 *
 */

#include "ort_impl.h"

void rm::OnnxRT::Impl::printModelInfo() noexcept
{
    printf("-------------- Input Layer --------------\n");
    std::size_t input_node = _p_session->GetInputCount();
    printf("the number of input node is: %zu\n", input_node);
    for (size_t i = 0; i < input_node; i++)
    {
        printf("[%zu]\t┬ name is: %s\n", i, _p_session->GetInputName(i, _allocator));
        auto input_dims = _p_session->GetInputTypeInfo(i).GetTensorTypeAndShapeInfo().GetShape();
        printf("\t│ dim is: [\n");
        for (auto dim : input_dims)
            printf("%ld, ", dim);
        printf("\b\b]\n");
        printf("\t└ type of each element is: %d\n", _p_session->GetInputTypeInfo(i).GetTensorTypeAndShapeInfo().GetElementType());
    }

    printf("\n------------- Output  Layer -------------\n");
    std::size_t output_node = _p_session->GetOutputCount();
    std::printf("the number of output node is: %zu\n", _p_session->GetOutputCount());
    for (std::size_t i = 0; i < output_node; i++)
    {
        printf("[%zu]\t┬ name is: %s\n", i, _p_session->GetOutputName(i, _allocator));
        std::vector<int64_t> output_dims = _p_session->GetOutputTypeInfo(i).GetTensorTypeAndShapeInfo().GetShape();
        printf("\t│ dim is: [");
        for (auto dim : output_dims)
            printf("%ld, ", dim);
        printf("\b\b]\n");
        printf("\t└ type of each element is: %d\n", _p_session->GetOutputTypeInfo(i).GetTensorTypeAndShapeInfo().GetElementType());
    }
}

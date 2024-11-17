/**
 * @file detector.h
 * @author RoboMaster Vision Community
 * @brief 抽象识别类头文件
 * @version 1.0
 * @date 2021-08-17
 *
 * @copyright Copyright 2021 (c), RoboMaster Vision Community
 *
 */

#pragma once

#include "rmvl/group/group.h"

#include "rmvl/algorithm/pretreat.hpp"

namespace rm
{

//! @addtogroup detector
//! @{

/**
 * @brief 识别信息结构体
 * @note
 * - 作为识别检测模块接口的返回值
 * @note
 * - 用于包含单独一帧识别到的所有特征、组合体，以及原图、各阶段的处理图
 */
struct RMVL_EXPORTS_W_AG DetectInfo
{
    RMVL_W_RW cv::Mat src;               //!< 原图
    RMVL_W_RW cv::Mat gray;              //!< 灰度图列表
    RMVL_W_RW cv::Mat bin;               //!< 二值图列表
    RMVL_W_RW std::vector<cv::Mat> rois; //!< ROI 列表
    RMVL_W_RW cv::Mat rendergraph;       //!< 渲染图

    RMVL_W_RW std::vector<combo::ptr> combos;     //!< 当前帧所有组合体
    RMVL_W_RW std::vector<feature::ptr> features; //!< 当前帧所有特征
};

//! 识别检测模块
class RMVL_EXPORTS_W_ABU detector
{
protected:
    double _tick;      //!< 每一帧对应的时间点
    ImuData _imu_data; //!< 每一帧对应的 IMU 数据

public:
    using ptr = std::unique_ptr<detector>;

    detector() = default;

    virtual ~detector() = default;

    /**
     * @brief 识别接口
     *
     * @param[in out] groups 序列组容器
     * @param[in] src 原图像
     * @param[in] color 待处理的颜色通道，可参考 rm::PixChannel
     * @param[in] imu_data IMU 数据
     * @param[in] tick 当前时间点
     * @return 识别信息结构体
     */
    RMVL_W virtual DetectInfo detect(std::vector<group::ptr> &groups, const cv::Mat &src, uint8_t color, const ImuData &imu_data, double tick) = 0;
};

//! @} detector

} // namespace rm

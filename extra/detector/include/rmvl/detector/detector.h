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

#include "rmvl/core/pretreat.hpp"

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
struct DetectInfo
{
    cv::Mat src;               //!< 原图
    cv::Mat gray;              //!< 灰度图列表
    cv::Mat bin;               //!< 二值图列表
    std::vector<cv::Mat> rois; //!< ROI 列表
    cv::Mat rendergraphs;      //!< 渲染图列表

    std::vector<combo::ptr> combos;     //!< 当前帧所有组合体
    std::vector<feature::ptr> features; //!< 当前帧所有特征
};

//! 识别检测模块
class detector
{
protected:
    double _tick;        //!< 每一帧对应的时间点
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
     * @param[in] color 待处理的颜色通道
     * @param[in] imu_data IMU 数据
     * @param[in] tick 当前时间点
     * @return 识别信息结构体
     */
    virtual DetectInfo detect(std::vector<group::ptr> &groups, cv::Mat &src, PixChannel color,
                              const ImuData &imu_data, double tick) = 0;
};

//! @} detector

} // namespace rm

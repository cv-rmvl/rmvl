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

#include "rmvl/imgproc/pretreat.h"

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
    cv::Mat src;         //!< 原图
    cv::Mat gray;        //!< 灰度图
    cv::Mat bin;         //!< 二值图
    cv::Mat rendergraph; //!< 渲染图

    std::vector<combo_ptr> combos;     //!< 当前帧所有组合体
    std::vector<feature_ptr> features; //!< 当前帧所有特征
};

//! 识别检测模块
class detector
{
protected:
    std::int64_t _tick;  //!< 每一帧对应的时间戳
    GyroData _gyro_data; //!< 每一帧对应的陀螺仪数据

public:
    detector() = default;

    virtual ~detector() = default;

    /**
     * @brief 识别接口
     *
     * @param[in out] groups 序列组容器
     * @param[in] src 原图像
     * @param[in] color 待处理的颜色通道
     * @param[in] gyro_data 陀螺仪数据
     * @param[in] record_time 时间戳
     * @return 识别信息结构体
     */
    virtual DetectInfo detect(std::vector<group_ptr> &groups, cv::Mat &src, PixChannel color,
                              const GyroData &gyro_data, int64 record_time) = 0;
};

// 抽象识别类智能指针
using detect_ptr = std::unique_ptr<detector>;

//! @} detector

} // namespace rm

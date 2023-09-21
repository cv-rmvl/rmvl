/**
 * @file compensator.h
 * @author RoboMaster Vision Community
 * @brief 抽象补偿类头文件
 * @version 1.0
 * @date 2021-08-24
 *
 * @copyright Copyright 2021 (c), RoboMaster Vision Community
 *
 */

#pragma once

#include <unordered_map>

#include "rmvl/group/group.h"

namespace rm
{

//! @addtogroup compensator
//! @{

/**
 * @brief 补偿模块信息
 * @note
 * - 作为弹道下坠补偿模块接口的返回值
 */
struct CompensateInfo
{
    /**
     * @brief 追踪器 - 补偿映射表
     * @note 记载每一个追踪器对应的补偿值，`x` 表示水平方向补偿增量，`y`
     *       表示垂直方向补偿增量
     */
    std::unordered_map<tracker_ptr, cv::Point2f> compensation;

    /**
     * @brief 飞行时间 (Time of Flying)
     * @note 记载每一个追踪器对应的飞行时间，表示弹丸击中目标所需要的子弹飞行时间
     */
    std::unordered_map<tracker_ptr, double> tof;
};

//! 弹道下坠补偿模块
class compensator
{
protected:
    float _yaw_static_com;   //!< yaw 轴静态补偿，方向与 yaw 一致
    float _pitch_static_com; //!< pitch 轴静态补偿，方向与 pitch 一致

public:
    compensator() = default;
    virtual ~compensator() = default;

    /**
     * @brief 补偿核心函数，（补偿角度方向与 gyro_angle 方向一致）
     *
     * @param[in] groups 所有序列组
     * @param[in] shoot_speed 子弹射速 (m/s)
     * @param[in] com_flag 手动调节补偿标志
     * @return 补偿模块信息
     */
    virtual CompensateInfo compensate(const std::vector<group_ptr> &groups, uint8_t shoot_speed,
                                      CompensateType com_flag) = 0;

    /**
     * @brief 获得补偿角度
     * @note
     * - 使用不动点迭代法进行补偿量的求解 \cite icra2019
     * - 需要严格满足相机对水平方向的夹角等于 `gyro_angle.y`
     *
     * @param[in] x 目标离相机的水平宽度
     * @param[in] y 目标离相机的铅垂高度
     * @param[in] velocity 枪口射速
     *
     * @return 补偿角度
     */
    static double getPitch(double x, double y, double velocity);

private:
    /**
     * @brief 计算真实的 y 坐标
     *
     * @param[in] distance 相机到装甲板的水平距离 (m)
     * @param[in] velocity 枪口射速 (m/s)
     * @param[in] angle 枪口与水平方向的夹角 (rad)
     *
     * @return 世界坐标系下真实的 y 坐标
     */
    static double bulletModel(double distance, double velocity, double angle);
};

//! 补偿类非共享指针
using compensate_ptr = std::unique_ptr<compensator>;

//! @} compensator

} // namespace rm

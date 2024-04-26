/**
 * @file gyro_compensator.cpp
 * @author RoboMaster Vision Community
 * @brief 重力模型补偿
 * @version 1.0
 * @date 2021-08-28
 *
 * @copyright Copyright 2021 (c), RoboMaster Vision Community
 *
 */

#include "rmvl/compensator/gyro_compensator.h"
#include "rmvl/group/gyro_group.h"
#include "rmvl/core/transform.hpp"

#include "rmvlpara/camera/camera.h"
#include "rmvlpara/compensator/gyro_compensator.h"

namespace rm
{

/**
 * @brief 弹道模型
 * 
 * @param[in] x 目标离相机的水平距离，单位 `m`
 * @param[in] v 枪口射速，单位 `m/s`
 * @param[in] angle 当前枪口仰角、俯角，`+` 表示仰角，`-` 表示俯角，单位 `rad`
 * @return 弹丸飞行至水平距离`x`时，落点的垂直距离
 * @note 返回值为正表示落点在枪口上方，为负表示落点在枪口下方，单位 `m`
 */
static double bulletModel(double x, double v, double angle)
{
    // 计算飞行 x 距离的子弹飞行时间
    double t = x / (v * cos(angle));
    // 计算子弹落点高度
    return v * sin(angle) * t - para::gyro_compensator_param.g * t * t / 2.0;
}

static void updateStaticCom(CompensateType com_flag, float &x_st, float &y_st)
{
    float com_step = para::gyro_compensator_param.MINIMUM_COM;
    switch (com_flag)
    {
    case CompensateType::UP:
        y_st += com_step;
        para::gyro_compensator_param.PITCH_COMPENSATE += com_step;
        break;
    case CompensateType::DOWN:
        y_st -= com_step;
        para::gyro_compensator_param.PITCH_COMPENSATE -= com_step;
        break;
    case CompensateType::LEFT:
        x_st += com_step;
        para::gyro_compensator_param.YAW_COMPENSATE += com_step;
        break;
    case CompensateType::RIGHT:
        x_st -= com_step;
        para::gyro_compensator_param.YAW_COMPENSATE -= com_step;
        break;
    default:
        break;
    }
}

/**
 * @brief 获得补偿角度
 * @note
 * - 需要严格满足相机对水平方向的夹角等于 `gyro_angle.y`
 *
 * @param[in] x 目标离相机的水平宽度
 * @param[in] y 目标离相机的铅垂高度
 * @param[in] velocity 枪口射速
 *
 * @return 补偿角度
 */
static double getPitch(double x, double y, double velocity)
{
    double y_temp = y;
    double angle = 0.f;
    // 使用迭代法求得补偿角度
    for (int i = 0; i < 50; i++)
    {
        angle = atan2(y_temp, x);
        double dy = y - bulletModel(x, velocity, angle);
        y_temp += dy;
        if (abs(dy) < 0.001)
            break;
    }
    return angle;
}

GyroCompensator::GyroCompensator()
{
    _pitch_static_com = para::gyro_compensator_param.PITCH_COMPENSATE;
    _yaw_static_com = para::gyro_compensator_param.YAW_COMPENSATE;
}

CompensateInfo GyroCompensator::compensate(const std::vector<group::ptr> &groups, float shoot_speed, CompensateType com_flag)
{
    CompensateInfo info;
    // 补偿手动调节
    updateStaticCom(com_flag, _yaw_static_com, _pitch_static_com);
    // 对每个序列组的中心点计算补偿
    for (auto p_group : groups)
    {
        auto p_gyro_group = GyroGroup::cast(p_group);
        if (p_gyro_group == nullptr)
            RMVL_Error(RMVL_BadDynamicType, "Fail to cast the type of \"p_group\" to \"GyroGroup::ptr\"");
        // 直线距离
        auto dis = getDistance(p_gyro_group->getCenter3D(), cv::Vec3f{}) / 1000.;
        // 目标转角
        auto relative_angle = calculateRelativeAngle(para::camera_param.cameraMatrix, p_group->getCenter());
        // 提取当前陀螺仪角度
        auto gyro_angle = cv::Point2f(p_gyro_group->getGyroData().rotation.yaw,
                                  p_gyro_group->getGyroData().rotation.pitch);
        // 目标与云台转轴的连线与水平方向的夹角
        double angle = gyro_angle.y + relative_angle.y;
        double gp = rad2deg(-getPitch(dis * cos(deg2rad(-angle)), // 模型中角度要求向上为正，这里需取反
                                      dis * sin(deg2rad(-angle)), static_cast<double>(shoot_speed)));
        double x_com = _yaw_static_com;
        double y_com = gp - angle + _pitch_static_com;
        double tf = dis / (static_cast<double>(shoot_speed) * cos(deg2rad(y_com + angle))); // 子弹飞行时间计算
        // 更新至每个 tracker
        for (auto p_tracker : p_group->data())
        {
            info.compensation.emplace(p_tracker, cv::Point2f(x_com, y_com));
            info.tof.emplace(p_tracker, tf);
        }
    }
    return info;
}

} // namespace rm
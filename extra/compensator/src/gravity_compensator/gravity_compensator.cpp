/**
 * @file gravity_compensator.cpp
 * @author RoboMaster Vision Community
 * @brief 重力模型补偿
 * @version 1.0
 * @date 2021-08-28
 *
 * @copyright Copyright 2021 (c), RoboMaster Vision Community
 *
 */

#include "rmvl/compensator/gravity_compensator.h"

#include "rmvlpara/compensator/gravity_compensator.h"

using namespace rm;
using namespace para;
using namespace std;
using namespace cv;

GravityCompensator::GravityCompensator()
{
    _pitch_static_com = gravity_compensator_param.PITCH_COMPENSATE;
    _yaw_static_com = gravity_compensator_param.YAW_COMPENSATE;
}

CompensateInfo GravityCompensator::compensate(const vector<group_ptr> &groups, uint8_t shoot_speed,
                                              CompensateType com_flag)
{
    CompensateInfo info{};
    // 补偿手动调节
    updateStaticCom(com_flag);
    // 对每个序列组的每个追踪器按照一种方式进行补偿计算
    for (auto &p_group : groups)
    {
        for (auto &p_tracker : p_group->data())
        {
            // 单位换算
            double dis = p_tracker->getExtrinsics().distance() / 1000.;
            // 提取当前陀螺仪角度
            auto gyro_angle = Point2f(p_tracker->front()->getGyroData().rotation.yaw,
                                      p_tracker->front()->getGyroData().rotation.pitch);
            // 目标与云台转轴的连线与水平方向的夹角
            double angle = gyro_angle.y + p_tracker->getRelativeAngle().y;
            double gp = rad2deg(-getPitch(dis * cos(deg2rad(-angle)), // 模型中角度要求向上为正，这里需取反
                                          dis * sin(deg2rad(-angle)), static_cast<double>(shoot_speed)));

            double x_com = _yaw_static_com;
            double y_com = gp - angle + _pitch_static_com;
            double tf = dis / (static_cast<double>(shoot_speed) * cos(deg2rad(y_com + angle))); // 子弹飞行时间计算
            // 更新
            info.compensation.emplace(p_tracker, Point2f(x_com, y_com));
            info.tof.emplace(p_tracker, tf);
        }
    }
    return info;
}

void GravityCompensator::updateStaticCom(CompensateType com_flag)
{
    float com_step = gravity_compensator_param.MINIMUM_COM;
    switch (com_flag)
    {
    case CompensateType::UP:
        _pitch_static_com += com_step;
        gravity_compensator_param.PITCH_COMPENSATE += com_step;
        break;
    case CompensateType::DOWN:
        _pitch_static_com -= com_step;
        gravity_compensator_param.PITCH_COMPENSATE -= com_step;
        break;
    case CompensateType::LEFT:
        _yaw_static_com += com_step;
        gravity_compensator_param.YAW_COMPENSATE += com_step;
        break;
    case CompensateType::RIGHT:
        _yaw_static_com -= com_step;
        gravity_compensator_param.YAW_COMPENSATE -= com_step;
        break;
    default:
        break;
    }
}

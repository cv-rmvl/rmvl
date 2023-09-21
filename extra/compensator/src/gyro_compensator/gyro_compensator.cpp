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
#include "rmvl/rmath/transform.h"

#include "rmvlpara/camera.hpp"
#include "rmvlpara/compensator/gyro_compensator.h"

using namespace cv;
using namespace std;
using namespace para;
using namespace rm;

GyroCompensator::GyroCompensator()
{
    _pitch_static_com = gyro_compensator_param.PITCH_COMPENSATE;
    _yaw_static_com = gyro_compensator_param.YAW_COMPENSATE;
}

CompensateInfo GyroCompensator::compensate(const vector<group_ptr> &groups,
                                           uint8_t shoot_speed, CompensateType com_flag)
{
    CompensateInfo info;
    // 补偿手动调节
    updateStaticCom(com_flag);
    // 对每个序列组的中心点计算补偿
    for (auto p_group : groups)
    {
        auto p_gyro_group = GyroGroup::cast(p_group);
        if (p_gyro_group == nullptr)
            RMVL_Error(RMVL_BadDynamicType, "Fail to cast the type of \"p_group\" to \"gyro_group_ptr\"");
        // 直线距离
        auto dis = getDistance(p_gyro_group->getCenter3D(), Vec3f{}) / 1000.;
        // 目标转角
        auto relative_angle = calculateRelativeAngle(camera_param.cameraMatrix, p_group->getCenter());
        // 提取当前陀螺仪角度
        auto gyro_angle = Point2f(p_gyro_group->getGyroData().rotation.yaw,
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
            info.compensation.emplace(p_tracker, Point2f(x_com, y_com));
            info.tof.emplace(p_tracker, tf);
        }
    }
    return info;
}

void GyroCompensator::updateStaticCom(CompensateType com_flag)
{
    float com_step = gyro_compensator_param.MINIMUM_COM;
    switch (com_flag)
    {
    case CompensateType::UP:
        _pitch_static_com += com_step;
        gyro_compensator_param.PITCH_COMPENSATE += com_step;
        break;
    case CompensateType::DOWN:
        _pitch_static_com -= com_step;
        gyro_compensator_param.PITCH_COMPENSATE -= com_step;
        break;
    case CompensateType::LEFT:
        _yaw_static_com += com_step;
        gyro_compensator_param.YAW_COMPENSATE += com_step;
        break;
    case CompensateType::RIGHT:
        _yaw_static_com -= com_step;
        gyro_compensator_param.YAW_COMPENSATE -= com_step;
        break;
    default:
        break;
    }
}

/**
 * @file compensator.cpp
 * @author RoboMaster Vision Community
 * @brief 补偿模块
 * @version 1.0
 * @date 2023-07-24
 * 
 * @copyright Copyright 2023 (c), RoboMaster Vision Community
 * 
 */

#include "rmvl/compensator/compensator.h"

double rm::compensator::bulletModel(double distance, double velocity, double angle)
{
    // x(m), v(m/s), angle(rad)
    // 忽略空气阻力影响
    double time = distance / (velocity * cos(angle));
    return velocity * sin(angle) * time - g * time * time / 2.0;
}

double rm::compensator::getPitch(double x, double y, double velocity)
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
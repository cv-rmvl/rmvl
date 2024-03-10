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

#include "rmvlpara/compensator/gravity_compensator.h"

#include "gravity_impl.h"

namespace rm
{

GravityCompensator::GravityCompensator() noexcept : _impl(new Impl)
{
    _pitch_static_com = para::gravity_compensator_param.PITCH_COMPENSATE;
    _yaw_static_com = para::gravity_compensator_param.YAW_COMPENSATE;
}

GravityCompensator::~GravityCompensator() { delete _impl; }

CompensateInfo GravityCompensator::compensate(const std::vector<group::ptr> &groups, float shoot_speed, CompensateType com_flag)
{
    return _impl->compensate(groups, shoot_speed, com_flag, _yaw_static_com, _pitch_static_com);
}

GravityCompensator::Impl::Impl()
{
    using para::gravity_compensator_param;
    double tmp = gravity_compensator_param.rho * gravity_compensator_param.A * gravity_compensator_param.V / (2 * gravity_compensator_param.m);
    double a22 = -tmp * gravity_compensator_param.Cd;
    double a24 = -tmp * gravity_compensator_param.Cl;
    double a42 = -a24, a44 = a22;

    Odes fs(4);
    fs[0] = [](double, const std::vector<double> &x) { return x[1]; };
    fs[1] = [=](double, const std::vector<double> &x) { return a22 * x[1] + a24 * x[3]; };
    fs[2] = [](double, const std::vector<double> &x) { return x[3]; };
    fs[3] = [=](double, const std::vector<double> &x) { return a42 * x[1] + a44 * x[3] - gravity_compensator_param.g; };
    _rk = std::make_unique<RungeKutta2>(fs);
}

std::pair<double, double> GravityCompensator::Impl::bulletModel(double x, double v, double angle)
{
    // 预估子弹飞行时间，并延长 15%
    double t_pre{x / (v * cos(angle)) * 1.15};
    // 使用 RK2 方法计算较长一段时间的弹道轨迹
    _rk->init(0, {0, v * cos(angle), 0, v * sin(angle)});
    auto res = _rk->solve(para::gravity_compensator_param.h, static_cast<size_t>(std::floor(t_pre / para::gravity_compensator_param.h)));
    // 在计算出的结果中根据 res[0] 二分查找最接近 x 的解
    size_t l{res.size() >> 1}, r{res.size() - 1};
    for (int i = 0; i < 2 * log2(res.size()); i++)
    {
        if (l == r)
            break;
        size_t m = (l + r) >> 1;
        res[m][0] < x ? (l = m + 1) : (r = m);
    }
    // 线性插值，获取 x 对应的落点高度 y 以及对应的子弹飞行时间 t
    size_t i1 = res[l][0] < x ? l : l - 1;
    size_t i2 = res[l][0] < x ? l + 1 : l;
    double retval_y = res[i1][2] + (res[i2][2] - res[i1][2]) / (res[i2][0] - res[i1][0]) * (x - res[i1][0]);
    double retval_t = (i1 + (x - res[i1][0]) / (res[i2][0] - res[i1][0])) * para::gravity_compensator_param.h;
    return {retval_y, retval_t};
}

std::pair<double, double> GravityCompensator::Impl::calc(double x, double y, double velocity)
{
    double y_temp{y};
    double angle{};
    double t{};
    // 使用迭代法求得补偿角度，并获取对应的子弹飞行时间
    for (int i = 0; i < 50; i++)
    {
        angle = atan2(y_temp, x);
        // 通过子弹模型计算落点以及子弹飞行时间
        auto [cur_y, cur_t] = bulletModel(x, velocity, angle);
        t = cur_t;
        double dy = y - cur_y;
        y_temp += dy;
        if (abs(dy) < 0.001)
            break;
    }
    return {angle, t};
}

void GravityCompensator::Impl::updateStaticCom(CompensateType com_flag, float &x_st, float &y_st)
{
    float com_step = para::gravity_compensator_param.MINIMUM_COM;
    switch (com_flag)
    {
    case CompensateType::UP:
        y_st += com_step;
        para::gravity_compensator_param.PITCH_COMPENSATE += com_step;
        break;
    case CompensateType::DOWN:
        y_st -= com_step;
        para::gravity_compensator_param.PITCH_COMPENSATE -= com_step;
        break;
    case CompensateType::LEFT:
        x_st += com_step;
        para::gravity_compensator_param.YAW_COMPENSATE += com_step;
        break;
    case CompensateType::RIGHT:
        x_st -= com_step;
        para::gravity_compensator_param.YAW_COMPENSATE -= com_step;
        break;
    default:
        break;
    }
}

CompensateInfo GravityCompensator::Impl::compensate(const std::vector<group::ptr> &groups, float shoot_speed,
                                                    CompensateType com_flag, float &yaw_static_com, float &pitch_static_com)
{
    CompensateInfo info{};
    // 补偿手动调节
    updateStaticCom(com_flag, yaw_static_com, pitch_static_com);
    // 对每个序列组的每个追踪器按照一种方式进行补偿计算
    for (auto &p_group : groups)
    {
        for (auto &p_tracker : p_group->data())
        {
            // 单位换算
            double dis = p_tracker->getExtrinsics().distance() / 1000.;
            // 提取当前陀螺仪角度
            auto gyro_angle = cv::Point2f(p_tracker->front()->getGyroData().rotation.yaw,
                                          p_tracker->front()->getGyroData().rotation.pitch);
            // 目标与云台转轴的连线与水平方向的夹角
            double angle = gyro_angle.y + p_tracker->getRelativeAngle().y;
            // 计算补偿角度和对应的子弹飞行时间（模型中角度要求向上为正，这里需取反）
            auto [angle_com, t_com] = calc(dis * cos(deg2rad(-angle)), dis * sin(deg2rad(-angle)), shoot_speed);
            double gp = rad2deg(-angle_com);
            double x_com = yaw_static_com;
            double y_com = gp - angle + pitch_static_com;
            // 更新
            info.compensation.emplace(p_tracker, cv::Point2f(x_com, y_com));
            info.tof.emplace(p_tracker, t_com);
        }
    }
    return info;
}

} // namespace rm
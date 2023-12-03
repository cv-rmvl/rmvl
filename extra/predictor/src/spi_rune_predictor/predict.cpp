/**
 * @file predict.cpp
 * @author RoboMaster Vision Community
 * @brief 系统参数辨识神符预测
 * @version 1.0
 * @date 2023-06-08
 *
 * @copyright Copyright 2023 (c), RoboMaster Vision Community
 *
 */

#include "rmvl/group/rune_group.h"
#include "rmvl/predictor/spi_rune_predictor.h"
#include "rmvl/tracker/rune_tracker.h"

#include "rmvlpara/camera/camera.h"
#include "rmvlpara/predictor/spi_rune_predictor.h"

using namespace rm;
using namespace para;
using namespace std;
using namespace cv;

/**
 * @brief 重置、初始化 Pm、xm
 *
 * @param[in] n 模型阶数
 * @param[out] Pm 协方差矩阵
 * @param[out] xm 待求解系数向量
 */
inline void resetPmxm(size_t n, Mat &Pm, Mat &xm)
{
    Pm = spi_rune_predictor_param.KP * Mat::eye(n + 1, n + 1, CV_64FC1);
    xm = Mat::zeros(n + 1, 1, CV_64FC1);
}

SpiRunePredictor::SpiRunePredictor() : _n(spi_rune_predictor_param.DIFF_ORDER),
                                       _interval(spi_rune_predictor_param.SAMPLE_INTERVAL)
{
    resetPmxm(_n, _pm, _xm);
}

void SpiRunePredictor::identifier(const deque<double> &raw_datas)
{
    // 允许辨识的最小容量
    if (raw_datas.size() < spi_rune_predictor_param.MAX_NF + spi_rune_predictor_param.DIFF_ORDER)
        return;
    // 获取 nf: 直接设定为最大值
    size_t nf = spi_rune_predictor_param.MAX_NF;
    // 系数提取
    Mat am = Mat(1, _n + 1, CV_64FC1);
    am.at<double>(0) = 1;
    for (uint i = 0; i < _n; ++i)
        am.at<double>(i + 1) = raw_datas[nf + i];
    Mat amt = am.t();
    double bm = raw_datas[0];
    // 最小二乘递推
    _pm = _pm - (_pm * amt * am * _pm) / (1 + Mat(am * _pm * amt).at<double>(0));
    _xm = _xm + _pm * amt * (bm - Mat(am * _xm).at<double>(0));
}

PredictInfo SpiRunePredictor::predict(const vector<group::ptr> &groups, const unordered_map<tracker::ptr, double> &tof)
{
    PredictInfo info{};
    if (groups.empty() || groups.front()->data().empty())
    {
        resetPmxm(_n, _pm, _xm);
        return info;
    }
    if (groups.size() > 1)
        RMVL_Error_(RMVL_StsBadArg, "Bad Argument of the \"groups\", size of the \"groups\" is %zu", groups.size());
    // ------------------- 系统参数辨识过程 -------------------
    auto p_rune_group = RuneGroup::cast(groups.front());
    const auto &trackers = p_rune_group->data();
    size_t trackers_num = trackers.size();
    const auto &raw_datas = p_rune_group->getRawDatas();
    identifier(raw_datas);
    // ---------------------- 预测量计算 ----------------------
    for (size_t i = 0; i < trackers_num; ++i)
    {
        // 静态预测角度增量
        auto dB = staticPredict(trackers[i]);
        // 动态预测角度增量
        double tf = (tof.find(trackers[i]) == tof.end()) ? 0. : tof.at(trackers[i]);
        auto dKt = anglePredict(raw_datas, tf);
        info.static_prediction[trackers[i]](ANG_Z) = dB;
        info.dynamic_prediction[trackers[i]](ANG_Z) = dKt;
    }
    return info;
}

float SpiRunePredictor::staticPredict(tracker::ptr p_tracker)
{
    auto p_rune_tracker = RuneTracker::cast(p_tracker);
    if (p_rune_tracker == nullptr)
        RMVL_Error(RMVL_BadDynamicType, "failed to convert the type of \"p_tracker\" to \"RuneTracker\"");
    return p_rune_tracker->getRotatedSpeed() * spi_rune_predictor_param.B;
}

double SpiRunePredictor::anglePredict(const deque<double> &raw_datas, double tf)
{
    if (raw_datas.empty())
        RMVL_Error(RMVL_StsBadSize, "Bad size of the \"_datas\", size = 0");
    if (tf <= 0)
        RMVL_Error_(RMVL_StsOutOfRange, "Flying time is <= 0, tf = %f", tf);
    // 允许预测的最小容量
    if (raw_datas.size() < spi_rune_predictor_param.MAX_NF + spi_rune_predictor_param.DIFF_ORDER)
        return sgn(raw_datas.front() - raw_datas.back()) * spi_rune_predictor_param.FIXED_ANGLE;
    size_t max_nf = spi_rune_predictor_param.MAX_NF;
    double nf = floor(1000 * tf / _interval) + 1;
    // 范围限制
    if (nf > static_cast<double>(max_nf))
        nf = max_nf;
    else if (nf < 1)
        nf = 1;
    double k1 = nf - 1000 * tf / _interval; // 预测量取 nf - 1 的置信权重
    double k2 = 1 - k1;                     // 预测量取 nf 的置信权重
    // 以 max_nf 为基准，实际 nf 进行预测需要滞后的帧数
    size_t delay_k2 = max_nf - static_cast<size_t>(nf); // 预测量取 nf 的滞后帧数
    size_t delay_k1 = delay_k2 + 1;                     // 预测量取 nf - 1 的滞后帧数
    // 系数提取
    Mat am1(1, _n + 1, CV_64FC1);
    am1.at<double>(0) = 1;
    for (uint i = 0; i < _n; ++i)
        am1.at<double>(i + 1) = raw_datas[delay_k1 + i];
    Mat am2(1, _n + 1, CV_64FC1);
    am2.at<double>(0) = 1;
    for (uint i = 0; i < _n; ++i)
        am2.at<double>(i + 1) = raw_datas[delay_k2 + i];
    // 预测
    auto x = Mat(am1 * _xm);
    // 线性插值得到 Δθ
    return Mat(am1 * _xm).at<double>(0) * k1 + Mat(am2 * _xm).at<double>(0) * k2 - raw_datas[0];
}

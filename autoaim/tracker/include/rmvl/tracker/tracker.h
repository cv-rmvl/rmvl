/**
 * @file tracker.h
 * @author RoboMaster Vision Community
 * @brief 追踪器抽象头文件
 * @version 1.0
 * @date 2021-08-20
 *
 * @copyright Copyright 2021 (c), RoboMaster Vision Community
 *
 */

#pragma once

#include <deque>

#include "rmvl/combo/combo.h"

namespace rm
{

//! @addtogroup tracker
//! @{

//! 组合体时间序列
class tracker
{
protected:
    std::deque<combo_ptr> _combo_deque; //!< 组合体时间队列
    uint32_t _vanish_num = 0U;          //!< 消失帧数

    RMStatus _type;                    //!< 追踪器类型
    float _height = 0.f;               //!< 追踪器高度（可表示修正后）
    float _width = 0.f;                //!< 追踪器宽度（可表示修正后）
    float _angle = 0.f;                //!< 追踪器角度（可表示修正后）
    cv::Point2f _center;               //!< 追踪器中心点（可表示修正后）
    cv::Point2f _relative_angle;       //!< 相对目标转角（可表示修正后）
    std::vector<cv::Point2f> _corners; //!< 追踪器角点（可表示修正后）
    ResultPnP<float> _pnp_data;        //!< PnP 数据（可表示修正后）
    cv::Point2f _speed;                //!< 相对目标转角速度

public:
    tracker() = default;

    virtual ~tracker() = default;

    /**
     * @brief 更新追踪器
     *
     * @param[in] p_combo 更新的组合体
     * @param[in] current_tick 当前时间戳
     * @param[in] gyro_data 当前陀螺仪信息
     */
    virtual void update(combo_ptr p_combo, int64 current_tick, const GyroData &gyro_data) = 0;

    //! 获取时间队列中最新的组合体
    inline combo_ptr front() { return _combo_deque.front(); }
    //! 获取时间队列中最后的组合体
    inline combo_ptr back() { return _combo_deque.back(); }
    //! 获取掉帧数
    inline uint32_t getVanishNumber() { return _vanish_num; }
    //! 获取序列数量信息
    inline size_t size() { return _combo_deque.size(); }
    //! 获取时间序列原始数据
    inline auto &data() { return _combo_deque; }
    //! 序列是否为空
    inline bool empty() { return _combo_deque.empty(); }
    //! 索引 - 容器仅能通过内部 at 实现访问保证下标安全
    inline combo_ptr at(size_t _n) const { return _combo_deque.at(_n); }

    //! 追踪器类型
    inline RMStatus getType() { return _type; }
    //! 追踪器修正后的高度
    inline float getHeight() { return _height; }
    //! 追踪器修正后的宽度
    inline float getWidth() { return _width; }
    //! 追踪器修正后的角度
    inline float getAngle() { return _angle; }
    //! 追踪器修正后的中心点
    inline const cv::Point2f &getCenter() { return _center; }
    //! 追踪器修正后的角点
    inline const std::vector<cv::Point2f> &getCorners() { return _corners; }
    //! 修正后的相对角度（角度制）
    inline const cv::Point2f &getRelativeAngle() { return _relative_angle; }
    //! 修正后的 PnP 解算信息
    inline const ResultPnP<float> &getPNP() { return _pnp_data; }
    //! 获取追踪器修正后的目标转角速度（角度制）
    inline const cv::Point2f &getSpeed() { return _speed; }
};

using tracker_ptr = std::shared_ptr<tracker>;

//! @} tracker

} // namespace rm

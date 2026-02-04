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

namespace rm {

//! @addtogroup tracker
//! @{

//! 组合体时间序列
class RMVL_EXPORTS_W_ABS tracker {
protected:
    std::deque<combo::ptr> _combo_deque; //!< 组合体时间队列
    uint32_t _vanish_num{};              //!< 消失帧数

    StateInfo _state{};                //!< 追踪器状态
    float _height{};                   //!< 追踪器高度（可表示修正后）
    float _width{};                    //!< 追踪器宽度（可表示修正后）
    float _angle{};                    //!< 追踪器角度（可表示修正后）
    cv::Point2f _center;               //!< 追踪器中心点（可表示修正后）
    cv::Point2f _relative_angle;       //!< 相对目标转角（可表示修正后）
    std::vector<cv::Point2f> _corners; //!< 追踪器角点（可表示修正后）
    CameraExtrinsics _extrinsic;       //!< 相机外参（可表示修正后）
    cv::Point2f _speed;                //!< 相对目标转角速度

public:
    using ptr = std::shared_ptr<tracker>;
    using const_ptr = std::shared_ptr<const tracker>;

    /**
     * @brief 从另一个追踪器进行构造
     *
     * @return 指向新追踪器的共享指针
     */
    RMVL_W virtual ptr clone() = 0;

    /**
     * @brief 使用已捕获的 `combo` 更新追踪器
     *
     * @param[in] p_combo 更新的组合体
     */
    RMVL_W virtual void update(combo::ptr p_combo) = 0;

    /**
     * @brief 未捕获 `combo`，但使用其余数据更新追踪器（即目标丢失时的操作）
     *
     * @param[in] tick 当前时间点
     * @param[in] imu_data 当前 IMU 信息
     */
    RMVL_W virtual void update(double tick, const ImuData &imu_data) = 0;

    //! 判断追踪器是否无效
    RMVL_W virtual bool invalid() const { return false; }

    //! 获取时间队列中最新的组合体
    RMVL_W combo::ptr front() const { return _combo_deque.front(); }
    //! 获取时间队列中最后的组合体
    RMVL_W combo::ptr back() const { return _combo_deque.back(); }
    //! 获取掉帧数
    RMVL_W uint32_t getVanishNumber() const { return _vanish_num; }
    //! 获取序列数量信息
    RMVL_W size_t size() const { return _combo_deque.size(); }
    //! 获取时间序列原始数据
    RMVL_W const std::deque<combo::ptr> &data() const { return _combo_deque; }
    //! 序列是否为空
    RMVL_W bool empty() const { return _combo_deque.empty(); }
    //! 索引 - 容器仅能通过内部 at 实现访问保证下标安全
    RMVL_W combo::ptr at(size_t _n) const { return _combo_deque.at(_n); }
    //! 追踪器状态
    RMVL_W const StateInfo &state() const { return _state; }
    //! 追踪器状态
    StateInfo &state() { return _state; }
    //! 追踪器修正后的高度
    RMVL_W float height() const { return _height; }
    //! 追踪器修正后的宽度
    RMVL_W float width() const { return _width; }
    //! 追踪器修正后的角度
    RMVL_W float angle() const { return _angle; }
    //! 追踪器修正后的中心点
    RMVL_W const cv::Point2f &center() const { return _center; }
    //! 追踪器修正后的角点
    RMVL_W const std::vector<cv::Point2f> &corners() const { return _corners; }
    //! 修正后的相对角度（角度制）
    RMVL_W const cv::Point2f &getRelativeAngle() const { return _relative_angle; }
    //! 修正后的相机外参
    RMVL_W const CameraExtrinsics &extrinsic() const { return _extrinsic; }
    //! 获取追踪器修正后的目标转角速度（角度制）
    RMVL_W const cv::Point2f &speed() const { return _speed; }
};

//! `rm::tracker` 追踪器类型转换宏
#define RMVL_TRACKER_CAST(name)                                                                           \
    static inline ptr cast(tracker::ptr p_tracker) { return std::dynamic_pointer_cast<name>(p_tracker); } \
    static inline const_ptr cast(tracker::const_ptr p_tracker) { return std::dynamic_pointer_cast<const name>(p_tracker); }

//! @} tracker

} // namespace rm

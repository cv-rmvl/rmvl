/**
 * @file utils.h
 * @author RoboMaster Vision Community
 * @brief
 * @version 1.0
 * @date 2023-04-28
 *
 * @copyright Copyright 2023 (c), RoboMaster Vision Community
 *
 */

#pragma once

#include <deque>
#include <opencv2/core/matx.hpp>

namespace rm
{

//! 序列组中的追踪器状态
class TrackerState
{
    std::size_t _index = 0; //!< 在车组中追踪器的序号
    float _radius = 0.f;    //!< 旋转半径
    float _delta_y = 0.f;   //!< 与旋转中心的高度差（向下为正）

public:
    TrackerState() = default;

    /**
     * @brief 追踪器信息结构体直接初始化
     *
     * @param[in] index 在车组中追踪器的序号
     * @param[in] radius 旋转半径
     * @param[in] delta_y 与旋转中心的高度差（向下为正）
     */
    TrackerState(int index, float radius, float delta_y)
        : _index(index), _radius(radius), _delta_y(delta_y) {}

    //! 获取在车组中追踪器的序号
    inline size_t index() const { return _index; }
    //! 获取旋转半径
    inline float radius() const { return _radius; }
    //! 与旋转中心的高度差（向下为正）
    inline float delta_y() const { return _delta_y; }
    //! 更新在车组中追踪器的序号
    inline void index(int idx) { _index = idx; }
    //! 更新旋转半径
    inline void radius(float r) { _radius = r; }
    //! 更新与旋转中心的高度差（向下为正）
    inline void delta_y(float dy) { _delta_y = dy; }
};

//! 旋转状态类型
enum class RotStatus : uint8_t
{
    LOW_ROT_SPEED,  //!< 低转速
    HIGH_ROT_SPEED, //!< 高转速
};

} // namespace rm

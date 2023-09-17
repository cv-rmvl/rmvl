/**
 * @file group.h
 * @author RoboMaster Vision Community
 * @brief 序列组抽象头文件
 * @version 1.0
 * @date 2022-02-27
 *
 * @copyright Copyright 2023 (c), RoboMaster Vision Community
 *
 */

#pragma once

#include "rmvl/tracker/tracker.h"

namespace rm
{

//! @addtogroup group
//! @{

//! 相关追踪器的集合（序列组）
class group
{
protected:
    std::vector<tracker_ptr> _trackers; //!< 同组追踪器
    cv::Point2f _center;                //!< 序列组中心
    bool _is_tracked{};                 //!< 是否为目标序列组
    uint32_t _vanish_num{};             //!< 丢帧数量
    RMStatus _type{};                   //!< 序列组各状态

public:
    group() = default;

    virtual ~group() = default;

    /**
     * @brief 序列组同步操作
     * @note 根据当前已知的所有的 `tracker` 信息，同步整体 `group`
     *       的内部数据，例如序列组中心，自主构造新的 `tracker` 等
     *
     * @param[in] gyro_data 最新陀螺仪数据
     * @param[in] tick 最新时间戳
     */
    virtual void sync(const GyroData &gyro_data, int64_t tick) = 0;

    /**
     * @brief 添加追踪器至序列组
     * @note
     * - 当捕获到的新追踪器 `p_tracker` 计划更新至内部 `_trackers`
     *   时，可调用此方法完成更新
     * - 需要注意的是，此方法不执行同步操作 `sync()`，其派生类在实现过程中也需统一，避免执行 `sync()`
     *
     * @param p_tracker 新的追踪器 `tracker`
     */
    virtual void add(tracker_ptr p_tracker) { _trackers.emplace_back(p_tracker); }

    /**
     * @brief 获取同组所有的追踪器数据
     * @note 若仅需要对 `vector<tracker_ptr>` 做数据处理，使用此方法可直接实现 group 至 tracker 的退化
     *
     * @return 同组追踪器
     */
    inline auto &data() { return _trackers; }

    /**
     * @brief 获取同组追踪器的数量
     *
     * @return 同组追踪器的数量
     */
    inline size_t size() const { return _trackers.size(); }

    /**
     * @brief 判断同组追踪器是否为空
     *
     * @return 同组追踪器是否为空
     */
    inline bool empty() const { return _trackers.empty(); }

    /**
     * @brief 获取指定追踪器
     *
     * @param[in] idx 追踪器下标
     */
    inline tracker_ptr at(size_t idx) { return _trackers.at(idx); }

    /**
     * @brief 获取序列组中心
     *
     * @return 序列组中心
     */
    inline cv::Point2f getCenter() { return _center; }

    /**
     * @brief 获取丢帧数量
     *
     * @return 丢帧数量
     */
    inline uint32_t getVanishNumber() { return _vanish_num; }

    /**
     * @brief 获取该组类型
     *
     * @return RMStatus 该组类型
     */
    inline RMStatus getType() { return _type; }
};

//! 序列组共享指针
using group_ptr = std::shared_ptr<group>;

//! 默认序列组（一般退化为 `trackers` 使用）
class DefaultGroup final : public group
{
public:
    DefaultGroup() = default;

    //! 构建 DefaultGroup
    static inline std::shared_ptr<DefaultGroup> make_group() { return std::make_shared<DefaultGroup>(); }

    //! DefaultGroup 同步操作
    void sync(const GyroData &, int64_t) override {}
};

//! @} group

} // namespace rm

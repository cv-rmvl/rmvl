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

//! 相关追踪器的空间集合（序列组）
class RMVL_EXPORTS_W_ABS group
{
protected:
    std::vector<tracker::ptr> _trackers; //!< 同组追踪器

    cv::Point2f _center;    //!< 序列组中心
    bool _is_tracked{};     //!< 是否为目标序列组
    uint32_t _vanish_num{}; //!< 丢帧数量
    RMStatus _type{};       //!< 序列组各状态

public:
    using ptr = std::shared_ptr<group>;
    using const_ptr = std::shared_ptr<const group>;

    /**
     * @brief 从另一个序列组进行构造
     *
     * @return 指向新序列组的共享指针
     */
    RMVL_W virtual ptr clone() = 0;

    /**
     * @brief 序列组同步操作
     * @note 根据当前已知的所有的 `tracker` 信息，同步整体 `group`
     *       的内部数据，例如序列组中心，自主构造新的 `tracker` 等
     *
     * @param[in] imu_data 最新 IMU 数据
     * @param[in] tick 最新时间点
     */
    RMVL_W virtual void sync(const ImuData &imu_data, double tick) = 0;

    /**
     * @brief 添加追踪器至序列组
     * @note
     * - 当捕获到的新追踪器 `p_tracker` 计划更新至内部 `_trackers` 时，可调用此方法完成更新
     * - 需要注意的是，此方法不执行同步操作 `sync()`，其派生类在实现过程中也需统一，避免执行 `sync()`
     *
     * @param p_tracker 新的追踪器 `tracker`
     */
    RMVL_W virtual void add(tracker::ptr p_tracker) { _trackers.emplace_back(p_tracker); }

    //! 判断是否为无效序列组
    RMVL_W virtual bool invalid() const { return false; }

    /**
     * @brief 获取同组所有的追踪器数据
     * @note 若仅需要对 `vector<tracker::ptr>` 做数据处理，使用此方法可直接实现 group 至 tracker 的退化
     *
     * @return 同组追踪器
     */
    RMVL_W inline std::vector<rm::tracker::ptr> &data() { return _trackers; }

    /**
     * @brief 获取同组追踪器的数量
     *
     * @return 同组追踪器的数量
     */
    RMVL_W inline size_t size() const { return _trackers.size(); }

    /**
     * @brief 判断同组追踪器是否为空
     *
     * @return 同组追踪器是否为空
     */
    RMVL_W inline bool empty() const { return _trackers.empty(); }

    /**
     * @brief 获取指定追踪器
     *
     * @param[in] idx 追踪器下标
     */
    RMVL_W inline tracker::ptr at(size_t idx) const { return _trackers.at(idx); }

    /**
     * @brief 获取序列组中心
     *
     * @return 序列组中心
     */
    RMVL_W inline const cv::Point2f &center() const { return _center; }

    /**
     * @brief 获取丢帧数量
     *
     * @return 丢帧数量
     */
    RMVL_W inline uint32_t getVanishNumber() const { return _vanish_num; }

    /**
     * @brief 获取该组类型
     *
     * @return RMStatus 该组类型
     */
    RMVL_W inline const RMStatus &type() const { return _type; }
};

#define RMVL_GROUP_CAST(name)                                                                       \
    static inline ptr cast(group::ptr p_group) { return std::dynamic_pointer_cast<name>(p_group); } \
    static inline const_ptr cast(group::const_ptr p_group) { return std::dynamic_pointer_cast<const name>(p_group); }

//! 默认序列组（一般退化为 `trackers` 使用）
class RMVL_EXPORTS_W_DES DefaultGroup final : public group
{
public:
    using ptr = std::shared_ptr<DefaultGroup>;
    using const_ptr = std::shared_ptr<const DefaultGroup>;

    //! 构建 DefaultGroup
    RMVL_W static inline ptr make_group() { return std::make_shared<DefaultGroup>(); }

    /**
     * @brief 从另一个序列组进行构造
     *
     * @return 指向新序列组的共享指针
     */
    RMVL_W group::ptr clone() override;

    RMVL_GROUP_CAST(DefaultGroup)

    /**
     * @brief DefaultGroup 同步操作（空实现，不执行任何操作）
     * 
     * @param[in] imu 最新 IMU 数据
     * @param[in] tick 最新时间点，可用 `rm::Timer::now()` 获取
     */
    RMVL_W void sync(const ImuData &imu, double tick) override;
};

//! @} group

} // namespace rm

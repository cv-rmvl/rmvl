/**
 * @file rune_group.h
 * @author RoboMaster Vision Community
 *
 * @brief 神符序列组头文件
 * @version 1.0
 * @date 2022-08-27
 *
 * @copyright Copyright 2023 (c), RoboMaster Vision Community
 */

#pragma once

#include "rmvl/group/group.h"

namespace rm
{

//! @addtogroup rune_group
//! @{

//! 神符序列组
class RuneGroup final : public group
{
    std::deque<double> _datas; //!< 原始角度数据

public:
    using ptr = std::shared_ptr<RuneGroup>;
    using const_ptr = std::shared_ptr<const RuneGroup>;

    //! 构建 RuneGroup
    static inline ptr make_group() { return std::make_shared<RuneGroup>(); }

    /**
     * @brief 从另一个序列组进行构造
     *
     * @return 指向新序列组的共享指针
     */
    group::ptr clone() override { return std::make_shared<RuneGroup>(*this); }

    /**
     * @brief 动态类型转换
     *
     * @param[in] p_group group::ptr 抽象指针
     * @return 派生对象指针
     */
    static inline ptr cast(group::ptr p_group) { return std::dynamic_pointer_cast<RuneGroup>(p_group); }

    /**
     * @brief 动态类型转换
     *
     * @param[in] p_group group::ptr 抽象指针
     * @return 派生对象指针
     */
    static inline const_ptr cast(group::const_ptr p_group) { return std::dynamic_pointer_cast<const RuneGroup>(p_group); }

    /**
     * @brief 神符序列组同步操作
     * @note
     * - 计算整个序列组当前帧的原始数据 `raw_data`，并同步至 `datas` 中
     *
     * @param[in] gyro_data 最新陀螺仪数据
     * @param[in] tick 最新时间点
     */
    void sync(const GyroData &gyro_data, double tick) override;

    /**
     * @brief 获取原始数据队列
     *
     * @return 原始数据队列
     */
    inline const auto &getRawDatas() const { return _datas; }
};

//! @} rune_group

} // namespace rm

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
class RMVL_EXPORTS_W_DES RuneGroup final : public group
{
public:
    using ptr = std::shared_ptr<RuneGroup>;
    using const_ptr = std::shared_ptr<const RuneGroup>;

    //! 构建 RuneGroup
    RMVL_W static inline ptr make_group() { return std::make_shared<RuneGroup>(); }

    /**
     * @brief 从另一个序列组进行构造
     *
     * @return 指向新序列组的共享指针
     */
    RMVL_W group::ptr clone() override;

    RMVL_GROUP_CAST(RuneGroup)

    /**
     * @brief 神符序列组同步操作
     * @note
     * - 计算整个序列组当前帧的原始数据 `raw_data`，并同步至 `datas` 中
     *
     * @param[in] imu_data 最新 IMU 数据
     * @param[in] tick 最新时间点
     */
    RMVL_W void sync(const ImuData &imu_data, double tick) override;

    /**
     * @brief 获取原始数据队列
     *
     * @return 原始数据队列
     */
    inline const std::deque<double> &getRawDatas() const { return _datas; }

private:
    std::deque<double> _datas; //!< 原始角度数据
};

//! @} rune_group

} // namespace rm

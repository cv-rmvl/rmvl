/**
 * @file gyro_compensator.h
 * @author RoboMaster Vision Community
 * @brief 整车状态补偿模块接口声明
 * @version 1.0
 * @date 2021-08-28
 *
 * @copyright Copyright 2021 (c), RoboMaster Vision Community
 *
 */

#include "compensator.h"

namespace rm
{

//! @addtogroup gyro_compensator
//! @{

//! 整车状态补偿模块
class GyroCompensator final : public compensator
{
public:
    GyroCompensator();

    //! 构造 GyroCompensator
    static inline std::unique_ptr<GyroCompensator> make_compensator()
    {
        return std::make_unique<GyroCompensator>();
    }

    /**
     * @brief 补偿核心函数
     *
     * @param[in] groups 所有序列组
     * @param[in] shoot_speed 子弹射速 (m/s)
     * @param[in] com_flag 手动调节补偿标志
     * @return 补偿模块信息
     */
    CompensateInfo compensate(const std::vector<group::ptr> &groups,
                              uint8_t shoot_speed, CompensateType com_flag) override;

private:
    /**
     * @brief 更新静态补偿量
     *
     * @param[in] com_flag 补偿类型
     */
    void updateStaticCom(CompensateType com_flag);
};

//! @} compensator

} // namespace rm

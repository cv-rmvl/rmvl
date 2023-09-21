/**
 * @file gravity_compensator.h
 * @author RoboMaster Vision Community
 * @brief 重力模型补偿头文件
 * @version 1.0
 * @date 2021-08-28
 *
 * @copyright Copyright 2021 (c), RoboMaster Vision Community
 *
 */

#include "compensator.h"

namespace rm
{

//! @addtogroup gravity_compensator
//! @{

//! 使用重力模型的补偿模块
class GravityCompensator final : public compensator
{
public:
    GravityCompensator();

    //! 构造 GravityCompensator
    static inline std::unique_ptr<GravityCompensator> make_compensator()
    {
        return std::make_unique<GravityCompensator>();
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

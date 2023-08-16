/**
 * @file types.hpp
 * @author RoboMaster Vision Community
 * @author 黄裕炯 (dglz.hyj@qq.com)
 * @brief 常用状态类型定义
 * @version 2.0
 * @date 2023-05-27
 *
 * @copyright Copyright 2023 (c), RoboMaster Vision Community
 *
 */

#pragma once

#include <string>

//! @defgroup types 类型系统

namespace rm
{

//! @addtogroup types
//! @{

//! 装甲板大小类型
enum class ArmorSizeType : uint8_t
{
    UNKNOWN, //!< 未知
    SMALL,   //!< 小装甲板
    BIG,     //!< 大装甲板
};

//! 能量机关激活类型
enum class RuneType : uint8_t
{
    UNKNOWN,  //!< 未知
    INACTIVE, //!< 未激活能量机关
    ACTIVE,   //!< 已激活能量机关
};

//! 机器人类型
enum class RobotType : uint8_t
{
    UNKNOWN,    //!< 未知
    HERO,       //!< 英雄机器人
    ENGINEER,   //!< 工程机器人
    INFANTRY_3, //!< 3 号步兵机器人
    INFANTRY_4, //!< 4 号步兵机器人
    INFANTRY_5, //!< 5 号步兵机器人
    OUTPOST,    //!< 前哨站
    BASE,       //!< 基地
    SENTRY      //!< 哨兵机器人
};

//! 目标切换类型
enum class TargetChangeType : uint8_t
{
    UNKNOWN, //!< 未知
    CHANGE,  //!< 强制切换目标
    AUTO,    //!< 自动切换目标
};

//! 运动类型
enum class MoveType : uint8_t
{
    UNKNOWN, //!< 未知
    STATIC,  //!< 静止
    MOVE,    //!< 移动
};

//! 强制补偿类型
enum class CompensateType : uint8_t
{
    UNKNOWN, //!< 未知
    UP,      //!< 向上强制补偿
    DOWN,    //!< 向下强制补偿
    LEFT,    //!< 向左强制补偿
    RIGHT,   //!< 向右强制补偿
};

//! 状态类型
struct RMStatus
{
    RuneType RuneTypeID = RuneType::UNKNOWN;                         //!< 能量机关激活类型
    MoveType MoveTypeID = MoveType::UNKNOWN;                         //!< 运动类型
    ArmorSizeType ArmorSizeTypeID = ArmorSizeType::UNKNOWN;          //!< 装甲板大小类型
    TargetChangeType TargetChangeTypeID = TargetChangeType::UNKNOWN; //!< 目标切换类型
    CompensateType CompensateTypeID = CompensateType::UNKNOWN;       //!< 强制补偿类型
    RobotType RobotTypeID = RobotType::UNKNOWN;                      //!< 机器人类型

    RMStatus() = default;

    RMStatus(RuneType id) : RuneTypeID(id) {}
    RMStatus(MoveType id) : MoveTypeID(id) {}
    RMStatus(ArmorSizeType id) : ArmorSizeTypeID(id) {}
    RMStatus(TargetChangeType id) : TargetChangeTypeID(id) {}
    RMStatus(CompensateType id) : CompensateTypeID(id) {}
    RMStatus(RobotType id) : RobotTypeID(id) {}

    /**
     * @brief 将类型转化为 `std::string` 类型
     *
     * @tparam Tp 数据类型
     * @param[in] type RMStatus 子类型
     * @return 字符串类型
     */
    template <typename Tp>
    static inline std::string to_string(Tp type) { return std::to_string(static_cast<uint8_t>(type)); }
};

//! @} types

} // namespace rm

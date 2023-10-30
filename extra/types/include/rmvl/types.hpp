/**
 * @file types.hpp
 * @author RoboMaster Vision Community
 * @brief 常用状态类型定义
 * @version 2.0
 * @date 2023-05-27
 *
 * @copyright Copyright 2023 (c), RoboMaster Vision Community
 *
 */

#pragma once

#include "rmvl/core/util.hpp"

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

//! AprilTag 视觉标签类型
enum class TagType : uint8_t
{
    UNKNOWN, //!< 未知
    NUM_0,   //!< 数字 `0`
    NUM_1,   //!< 数字 `1`
    NUM_2,   //!< 数字 `2`
    NUM_3,   //!< 数字 `3`
    NUM_4,   //!< 数字 `4`
    NUM_5,   //!< 数字 `5`
    NUM_6,   //!< 数字 `6`
    NUM_7,   //!< 数字 `7`
    NUM_8,   //!< 数字 `8`
    NUM_9,   //!< 数字 `9`
    A,       //!< 字母 `A`
    B,       //!< 字母 `B`
    C,       //!< 字母 `C`
    D,       //!< 字母 `D`
    E,       //!< 字母 `E`
    F,       //!< 字母 `F`
    G,       //!< 字母 `G`
    H,       //!< 字母 `H`
    I,       //!< 字母 `I`
    J,       //!< 字母 `J`
    K,       //!< 字母 `K`
    L,       //!< 字母 `L`
    M,       //!< 字母 `M`
    N,       //!< 字母 `N`
    O,       //!< 字母 `O`
    P,       //!< 字母 `P`
    Q,       //!< 字母 `Q`
    R,       //!< 字母 `R`
    S,       //!< 字母 `S`
    T,       //!< 字母 `T`
    U,       //!< 字母 `U`
    V,       //!< 字母 `V`
    W,       //!< 字母 `W`
    X,       //!< 字母 `X`
    Y,       //!< 字母 `Y`
    Z        //!< 字母 `Z`
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
    ArmorSizeType ArmorSizeTypeID{};       //!< 装甲板大小类型
    RuneType RuneTypeID{};                 //!< 能量机关激活类型
    TagType TagTypeID{};                   //!< AprilTag 视觉标签类型
    MoveType MoveTypeID{};                 //!< 运动类型
    TargetChangeType TargetChangeTypeID{}; //!< 目标切换类型
    CompensateType CompensateTypeID{};     //!< 强制补偿类型
    RobotType RobotTypeID{};               //!< 机器人类型

    /**
     * @brief 将类型转化为 `std::string` 类型
     *
     * @tparam Tp 数据类型
     * @param[in] type RMStatus 子类型
     * @return 字符串类型
     */
    template <typename Tp>
    static std::string to_string(Tp type) { return std::to_string(static_cast<uint8_t>(type)); }

    /**
     * @brief 将机器人类型转化为 `std::string` 类型
     *
     * @param[in] type RobotType 类型
     * @return 字符串类型
     */
    static std::string to_string(RobotType type);

};

/**
 * @brief 判断两个 RMStatus 是否相等
 * 
 * @param[in] lhs 左操作数
 * @param[in] rhs 右操作数
 * @return 是否相等
 */
inline bool operator==(const RMStatus &lhs, const RMStatus &rhs) { return reflect::equal(lhs, rhs); }

//! @} types

} // namespace rm

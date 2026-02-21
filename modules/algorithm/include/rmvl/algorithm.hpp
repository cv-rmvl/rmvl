/**
 * @file algorithm.hpp
 * @author zhaoxi (535394140@qq.com)
 * @brief 数学、算法模块头文件
 * @version 1.0
 * @date 2024-07-09
 *
 * @copyright Copyright 2024 (c), zhaoxi
 *
 */

#pragma once

/**
 * @defgroup algorithm 数学、算法模块
 * @{
 *   @brief 提供有关基础数学库、数值计算、信号处理、数据结构和图像处理等相关内容
 *   @defgroup algorithm_math 基础数学库
 *   @{
 *     @brief 提供了 STL 中没有的常量定义、变换、广义位移计算、常用数学公式、数学模型算法的实现
 *     @details
 *     - 常量定义包括了 \f$\pi\f$、\f$e\f$、\f$\sqrt2\f$ 等常用数学常量的定义，但在未来可能会移除
 *     - 变换包括了弧角转换、坐标变换的内容
 *     - 广义位移计算包括了平面向量外积、平面欧式距离、空间欧式距离、点到直线距离等算法的实现
 *     - 常用数学公式包括了正割、余割、余切、符号函数、sigmoid 函数等的实现
 *     - 数学模型算法包括了熵权 TOPSIS、KM 算法等算法的实现
 *   @}
 *   @defgroup algorithm_cal 数据与信号处理
 *   @{
 *     @brief 涵盖的内容非常广泛，包含了数值计算、最优化算法、信号处理等内容
 *     @details
 *     - 数值计算、最优化算法请参考 @ref algorithm_numcal 和 @ref algorithm_optimal 两个子专题
 *     - 包括了诸如 Kalman 滤波在内的最优数字递归滤波器，可参考 @ref algorithm_kalman
 *     - 包括了诸如傅里叶变换在内的常用数字信号处理算法，可参考 @ref algorithm_dsp
 *   @}
 *   @defgroup algorithm_data 数据结构
 *   @{
 *     @brief 提供了 STL 以外的一些常用数据结构的实现
 *     @details
 *     - 包含了并查集、支持随机访问的堆、带索引计数的哈希表等数据结构的实现
 *   @}
 *   @defgroup algorithm_img 图像处理
 *   @{
 *     @brief 目前仅提供了图像预处理相关的算法实现
 *   @}
 * @}
 */

// 基础数学库
#include "algorithm/math.hpp"
#include "algorithm/transform.hpp"

// 数值计算、信号处理
#include "algorithm/dsp.hpp"
#include "algorithm/kalman.hpp"
#include "algorithm/numcal.hpp"

// 数据结构
#include "algorithm/datastruct.hpp"

// 图像处理
#include "algorithm/pretreat.hpp"

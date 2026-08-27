/**
 * @file nav.hpp
 * @author zhaoxi (535394140@qq.com)
 * @brief RMVL 导航功能
 * @version 1.0
 * @date 2026-08-16
 *
 * @copyright Copyright 2026 (c), zhaoxi
 *
 */

#pragma once

/**
 * @defgroup nav 导航框架与系统
 * @{
 * @brief 提供视觉定位接入、二维地图、路径规划、路径跟踪和安全控制等导航功能
 * @details
 * - @ref nav_localization 负责传感器帧同步、第三方 VO/SLAM 后端接入和标准导航输出
 * - @ref nav_map 负责二维占据栅格、局部障碍融合、障碍膨胀和 footprint 碰撞检测
 * - @ref nav_planner 负责基于二维代价地图的全局路径搜索、简化和平滑
 * - @ref nav_controller 负责基础路径跟踪和短时预测碰撞刹停
 *
 * Nav 公开接口不依赖 ROS、OpenCV、Eigen、PCL、相机 SDK 或具体 SLAM 实现。坐标变换由
 * rm::lpss::tf 提供，第三方定位与建图算法通过后端接口按需接入。
 *
 * 使用方法和当前能力边界见 @ref tutorial_modules_nav 。
 * @}
 */

#include "nav/controller.hpp"
#include "nav/localization.hpp"
#include "nav/map.hpp"
#include "nav/planner.hpp"

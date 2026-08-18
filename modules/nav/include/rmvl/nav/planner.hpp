/**
 * @file planner.hpp
 * @author zhaoxi (535394140@qq.com)
 * @brief 二维栅格路径规划
 * @version 1.0
 * @date 2026-08-17
 *
 * @copyright Copyright 2026 (c), zhaoxi
 *
 */

#pragma once

#include <cstddef>
#include <cstdint>

#include "rmvl/nav/map.hpp"
#include "rmvlmsg/nav/path.hpp"

namespace rm::nav {

//! @defgroup nav_planner 二维路径规划
//! @ingroup nav
//! @{
//! @brief 提供基于代价地图的 A* 搜索、路径简化和平滑

//! 路径规划状态
enum class PlanningStatus : uint8_t {
    Ok,             //!< 规划成功
    InvalidOptions, //!< 规划参数无效
    InvalidMap,     //!< 代价地图无效
    InvalidStart,   //!< 起点不是有限坐标或位于地图外
    InvalidGoal,    //!< 终点不是有限坐标或位于地图外
    StartBlocked,   //!< 起点不可通行
    GoalBlocked,    //!< 终点不可通行
    NoPath,         //!< 起点与终点之间不存在可通行路径
};

/**
 * @brief 获取路径规划状态的稳定文本描述
 *
 * @param[in] status 路径规划状态
 * @return 指向静态文本的字符串指针
 */
const char *to_string(PlanningStatus status) noexcept;

//! A* 规划配置
struct AStarOptions {
    bool allow_diagonal{true};             //!< 是否允许八邻域对角移动
    bool allow_corner_cutting{false};       //!< 对角移动时是否允许穿过两个障碍栅格的夹角
    bool traverse_unknown{false};           //!< 是否允许经过未知栅格
    uint8_t max_cost{252};                  //!< 可通行栅格的最大代价值
    double cost_weight{1.0};                //!< 代价值相对几何距离的附加权重
    bool simplify{true};                    //!< 是否使用可见性检测删除冗余路径点
    std::size_t smoothing_iterations{2};    //!< 路径平滑迭代次数，0 表示不平滑
    double smoothing_weight{0.25};          //!< 每次迭代向相邻点中点移动的比例，范围为 [0, 1]
};

//! 路径规划结果
struct PlanningResult {
    PlanningStatus status{PlanningStatus::InvalidMap}; //!< 规划状态
    msg::Path path{};                                  //!< 规划成功时的路径
    double cost{};                                     //!< 搜索得到的累计代价
    std::size_t expanded{};                            //!< 搜索过程中展开的栅格数量

    //! @return 是否成功获得路径
    explicit operator bool() const noexcept { return status == PlanningStatus::Ok; }
};

/**
 * @brief 基于二维代价地图的 A* 路径规划器
 * @details
 * - 默认使用八邻域搜索并禁止夹角穿越
 * - 将代价地图中的膨胀代价加入移动代价
 * - 搜索完成后可执行视线简化和受碰撞约束的迭代平滑
 * @note 起点、终点和返回路径均使用代价地图的坐标系。
 */
class AStarPlanner {
public:
    /**
     * @brief 创建 A* 路径规划器
     *
     * @param[in] options 规划配置
     */
    explicit AStarPlanner(AStarOptions options = {}) noexcept;

    //! @return 当前规划配置
    const AStarOptions &options() const noexcept;

    /**
     * @brief 在代价地图上规划路径
     *
     * @param[in] costmap 已完成图层合成和膨胀的代价地图
     * @param[in] start 起点位姿，仅使用位置分量
     * @param[in] goal 终点位姿，仅使用位置分量
     * @return 路径规划结果
     */
    PlanningResult plan(const Costmap &costmap, const msg::Pose &start, const msg::Pose &goal) const;

private:
    AStarOptions _options{};
};

//! @} nav_planner

} // namespace rm::nav

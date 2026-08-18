/**
 * @file planner.cpp
 * @author zhaoxi (535394140@qq.com)
 * @brief 二维栅格路径规划实现
 * @version 1.0
 * @date 2026-08-17
 *
 * @copyright Copyright 2026 (c), zhaoxi
 *
 */

#include <algorithm>
#include <array>
#include <cmath>
#include <limits>
#include <queue>
#include <string>
#include <utility>
#include <vector>

#include "rmvl/nav/planner.hpp"

namespace rm::nav {

namespace {

constexpr double kDiagonalCost = 1.4142135623730950488;

struct OpenNode {
    std::size_t index{};
    double cost{};
    double estimate{};
};

struct OpenNodeGreater {
    bool operator()(const OpenNode &lhs, const OpenNode &rhs) const noexcept {
        if (lhs.estimate != rhs.estimate)
            return lhs.estimate > rhs.estimate;
        return lhs.cost > rhs.cost;
    }
};

bool validOptions(const AStarOptions &options) noexcept {
    return options.max_cost < Inscribed && std::isfinite(options.cost_weight) && options.cost_weight >= 0.0 &&
           std::isfinite(options.smoothing_weight) && options.smoothing_weight >= 0.0 && options.smoothing_weight <= 1.0;
}

bool finitePoint(const msg::Point &point) noexcept {
    return std::isfinite(point.x) && std::isfinite(point.y) && std::isfinite(point.z);
}

std::size_t indexOf(uint32_t width, Cell cell) noexcept {
    return static_cast<std::size_t>(cell.y) * width + cell.x;
}

Cell cellOf(uint32_t width, std::size_t index) noexcept {
    return {static_cast<uint32_t>(index % width), static_cast<uint32_t>(index / width)};
}

bool traversable(const Costmap &costmap, Cell cell, const AStarOptions &options) noexcept {
    const auto cost = costmap.at(cell.x, cell.y);
    if (!cost)
        return false;
    if (*cost == Unknown)
        return options.traverse_unknown;
    return *cost <= options.max_cost;
}

double traversalPenalty(uint8_t cost, const AStarOptions &options) noexcept {
    if (cost == Unknown)
        return options.cost_weight;
    return options.cost_weight * static_cast<double>(cost) / static_cast<double>(Inscribed - 1);
}

double heuristic(Cell from, Cell to, bool diagonal) noexcept {
    const double dx = std::abs(static_cast<double>(from.x) - to.x);
    const double dy = std::abs(static_cast<double>(from.y) - to.y);
    if (!diagonal)
        return dx + dy;
    const double minimum = std::min(dx, dy);
    return dx + dy + (kDiagonalCost - 2.0) * minimum;
}

bool lineClear(const Costmap &costmap, Cell start, Cell goal, const AStarOptions &options) {
    int x = static_cast<int>(start.x);
    int y = static_cast<int>(start.y);
    const int goal_x = static_cast<int>(goal.x);
    const int goal_y = static_cast<int>(goal.y);
    const int dx = std::abs(goal_x - x);
    const int dy = std::abs(goal_y - y);
    const int step_x = x < goal_x ? 1 : -1;
    const int step_y = y < goal_y ? 1 : -1;
    int error = dx - dy;

    while (true) {
        if (!traversable(costmap, {static_cast<uint32_t>(x), static_cast<uint32_t>(y)}, options))
            return false;
        if (x == goal_x && y == goal_y)
            return true;

        const int previous_x = x;
        const int previous_y = y;
        const int twice_error = 2 * error;
        if (twice_error > -dy) {
            error -= dy;
            x += step_x;
        }
        if (twice_error < dx) {
            error += dx;
            y += step_y;
        }
        if (!options.allow_corner_cutting && x != previous_x && y != previous_y) {
            const Cell side_x{static_cast<uint32_t>(x), static_cast<uint32_t>(previous_y)};
            const Cell side_y{static_cast<uint32_t>(previous_x), static_cast<uint32_t>(y)};
            if (!traversable(costmap, side_x, options) || !traversable(costmap, side_y, options))
                return false;
        }
    }
}

void assignOrientations(msg::Path &path) noexcept {
    if (path.poses.empty())
        return;
    if (path.poses.size() == 1) {
        path.poses.front().pose.orientation.w = 1.0;
        return;
    }
    for (std::size_t i = 0; i < path.poses.size(); ++i) {
        const std::size_t first = i + 1 < path.poses.size() ? i : i - 1;
        const std::size_t second = i + 1 < path.poses.size() ? i + 1 : i;
        const auto &from = path.poses[first].pose.position;
        const auto &to = path.poses[second].pose.position;
        const double yaw = std::atan2(to.y - from.y, to.x - from.x);
        path.poses[i].pose.orientation = {0.0, 0.0, std::sin(yaw * 0.5), std::cos(yaw * 0.5)};
    }
}

void simplifyPath(const Costmap &costmap, msg::Path &path, const AStarOptions &options) {
    if (!options.simplify || path.poses.size() < 3)
        return;
    std::vector<msg::PoseStamped> simplified{};
    simplified.reserve(path.poses.size());
    std::size_t anchor = 0;
    simplified.push_back(path.poses.front());
    while (anchor + 1 < path.poses.size()) {
        std::size_t next = anchor + 1;
        const auto start = costmap.worldToMap(path.poses[anchor].pose.position.x, path.poses[anchor].pose.position.y);
        if (!start)
            return;
        for (std::size_t candidate = path.poses.size() - 1; candidate > anchor; --candidate) {
            const auto goal = costmap.worldToMap(path.poses[candidate].pose.position.x, path.poses[candidate].pose.position.y);
            if (goal && lineClear(costmap, *start, *goal, options)) {
                next = candidate;
                break;
            }
        }
        simplified.push_back(path.poses[next]);
        anchor = next;
    }
    path.poses = std::move(simplified);
}

void smoothPath(const Costmap &costmap, msg::Path &path, const AStarOptions &options) {
    if (path.poses.size() < 3 || options.smoothing_iterations == 0 || options.smoothing_weight == 0.0)
        return;
    for (std::size_t iteration = 0; iteration < options.smoothing_iterations; ++iteration) {
        const auto previous_path = path.poses;
        for (std::size_t i = 1; i + 1 < path.poses.size(); ++i) {
            const auto &previous = path.poses[i - 1].pose.position;
            const auto &current = previous_path[i].pose.position;
            const auto &next = previous_path[i + 1].pose.position;
            msg::Point candidate{
                current.x + options.smoothing_weight * ((previous.x + next.x) * 0.5 - current.x),
                current.y + options.smoothing_weight * ((previous.y + next.y) * 0.5 - current.y),
                0.0};
            const auto previous_cell = costmap.worldToMap(previous.x, previous.y);
            const auto candidate_cell = costmap.worldToMap(candidate.x, candidate.y);
            const auto next_cell = costmap.worldToMap(next.x, next.y);
            if (!previous_cell || !candidate_cell || !next_cell || !traversable(costmap, *candidate_cell, options) ||
                !lineClear(costmap, *previous_cell, *candidate_cell, options) ||
                !lineClear(costmap, *candidate_cell, *next_cell, options))
                continue;
            path.poses[i].pose.position = candidate;
        }
    }
}

msg::Path makePath(const Costmap &costmap, const std::vector<Cell> &cells,
                   const msg::Pose &start, const msg::Pose &goal, const AStarOptions &options) {
    msg::Path path{};
    path.header.frame_id = std::string(costmap.frameId());
    path.poses.reserve(cells.size() + 1);
    for (const auto cell : cells) {
        const auto point = costmap.mapToWorld(cell.x, cell.y);
        if (!point)
            return {};
        msg::PoseStamped pose{};
        pose.header = path.header;
        pose.pose.position = *point;
        pose.pose.orientation.w = 1.0;
        path.poses.push_back(std::move(pose));
    }
    if (path.poses.empty())
        return path;
    path.poses.front().pose.position = {start.position.x, start.position.y, 0.0};
    if (path.poses.size() == 1 && std::hypot(goal.position.x - start.position.x, goal.position.y - start.position.y) > 1e-9) {
        auto goal_pose = path.poses.front();
        goal_pose.pose.position = {goal.position.x, goal.position.y, 0.0};
        path.poses.push_back(std::move(goal_pose));
    } else {
        path.poses.back().pose.position = {goal.position.x, goal.position.y, 0.0};
    }
    simplifyPath(costmap, path, options);
    smoothPath(costmap, path, options);
    assignOrientations(path);
    return path;
}

} // namespace

const char *to_string(PlanningStatus status) noexcept {
    switch (status) {
    case PlanningStatus::Ok: return "ok";
    case PlanningStatus::InvalidOptions: return "invalid planner options";
    case PlanningStatus::InvalidMap: return "invalid costmap";
    case PlanningStatus::InvalidStart: return "invalid start pose";
    case PlanningStatus::InvalidGoal: return "invalid goal pose";
    case PlanningStatus::StartBlocked: return "start is blocked";
    case PlanningStatus::GoalBlocked: return "goal is blocked";
    case PlanningStatus::NoPath: return "no path";
    }
    return "unknown planning status";
}

AStarPlanner::AStarPlanner(AStarOptions options) noexcept : _options(options) {}

const AStarOptions &AStarPlanner::options() const noexcept { return _options; }

PlanningResult AStarPlanner::plan(const Costmap &costmap, const msg::Pose &start, const msg::Pose &goal) const {
    PlanningResult result{};
    if (!validOptions(_options)) {
        result.status = PlanningStatus::InvalidOptions;
        return result;
    }
    if (!costmap.valid() || costmap.width() == 0 || costmap.height() == 0 ||
        static_cast<std::size_t>(costmap.width()) > std::numeric_limits<std::size_t>::max() / costmap.height()) {
        result.status = PlanningStatus::InvalidMap;
        return result;
    }
    if (!finitePoint(start.position)) {
        result.status = PlanningStatus::InvalidStart;
        return result;
    }
    if (!finitePoint(goal.position)) {
        result.status = PlanningStatus::InvalidGoal;
        return result;
    }
    const auto start_cell = costmap.worldToMap(start.position.x, start.position.y);
    if (!start_cell) {
        result.status = PlanningStatus::InvalidStart;
        return result;
    }
    const auto goal_cell = costmap.worldToMap(goal.position.x, goal.position.y);
    if (!goal_cell) {
        result.status = PlanningStatus::InvalidGoal;
        return result;
    }
    if (!traversable(costmap, *start_cell, _options)) {
        result.status = PlanningStatus::StartBlocked;
        return result;
    }
    if (!traversable(costmap, *goal_cell, _options)) {
        result.status = PlanningStatus::GoalBlocked;
        return result;
    }

    const uint32_t width = costmap.width();
    const std::size_t area = static_cast<std::size_t>(width) * costmap.height();
    const std::size_t start_index = indexOf(width, *start_cell);
    const std::size_t goal_index = indexOf(width, *goal_cell);
    const std::size_t no_parent = std::numeric_limits<std::size_t>::max();
    std::vector<double> costs(area, std::numeric_limits<double>::infinity());
    std::vector<std::size_t> parents(area, no_parent);
    std::vector<bool> closed(area, false);
    std::priority_queue<OpenNode, std::vector<OpenNode>, OpenNodeGreater> open{};
    costs[start_index] = 0.0;
    open.push({start_index, 0.0, heuristic(*start_cell, *goal_cell, _options.allow_diagonal)});

    constexpr std::array<std::pair<int, int>, 8> directions{{
        {1, 0}, {-1, 0}, {0, 1}, {0, -1}, {1, 1}, {1, -1}, {-1, 1}, {-1, -1}}};
    while (!open.empty()) {
        const auto current = open.top();
        open.pop();
        if (closed[current.index] || current.cost > costs[current.index])
            continue;
        closed[current.index] = true;
        ++result.expanded;
        if (current.index == goal_index)
            break;
        const Cell cell = cellOf(width, current.index);
        for (std::size_t direction = 0; direction < directions.size(); ++direction) {
            const auto [offset_x, offset_y] = directions[direction];
            const bool diagonal = offset_x != 0 && offset_y != 0;
            if (diagonal && !_options.allow_diagonal)
                continue;
            const int next_x = static_cast<int>(cell.x) + offset_x;
            const int next_y = static_cast<int>(cell.y) + offset_y;
            if (next_x < 0 || next_y < 0 || next_x >= static_cast<int>(costmap.width()) ||
                next_y >= static_cast<int>(costmap.height()))
                continue;
            const Cell next{static_cast<uint32_t>(next_x), static_cast<uint32_t>(next_y)};
            if (!traversable(costmap, next, _options))
                continue;
            if (diagonal && !_options.allow_corner_cutting) {
                const Cell side_x{static_cast<uint32_t>(next_x), cell.y};
                const Cell side_y{cell.x, static_cast<uint32_t>(next_y)};
                if (!traversable(costmap, side_x, _options) || !traversable(costmap, side_y, _options))
                    continue;
            }
            const auto cell_cost = costmap.at(next.x, next.y);
            if (!cell_cost)
                continue;
            const double step = diagonal ? kDiagonalCost : 1.0;
            const double candidate = costs[current.index] + step * (1.0 + traversalPenalty(*cell_cost, _options));
            const std::size_t next_index = indexOf(width, next);
            if (candidate >= costs[next_index])
                continue;
            costs[next_index] = candidate;
            parents[next_index] = current.index;
            open.push({next_index, candidate, candidate + heuristic(next, *goal_cell, _options.allow_diagonal)});
        }
    }

    if (!closed[goal_index]) {
        result.status = PlanningStatus::NoPath;
        return result;
    }
    std::vector<Cell> cells{};
    for (std::size_t index = goal_index;; index = parents[index]) {
        cells.push_back(cellOf(width, index));
        if (index == start_index)
            break;
        if (parents[index] == no_parent) {
            result.status = PlanningStatus::NoPath;
            return result;
        }
    }
    std::reverse(cells.begin(), cells.end());
    result.path = makePath(costmap, cells, start, goal, _options);
    if (result.path.poses.empty()) {
        result.status = PlanningStatus::NoPath;
        return result;
    }
    result.cost = costs[goal_index];
    result.status = PlanningStatus::Ok;
    return result;
}

} // namespace rm::nav

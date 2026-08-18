/**
 * @file controller.cpp
 * @author zhaoxi (535394140@qq.com)
 * @brief 二维路径跟踪与碰撞刹停实现
 * @version 1.0
 * @date 2026-08-17
 *
 * @copyright Copyright 2026 (c), zhaoxi
 *
 */

#include <algorithm>
#include <cmath>
#include <limits>

#include "rmvl/nav/controller.hpp"

namespace rm::nav {

namespace {

constexpr double kPi = 3.14159265358979323846;
constexpr std::size_t kMaxCollisionSamples = 100000;

double normalizeAngle(double angle) noexcept {
    return std::remainder(angle, 2.0 * kPi);
}

bool planarYaw(const msg::Quaternion &orientation, double &yaw) noexcept {
    if (!std::isfinite(orientation.x) || !std::isfinite(orientation.y) ||
        !std::isfinite(orientation.z) || !std::isfinite(orientation.w))
        return false;
    const double norm2 = orientation.x * orientation.x + orientation.y * orientation.y +
                         orientation.z * orientation.z + orientation.w * orientation.w;
    if (!(norm2 > std::numeric_limits<double>::epsilon()) || !std::isfinite(norm2))
        return false;
    const double inverse_norm = 1.0 / std::sqrt(norm2);
    const double x = orientation.x * inverse_norm;
    const double y = orientation.y * inverse_norm;
    const double z = orientation.z * inverse_norm;
    const double w = orientation.w * inverse_norm;
    if (std::abs(x) > 1e-9 || std::abs(y) > 1e-9)
        return false;
    yaw = std::atan2(2.0 * w * z, 1.0 - 2.0 * z * z);
    return true;
}

bool finitePosition(const msg::Point &point) noexcept {
    return std::isfinite(point.x) && std::isfinite(point.y) && std::isfinite(point.z);
}

bool finiteVector(const msg::Vector3 &vector) noexcept {
    return std::isfinite(vector.x) && std::isfinite(vector.y) && std::isfinite(vector.z);
}

bool validOptions(const PurePursuitOptions &options) noexcept {
    return std::isfinite(options.lookahead_distance) && options.lookahead_distance > 0.0 &&
           std::isfinite(options.target_speed) && options.target_speed >= 0.0 &&
           std::isfinite(options.max_angular_speed) && options.max_angular_speed > 0.0 &&
           std::isfinite(options.goal_tolerance) && options.goal_tolerance >= 0.0 &&
           std::isfinite(options.slowdown_distance) && options.slowdown_distance > 0.0 &&
           std::isfinite(options.rotate_to_path_threshold) && options.rotate_to_path_threshold > 0.0 &&
           options.rotate_to_path_threshold <= kPi && std::isfinite(options.heading_gain) && options.heading_gain > 0.0;
}

bool validPath(const msg::Path &path) noexcept {
    if (path.poses.empty())
        return false;
    return std::all_of(path.poses.begin(), path.poses.end(), [&path](const msg::PoseStamped &pose) {
        return pose.header.frame_id == path.header.frame_id && finitePosition(pose.pose.position);
    });
}

bool validOptions(const CollisionStopOptions &options) noexcept {
    return std::isfinite(options.prediction_horizon) && options.prediction_horizon >= 0.0 &&
           std::isfinite(options.linear_step) && options.linear_step > 0.0 &&
           std::isfinite(options.angular_step) && options.angular_step > 0.0;
}

msg::Quaternion yawQuaternion(double yaw) noexcept {
    return {0.0, 0.0, std::sin(yaw * 0.5), std::cos(yaw * 0.5)};
}

} // namespace

const char *to_string(TrackingStatus status) noexcept {
    switch (status) {
    case TrackingStatus::Ok: return "ok";
    case TrackingStatus::GoalReached: return "goal reached";
    case TrackingStatus::InvalidOptions: return "invalid tracking options";
    case TrackingStatus::InvalidPose: return "invalid robot pose";
    case TrackingStatus::InvalidPath: return "invalid path";
    }
    return "unknown tracking status";
}

PurePursuit::PurePursuit(PurePursuitOptions options) noexcept : _options(options) {}

const PurePursuitOptions &PurePursuit::options() const noexcept { return _options; }

TrackingResult PurePursuit::compute(const msg::Pose &pose, const msg::Path &path) const {
    TrackingResult result{};
    if (!validOptions(_options)) {
        result.status = TrackingStatus::InvalidOptions;
        return result;
    }
    double yaw{};
    if (!finitePosition(pose.position) || !planarYaw(pose.orientation, yaw)) {
        result.status = TrackingStatus::InvalidPose;
        return result;
    }
    if (!validPath(path)) {
        result.status = TrackingStatus::InvalidPath;
        return result;
    }

    const auto distance_to = [&pose](const msg::Point &point) {
        return std::hypot(point.x - pose.position.x, point.y - pose.position.y);
    };
    const auto &goal = path.poses.back().pose.position;
    const double goal_distance = distance_to(goal);
    if (goal_distance <= _options.goal_tolerance) {
        result.status = TrackingStatus::GoalReached;
        result.lookahead = goal;
        result.lookahead_index = path.poses.size() - 1;
        return result;
    }

    std::size_t closest = 0;
    double closest_distance = std::numeric_limits<double>::infinity();
    for (std::size_t i = 0; i < path.poses.size(); ++i) {
        const double distance = distance_to(path.poses[i].pose.position);
        if (distance < closest_distance) {
            closest = i;
            closest_distance = distance;
        }
    }
    std::size_t target = path.poses.size() - 1;
    for (std::size_t i = closest; i < path.poses.size(); ++i) {
        if (distance_to(path.poses[i].pose.position) >= _options.lookahead_distance) {
            target = i;
            break;
        }
    }
    result.lookahead_index = target;
    result.lookahead = path.poses[target].pose.position;

    const double dx = result.lookahead.x - pose.position.x;
    const double dy = result.lookahead.y - pose.position.y;
    const double local_x = std::cos(yaw) * dx + std::sin(yaw) * dy;
    const double local_y = -std::sin(yaw) * dx + std::cos(yaw) * dy;
    const double target_distance2 = local_x * local_x + local_y * local_y;
    const double heading_error = normalizeAngle(std::atan2(local_y, local_x));
    if (std::abs(heading_error) >= _options.rotate_to_path_threshold || local_x <= 0.0) {
        result.command.angular.z = std::clamp(
            _options.heading_gain * heading_error, -_options.max_angular_speed, _options.max_angular_speed);
        result.status = TrackingStatus::Ok;
        return result;
    }

    const double speed_scale = std::min(1.0, goal_distance / _options.slowdown_distance);
    result.command.linear.x = _options.target_speed * speed_scale;
    if (target_distance2 > std::numeric_limits<double>::epsilon()) {
        const double curvature = 2.0 * local_y / target_distance2;
        result.command.angular.z = std::clamp(
            result.command.linear.x * curvature, -_options.max_angular_speed, _options.max_angular_speed);
    }
    result.status = TrackingStatus::Ok;
    return result;
}

CollisionStop::CollisionStop(CollisionStopOptions options) noexcept : _options(options) {}

const CollisionStopOptions &CollisionStop::options() const noexcept { return _options; }

CollisionStopResult CollisionStop::filter(const Costmap &costmap, const std::vector<msg::Point> &footprint,
                                          const msg::Pose &pose, const msg::Twist &command) const {
    CollisionStopResult result{};
    double yaw{};
    if (!validOptions(_options) || !costmap.valid() || !finitePosition(pose.position) ||
        !planarYaw(pose.orientation, yaw) || !finiteVector(command.linear) || !finiteVector(command.angular) ||
        costmap.collides(footprint, pose)) {
        result.stopped = true;
        return result;
    }

    const double linear_speed = std::hypot(command.linear.x, command.linear.y);
    const double linear_samples = linear_speed * _options.prediction_horizon / _options.linear_step;
    const double angular_samples = std::abs(command.angular.z) * _options.prediction_horizon / _options.angular_step;
    const double requested_samples = std::ceil(std::max({1.0, linear_samples, angular_samples}));
    if (!std::isfinite(requested_samples) || requested_samples > static_cast<double>(kMaxCollisionSamples)) {
        result.stopped = true;
        return result;
    }
    const std::size_t samples = static_cast<std::size_t>(requested_samples);
    const double time_step = samples == 0 ? 0.0 : _options.prediction_horizon / static_cast<double>(samples);
    msg::Pose predicted = pose;
    for (std::size_t i = 0; i < samples; ++i) {
        const double midpoint_yaw = yaw + command.angular.z * time_step * 0.5;
        predicted.position.x += (std::cos(midpoint_yaw) * command.linear.x -
                                 std::sin(midpoint_yaw) * command.linear.y) * time_step;
        predicted.position.y += (std::sin(midpoint_yaw) * command.linear.x +
                                 std::cos(midpoint_yaw) * command.linear.y) * time_step;
        yaw = normalizeAngle(yaw + command.angular.z * time_step);
        predicted.orientation = yawQuaternion(yaw);
        if (costmap.collides(footprint, predicted)) {
            result.stopped = true;
            return result;
        }
    }
    result.command = command;
    return result;
}

} // namespace rm::nav
